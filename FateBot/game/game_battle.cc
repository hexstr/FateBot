#include "game_battle.hh"

#include <random>
#include <sstream>
#include <string>
#include <vector>

#include <absl/container/btree_map.h>
#include <absl/container/flat_hash_map.h>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "base_http.hh"
#include "drop_item.hh"
#include "struct/top_login.hh"
#include "user_manager.hh"
#include "utils/CatAndMouseGame.hh"
#include "utils/config.hh"
#include "utils/crc32.hh"
#include "utils/logger.hh"
#include "utils/msgpack11.hh"
#include "utils/my_exception.hh"
#include "utils/mytime.hh"

using std::pair;
using std::string;
using std::stringstream;
using std::thread;
using std::to_string;
using std::vector;

using namespace cpr;
using namespace rapidjson;

vector<thread> GameBattle::battle_pool_;
bool GameBattle::force_battle_ = false;
bool GameBattle::is_exit_ = false;

static std::array<std::string, 9> BattleLogsList = {
    "1B",
    "1C",
    "1D",

    "2B",
    "2C",
    "2D",

    "3B",
    "3C",
    "3D"
};

GameBattle::GameBattle(User& user) : user_(user) {
    if (auto itor = Task::task_list_.find(user.task_id_); itor != Task::task_list_.end()) {
        battle_task_ = &itor->second;
        battle_follower_id_ = itor->second.follower_id_;
        battle_follower_type_ = itor->second.follower_type_;
        last_act_cost_ = itor->second.quest_cost_;
        if (itor->second.quest_id_ == 0) {
            battle_quest_id_ = UserManager::daily_id_;
        }
        else {
            battle_quest_id_ = itor->second.quest_id_;
        }
    }
    else {
        error("[GameBattle] The task id %d was not found in task.json", user.task_id_);
    }
};

void GameBattle::APRecovery(const char* AppleType) {
    user_.builder_.AddParameter("num", "1");
    user_.builder_.AddParameter("recoverId", AppleType);
    auto doc = user_.PostRequest(Url(GameServer "item/recover?_userId=" + user_.id_));
    // Calc now action points
    user_.user_game_.Unmarshal(doc["cache"]["updated"]["userGame"][0]);
    user_.now_act_point_ = user_.GetActPoint(doc["cache"]["serverTime"].GetInt());

    prim("[APRecovery]%s - [%d - %d] AP: %d",
         user_.name_,
         user_.recover_count_ + 1,
         UserManager::recover_limit_,
         user_.now_act_point_);
}

Document GameBattle::FollowerList() {
    user_.builder_.AddParameter("questId", battle_quest_id_);
    user_.builder_.AddParameter("questPhase", battle_unlocked_phase_);
    user_.builder_.AddParameter("refresh", "1");
    return user_.PostRequest(Url(GameServer "follower/list?_userId=" + user_.id_));
}

void GameBattle::BattleScenario() {
    warn("[BattleScenario] [%s]", user_.name_);
    user_.builder_.AddParameter("questId", battle_quest_id_);
    user_.builder_.AddParameter("questPhase", battle_unlocked_phase_);
    user_.builder_.AddParameter("routeSelect", "[]");
    user_.PostRequest(Url(GameServer "battle/scenario?_userId=" + user_.id_));
}

void DropInfo(const Value& battleInfo) {
    stringstream logcat;
    absl::btree_map<int, pair<int, int>> result; // key - id
                                                 // value - first: additional second: total

    for (auto& enemy_pages : battleInfo["enemyDeck"].GetArray()) {
        for (auto& enemies : enemy_pages["svts"].GetArray()) {
            for (auto& drop_info : enemies["dropInfos"].GetArray()) {
                int id = drop_info["objectId"].GetInt();
                int orig = drop_info["originalNum"].GetInt();
                int num = drop_info["num"].GetInt();

                if (auto itor = result.find(id); itor == result.end()) {
                    result.emplace(id, std::make_pair(num - orig, num));
                }
                else {
                    itor->second.second += num;
                }
            }
        }
    }

    logcat << "\t=================" << std::endl;
    if (result.size()) {
        for (auto& item : result) {
            if (auto item_str = DropItem::GetInstance().GetItemName(item.first); item_str.empty() == false) {
                logcat << "\t" << item_str;
            }

            else if (item.first > 90000000) {
                int last_bit = item.first % 10;
                switch (last_bit) {
                    case 1:
                        logcat << "\t活动-铜材料";
                        break;
                    case 2:
                        logcat << "\t活动-银材料";
                        break;
                    case 3:
                        logcat << "\t活动-金材料";
                        break;
                    case 4:
                        logcat << "\t活动-A点数";
                        break;
                    case 5:
                        logcat << "\t活动-B点数";
                        break;
                    case 6:
                        logcat << "\t活动-C点数";
                        break;
                    default:
                        logcat << "\t未知活动材料: " << item.first;
                }
            }

            else if (item.first > 9400000) {
                logcat << "\t♥礼装掉落♥";
            }

            else {
                logcat << "\t" << item.first;
            }

            if (item.second.first) {
                logcat << " x " << item.second.second << " (+" << item.second.first << ")" << std::endl;
            }
            else {
                logcat << " x " << item.second.second << std::endl;
            }
        }
    }
    else {
        logcat << "\t没有掉落物" << std::endl;
    }
    logcat << "\t=================";
    info("%s", logcat.str());
}

