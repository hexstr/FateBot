#include "base_http.hh"

#include <string_view>

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "utils/CatAndMouseGame.hh"
#include "utils/config.hh"
#include "utils/crc32.hh"
#include "utils/my_exception.hh"
#include "utils/mytime.hh"
#include "utils/sha1.hh"
#include "utils/uuid.hh"

using std::string;
using std::string_view;
using std::to_string;

using namespace cpr;
using namespace rapidjson;

string BaseHttp::ver_code_;
string BaseHttp::app_ver_;
string BaseHttp::asset_bundle_folder_;
uint64_t BaseHttp::data_server_folder_crc_;
string BaseHttp::data_ver_;
string BaseHttp::date_ver_;
string BaseHttp::proxy_{};
int BaseHttp::update_time_ = 0;

absl::flat_hash_set<std::string> BaseHttp::user_agent_list_{
    "Dalvik/2.1.0 (Linux; U; Android 9; ONEPLUS A5010 Build/PQ3A.190801.002)"
};

void BaseHttp::Initialization() {
    auto config = Utility::LoadJsonFromFile(GLOBALSETTINGPATH "cfg.json");
    int readed = 0;
    if (auto itor = config.FindMember("appVer"); itor != config.MemberEnd() && itor->value.IsString()) {
        app_ver_ = config["appVer"].GetString();
        readed++;
    }

    if (auto itor = config.FindMember("assetbundleFolder"); itor != config.MemberEnd() && itor->value.IsString()) {
        asset_bundle_folder_ = config["assetbundleFolder"].GetString();
        readed++;
    }

    if (auto itor = config.FindMember("dataServerFolderCrc"); itor != config.MemberEnd() && itor->value.IsInt64()) {
        data_server_folder_crc_ = config["dataServerFolderCrc"].GetInt64();
        readed++;
    }

    if (auto itor = config.FindMember("dataVer"); itor != config.MemberEnd() && itor->value.IsInt()) {
        data_ver_ = to_string(config["dataVer"].GetInt());
        readed++;
    }

    if (auto itor = config.FindMember("dateVer"); itor != config.MemberEnd() && itor->value.IsInt()) {
        date_ver_ = to_string(config["dateVer"].GetInt());
        readed++;
    }

    if (auto itor = config.FindMember("verCode"); itor != config.MemberEnd() && itor->value.IsString()) {
        ver_code_ = config["verCode"].GetString();
        readed++;
    }

    if (auto itor = config.FindMember("updateTime"); itor != config.MemberEnd() && itor->value.IsInt()) {
        update_time_ = config["updateTime"].GetInt();
        readed++;
    }

    if (readed != 7) {
        throw MyEx("Incomplete configuration file cfg.json.");
    }

    auto ua = Utility::LoadJsonFromFile(GLOBALSETTINGPATH "ua.json");
    if (ua.IsArray() && ua.Size()) {
        user_agent_list_.clear();
        for (auto& item : ua.GetArray()) {
            if (item.IsString()) {
                user_agent_list_.insert(item.GetString());
            }
        }
    }
}

Document BaseHttp::HttpGet(Url url) {
    auto rsp = Get(url,
                   Header{ { "Connection", "Keep-Alive, TE" },
                           { "TE", "identity" },
                           { "Accept-Encoding", "gzip, identity" } },
                   UserAgent(
                       "Dalvik/2.1.0 (Linux; U; Android 6.0.1; MI 6 Build/V417IR)"),
                   Proxies{ { "https", proxy_ } }, VerifySsl(false));
    if (rsp.error) {
        throw MyEx(rsp.error.message);
    }

    Document doc;
    doc.Parse(rsp.text.c_str());
    if (strcmp(doc["response"][0]["resCode"].GetString(), "00")) {
        if (strcmp(doc["response"][0]["resCode"].GetString(), "01") == 0) {
            info("New Version Need Updated.");
            Value& Detail = doc["response"][0]["fail"]["detail"];
            string_view s(Detail.GetString());
            if (auto Start = s.find("ver."); Start != string::npos) {
                string_view SubStr(&s[Start], Detail.GetStringLength() - Start);
                if (auto New = SubStr.find("、"); New != string::npos) {
                    string_view NewVerStr(&SubStr[7], New - 7);
                    info("NewVer: |%s|", NewVerStr);
                    BaseHttp::app_ver_ = NewVerStr;
                    return doc;
                }
            }
            throw MyEx("The new version string could not be found.");
        }
        else {
            string response = absl::StrFormat(LOG_CLRSTR_RED "ErrorCode: %s\n" LOG_CLRSTR_YELLOW "%s",
                                              doc["response"][0]["resCode"].GetString(),
                                              doc["response"][0]["fail"]["detail"].GetString());
            throw MyEx(response);
        }
    }

    return doc;
}

