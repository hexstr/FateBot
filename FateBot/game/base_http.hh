#ifndef BASEHTTP_HEADER
#define BASEHTTP_HEADER

#ifdef __linux__
#include <unistd.h>
#define sleepsec(sec) sleep(sec)
#endif
#ifdef _WIN32
#define sleepsec(sec) Sleep((sec)*1000)
#endif

#include <string>

#include <absl/container/btree_map.h>
#include <absl/container/flat_hash_set.h>
#include <cpr/cpr.h>
#include <rapidjson/document.h>

#define GameServer "https://game.fate-go.jp/"

class ParameterBuilder {
public:
    ParameterBuilder(const char* auth_key, const char* secret_key, int id);
    void AddParameter(std::string, std::string);
    void AddParameter(std::string, int64_t);
    void AddParameter(cpr::Parameter&&);
    std::string& AcquireParameter(const std::string&);
    std::string Build();
    void Clean();

    std::string id_;
    std::string auth_key_;
    std::string secret_key_;

private:
    absl::btree_map<std::string, std::string> parameter_list_;
};

namespace BaseHttp {
    extern std::string ver_code_;
    extern std::string app_ver_;
    extern std::string asset_bundle_folder_;
    extern uint64_t data_server_folder_crc_;
    extern std::string data_ver_;
    extern std::string date_ver_;
    extern std::string proxy_;
    extern int update_time_;
    extern absl::flat_hash_set<std::string> user_agent_list_;

    void Initialization();
    rapidjson::Document HttpGet(cpr::Url);
    void GameData(bool is_force_check_update);
}; // namespace BaseHttp
#endif