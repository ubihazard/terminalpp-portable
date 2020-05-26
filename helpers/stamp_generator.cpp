#include <cstdlib>
#include <iostream>
#include <fstream>

#include "helpers/time.h"
#include "helpers/git.h"
#include "helpers/filesystem.h"

/** Stamp generator. 
 */

std::string NoOverrideArg{"--no-override"};

/** Creates the build stamp file.
 
    Arguments are the file where to store the stamp. Optionally, the `--no-override` argument can be used which will not override the stamp file if one already exists. 
        
 */
int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3)
        THROW(helpers::Exception()) << "Invalid number of arguments";
    std::string stampFile{argv[1]};
    if (argc == 3 && argv[2] == NoOverrideArg && std::filesystem::is_regular_file(stampFile)) {
        std::cout << "Stamp already exists and will *not* be overriden:" << std::endl;
        std::cout << std::endl << helpers::ReadEntireFile(stampFile) << std::endl;
        return EXIT_SUCCESS;
    }
    // open the file
    std::ofstream f(stampFile);
    if (!f.good())
        THROW(helpers::IOError()) << "Unable to open file " << stampFile;
    // gather stamp information
    helpers::GitRepo repo{"."};
    std::string gitCommit = repo.currentCommit();
    std::string gitDirty = repo.hasPendingChanges() ? "true" : "false";
    std::string buildTime = helpers::TimeInISO8601();
    // write 
    std::cout << "Writing stamp to file " << stampFile << ":" << std::endl;
    f << "/* AUTOGENERATED FILE, DO NOT EDIT! \n";
    f << "   This file was produced by the following command:\n ";
    for (int i = 0; i < argc; ++i)
        f << " " << argv[i]; 
    f << "\n";
    f << " */\n";
    f << "#include<string_view>\n";
    std::stringstream stamp;
    stamp << "namespace stamp {\n";
    // fill in the stamp details
    stamp << "    std::string_view constexpr version               = \"" << PROJECT_VERSION << "\";\n";
    stamp << "    std::string_view constexpr arch                  = \"" << ARCH << "\";\n";
    stamp << "    std::string_view constexpr arch_size             = \"" << ARCH_SIZE << "\";\n";
    stamp << "    std::string_view constexpr arch_compiler         = \"" << ARCH_COMPILER << "\";\n";
    stamp << "    std::string_view constexpr arch_compiler_version = \"" << ARCH_COMPILER_VERSION << "\";\n";
#ifdef NDEBUG
    // the debug flag is guarded by a macro
    stamp << "    std::string_view constexpr build                 = \"release\";\n";
#else
    stamp << "    std::string_view constexpr build                 = \"debug\";\n";
#endif
    // build time
    stamp << "    std::string_view constexpr build_time            = \"" << buildTime << "\";\n";
    // finally, figure the git version and whether it is dirty or not
    stamp << "    std::string_view constexpr commit                = \"" << gitCommit << "\";\n";
    stamp << "    bool             constexpr dirty                 = " << gitDirty << ";\n";
    stamp << "} // namespace stamp\n";
    std::string stmp = stamp.str();
    std::cout << std::endl << stmp << std::endl;
    f << stmp;
}
