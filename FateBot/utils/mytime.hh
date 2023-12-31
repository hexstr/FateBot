#ifndef MYTIME_HEADER
#define MYTIME_HEADER

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>

namespace Utility {
    inline std::string GetTimestamp10() {
        return std::to_string(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch())
                .count());
    }
    inline int64_t GetTimestamp13Int() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
    }
    inline int GetTimestamp10Int() {
        return (int)std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
    }
    inline void PrintTimestamp(int timestamp) {
        std::time_t t(timestamp);
        std::cout << std::put_time(std::localtime(&t), "%Y-%m-%d %X") << std::endl;
    }
    inline void PrintNowTime() {
        auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::cout << std::put_time(std::localtime(&t), "%Y-%m-%d %X") << std::endl;
    }
} // namespace Utility

#endif // mytime.hpp