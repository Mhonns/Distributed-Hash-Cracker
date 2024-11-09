#include <iostream>
#include <string>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <cstring>
#include <iomanip>
#include <sstream>

class SHA512Hasher {
private:
    static const int SALT_LENGTH = 16;
    static const int ROUNDS = 5000;

    // Convert binary to base64
    std::string to_base64(const unsigned char* input, size_t length) {
        static const char b64_chars[] = 
            "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        
        std::string result;
        uint32_t bits = 0;
        int bit_count = 0;
        
        for (size_t i = 0; i < length; ++i) {
            bits = (bits << 8) | input[i];
            bit_count += 8;
            
            while (bit_count >= 6) {
                bit_count -= 6;
                result += b64_chars[(bits >> bit_count) & 0x3F];
            }
        }
        
        if (bit_count > 0) {
            bits <<= (6 - bit_count);
            result += b64_chars[bits & 0x3F];
        }
        
        return result;
    }

    // Generate random salt
    std::string generate_salt() {
        unsigned char salt_bytes[SALT_LENGTH];
        RAND_bytes(salt_bytes, SALT_LENGTH);
        return to_base64(salt_bytes, SALT_LENGTH);
    }

    // Perform the actual SHA-512 hashing
    std::string sha512_crypt(const std::string& password, const std::string& salt) {
        unsigned char key[SHA512_DIGEST_LENGTH];
        unsigned char final_hash[SHA512_DIGEST_LENGTH];
        
        // Initial hash
        std::string salted = password + salt;
        SHA512_CTX ctx;
        SHA512_Init(&ctx);
        SHA512_Update(&ctx, salted.c_str(), salted.length());
        SHA512_Final(key, &ctx);
        
        // Multiple rounds of hashing
        for (int i = 0; i < ROUNDS - 1; ++i) {
            SHA512_Init(&ctx);
            SHA512_Update(&ctx, key, SHA512_DIGEST_LENGTH);
            SHA512_Final(key, &ctx);
        }
        
        memcpy(final_hash, key, SHA512_DIGEST_LENGTH);
        
        return to_base64(final_hash, SHA512_DIGEST_LENGTH);
    }

public:
    // Hash password with provided salt
    std::string hash_with_salt(const std::string& password, const std::string& salt) {
        return "$6$" + salt + "$" + sha512_crypt(password, salt);
    }

    // Hash password with random salt
    std::string hash(const std::string& password) {
        return hash_with_salt(password, generate_salt());
    }

    // Verify password against hash
    bool verify(const std::string& password, const std::string& hash) {
        if (hash.length() < 4 || hash.substr(0, 3) != "$6$") {
            return false;
        }

        size_t second_dollar = hash.find('$', 3);
        if (second_dollar == std::string::npos) {
            return false;
        }

        std::string salt = hash.substr(3, second_dollar - 3);
        std::string generated = hash_with_salt(password, salt);
        return generated == hash;
    }
};

int main() {
    SHA512Hasher hasher;
    std::string password = "aaaaaaa";
    std::string known_salt = "1UNfGmmL8v6itaPG";
    
    // Hash with known salt
    std::string hashed = hasher.hash_with_salt(password, known_salt);
    std::cout << "Hash with known salt: " << hashed << std::endl;
    
    // Verify the known hash
    std::string known_hash = "$6$1UNfGmmL8v6itaPG$j54ho2EN0nnSk14Bd0AG2JoP5Zsue1I1DT/VDOFRPAonZg1J7PdlIknsEvDB2srHXV/zCkSr10sBDIOV.PF8f/";
    bool verified = hasher.verify(password, known_hash);
    std::cout << "Verification result: " << (verified ? "Success" : "Failed") << std::endl;
    
    // Generate new hash with random salt
    std::string new_hash = hasher.hash(password);
    std::cout << "New hash with random salt: " << new_hash << std::endl;
    
    return 0;
}