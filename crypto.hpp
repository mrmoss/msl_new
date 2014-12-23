//Windows Dependencies:
//		-lcrypto

//Linux Dependencies:
//		-lcrypto

#ifndef MSL_C11_CRYPTO_HPP
#define MSL_C11_CRYPTO_HPP

#include <string>

bool encrypt_rsa(const void* plain,const size_t size,const std::string& key,std::string& cipher);
bool encrypt_rsa(const std::string& plain,const std::string& key,std::string& cipher);
bool decrypt_rsa(const void* cipher,const size_t size,const std::string& key,std::string& plain);
bool decrypt_rsa(const std::string& cipher,const std::string& key,std::string& plain);

bool encrypt_aes(const void* buffer,const size_t size,const std::string& key,const std::string& iv,std::string& cipher);
bool encrypt_aes(const std::string& plain,const std::string& key,const std::string& iv,std::string& cipher);
bool decrypt_aes(const void* buffer,const size_t size,const std::string& key,const std::string& iv,std::string& plain);
bool decrypt_aes(const std::string& cipher,const std::string& key,const std::string& iv,std::string& plain);

#endif