void GameBattle::BattleSetup() {
    user_.builder_.AddParameter("activeDeckId", user_.active_deck_id_);
    user_.builder_.AddParameter("boostId", "0");
    user_.builder_.AddParameter("campaignItemId", "0");
    user_.builder_.AddParameter("choiceRandomLimitCounts", "{}");
    user_.builder_.AddParameter("enemySelect", "0");
    user_.builder_.AddParameter("followerClassId", battle_task_->follower_class_id_);
    user_.builder_.AddParameter("followerId", battle_follower_id_);
    user_.builder_.AddParameter("followerRandomLimitCount", "0");
    user_.builder_.AddParameter("followerSupportDeckId", battle_follower_support_deck_id_);
    user_.builder_.AddParameter("followerType", battle_follower_type_);
    user_.builder_.AddParameter("itemId", "0");
    user_.builder_.AddParameter("questId", battle_quest_id_);
    user_.builder_.AddParameter("questPhase", battle_unlocked_phase_);
    user_.builder_.AddParameter("questSelect", "0");
    user_.builder_.AddParameter("routeSelect", "[]");
    user_.builder_.AddParameter("userEquipId", "0");
    auto battleSetup = user_.PostRequest(Url(GameServer "battle/setup?_userId=" + user_.id_));
    battle_id_ = battleSetup["cache"]["replaced"]["battle"][0]["id"].GetInt64();
    enemy_deck_unique_id_.clear();
    const Value& battleInfo = battleSetup["cache"]["replaced"]["battle"][0]["battleInfo"];

    my_deck_svt_count_ = battleInfo["myDeck"]["svts"].Size();
    page_of_enemy_deck_ = battleInfo["enemyDeck"].Size();
    // The last page of enemy deck
    const Value& enemyDeck = battleInfo["enemyDeck"][page_of_enemy_deck_ - 1]["svts"];
    for (int i = 0; i < (int)enemyDeck.Size(); ++i) {
        enemy_deck_unique_id_.push_back(enemyDeck[i]["uniqueId"].GetInt());
    }
    // Calc now action points
    user_.user_game_.Unmarshal(battleSetup["cache"]["updated"]["userGame"][0]);
    int NewActPoint = user_.GetActPoint(battleSetup["cache"]["serverTime"].GetInt());
    last_act_cost_ = user_.now_act_point_ - NewActPoint;
    user_.now_act_point_ = NewActPoint;

    warn("[BattleSetup] [%s] %d - %d | AP: %d Cost: %d",
         user_.name_, battle_quest_id_, battle_unlocked_phase_, NewActPoint, last_act_cost_);

    DropInfo(battleInfo);
}

void GameBattle::BattleResult() {
    succ("[BattleResult] [%s]", user_.name_);
    user_.builder_.AddParameter("result", CreateBattleResult());
    auto doc = user_.PostRequest(Url(GameServer "battle/result?_userId=" + user_.id_));
    for (const auto& Svt : doc["cache"]["updated"]["userSvtCollection"].GetArray()) {
        info("[%s] svtId: %7d | fp: %2d - %7d", user_.name_, Svt["svtId"].GetInt(), Svt["friendshipRank"].GetInt(), Svt["friendship"].GetInt());
    }
    if (battle_unlocked_phase_ < battle_task_->quest_phase_) {
        battle_unlocked_phase_++;
    }
}

int64_t GameBattle::CalcBattleStatus() {
    int64_t num = 0LL;
    int64_t battle_status[5];
    battle_status[0] = user_.uid_ + 1;
    battle_status[1] = num - 0x408FD5;
    battle_status[2] = num / 2;
    battle_status[3] = battle_id_ - 0x7FFFFFFF;
    battle_status[4] = num - 0x25ACF6;
    return Utility::Crc32Compute(battle_status, 40);
}

