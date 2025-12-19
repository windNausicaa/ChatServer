#ifndef USER_H
#define USER_H
#include <string>
#include <openssl/sha.h> // 预防SQL注入，加密
#include <openssl/evp.h>  
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <muduo/base/Logging.h>

using namespace std;

// User表的ORM类
//ORM = 对象映射类
class User{
public:
    User(int id = -1,string name = "",string pwd="",string state="offline"){
        this->id = id;
        this->name = name;
        this->password =pwd;
        this->state =state;
    }
    void setId(int id){this->id =id;}
    void setName(string name){this->name =name;}
    //void setPwd(string pwd){this->password=pwd;}
    void setState(string state){this->state = state;}

    int getId(){return this->id;}
    string getName(){return this->name;}
    string getPwd(){return this->password;}
    string getState(){return this->state;};


    //预防SQL注入
    // void setPwd(const std::string& pwd) {
    //     this->password = pwd;
    // }

    // 原加密
    // void setPwd(const std::string& pwd) {
    //     unsigned char hash[SHA256_DIGEST_LENGTH];
    //     SHA256(reinterpret_cast<const unsigned char*>(pwd.c_str()), pwd.length(), hash);
    //     std::string hashedPwd;
    //     for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
    //         char buf[3];
    //         sprintf(buf, "%02x", hash[i]);
    //         hashedPwd += buf;
    //     }
    //     this->password = hashedPwd;  
    // }

    void setPwd(const std::string& pwd) {
    //     LOG_INFO << "pwd"<< pwd;
    // // 先SHA256哈希
    //     unsigned char hash[SHA256_DIGEST_LENGTH];
    //     SHA256(reinterpret_cast<const unsigned char*>(pwd.c_str()), pwd.length(), hash);

    //     LOG_INFO << "hash"<< hash;
    //     // Base64编码 (32字节 → 44字符)
    //     BIO *bio, *b64;
    //     BUF_MEM *bufferPtr;

    //     b64 = BIO_new(BIO_f_base64());
    //     bio = BIO_new(BIO_s_mem());
    //     bio = BIO_push(b64, bio);

    //     BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);  // 无换行
    //     BIO_write(bio, hash, SHA256_DIGEST_LENGTH);
    //     BIO_flush(bio);
    //     BIO_get_mem_ptr(bio, &bufferPtr);

    //     std::string hashedPwd(bufferPtr->data, bufferPtr->length);

    //     BIO_free_all(bio);

    //     LOG_INFO << "第一次转换时的哈希值: " << hashedPwd;

        this->password = pwd;
    }


private:
    int id;

    string name;

    string password;
    string state;

};

#endif