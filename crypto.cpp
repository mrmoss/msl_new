#include "crypto.hpp"

#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <cstdint>

#define RSA_PKCS1_PADDING_SIZE			11
#define RSA_PKCS1_OAEP_PADDING_SIZE		41

bool encrypt_rsa(const void* plain,const size_t size,const std::string& key,std::string& cipher)
{
	bool success=true;
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	BIO* keybio=BIO_new_mem_buf((uint8_t*)key.c_str(),-1);

	if(keybio==nullptr)
		success=false;

	RSA* rsa=nullptr;

	if(success)
		rsa=PEM_read_bio_RSA_PUBKEY(keybio,&rsa,nullptr,nullptr);

	if(rsa==nullptr)
		success=false;

	if(size>(size_t)RSA_size(rsa)-RSA_PKCS1_OAEP_PADDING_SIZE)
		success=false;

	size_t temp_size=(size_t)-1;
	uint8_t* temp_data=new uint8_t[RSA_size(rsa)];

	if(success)
		temp_size=RSA_public_encrypt(size,(uint8_t*)plain,temp_data,rsa,RSA_PKCS1_OAEP_PADDING);

	if(success&&temp_size==(size_t)~0)
		success=false;

	if(success)
		cipher=std::string((char*)temp_data,temp_size);

	delete[] temp_data;
	BIO_free(keybio);
	RSA_free(rsa);
	ERR_free_strings();
	EVP_cleanup();
	ERR_remove_state(0);
	CRYPTO_cleanup_all_ex_data();
	return success;
}

bool encrypt_rsa(const std::string& plain,const std::string& key,std::string& cipher)
{
	return encrypt_rsa(plain.c_str(),plain.size(),key,cipher);
}

bool decrypt_rsa(const void* cipher,const size_t size,const std::string& key,std::string& plain)
{
	bool success=true;
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	BIO* keybio=BIO_new_mem_buf((uint8_t*)key.c_str(),-1);

	if(keybio==nullptr)
		success=false;

	RSA* rsa=nullptr;

	if(success)
		rsa=PEM_read_bio_RSAPrivateKey(keybio,&rsa,nullptr,nullptr);

	if(rsa==nullptr)
		success=false;

	if(size>(size_t)RSA_size(rsa))
		success=false;

	size_t temp_size=(size_t)-1;
	uint8_t* temp_data=new uint8_t[RSA_size(rsa)];

	if(success)
		temp_size=RSA_private_decrypt(size,(uint8_t*)cipher,temp_data,rsa,RSA_PKCS1_OAEP_PADDING);

	if(temp_size==(size_t)~0)
		success=false;

	if(success)
		plain=std::string((char*)temp_data,temp_size);

	delete[] temp_data;
	BIO_free(keybio);
	RSA_free(rsa);
	ERR_free_strings();
	EVP_cleanup();
	ERR_remove_state(0);
	CRYPTO_cleanup_all_ex_data();
	return success;
}

bool decrypt_rsa(const std::string& cipher,const std::string& key,std::string& plain)
{
	return decrypt_rsa(cipher.c_str(),cipher.size(),key,plain);
}

bool encrypt_aes256(const void* plain,const size_t size,const std::string& key,const std::string& iv,std::string& cipher)
{
	bool success=false;
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	EVP_CIPHER_CTX* ctx=EVP_CIPHER_CTX_new();
	uint8_t* temp_data=new uint8_t[(size/16+1)*16];

	int temp_length;
	int temp_unaligned_length;

	if(ctx!=nullptr&&EVP_CIPHER_CTX_set_padding(ctx,1)!=0&&
		EVP_EncryptInit_ex(ctx,EVP_aes_256_cbc(),nullptr,(uint8_t*)key.c_str(),(uint8_t*)iv.c_str())!=0&&
		EVP_EncryptUpdate(ctx,temp_data,&temp_length,(uint8_t*)plain,size)!=0&&
		EVP_EncryptFinal_ex(ctx,temp_data+temp_length,&temp_unaligned_length)!=0)
	{
		cipher=std::string((char*)temp_data,temp_length+temp_unaligned_length);
		success=true;
	}

	delete[] temp_data;
	EVP_CIPHER_CTX_free(ctx);
	ERR_free_strings();
	EVP_cleanup();
	ERR_remove_state(0);
	CRYPTO_cleanup_all_ex_data();
	return success;
}

bool encrypt_aes256(const std::string& plain,const std::string& key,const std::string& iv,std::string& cipher)
{
	return encrypt_aes256(plain.c_str(),plain.size(),key,iv,cipher);
}

bool decrypt_aes256(const void* cipher,const size_t size,const std::string& key,const std::string& iv,std::string& plain)
{
	bool success=false;
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	EVP_CIPHER_CTX* ctx=EVP_CIPHER_CTX_new();
	uint8_t* temp_data=new uint8_t[(size/16+1)*16];

	int temp_length;
	int temp_unaligned_length;

	if(ctx!=nullptr&&
		EVP_DecryptInit_ex(ctx,EVP_aes_256_cbc(),nullptr,(uint8_t*)key.c_str(),(uint8_t*)iv.c_str())!=0&&
		EVP_DecryptUpdate(ctx,temp_data,&temp_length,(uint8_t*)cipher,size)!=0&&
		EVP_DecryptFinal_ex(ctx,temp_data+temp_length,&temp_unaligned_length)!=0)
	{
		plain=std::string((char*)temp_data,temp_length+temp_unaligned_length);
		success=true;
	}

	delete[] temp_data;
	EVP_CIPHER_CTX_free(ctx);
	ERR_free_strings();
	EVP_cleanup();
	ERR_remove_state(0);
	CRYPTO_cleanup_all_ex_data();
	return success;
}

bool decrypt_aes256(const std::string& cipher,const std::string& key,const std::string& iv,std::string& plain)
{
	return decrypt_aes256(cipher.c_str(),cipher.size(),key,iv,plain);
}

std::string hash_sha256(const std::string& plain)
{
	std::string hash;
	hash.resize(SHA256_DIGEST_LENGTH);

	SHA256_CTX ctx;
	SHA256_Init(&ctx);
	SHA256_Update(&ctx,(unsigned char*)plain.c_str(),plain.size());
	SHA256_Final((unsigned char*)hash.c_str(),&ctx);

	return hash;
}

std::string hash_sha512(const std::string& plain)
{
	std::string hash;
	hash.resize(SHA512_DIGEST_LENGTH);

	SHA512_CTX ctx;
	SHA512_Init(&ctx);
	SHA512_Update(&ctx,(unsigned char*)plain.c_str(),plain.size());
	SHA512_Final((unsigned char*)hash.c_str(),&ctx);

	return hash;
}