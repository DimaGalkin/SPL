#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>

#include "libs/tokens.hpp"
#include "libs/syntaxtree.hpp"

class assembler {
    private:
        std::fstream target;
        std::deque<std::string> contents;

        syntax_tree* abstract_syntax_tree;

    public:

        assembler() {
            abstract_syntax_tree = new syntax_tree();
        };

        errors assemble(std::string filename) {
            target.open(filename, std::ios::in);
        
            if (!target) {
                std::cout << coloured(filename + " does not exist!", 'r') << std::endl;
            }

            read_target();

            abstract_syntax_tree->create_tree(contents);

            return ok;
        }

        errors read_target() {
            if (!target.is_open()) {
                std::cout << coloured("Target file is not open!", 'r') << std::endl;
                return file_not_open;
            }

            std::string line;

            while (std::getline(target, line)) {
                boost::algorithm::trim(line);

                if (!line.empty()) {
                    contents.push_back(line);
                }
            }

            return ok;
        }

        std::string coloured(std::string text, char color) {
            switch (color) {
                case 'r':
                    return "\033[31m" + text + "\033[0m";
                case 'g':
                    return "\033[32m" + text + "\033[0m";
                case 'y':
                    return "\033[33m" + text + "\033[0m";
                case 'b':
                    return "\033[34m" + text + "\033[0m";
                default:
                    return "\033[0m" + text + "\033[0m";
            }
        }

        ~assembler() {
            target.close();
            delete abstract_syntax_tree;
        };
};

int main(int argc, char** argv) {

	if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    } 

    assembler dasm;

    dasm.assemble(argv[1]);

	return 0;
}