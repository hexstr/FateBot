#ifndef GAMEBATTLE_HEADER
#define GAMEBATTLE_HEADER

#include <array>
#include <thread>

#include "task.hh"
#include "user.hh"

class GameBattle {
public:
    static bool force_battle_;
    static bool is_exit_;

    GameBattle(User& user);
    static void StartThread(int times, int index = 0);
    static void ReadRecoveryTimestamp();
    static void WriteRecoveryTimestamp();

    Task* battle_task_ = nullptr;

    int last_act_cost_ = 0;
    int battle_unlocked_phase_ = 1;
    int battle_quest_id_ = 0;
    int battle_follower_id_ = 0;
    int battle_follower_type_ = -233;
    std::string battle_follower_support_deck_id_ = "1";
    int64_t battle_id_ = -233;
    int my_deck_svt_count_ = 3;
    int page_of_enemy_deck_ = 0;
    std::vector<int> enemy_deck_unique_id_;

    static std::vector<std::thread> battle_pool_;

    void APRecovery(const char* = "2"); // 彩苹果 = 1 | 金苹果 = 2 | 银苹果 = 3 | 铜苹果 = 4
    rapidjson::Document FollowerList();
    void BattleScenario();
    void BattleSetup(); // All = 0; Saber = 1; Archer = 2; Lancer = 3; Rider = 4; Caster = 5 Assassin = 6; Berserker = 7
    void BattleResult();
    std::string CreateBattleResult();
    void BattleStart();
    void PassQuest();
    int64_t CalcBattleStatus();

private:
    User& user_;
};
#endif // GAMEBATTLE_HEADER