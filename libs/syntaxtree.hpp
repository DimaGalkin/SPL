#pragma once

#include <iostream>
#include <deque>

#include "tokens.hpp"

class syntax_tree {
    public:
        std::deque<token_T> children;

        errors add_child(token_T child) {
            children.push_back(child);

            return ok;
        }

        errors create_tree(std::deque<std::string> split_contents) {
            token_analysis analyser("data/insts.json");
            
            for (auto& line : split_contents) {
                std::deque<std::string> split_line = split(line, ' ');
                
                instruction_token line_token;

                for (auto& str_token : split_line) {
                    token_type status =  analyser.determine_type(str_token);

                    if (status == UNDEF) {
                        return invalid_token;
                    }

                    if (status == INSTRUCTION) {
                        line_token.set_value(str_token);
                    } else if (status == REGISTER) {
                        register_token reg_token;

                        str_token = sanitize(str_token, "{}");

                        reg_token.set_value(str_token);
                        line_token.children.push_back(reg_token);
                    } else if (status == IMMIDIATE) {
                        immidiate_token imm_token;
                        
                        str_token = sanitize(str_token, "[]");

                        imm_token.set_value(str_token);
                        line_token.children.push_back(imm_token);

                    }
                }

                add_child(line_token);
            }

            draw_tree();
            return ok;
        }

        void draw_tree() {
            for (auto& child : children) {
                std::cout << child.value << ": ";

                for (auto& gr : child.children) {
                    std::cout << gr.value << "\t";
                }

                std::cout << std::endl;
            }
        }

        std::string sanitize (std::string word, std::string delims) {
            int spos = 0;

            while ((spos = word.find_first_of (delims, spos)) != std::string::npos)
                    word.erase (spos, 1);
            return word;
        }

        std::deque<std::string> split(std::string line, char delim) {
            size_t pos = 0;
            std::string token;

            std::deque<std::string> lines;

            while (pos != std::string::npos) {
                pos = line.find(delim);

                token = line.substr(0, pos);
                lines.push_back(token);

                line.erase(0, pos + 1);
            }

            return lines;
        }
};