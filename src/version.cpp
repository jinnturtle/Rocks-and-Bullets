#include "version.hpp"

#include <sstream>
#include <array>

#define EXPAND(x) #x
// quotes are added automatically when a macro calls a macro
#define EXPAND_QUOTED(x) EXPAND(x)

std::string version_str()
{
    std::stringstream buf;
    buf
#ifdef DEBUG
    << "DBG-"
#endif // #ifdef DEBUG
    << "v" << EXPAND_QUOTED(PROGRAM_VERSION);

    return buf.str();
}

