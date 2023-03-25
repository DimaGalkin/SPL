#pragma once

#include <iostream>
#include <deque>
#include <fstream>
#include <stack>

#include "json.hpp"
#include "errors.hpp"

enum token_type {
    REGISTER = 1000,
    IMMIDIATE = 1001,
    INSTRUCTION = 1002,
    LABEL = 1003,
    LABEL_REFERENCE = 1004,
    CALCUALTION = 1005,
    ADDRESS = 1006,
    UNDEF = 9999
};

class token_T {
    public:
        token_type type;
        std::string value;
        std::deque<token_T> children;

        virtual errors set_value(std::string value) { return incomplete_code; };
};

class instruction_token : public token_T {
    public:
        token_type type = INSTRUCTION;

        errors set_value(std::string name) {
            value = name;

            return ok;
        }
        
};

class immidiate_token : public token_T {
    public:
        token_type type = IMMIDIATE;

        errors set_value(std::string value_string) override {
            int asint = std::stoi(value_string);

            if (asint > 65535) {
                value = "0";
                return integer_overflow;
            } else if (asint < 0) {
                value = "0";
                return integer_underflow;
            }

            value = std::to_string(asint);

            return ok;
        }
};

class register_token : public token_T {
    public:
        token_type type = REGISTER;
        std::deque<std::string> valid_registers = {"A", "B", "C", "D"};

        errors set_value(std::string name) override {
            value = name;

            return ok;
        }
};

class address_token : public token_T {
    public:
        token_type type = ADDRESS;
        uint16_t value;

        errors set_value(std::string value_string) override {
            int asint = std::stoi(value_string);

            if (asint > 65535) {
                value = 0;
                return integer_overflow;
            } else if (asint < 0) {
                value = 0;
                return integer_underflow;
            }

            value = asint;

            return ok;
        }
};

class token_analysis {
    public:
        std::string json_file;

        nlohmann::json token_info;

        token_analysis(std::string filename) {
            json_file = filename;

            errors status = get_token_info();

            if (status != ok) {
                std::cout << "JSON Was Not Loaded!" << std::endl;
            }
        }

        token_type determine_type(std::string token) {
            if (token.length() < 1) {
                return UNDEF;
            }

            nlohmann::json instructions = token_info["instructions"];

            if (in(token, instructions)) {
                return INSTRUCTION;
            }

            if (token.find('[') != std::string::npos) {
                if (bracket_check(token, '[', ']')) {
                    return IMMIDIATE;
                } else {
                    return UNDEF;
                }
            }

            if (token.find('{') != std::string::npos) {
                if (bracket_check(token, '{', '}')) {
                    return REGISTER;
                } else {
                    return UNDEF;
                }
            }

            if (token.find('&') != std::string::npos) {
                return ADDRESS;
            }
            
            return UNDEF;
        }

        bool bracket_check(std::string line, char o, char c) {
            std::stack<char> bracket_stack;

            for (char& chr : line) {
                if (chr == o) {
                    bracket_stack.push(chr);
                } else if (chr == c) {
                    if (bracket_stack.empty()) {
                        return false;
                    } else {
                        bracket_stack.pop();
                    }
                }
            }

            if (bracket_stack.empty()) {
                return true;
            } else {
                return false;
            }
        }

        bool in(std::string key, nlohmann::json v) {
            if (std::find(v.begin(), v.end(), key) != v.end()) {
                return true;
            }
            else {
                return false;
            }
        }

        errors get_token_info() {
            std::ifstream token_file(json_file);

            if (!token_file.is_open()) {
                return file_not_open;
            }

            token_info = nlohmann::json::parse(token_file);

            token_file.close();

            return ok;
        }
};