#include "sha1.hh"

#include <openssl/sha.h>

namespace Utility {
    std::string CalcSHA1(const void* input, size_t length) {
        std::string result(20, 0);
        SHA1(reinterpret_cast<const unsigned char*>(input), length,
             reinterpret_cast<unsigned char*>(&result[0]));
        return result;
    }

    std::string CalcSHA1(const std::string& input) {
        return CalcSHA1(input.data(), input.size());
    }
} // namespace Utility