#ifndef CAG_HEADER
#define CAG_HEADER

#include "base64.hh"
#include "logger.hh"
#include "rijndael.h"
#include <string>

namespace CatAndMouseGame {
    inline void DumpHex(unsigned char* ptr, int line, int longLine = 0) {
        if (longLine) {
            for (int i = 0; i < line; ++i) {
                prim("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
                       ptr[i * 0x10], ptr[i * 0x10 + 1], ptr[i * 0x10 + 2], ptr[i * 0x10 + 3],
                       ptr[i * 0x10 + 4], ptr[i * 0x10 + 5], ptr[i * 0x10 + 6], ptr[i * 0x10 + 7],
                       ptr[i * 0x10 + 8], ptr[i * 0x10 + 9], ptr[i * 0x10 + 0xA], ptr[i * 0x10 + 0xB],
                       ptr[i * 0x10 + 0xC], ptr[i * 0x10 + 0xD], ptr[i * 0x10 + 0xE], ptr[i * 0x10 + 0xF]);
            }
        } else {
            for (int i = 0; i < line; ++i) {
                prim("%02X %02X %02X %02X", ptr[i * 4], ptr[i * 4 + 1], ptr[i * 4 + 2], ptr[i * 4 + 3]);
            }
        }
    }

    std::string AESEncryptWithPKCS7(std::string&, unsigned char*, const unsigned char*);

    std::string AESEncryptWithPKCS7(const char* data, int datalen, unsigned char*, const unsigned char*);

    std::string AESDecryptWithPKCS7(const void*, unsigned int, unsigned char*, const unsigned char*);

    std::string CatGame5(std::string&);

    std::string CatGame5(const char*, int);

    std::string MouseGame5(const std::string&);

    std::string MouseInfoMsgPack(const char*);
} // namespace CatAndMouseGame
#endif