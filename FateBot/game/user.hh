#ifndef USER_HEADER
#define USER_HEADER

#include <memory>
#include <string>

#include <absl/container/flat_hash_map.h>
#include <absl/types/span.h>

#include "base_http.hh"
#include "struct/top_login.hh"

class User {
public:
    static bool check_shop_;
    static bool donot_exchange_;
    static void CheckShopThread();

    UserGame user_game_;

    int uid_;
    int task_id_;
    int now_act_point_;
    int recover_count_ = 0;
    std::string name_;
    std::string id_;
    std::string active_deck_id_;
    curl_slist* host_;
    ParameterBuilder builder_;

    int latest_quest_id_ = 0;
    int latest_quest_phase_ = 0;

    User(int id, const char* name, const char* authKey, const char* secretKey, int taskId);
    rapidjson::Document TopLogin();
    rapidjson::Document TopHome();

    rapidjson::Document BuyItem(int id, int num);
    rapidjson::Document SellSvt(std::vector<int64_t>& list);
    rapidjson::Document ChangeItemLockStatus(const std::string& item_list, bool is_revoke = true);
    rapidjson::Document PresentList();

    std::vector<std::tuple<int, int64_t>> RecvPresents(const std::vector<int64_t>& present_list);
    rapidjson::Document RecvPresents(const std::vector<int64_t>& present_list, int item_select_index);

    void MissionComplete(const rapidjson::GenericValue<rapidjson::UTF8<>>&);

    int GetActPoint(int64_t server_timestamp);
    rapidjson::Document PostRequest(const cpr::Url&);

private:
    std::shared_ptr<cpr::Session> session_;

    rapidjson::Document RecvPresents(absl::Span<const int64_t> subList);
};

#endif // user.hh