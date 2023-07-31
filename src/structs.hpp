#pragma once

#include <string>
#include <vector>
#include <memory>

#include "types.hpp"

struct Token_Child {
    public:
        Types type = NONE;
        std::string name = "";
        std::string value = "";

        Token_Child() = default;
        virtual ~Token_Child() = default;
};
using TKCptr = std::shared_ptr<Token_Child>;

struct Token {
    Instructions type;

    std::vector<TKCptr> children;

    Token(Instructions type, std::vector<TKCptr> children) {
        this->type = type;
        this->children = children;
    }

    virtual ~Token() = default;
};
using TKptr = std::shared_ptr<Token>;

struct Register : public Token_Child {
    Register(Types type, std::string name) {
        this->type = type;
        this->name = name;
    }
};
using REGptr = std::shared_ptr<Register>;

struct Immediate : public Token_Child {
    Immediate(Types type, std::string value) {
        this->type = type;
        this->value = value;
    }
};
using IMMptr = std::shared_ptr<Immediate>;

struct InstructionCode {
    public:
        virtual std::string get_code(const Types first, const Types second, const std::string first_val, const std::string second_val) = 0;
        virtual ~InstructionCode() = default;
};
using InstPtr = std::shared_ptr<InstructionCode>;