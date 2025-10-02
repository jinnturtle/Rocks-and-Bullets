#ifndef SRC_VERSION_HPP_
#define SRC_VERSION_HPP_

#include <string>

/* PROGRAM_VERSION is expedted to be defined via compiler flags, likely stored
 * in a text file in the project directory */
#ifndef PROGRAM_VERSION
    #warning "PROGRAM_VERSION is not defined"
    #define PROGRAM_VERSION unknown-version
#endif

std::string version_str();

#endif // SRC_VERSION_HPP_