void BaseHttp::GameData(bool is_force_update) {
    int japan_now_time = Utility::GetTimestamp10Int() + 3600;
    if (is_force_update) {
        BaseHttp::update_time_ = 0;
    }

    if (japan_now_time > BaseHttp::update_time_) {
        succ("[GameData] Checking Update...");
        auto doc = BaseHttp::HttpGet(Url(GameServer "gamedata/top?appVer=" +
                                         BaseHttp::app_ver_ +
                                         "&dataVer=" + BaseHttp::data_ver_ +
                                         "&dateVer=" + BaseHttp::date_ver_));

        if (strcmp(doc["response"][0]["resCode"].GetString(), "01") == 0) {
            std::string url = GameServer "gamedata/top?appVer=";
            absl::StrAppend(&url, BaseHttp::app_ver_, "&dataVer=", BaseHttp::data_ver_, "&dateVer=", BaseHttp::date_ver_);
            doc = BaseHttp::HttpGet(Url(url));
        }
        int server_date_ver = doc["response"][0]["success"]["dateVer"].GetInt();
        int server_data_ver = doc["response"][0]["success"]["dataVer"].GetInt();
        const char* asset_bundle = doc["response"][0]["success"]["assetbundle"].GetString();
        if (server_date_ver != stoi(BaseHttp::date_ver_) || server_data_ver != stoi(BaseHttp::data_ver_)) {
            warn("[GameData] Updating\n\tdateVer: %d\n\tdataVer: %d",
                 server_date_ver,
                 server_data_ver);
            BaseHttp::date_ver_ = to_string(server_date_ver);
            BaseHttp::data_ver_ = to_string(server_data_ver);

            string str = CatAndMouseGame::MouseInfoMsgPack(asset_bundle);
            int index = 0xD;
            for (; index < str.size() - 0xD; ++index) {
                if ((unsigned char)str[index] == 0xAA) {
                    break;
                }
            }
            BaseHttp::asset_bundle_folder_ = str.substr(0xD, index - 0xD);
            BaseHttp::data_server_folder_crc_ = Utility::Crc32Compute(BaseHttp::asset_bundle_folder_);
        }

        std::time_t t = time(nullptr);
        tm* tm = std::localtime(&t);
        tm->tm_min = 0;
        tm->tm_sec = 0;

        if (tm->tm_hour == 23) {
            tm->tm_mday++;
        }

        tm->tm_hour = 18;
        BaseHttp::update_time_ = mktime(tm);

        if (japan_now_time > BaseHttp::update_time_) {
            tm->tm_mday++;
            tm->tm_hour = 00;
            BaseHttp::update_time_ = mktime(tm);
        }

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        writer.StartObject();
        writer.Key("appVer");
        writer.String(BaseHttp::app_ver_.c_str());
        writer.Key("assetbundleFolder");
        writer.String(BaseHttp::asset_bundle_folder_.c_str());
        writer.Key("dataServerFolderCrc");
        writer.Int64(BaseHttp::data_server_folder_crc_);
        writer.Key("dataVer");
        writer.Int(stoi(BaseHttp::data_ver_));
        writer.Key("dateVer");
        writer.Int(stoi(BaseHttp::date_ver_));
        writer.Key("verCode");
        writer.String(BaseHttp::ver_code_.c_str());
        writer.Key("updateTime");
        writer.Int(BaseHttp::update_time_);
        writer.EndObject();
        std::ofstream out(GLOBALSETTINGPATH "cfg.json");
        out.write(buffer.GetString(), buffer.GetSize());
        out.close();
    }
}

ParameterBuilder::ParameterBuilder(const char* auth_key, const char* secret_key, int id)
    : id_(to_string(id)),
      auth_key_(auth_key),
      secret_key_(secret_key) {
    parameter_list_ = {
        { "appVer", BaseHttp::app_ver_ },
        { "authKey", auth_key_ },
        { "dataVer", BaseHttp::data_ver_ },
        { "dateVer", BaseHttp::date_ver_ },
        { "idempotencyKey", Utility::generate_uuid() },
        { "lastAccessTime", Utility::GetTimestamp10() },
        { "userId", id_ },
        { "verCode", BaseHttp::ver_code_ }
    };
}

// Maybe i'd better url_encode key&value here.
void ParameterBuilder::AddParameter(Parameter&& par) {
    parameter_list_.emplace(std::move(par.key), std::move(par.value));
}

void ParameterBuilder::AddParameter(std::string key, int64_t value) {
    parameter_list_.emplace(std::move(key), to_string(value));
}

void ParameterBuilder::AddParameter(std::string key, std::string value) {
    parameter_list_.emplace(std::move(key), std::move(value));
}

std::string& ParameterBuilder::AcquireParameter(const std::string& key) {
    return parameter_list_[key];
}

std::string ParameterBuilder::Build() {
    std::string temp{};
    std::string content{};
    CurlHolder holder;

    for (const auto& element : parameter_list_) {
        if (!temp.empty()) {
            absl::StrAppend(&temp, "&");
            absl::StrAppend(&content, "&");
        }

        auto escaped_key = holder.urlEncode(element.first);
        if (element.second.empty()) {
            absl::StrAppend(&temp, element.first, "=");
            absl::StrAppend(&content, escaped_key, "=");
        }
        else {
            auto escaped_value = holder.urlEncode(element.second);
            absl::StrAppend(&temp, element.first, "=", element.second);
            absl::StrAppend(&content, escaped_key, "=", escaped_value);
        }
    }

    absl::StrAppend(&temp, ":", secret_key_);
    absl::StrAppend(&content, "&authCode=", holder.urlEncode(Utility::b64encode(Utility::CalcSHA1(temp))));

    return content;
}

void ParameterBuilder::Clean() {
    parameter_list_.clear();
    parameter_list_ = {
        { "appVer", BaseHttp::app_ver_ },
        { "authKey", auth_key_ },
        { "dataVer", BaseHttp::data_ver_ },
        { "dateVer", BaseHttp::date_ver_ },
        { "idempotencyKey", Utility::generate_uuid() },
        { "lastAccessTime", Utility::GetTimestamp10() },
        { "userId", id_ },
        { "verCode", BaseHttp::ver_code_ }
    };
}