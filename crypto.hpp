//Windows Dependencies:
//		-lcrypto -lgdi32 -lpthread

//Linux Dependencies:
//		-lcrypto -lpthread

#ifndef MSL_C11_CRYPTO_HPP
#define MSL_C11_CRYPTO_HPP

#include <string>

namespace msl
{
	bool encrypt_rsa(const std::string& plain,const std::string& key,std::string& cipher);
	bool decrypt_rsa(const std::string& cipher,const std::string& key,std::string& plain);

	bool encrypt_aes256(const std::string& plain,const std::string& key,
		const std::string& iv,std::string& cipher);
	bool decrypt_aes256(const std::string& cipher,const std::string& key,
		const std::string& iv,std::string& plain);

	bool hash_md5(const std::string& plain,std::string& hash);
	bool hash_sha256(const std::string& plain,std::string& hash);
	bool hash_sha512(const std::string& plain,std::string& hash);

	bool hmac_md5(std::string key,const std::string& plain,std::string& hash);
	bool hmac_sha256(std::string key,const std::string& plain,std::string& hash);
	bool hmac_sha512(std::string key,const std::string& plain,std::string& hash);
}

#endif