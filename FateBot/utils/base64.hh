#ifndef BASE64_HEADER
#define BASE64_HEADER

#include <string>

namespace Utility {
    std::string b64encode(const void* input, int length);
    std::string b64decode(const void* input, int length);

    std::string b64encode(const std::string& str);
    std::string b64decode(const std::string& str64);
} // namespace Utility

#endif