#ifndef LOGGER_HEADER
#define LOGGER_HEADER

#define LOG_CLRSTR_NONE         "\033[0m"               // 默认显示
#define LOG_CLRSTR_RED          "\033[0;31m"            // 红色
#define LOG_CLRSTR_GREEN        "\033[0;32m"            // 绿色
#define LOG_CLRSTR_BLUE         "\033[0;34m"            // 蓝色
#define LOG_CLRSTR_DARK_GRAY    "\033[1;30m"            // 灰色
#define LOG_CLRSTR_CYAN         "\033[0;36m"            // 青色
#define LOG_CLRSTR_PURPLE       "\033[0;35m"            // 紫色
#define LOG_CLRSTR_YELLOW       "\033[0;33m"            // 黄色
#define LOG_CLRSTR_WHITE        "\033[1;37m"            // 白色

#include <absl/strings/str_format.h>

#define info(str, ...) absl::PrintF(LOG_CLRSTR_CYAN str LOG_CLRSTR_NONE "\n", ##__VA_ARGS__)
#define warn(str, ...) absl::PrintF(LOG_CLRSTR_YELLOW str LOG_CLRSTR_NONE "\n", ##__VA_ARGS__)
#define error(str, ...) absl::PrintF(LOG_CLRSTR_RED str LOG_CLRSTR_NONE "\n", ##__VA_ARGS__)
#define succ(str, ...) absl::PrintF(LOG_CLRSTR_GREEN str LOG_CLRSTR_NONE "\n", ##__VA_ARGS__)
#define prim(str, ...) absl::PrintF(LOG_CLRSTR_BLUE str LOG_CLRSTR_NONE "\n", ##__VA_ARGS__)

#endif