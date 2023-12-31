#include "user.hh"

#include <cstdint>
#include <random>
#include <vector>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <rapidjson/error/en.h>

#include "base_http.hh"
#include "drop_item.hh"
#include "game_battle.hh"
#include "items.hh"
#include "shop.hh"
#include "user_manager.hh"
#include "utils/base64.hh"
#include "utils/config.hh"
#include "utils/logger.hh"
#include "utils/msgpack11.hh"
#include "utils/my_exception.hh"

using std::string;
using std::to_string;
using std::vector;

using namespace rapidjson;
using namespace cpr;

bool User::check_shop_ = false;
bool User::donot_exchange_ = false;

User::User(int id, const char* name, const char* authKey, const char* secretKey, int taskId)
    : uid_(id),
      task_id_(taskId),
      name_(name),
      id_(to_string(id)),
      builder_(authKey, secretKey, id) {
    session_ = std::make_shared<Session>();
    session_->SetOption(VerifySsl(false));
    session_->SetOption(Proxies{ { "https", BaseHttp::proxy_ } });
    session_->SetOption(Header{ { "Connection", "Keep-Alive, TE" },
                                { "TE", "identity" },
                                { "Accept-Encoding", "gzip, identity" } });
    auto random_index = Utility::GetRandom(std::uniform_int_distribution<unsigned>(0, BaseHttp::user_agent_list_.size() - 1));
    auto itor = BaseHttp::user_agent_list_.begin();
    std::advance(itor, random_index);
    session_->SetOption(UserAgent(*itor));
}

Document User::TopLogin() {
    succ("[TopLogin] [%s]", name_);
    int last_access_time = std::stoi(builder_.AcquireParameter("lastAccessTime"));
    int user_state = (-last_access_time >> 2) ^ (uid_ & BaseHttp::data_server_folder_crc_);

    builder_.AddParameter("assetbundleFolder", BaseHttp::asset_bundle_folder_);
    builder_.AddParameter("isTerminalLogin", "1");
    builder_.AddParameter("userState", user_state);
    auto doc = PostRequest(Url(GameServer "login/top?_userId=" + id_));

    user_game_.Unmarshal(doc["cache"]["replaced"]["userGame"][0]);
    now_act_point_ = GetActPoint(doc["cache"]["serverTime"].GetInt64());

    active_deck_id_ = to_string(user_game_.active_deck_id_);
    for (auto& quest : doc["cache"]["replaced"]["userQuest"].GetArray()) {
        int id = quest["questId"].GetInt();
        if (id < 90000000) {
            if (id > latest_quest_id_) {
                latest_quest_id_ = id;
                latest_quest_phase_ = quest["questPhase"].GetInt();
            }
        }
    }

    info("\t[%s] Quest: %d - %d", name_, latest_quest_id_, latest_quest_phase_);
    info("\t[%s] Mana: %d - AP: %d", name_, user_game_.mana_, now_act_point_);

    MissionComplete(doc["cache"]["replaced"]["userEventMission"]);

    vector<int64_t> receive_ids;
    vector<int64_t> sell_ids;
    absl::flat_hash_set<int64_t> deck_svt_ids;
    absl::flat_hash_map<int, vector<int64_t>> exchange_ids;

    for (auto& deck : doc["cache"]["replaced"]["userDeck"].GetArray()) {
        const auto& deckInfo = deck["deckInfo"].GetObject();
        const auto& svts = deckInfo["svts"].GetArray();
        for (auto& svt : svts) {
            deck_svt_ids.insert(svt["userSvtId"].GetUint64());
        }
    }

    for (auto& userSvt : doc["cache"]["replaced"]["userSvt"].GetArray()) {
        int svtId = userSvt["svtId"].GetInt();
        int64_t id = userSvt["id"].GetUint64();
        int lv = userSvt["lv"].GetInt();
        Items::StatusFlag status = static_cast<Items::StatusFlag>(userSvt["status"].GetInt());

        if (Items::IsSellServant(svtId) && status == Items::StatusFlag::UNLOCK && deck_svt_ids.contains(id) == false) {
            if (lv == 1) {
                sell_ids.push_back(id);
            }
            else {
                warn("lv != 1 (%d) but is sell svt", lv);
            }
        }
    }

    const auto& present_box = doc["cache"]["replaced"]["userPresentBox"].GetArray();
    info("[PresentBox] Has %3d Items", present_box.Size());

    for (int i = 0; i < present_box.Size(); ++i) {
        int64_t present_id = present_box[i]["presentId"].GetInt64();
        int object_id = present_box[i]["objectId"].GetInt();

        if (object_id > 10000 && object_id < 10200 && User::donot_exchange_) {
            if (auto itor = exchange_ids.find(object_id); itor == exchange_ids.end()) {
                exchange_ids.emplace(object_id, vector<int64_t>{ present_id });
            }
            else {
                itor->second.push_back(present_id);
            }
        }

        else if (Items::IsRecvPresent(object_id)) {
            receive_ids.push_back(present_id);
        }
    }

    if (receive_ids.size() != 0) {
        if (auto received_presents = RecvPresents(receive_ids); received_presents.size() != 0) {
            info("%zu items received", received_presents.size());
            for (auto& item : received_presents) {
                auto received_id = std::get<0>(item);
                auto item_name = DropItem::GetInstance().GetItemName(received_id);
                if (item_name.empty()) {
                    info("\t%d", received_id);
                }
                else {
                    info("\t%s", item_name);
                }

                if (Items::IsSellServant(std::get<0>(item))) {
                    sell_ids.push_back(std::get<1>(item));
                }
            }
        }
    }

    if (exchange_ids.size() != 0) {
        for (auto& item : exchange_ids) {
            RecvPresents(item.second, 1);
        }
    }

    if (sell_ids.size() != 0) {
        SellSvt(sell_ids);
    }

    return doc;
}

