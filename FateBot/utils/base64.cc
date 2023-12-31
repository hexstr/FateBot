#include "base64.hh"

#include <openssl/evp.h>
#include <stdlib.h>

#include <iostream>

namespace Utility {
    std::string b64encode(const void* input, int length) {
        std::string result((length + 2) / 3 * 4, '=');
        const auto ol = EVP_EncodeBlock(reinterpret_cast<unsigned char*>(&result[0]), reinterpret_cast<const unsigned char*>(input), length);
        if (result.size() != ol) {
            std::cerr << "Whoops, encode predicted " << result.size() << " but we got " << ol << "\n";
        }
        return result;
    }

    std::string b64decode(const void* input, int length) {
        const unsigned char* data = reinterpret_cast<const unsigned char*>(input);
        int pad1 = length % 4 || data[length - 1] == '=',
            pad2 = pad1 && data[length - 2] == '=';
        int outlen = length / 4 * 3 - pad1 - pad2;
        int decode_length;
        std::string result(outlen, 0);
        unsigned char* dest = reinterpret_cast<unsigned char*>(&result[0]);
        EVP_ENCODE_CTX* ctx = EVP_ENCODE_CTX_new();
        EVP_DecodeInit(ctx);
        EVP_DecodeUpdate(ctx, dest, &decode_length, data, length);
        OPENSSL_assert(outlen == decode_length);
        EVP_ENCODE_CTX_free(ctx);
        return result;
    }

    std::string b64encode(const std::string& str) {
        return b64encode(str.c_str(), str.size());
    }

    std::string b64decode(const std::string& str64) {
        return b64decode(str64.c_str(), str64.size());
    }
} // namespace Utility