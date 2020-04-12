// %%cpp libcrypto_example/main.cpp
// %run mkdir libcrypto_example/build 
// %run cd libcrypto_example/build && cmake .. > /dev/null && make  
// %run libcrypto_example/build/main 
// %run rm -r libcrypto_example/build

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <iostream>

#define EVP_ASSERT(stmt) do { if (!(stmt)) { \
    fprintf(stderr, "Statement failed: %s\n", #stmt); \
    ERR_print_errors_fp(stderr); \
    abort(); \
} } while (false)

struct TByteString: std::vector<unsigned char> {
    using std::vector<unsigned char>::vector;
    int ssize() { return static_cast<int>(size()); }
    char* SignedData() { reinterpret_cast<const char*>(data()); };
};

TByteString operator "" _b(const char* data, std::size_t len) {
    auto start = reinterpret_cast<const unsigned char*>(data);
    return {start, start + len};
}

TByteString Encrypt(const TByteString& plaintext, const TByteString& key, const TByteString& iv) {
    TByteString ciphertext(plaintext.size(), 0); // Верно для режима CTR, для остальных может быть не так
     
    auto* ctx = EVP_CIPHER_CTX_new();
    EVP_ASSERT(ctx);

    assert(key.size() * 8 == 256); // check key size for aes_256
    assert(iv.size() * 8 == 128); // check iv size for cipher with block size of 128 bits
    EVP_ASSERT(1 == EVP_EncryptInit_ex(ctx, EVP_aes_256_ctr(), NULL, key.data(), iv.data()));

    int len;
    // В эту функцию можно передавать исходный текст по частям, выход так же пишется по частям
    EVP_ASSERT(1 == EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size()));
    
    // В конце что-то могло остаться в буфере ctx и это нужно дописать
    EVP_ASSERT(1 == EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len));
    
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext;
}

TByteString Decrypt(const TByteString& ciphertext, const TByteString& key, const TByteString& iv) {
    TByteString plaintext(ciphertext.size(), 0);
    
    auto* ctx = EVP_CIPHER_CTX_new();
    EVP_ASSERT(ctx);
    
    assert(key.size() * 8 == 256); // check key size for aes_256
    assert(iv.size() * 8 == 128); // check iv size for cipher with block size of 128 bits
    EVP_ASSERT(1 == EVP_DecryptInit_ex(ctx, EVP_aes_256_ctr(), NULL, key.data(), iv.data()));

    int len;
    EVP_ASSERT(1 == EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()));
    EVP_ASSERT(1 == EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len));
    
    EVP_CIPHER_CTX_free(ctx);
    return plaintext;
}

int main () {
    TByteString key = "01234567890123456789012345678901"_b; // A 256 bit key 
    TByteString iv = "0123456789012355"_b; // A 128 bit IV (initialization vector)
    TByteString plaintext = "The quick brown fox jumps over the lazy dog"_b; // Message to be encrypted

    TByteString ciphertext = Encrypt(plaintext, key, iv); // Encrypt the plaintext

    // Do something useful with the ciphertext here
    printf("Ciphertext is:\n");
    BIO_dump_fp(stdout, ciphertext.SignedData(), ciphertext.size());
    
    TByteString decryptedtext = Decrypt(ciphertext, key, iv); // Decrypt the ciphertext

    printf("Decrypted text is: '%.*s'\n", decryptedtext.ssize(), decryptedtext.SignedData());
    return 0;
}

