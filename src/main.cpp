#include "compiler.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    if (std::string(argv[1]) == "-h") {
        std::cout << "Usage: " << argv[0] << " <filename>" << " <output type?>" << std::endl;
        std::cout << "Output types: S16, S8, D8" << std::endl;
        return 0;
    }

    SCompiler compiler;

    if (argc == 3) {
        bool ok = compiler.set_output(argv[2]);

        if (!ok) {
            std::cout << "Usage: " << argv[0] << " <filename>" << " <output type>" << std::endl;
            std::cout << "use -h for help" << std::endl;
            return 1;
        }
    }

    return compiler.compile(argv[1]);
}