string GameBattle::CreateBattleResult() {
    StringBuffer logs;
    Writer<StringBuffer> logsWriter(logs);
    // start fake battle action
    logsWriter.StartObject();

    // logs start
    logsWriter.Key("logs");
    /*
        'A' + ty
        Art     -   1   -   B
        Buster  -   2   -   C
        Quick   -   3   -   D
    */
    std::stringstream ss1;
    std::default_random_engine e;
    my_deck_svt_count_ = my_deck_svt_count_ < 3 ? my_deck_svt_count_ : 3;
    std::uniform_int_distribution<unsigned> u(0, 3 * my_deck_svt_count_ - 1);
    for (int i = 0; i < page_of_enemy_deck_; ++i) {
        for (int o = 0; o < 3; ++o) {
            // create 3 command cards.
            ss1 << BattleLogsList[u(e)];
        }
    }
    logsWriter.String(ss1.rdbuf()->str().c_str());
    // logs end

    // dt start
    logsWriter.Key("dt");
    std::stringstream ss2;
    for (auto uniqueId : enemy_deck_unique_id_) {
        ss2 << "u";
        ss2 << uniqueId;
    }
    logsWriter.String(ss2.rdbuf()->str().c_str());
    // dt end

    // others start
    logsWriter.Key("hd");
    logsWriter.String("");
    logsWriter.Key("data");
    logsWriter.String("");
    // others end
    logsWriter.EndObject();
    // end fake battle action

    msgpack11::MsgPack::array usedTurnList;
    for (int i = 0; i < page_of_enemy_deck_; ++i) {
        usedTurnList.emplace_back(1);
    }

    msgpack11::MsgPack pack = msgpack11::MsgPack::object{
        { "battleId", battle_id_ },
        { "battleResult", 1 },
        { "winResult", 1 },
        { "scores", "" },

        { "action", logs.GetString() },

        { "raidResult", "[]" },
        { "superBossResult", "[]" },

        { "elapsedTurn", 1 },
        { "recordType", 1 },

        { "recordValueJson", msgpack11::MsgPack::object{
                                 { "turnMaxDamage", 0 },
                                 { "knockdownNum", 0 },
                                 { "totalDamageToAliveEnemy", 0 } } },
        { "tdPlayed", "[]" },

        { "usedEquipSkillList", msgpack11::MsgPack::object{} },
        { "svtCommonFlagList", msgpack11::MsgPack::object{} },
        { "skillShiftUniqueIds", msgpack11::MsgPack::array{} },
        { "skillShiftNpcSvtIds", msgpack11::MsgPack::array{} },
        { "calledEnemyUniqueIds", msgpack11::MsgPack::array{} },
        { "aliveUniqueIds", msgpack11::MsgPack::array{} },
        { "battleStatus", CalcBattleStatus() },
        { "voicePlayedList", "[]" },
        { "usedTurnList", usedTurnList }
    };
    auto result = pack.dump();
    return CatAndMouseGame::CatGame5(result);
}

void GameBattle::BattleStart() {
    if (battle_task_->follower_id_ == 0) {
        // Traverse follower list, find the target follower
        // repeat 10 times
        bool has_target_follower = false;
        for (size_t i = 0; i < 10; ++i) {
            if (i) {
                sleepsec(3);
            }
            auto follower = FollowerList();
            const auto& followerInfo = follower["cache"]["updated"]["userFollower"][0]["followerInfo"].GetArray();
            for (size_t f = 0; f < followerInfo.Size(); ++f) {
                battle_follower_type_ = followerInfo[f]["type"].GetInt();
                if (battle_task_->follower_type_ >= battle_follower_type_) {
                    const char* table_name = nullptr;
                    if (battle_task_->is_event_) {
                        table_name = "eventUserSvtLeaderHash";
                    }

                    else {
                        table_name = "userSvtLeaderHash";
                    }
                    const auto& support_array = followerInfo[f][table_name].GetArray();

                    int exists_support_count = support_array.Size() - 1;
                    int max_svt_class_count = 7 < exists_support_count ? 7 : exists_support_count;

                    if (battle_task_->is_event_) {
                        for (int o = 0; o <= max_svt_class_count; ++o) {
                            int svt_equip = support_array[o]["equipTarget1"]["svtId"].GetInt();
                            int svt_class_id = support_array[o]["classId"].GetInt();
                            int svt_equip_level = support_array[o]["equipTarget1"]["limitCount"].GetInt();
                            if (svt_equip == battle_task_->follower_equip_id_ && svt_equip_level == 4) {
                                battle_follower_id_ = followerInfo[f]["userId"].GetInt();
                                battle_follower_support_deck_id_ = to_string(support_array[o]["supportDeckId"].GetInt());
                                battle_task_->follower_class_id_ = svt_class_id;
                                goto CONTINUE;
                                break;
                            }
                        }
                    }

                    else if (support_array.Size() > battle_task_->follower_class_id_) {
                        int svt_equip = support_array[battle_task_->follower_class_id_]["equipTarget1"]["svtId"].GetInt();
                        int svt_class_id = support_array[battle_task_->follower_class_id_]["classId"].GetInt();
                        int svt_equip_level = support_array[battle_task_->follower_class_id_]["equipTarget1"]["limitCount"].GetInt();
                        if (svt_equip == battle_task_->follower_equip_id_ && svt_equip_level == 4 && svt_class_id == battle_task_->follower_class_id_) {
                            battle_follower_id_ = followerInfo[f]["userId"].GetInt();
                            battle_follower_support_deck_id_ = to_string(support_array[battle_task_->follower_class_id_]["supportDeckId"].GetInt());
                            goto CONTINUE;
                            break;
                        }
                    }
                }
            }
        }
        if (!has_target_follower) {
            throw MyEx("No target follower.");
        }
    }
    else {
        // No specified follower id, skip
        FollowerList();
    }
CONTINUE:
    sleepsec(3);
    BattleSetup();
    sleepsec(30);
    BattleResult();
    sleepsec(3);
}

