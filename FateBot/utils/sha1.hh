#ifndef SHA1_HEADER
#define SHA1_HEADER

#include <string>

namespace Utility {
    std::string CalcSHA1(const void*, size_t);
    std::string CalcSHA1(const std::string&);
} // namespace Utility

#endif // sha1.hh