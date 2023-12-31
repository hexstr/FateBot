#ifndef CRC32_HEADER
#define CRC32_HEADER

#include <string>

namespace Utility {
    int64_t Crc32Compute(const void*, int);
    int64_t Crc32Compute(const std::string&);
}; // namespace Utility

#endif // crc32.hh