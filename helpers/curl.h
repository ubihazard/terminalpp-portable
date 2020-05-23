#pragma once
#include <string>

#include "helpers/helpers.h"
#include "helpers/process.h"

namespace helpers {

    /** Very simple download of specified URL. 
     
        On Windows, uses the powershell to download the required content, while on Linux the curl command itself is used. 

        Note that this function is extremely barebones and very ineffective since it creates a new process for each download. It is ok to use it for one-time downloads, but is completely unsuited for batch downloads, etc. 

        This could easily be remedied by switching to libcurl on Linux, on Windows, the situation is more complex (although libcurl can be used as well).
     */
    std::string Curl(std::string const & url) {
#if (ARCH_WINDOWS)
        return Exec(Command{"powershell.exe", { STR("(curl " << url << " -UseBasicParsing).Content")}}, "");
#elif (ARCH_UNIX)
        return Exec(Command{"curl", {"-i", url}}, "");
#endif
    }

} // namespace helpers