void GameBattle::StartThread(int times, int index) {
    int timestamp = Utility::GetTimestamp10Int();
    for (int i = index; i < UserManager::user_list_.size(); ++i) {
        auto& user = UserManager::user_list_[i];
        if (is_exit_) {
            break;
        }
        if (force_battle_ || timestamp + 60 > user.user_game_.act_recover_at_) {
            GameBattle::battle_pool_.emplace_back(
                [&user, times] {
                    try {
                        GameBattle user_battle(user);
                        int task_quest_id = user_battle.battle_quest_id_;
                        int task_quest_phase = user_battle.battle_task_->quest_phase_;

                        if (is_exit_) {
                            return;
                        }

                        auto doc = user.TopLogin();

                        for (auto& quest : doc["cache"]["replaced"]["userQuest"].GetArray()) {
                            int quest_id = quest["questId"].GetInt();
                            int quest_phase = quest["questPhase"].GetInt();

                            if (quest_id == task_quest_id) {
                                if (quest_phase <= task_quest_phase) {
                                    user_battle.battle_unlocked_phase_ = quest_phase;
                                    break;
                                }
                            }
                        }

                        if (is_exit_) {
                            return;
                        }
                        user.TopHome();

                        sleepsec(5);

                        for (int j = 0; j < times; ++j) {
                            if (is_exit_) {
                                break;
                            }
                            if (user.now_act_point_ < user_battle.last_act_cost_) {
                                if (user.recover_count_ < UserManager::recover_limit_) {
                                    user_battle.APRecovery(); // Does this need try-catch?
                                    user.recover_count_++;
                                    sleepsec(3);
                                }
                                else {
                                    error("[StartThread] [%s] AP Not Enough", user.name_);
                                    break;
                                }
                            }
                            if (is_exit_) {
                                break;
                            }
                            user_battle.BattleStart(); // Does this need try-catch?
                        }

                    } catch (MyException& ex) {
                        error("Exception: [%s] %s", user.name_, ex.what());
                    }
                });
            sleepsec(3);
        }
    }
    for (auto& handle : GameBattle::battle_pool_) {
        if (handle.joinable()) {
            handle.join();
        }
    }
    std::cout << "\033[0;33m[OrderCompleted] ";
    Utility::PrintNowTime();
    WriteRecoveryTimestamp();
    auto ip = Get(Url("http://ip.p3terx.com/"), Proxies{ { "https", BaseHttp::proxy_ } }, VerifySsl(false)).text;
    info("IP Address: %s", ip);
}

void GameBattle::ReadRecoveryTimestamp() {
    try {
        auto status = Utility::LoadJsonFromFile(GLOBALSETTINGPATH "status.json");
        for (auto& user : UserManager::user_list_) {
            if (auto itor = status.FindMember(user.id_.c_str()); itor != status.MemberEnd() && itor->value.IsInt()) {
                user.user_game_.act_recover_at_ = itor->value.GetInt();
            }
        }
    } catch (const MyException&) {
        // Ignore this exception
    }
}

void GameBattle::WriteRecoveryTimestamp() {
    StringBuffer status;
    Writer<StringBuffer> statusWriter(status);

    statusWriter.StartObject();
    for (auto& user : UserManager::user_list_) {
        statusWriter.Key(user.id_.c_str());
        statusWriter.Int(user.user_game_.act_recover_at_);
        std::cout << "\033[0;36m[RecoveryTime] [" << user.name_ << "]\t";
        Utility::PrintTimestamp(user.user_game_.act_recover_at_);
    }
    statusWriter.EndObject();

    std::ofstream out(GLOBALSETTINGPATH "status.json");
    out.write(status.GetString(), status.GetSize());
    out.close();
}