Document User::TopHome() {
    return PostRequest(Url(GameServer "home/top?_userId=" + id_));
}

Document User::BuyItem(int id, int num) {
    succ("[BuyItem] [%s] %d - %d", name_, id, num);
    builder_.AddParameter("id", id);
    builder_.AddParameter("num", num);
    return PostRequest(Url(GameServer "shop/purchase?_userId=" + id_));
}

Document User::SellSvt(vector<int64_t>& list) {
    succ("[SellSvt] [%s] size: %d", name_, list.size());

    msgpack11::MsgPack::array msg;
    for (auto id : list) {
        msgpack11::MsgPack::object obj;
        obj.emplace("id", id);
        obj.emplace("num", 1);
        msg.emplace_back(std::move(obj));
    }
    list.clear();
    msgpack11::MsgPack pack = msg;
    auto res = Utility::b64encode(pack.dump());

    builder_.AddParameter("sellData", res);
    builder_.AddParameter("sellCommandCode", "kA==");
    return PostRequest(Url(GameServer "shop/sellSvt?_userId=" + id_));
}

Document User::ChangeItemLockStatus(const string& item_list, bool is_revoke) {
    succ("[ChangeLock] [%s]", name_);
    builder_.AddParameter("isLock", "1");

    if (is_revoke) {
        builder_.AddParameter("revokeUserSvtIds", item_list);
    }
    else {
        builder_.AddParameter("changeUserSvtIds", item_list);
    }

    return PostRequest(Url(GameServer "card/statusSync?_userId=" + id_));
}

Document User::PresentList() {
    return PostRequest(Url(GameServer "present/list?_userId=" + id_));
}

void User::MissionComplete(const rapidjson::GenericValue<rapidjson::UTF8<>>& input) {
    succ("[MissionComplete] [%s]", name_);

    for (const auto& mission : input.GetArray()) {
        if (mission["missionProgressType"].GetInt() == 4) {
            int missionId = mission["missionId"].GetInt();
            builder_.AddParameter("missionId", missionId);
            auto doc = PostRequest(Url(GameServer "eventMission/receive?_userId=" + id_));

            if (auto itor = doc["cache"]["updated"].FindMember("userItem"); itor != doc["cache"]["updated"].MemberEnd()) {
                for (auto& item : itor->value.GetArray()) {
                    auto item_name = DropItem::GetInstance().GetItemName(item["itemId"].GetInt());
                    if (item_name.empty() == false) {
                        info("[Reward] Earn %s X %d", item_name, item["num"].GetInt());
                    }
                    else {
                        info("[Reward] Earn %d X %d", item["itemId"].GetInt(), item["num"].GetInt());
                    }
                }
            }

            sleepsec(3);
        }
    }
}

Document User::RecvPresents(absl::Span<const int64_t> subList) {
    msgpack11::MsgPack::array presents;
    for (auto id : subList) {
        presents.emplace_back(id);
    }
    msgpack11::MsgPack arr = presents;
    auto str = arr.dump();
    auto encode = Utility::b64encode(str);

    builder_.AddParameter("presentIds", encode);
    return PostRequest(Url(GameServer "present/receive?_userId=" + id_));
}

