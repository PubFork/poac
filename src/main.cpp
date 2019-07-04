#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <variant>

#include <poac/poac.hpp>

template <typename VS>
int handle(std::string&& str, VS&& vs) {
    namespace cli = poac::core::cli;
    namespace except = poac::core::except;
    namespace term = poac::io::term;
    using namespace std::string_literals;

    try {
        const auto result = cli::exec(std::forward<std::string>(str), std::forward<VS>(vs));
        if (!result.has_value()) {
            return EXIT_SUCCESS;
        }

        const except::Error err = result.value();
        if (std::holds_alternative<except::Error::InvalidSecondArg>(err.state)) {
            cli::exec("help"s, VS{err.what()});
        }
        else {
            std::cerr << term::error << err.what() << std::endl;
        }
        return EXIT_FAILURE;
    }
    catch (const except::error& e) {
        std::cerr << term::error << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const YAML::BadConversion& e) {
        std::cout << term::error << "poac.yml " << e.what()
                  << std::endl;
        return EXIT_SUCCESS;
    }
    catch (...) {
        std::cerr << term::error << "Unexpected error" << std::endl;
        return EXIT_FAILURE;
    }
}

int main(int argc, const char** argv) {
    using namespace std::string_literals;
    // argv[0]: poac, argv[1]: install, argv[2]: 1, ...

    //$ poac install --help => exec("--help", ["install"])
    if (argc == 3 && ((argv[2] == "-h"s) || (argv[2] == "--help"s))) {
        return handle(argv[2], std::vector<std::string>{argv[1]});
    }
    //$ poac install 1 2 3 => exec("install", ["1", "2", "3"])
    else if (argc >= 3) {
        return handle(argv[1], std::vector<std::string>(argv + 2, argv + argc));
    }
    //$ poac install => exec("install", [])
    else if (argc >= 2) {
        return handle(argv[1], std::vector<std::string>{});
    }
    //$ poac => exec("--help", [])
    else {
        handle("help", std::vector<std::string>{});
        return EXIT_FAILURE;
    }
}
