#ifndef GET_SPECS_H
#define GET_SPECS_H

#include <string>

std::string hash_password(const std::string& password, const std::string& salt);

#endif // GET_SPECS_H