std::vector<std::tuple<int, int64_t>> User::RecvPresents(const vector<int64_t>& present_list) {
    succ("[RecvPresents] [%s]", name_);

    auto span_list = absl::Span<const int64_t>(present_list.data(), present_list.size());

    auto left = span_list.begin();
    auto right = span_list.end();

    std::vector<std::tuple<int, int64_t>> received_presents;

    while (left != right) {
        absl::Span<const int64_t> subSpan;
        if (auto dist = std::distance(left, right); dist < 6) {
            subSpan = absl::Span<const int64_t>(&*left, dist);
            left += dist;
        }
        else {
            subSpan = absl::Span<const int64_t>(&*left, 6);
            left += 6;
        }

        auto userSvt = RecvPresents(subSpan);
        for (auto& item : userSvt["cache"]["updated"]["userSvt"].GetArray()) {
            int svtId = item["svtId"].GetInt();
            int64_t id = item["id"].GetInt64();
            received_presents.emplace_back(svtId, id);
        }
    }
    return received_presents;
}

Document User::RecvPresents(const vector<int64_t>& present_list, int item_select_index) {
    succ("[RecvPresents] [%s]", name_);

    msgpack11::MsgPack::array presents;
    for (auto id : present_list) {
        presents.emplace_back(id);
    }
    msgpack11::MsgPack arr = presents;
    auto str = arr.dump();
    auto encode = Utility::b64encode(str);

    builder_.AddParameter("itemSelectIdx", item_select_index);
    builder_.AddParameter("itemSelectNum", present_list.size());
    builder_.AddParameter("presentIds", encode);
    return PostRequest(Url(GameServer "present/receive?_userId=" + id_));
}

Document User::PostRequest(const Url& url) {
    Body body(builder_.Build());
    session_->SetOption(url);

    int retry_count = 5;
    Response rsp;
    builder_.Clean();

    do {
        if (!retry_count) {
            throw MyEx("Maximum attempts to retry.");
        }

        if (rsp.error) {
            info("[%s] %s\n%s", name_, url.c_str(), rsp.error.message);
        }

        sleepsec(3);

        session_->SetOption(body);
        rsp = session_->Post();

        retry_count--;
    } while (rsp.error);

    Document doc;
    if (doc.Parse(rsp.text.c_str()).HasParseError()) {
        string reason = absl::StrFormat("Url: %s\nOffset: %zu\nError: %s\n%s",
                                        url.c_str(),
                                        doc.GetErrorOffset(),
                                        GetParseError_En(doc.GetParseError()),
                                        rsp.text);
        throw MyEx(reason);
    }
    if (strcmp(doc["response"][0]["resCode"].GetString(), "00")) {
        string reason = absl::StrFormat("Url: %s\nErrorCode: %s\nReason: \033[0;33m%s",
                                        url.c_str(),
                                        doc["response"][0]["resCode"].GetString(),
                                        doc["response"][0]["fail"]["detail"].GetString());
        throw MyEx(reason);
    }
    return doc;
}

int User::GetActPoint(int64_t server_timestamp) {
    int num = user_game_.act_recover_at_ - server_timestamp;
    if (num > 0) {
        int lost_act_point = (num + 299) / 300;
        return (user_game_.act_max_ <= lost_act_point) ? 0 : (user_game_.act_max_ - lost_act_point);
    }
    return user_game_.act_max_ + user_game_.carry_over_act_point_;
}

void User::CheckShopThread() {
    if (Shop::GetInstance().opening_list_.size() == 0) {
        error("[CheckShop] The shop list cannot be initialized.");
        return;
    }

    for (auto& user : UserManager::user_list_) {
        if (GameBattle::is_exit_) {
            break;
        }

        GameBattle::battle_pool_.emplace_back(
            [&user] {
                if (GameBattle::is_exit_) {
                    return;
                }

                auto doc = user.TopLogin();

                const auto& user_shop = doc["cache"]["updated"]["userShop"].GetArray();

                absl::flat_hash_map<int, int> shop_ticket = Shop::GetInstance().opening_list_;

                for (auto& item : user_shop) {
                    int shop_id = item["shopId"].GetInt();
                    int bought_num = item["num"].GetInt();
                    int limit_num = Shop::GetInstance().GetLimitNumber(shop_id);
                    int rest_num = limit_num - bought_num;
                    if (rest_num > 0) {
                        shop_ticket[shop_id] = rest_num;
                    }
                    else {
                        shop_ticket.erase(shop_id);
                    }
                }
                succ("[CheckShop] [%s] Ticket[s]: %d", user.name_, shop_ticket.size());

                user.TopHome();

                sleepsec(3);

                for (auto& item : shop_ticket) {
                    try {
                        user.BuyItem(item.first, item.second);
                        sleepsec(3);
                    } catch (MyException& ex) {
                        error("[CheckShop] [%s] id: %d - %d %s", user.name_, item.first, item.second, ex.what());
                    }
                }
            });
        sleepsec(3);
    }
    for (auto& handle : GameBattle::battle_pool_) {
        if (handle.joinable()) {
            handle.join();
        }
    }
}