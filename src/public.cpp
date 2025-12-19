#include "public.hpp"

namespace ChatCrypto {
    std::string encryptMessage(const std::string& msg, const std::string& key) {
        AES_KEY aesKey;
        AES_set_encrypt_key(reinterpret_cast<const unsigned char*>(key.c_str()), 128, &aesKey);
        std::vector<unsigned char> encrypted(msg.size() + AES_BLOCK_SIZE);
        int len = msg.size();
        AES_encrypt(reinterpret_cast<const unsigned char*>(msg.c_str()), encrypted.data(), &aesKey);
        return std::string(encrypted.begin(), encrypted.begin() + len);
    }

    std::string decryptMessage(const std::string& encrypted, const std::string& key) {
        AES_KEY aesKey;
        AES_set_decrypt_key(reinterpret_cast<const unsigned char*>(key.c_str()), 128, &aesKey);
        std::vector<unsigned char> decrypted(encrypted.size());
        AES_decrypt(reinterpret_cast<const unsigned char*>(encrypted.c_str()), decrypted.data(), &aesKey);
        return std::string(decrypted.begin(), decrypted.end());
    }
}