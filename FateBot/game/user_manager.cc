#include "user_manager.hh"

#include "utils/config.hh"
#include "utils/logger.hh"
#include "utils/mytime.hh"

using absl::Status;

std::vector<User> UserManager::user_list_;
int UserManager::user_amount_ = 0;
int UserManager::daily_id_;
int UserManager::recover_limit_ = 0;

int DailyBattleIdPrimary[7]{
    94061631,
    94061601,
    94061606,
    94061611,
    94061616,
    94061621,
    94061626
};

int DailyBattleIdIntermediate[7]{
    94061633,
    94061603,
    94061608,
    94061613,
    94061618,
    94061623,
    94061628
};

void UserManager::Initialization() {
    int timestamp = Utility::GetTimestamp10Int() + 3600;
    std::time_t jp_t(timestamp);
    tm* ltm = std::localtime(&jp_t);
    daily_id_ = DailyBattleIdPrimary[ltm->tm_wday];

    auto result = Utility::LoadJsonFromFile(GLOBALSETTINGPATH "user.json");
    const auto& user_data = result.GetArray();

    for (auto& user : user_data) {
        user_list_.emplace_back(user["id"].GetInt(),
                                user["name"].GetString(),
                                user["authKey"].GetString(),
                                user["secretKey"].GetString(),
                                user["task"].GetInt());
    }
    info("[UserManager] %d users loaded.", user_list_.size());
}