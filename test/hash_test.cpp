#include <iostream>
#include <string>
#include <openssl/sha.h>   // 用于SHA256
#include <openssl/bio.h>   // 用于Base64
#include <openssl/evp.h>
#include <openssl/buffer.h>

// 将你的哈希生成代码封装成一个独立的函数
std::string generateHash(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(input.c_str()), input.length(), hash);

        // Base64编码
        BIO *bio, *b64;
        BUF_MEM *bufferPtr;

        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio);

        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO_write(bio, hash, SHA256_DIGEST_LENGTH);
        BIO_flush(bio);
        BIO_get_mem_ptr(bio, &bufferPtr);

        std::string hashedInput(bufferPtr->data, bufferPtr->length);

        BIO_free_all(bio);

        return hashedInput;

}

std::string setPwd(const std::string& pwd) {
    // 先SHA256哈希
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(pwd.c_str()), pwd.length(), hash);

        // Base64编码 (32字节 → 44字符)
        BIO *bio, *b64;
        BUF_MEM *bufferPtr;

        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio);

        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);  // 无换行
        BIO_write(bio, hash, SHA256_DIGEST_LENGTH);
        BIO_flush(bio);
        BIO_get_mem_ptr(bio, &bufferPtr);

        std::string hashedPwd(bufferPtr->data, bufferPtr->length);

        BIO_free_all(bio);


       return hashedPwd;

}

int main() {
    std::string testPassword;
    std::cout << "请输入测试密码: ";
    std::cin >> testPassword;

    // 关键部分：连续调用两次，模拟注册和登录
    std::cout << "\n=== 哈希值比对 ===\n";
    std::string hash1 = generateHash(testPassword);
    std::cout << "第一次生成 (模拟注册): " << hash1 << std::endl;

    std::string hash2 = setPwd(testPassword); // 使用相同的输入
    std::cout << "第二次生成 (模拟登录): " << hash2 << std::endl;

    std::cout << "\n=== 比对结果 ===\n";
    if (hash1 == hash2) {
        std::cout << "✅ 成功！两次生成的哈希值完全一致。\n";
        std::cout << "问题可能出在其他环节，比如数据库存储或字段比较。\n";
    } else {
        std::cout << "❌ 不一致！两次生成的哈希值不同。\n";
        std::cout << "这表明 generateHash 函数内部存在不确定性。\n";
    }

    return 0;
}