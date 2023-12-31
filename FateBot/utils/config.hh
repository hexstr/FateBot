#ifndef CONFIG_HEADER
#define CONFIG_HEADER

// #define ENABLEPROXY
#define GLOBALSETTINGPATH "config/"

#include <random>

#include <rapidjson/document.h>

namespace Utility {
    int GetRandom(std::uniform_int_distribution<unsigned>);
    int GetRandom();
    int GetRandomShort();
    rapidjson::Document LoadJsonFromFile(const char*);
}; // namespace Utility

#endif // config.hh