#include <iostream>
#include <string>
#include <cstring>
#include <crypt.h>

std::string hash_password(const std::string& password, const std::string& salt) {
    // Prepend "$6$" to specify SHA-512 hashing algorithm with salt
    std::string full_salt = "$6$" + salt;
    
    // Call crypt() to generate the hash
    char* hashed_cstr = crypt(password.c_str(), full_salt.c_str());
    if (hashed_cstr == nullptr) {
        throw std::runtime_error("Hashing failed using crypt()");
    }
    
    // Convert the C-style string to a std::string and return
    return std::string(hashed_cstr);
}

// int main() {
//     std::string password = "aaaaaaa";
//     std::string salt = "1UNfGmmL8v6itaPG";

//     // Generate hash with specified salt
//     std::string hash = hash_password(password, salt);
//     std::cout << "Hashed password: " << hash << std::endl;

//     return 0;
// }

// g++ sha512.cpp -o sha512 -lcrypt