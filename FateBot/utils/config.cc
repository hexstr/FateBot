#include "config.hh"
#include "utils/mytime.hh"

#include <fstream>
#include <string>

#include <absl/strings/str_format.h>

#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

using namespace rapidjson;

#include "my_exception.hh"

namespace Utility {
    std::default_random_engine e(Utility::GetTimestamp10Int());
    std::uniform_int_distribution<unsigned> u(0, 6);
    std::uniform_int_distribution<unsigned> us(1, 3);

    int GetRandom(std::uniform_int_distribution<unsigned> g) {
        return g(Utility::e);
    }

    int GetRandom() {
        return Utility::u(Utility::e);
    }
    int GetRandomShort() {
        return Utility::us(Utility::e);
    }

    rapidjson::Document LoadJsonFromFile(const char* file_path) {
        Document doc;
        std::ifstream file(file_path);
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)),
                                (std::istreambuf_iterator<char>()));
            if (doc.Parse(content.c_str()).HasParseError()) {
                if (content.size() > 2) {
                    content[doc.GetErrorOffset() - 2] = '-';
                    content[doc.GetErrorOffset() - 1] = '>';
                }
                std::string reason = absl::StrFormat("File: %s %s %zu %s", file_path, GetParseError_En(doc.GetParseError()), doc.GetErrorOffset(), content);
                throw MyEx(reason);
            }
        }
        else {
            std::string reason = absl::StrFormat("Could not read file: %s", file_path);
            throw MyEx(reason);
        }
        return doc;
    }
} // namespace Utility