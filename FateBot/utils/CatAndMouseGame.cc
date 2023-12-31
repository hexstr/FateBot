#include "CatAndMouseGame.hh"

#include <cstring>

#include "gzip/decompress.hpp"

using std::string;

#define BLOCK_SIZE 32

static unsigned char BattleKeyArray[32] = {
    0x6F, 0x7E, 0x60, 0x49, 0x70, 0x74, 0x69, 0x7E, 0x75, 0x47, 0x4C, 0x45,
    0x62, 0x7C, 0x34, 0x34, 0x77, 0x65, 0x51, 0x35, 0x63, 0x4D, 0x6C, 0x50,
    0x6E, 0x5D, 0x47, 0x71, 0x4B, 0x40, 0x35, 0x4E
};

static unsigned char BattleIVArray[32] = {
    0x7B, 0x7C, 0x79, 0x7C, 0x61, 0x7B, 0x50, 0x7B, 0x4F, 0x51, 0x79, 0x5A,
    0x5E, 0x6B, 0x79, 0x7A, 0x40, 0x5A, 0x71, 0x6C, 0x62, 0x3B, 0x63, 0x3E,
    0x7E, 0x42, 0x4B, 0x71, 0x5B, 0x7D, 0x3B, 0x6F
};

static unsigned char MouseInfoKeyArray[32] = {
    0x57, 0x30, 0x4A, 0x75, 0x68, 0x34, 0x63, 0x46, 0x4A, 0x53, 0x59, 0x50,
    0x6B, 0x65, 0x62, 0x4A, 0x42, 0x39, 0x57, 0x70, 0x73, 0x77, 0x4E, 0x46,
    0x35, 0x31, 0x6F, 0x61, 0x36, 0x47, 0x6D, 0x37
};

// BattleKey = Encoding.UTF8.GetBytes("kzdMtpmzqCHAfx00saU1gIhTjYCuOD1J");
// BattleIV = Encoding.UTF8.GetBytes("stqtisXsGYqRVcqrHRydj3k6vJCySu3g");

namespace CatAndMouseGame {
    string CatGame5(std::string& str) {
        return AESEncryptWithPKCS7(str, BattleKeyArray, BattleIVArray);
    }

    string CatGame5(const char* data, int datalen) {
        return AESEncryptWithPKCS7(data, datalen, BattleKeyArray, BattleIVArray);
    }

    string MouseGame5(const string& str) {
        auto data = Utility::b64decode(str);
        return AESDecryptWithPKCS7(data.c_str(), data.length(), BattleKeyArray, BattleIVArray);
    }

    bool PKCS7Padding(string& str) {
        int padding_size = BLOCK_SIZE - str.length() % BLOCK_SIZE;
        str.append(padding_size, padding_size);
        return true;
    }

    bool UnPKCS7Padding(string& str) {
        if (str.length() <= 0) {
            return false;
        }

        int len = static_cast<int>(str.back());
        if (str.length() < len)
            return false;

        str.erase(str.end() - len, str.end());
        return true;
    }

    string AESEncryptWithPKCS7(string& data, unsigned char* key, const unsigned char* iv) {
        PKCS7Padding(data);
        unsigned char* out = new unsigned char[data.length()];
        rijn_context rijn;
        rijn_set_key(&rijn, key, 256, 256);
        rijn_cbc_encrypt(&rijn, iv, (const unsigned char*)data.c_str(), out, data.length());
        auto enc = Utility::b64encode(out, data.length());
        delete[] out;
        return enc;
    }

    string MouseInfoMsgPack(const char* str) {
        auto raw = Utility::b64decode(str, std::strlen(str));
        const char* data = &(raw.c_str())[32];
        const unsigned char* iv = (const unsigned char*)raw.c_str();
        return AESDecryptWithPKCS7(data, raw.length() - 32, MouseInfoKeyArray, iv);
    }

    string AESEncryptWithPKCS7(const char* data, int datalen, unsigned char* key, const unsigned char* iv) {
        int padding_size = BLOCK_SIZE - datalen % BLOCK_SIZE;
        int newlen = datalen + padding_size;
        unsigned char* in = new unsigned char[newlen];
        unsigned char* out = new unsigned char[newlen];
        std::memcpy(in, data, datalen);
        for (int i = 0; i < padding_size; ++i) {
            in[datalen + i] = padding_size;
        }

        rijn_context rijn;
        rijn_set_key(&rijn, key, 256, 256);
        rijn_cbc_encrypt(&rijn, iv, in, out, newlen);
        auto enc = Utility::b64encode(out, newlen);
        delete[] in;
        delete[] out;
        return enc;
    }

    string AESDecryptWithPKCS7(const void* data, unsigned int datalen, unsigned char* key, const unsigned char* iv) {
        unsigned char* out = new unsigned char[datalen];
        rijn_context rijn;
        rijn_set_key(&rijn, key, 256, 256);
        rijn_cbc_decrypt(&rijn, iv, (const unsigned char*)data, out, datalen);
        // remove pkcs7padding
        int len = out[datalen - 1];
        string dec;
        if (0x00 < len && len < datalen) {
            dec = gzip::decompress((const char*)out, datalen - len);
        }
        else {
            dec = gzip::decompress((const char*)out, datalen);
        }
        delete[] out;
        return dec;
    }
} // namespace CatAndMouseGame