#pragma once

#include <iostream>

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <bitset>
#include <iomanip>

enum Types {
    REG,
    IMM,
    REG_IMM,
    NONE
};

enum OutputTypes {
    HALF_DUAL_WORD, // EEPROM which outputs data at address n and n+1, 2 8 bit outputs
    HALF_SINGLE_WORD, // EEPROM which outputs data at address n so 2 have to be used as data width is 8 bits
    FULL_SINGLE_WORD // EEPROM which outputs data at address n but data width is 16 bits
};

enum Errors {
    OK,
    file_not_found,
    file_write_error,
    invalid_instruction,
    invalid_register,
    invalid_immediate,
};

enum Instructions {
    MOV,
    WR,
    RD,
    EXEC,
    ADD,
    SUB,
    JMP,
    CMP,
    JE,
    JNE,
    HLT
};

struct Token_Child {
    public:
        Types type_ = NONE;
        std::string name_ = "";
        std::string value_ = "";

        Token_Child() = default;
        virtual ~Token_Child() = default;
};
using TKCptr = std::shared_ptr<Token_Child>;

struct Token {
    Instructions type_;

    std::vector<TKCptr> children_;

    Token() = delete;

    Token(Instructions type, std::vector<TKCptr> children) {
        type_ = type;
        children_ = children;
    }

    virtual ~Token() = default;
};
using TKptr = std::shared_ptr<Token>;

struct Register : public Token_Child {

    Register() = delete;

    Register(Types type, const std::string& name) {
        type_ = type;
        name_ = name;
    }
};
using REGptr = std::shared_ptr<Register>;

struct Immediate : public Token_Child {

    Immediate() = delete;

    Immediate(Types type, const std::string& value) {
        type_ = type;
        value_ = value;
    }
};
using IMMptr = std::shared_ptr<Immediate>;

struct InstructionCode {
    public:
        virtual std::string get_code(const Types first, const Types second, const std::string& first_val, const std::string& second_val) = 0;
        virtual ~InstructionCode() = default;
};
using InstPtr = std::shared_ptr<InstructionCode>;

struct RegisterCode {
    std::string name_;
    std::string from_bus_;
    std::string to_bus_;

    RegisterCode() = delete;

    RegisterCode(const std::string& name, const std::string& from_bus, const std::string& to_bus) {
        name_ = name;
        from_bus_ = from_bus;
        to_bus_ = to_bus;
    }
};

struct Instruction {
    std::string name_;

    int children_;
    std::vector<Types> child_types_;
    Instructions type_;

    Instruction() = delete;

    Instruction(const std::string& name, int children, std::vector<Types> child_types, Instructions type) {
        name_ = name;
        children_ = children;
        child_types_ = child_types;
        type_ = type;
    }

    virtual ~Instruction() = default;
};

const std::unordered_map<std::string, RegisterCode> register_codes = {
    {"a", RegisterCode("a", "00001", "00001")},
    {"b", RegisterCode("b", "00011", "00010")},
    {"c", RegisterCode("c", "01000", "00111")},
    {"acc", RegisterCode("acc", "N_ALWD", "00100")},
    {"flgs", RegisterCode("flgs", "N_ALWD", "00101")},
    {"lgc", RegisterCode("lgc", "N_ALWD", "01000")},
};

class SCompiler {
    private:
        const std::unordered_map<std::string, Instruction> instructions_ = {
            {"mov",  Instruction("mov", 2, {REG, REG_IMM}, MOV)},
            {"wr", Instruction("wr", 2, {REG_IMM, REG}, WR)},
            {"rd", Instruction("wr", 2, {REG_IMM, REG}, RD)},
            {"exec", Instruction("exec", 1, {REG_IMM}, EXEC)},
            {"add", Instruction("add", 2, {REG_IMM, REG_IMM}, ADD)},
            {"sub", Instruction("sub", 2, {REG_IMM, REG_IMM}, SUB)},
            {"cmp", Instruction("cmp", 2, {REG_IMM, REG_IMM}, CMP)},
            {"jmp", Instruction("jmp", 1, {REG_IMM}, JMP)},
            {"je", Instruction("je", 1, {REG_IMM}, JE)},
            {"jne", Instruction("jne", 1, {REG_IMM}, JNE)},
            {"hlt", Instruction("hlt", 0, {}, HLT)}
        };

        const std::unordered_map<std::string, Register> registers_ = {
            {"a", Register(REG, "a")},
            {"b", Register(REG, "b")},
            {"c", Register(REG, "c")},
            {"acc", Register(REG, "acc")},
            {"flgs", Register(REG, "flgs")},
            {"lgc", Register(REG, "lgc")},
        };

        std::vector<std::string> lines_;
        std::vector<TKptr> tokens_;
        std::string output_;
        OutputTypes output_type_;

        Errors load_file(const std::string& file_name);
        Errors parse_file();
        Errors parse_tree();
        Errors write_file();
        Errors locate_instruction(const std::string& inst_name);
    
        TKCptr make_token(Instruction inst, const std::string& reg, const std::string& imm, const int token_num);
        InstPtr get_instruction_struct(const Instructions& inst);
        
        void split (std::string str, const std::string& delim, std::vector<std::string>& out);

        bool is_valid_reg(const std::string& reg);
        bool is_valid_imm(const std::string& imm);
    public:
        // TODO: Set most methods to private;
        SCompiler() {
            output_type_ = HALF_DUAL_WORD;
        };

        static std::string htos(const std::string& hex_value);
        bool set_output(const std::string& format);
        int compile(const std::string& file_name);

        ~SCompiler() = default;
};

struct MOV_Inst : public InstructionCode {
    std::string get_code(const Types first, const Types second, const std::string& first_val, const std::string& second_val) override {
        if (first == REG && second == REG) {
            return "0000" + register_codes.at(first_val).from_bus_ + register_codes.at(second_val).to_bus_ + "00\n";
        } else if (first == REG && second == IMM) {
            return "0000" + register_codes.at(first_val).from_bus_ + "0000001\n" + SCompiler::htos(second_val) + "\n";
        }

        return "error";
    }

    virtual ~MOV_Inst() = default;
};

struct HLT_Inst : public InstructionCode {
    std::string get_code(const Types first, const Types second, const std::string& first_val, const std::string& second_val) override {
        return SCompiler::htos("0x7C") + "\n";
    }

    virtual ~HLT_Inst() = default;
};

struct WR_Inst : public InstructionCode {
    std::string get_code(const Types first, const Types second, const std::string& first_val, const std::string& second_val) override {
        // load address into mar (from imm or from reg)
        std::string mar_code = "";
        std::string wr_code = "";

        if (first == REG) {
            mar_code = "000000111" + register_codes.at(first_val).to_bus_ + "00\n";
        } else if (first == IMM) {
            mar_code = "0000001110000001\n" + SCompiler::htos(first_val) + "\n";
        }

        wr_code = "100000010" + register_codes.at(second_val).to_bus_ + "11\n";
        
        return mar_code + wr_code;
    }

    virtual ~WR_Inst() = default;
};

struct RD_Inst : public InstructionCode {
    std::string get_code(const Types first, const Types second, const std::string& first_val, const std::string& second_val) override {
        // load address into mar (from imm or from reg)
        std::string mar_code = "";
        std::string rd_code = "";

        if (first == REG) {
            mar_code = "000000111" + register_codes.at(first_val).to_bus_ + "00\n";
        } else if (first == IMM) {
            mar_code = "0000001110000001\n" + SCompiler::htos(first_val) + "\n";
        }

        rd_code = "0000" + register_codes.at(second_val).from_bus_ + "01001" + "00\n";
        
        return mar_code + rd_code;
    }

    virtual ~RD_Inst() = default;
};

struct EXEC_Inst : public InstructionCode {
    std::string get_code(const Types first, const Types second, const std::string& first_val, const std::string& second_val) override {
        // load address into mar (from imm or from reg)
        std::string mar_code = "";
        std::string rd_code = "";

        if (first == REG) {
            mar_code = "000000111" + register_codes.at(first_val).to_bus_ + "00\n";
        } else if (first == IMM) {
            mar_code = "0000001110000001\n" + SCompiler::htos(first_val) + "\n";
        }

        rd_code = "0000010010100100\n";
        
        return mar_code + rd_code;
    }

    virtual ~EXEC_Inst() = default;
};

struct ADD_Inst : public InstructionCode {
    std::string get_code(const Types first, const Types second, const std::string& first_val, const std::string& second_val) override {
        std::string first_int = "";
        std::string second_int = "";

        if (first == REG) {
            first_int = "000000100" + register_codes.at(first_val).to_bus_ +  "00\n";
        } else if (first == IMM) {
            first_int = "0000001000000001\n" + SCompiler::htos(first_val) + "\n";
        }

        if (second == REG) {
            second_int = "000100000" + register_codes.at(second_val).to_bus_ + "00\n";
        } else if (second == IMM) {
            second_int = "0001000000000001\n" + SCompiler::htos(second_val) + "\n";
        }

        return first_int + second_int;
    }

    virtual ~ADD_Inst() = default;
};

struct SUB_Inst : public InstructionCode {
    std::string get_code(const Types first, const Types second, const std::string& first_val, const std::string& second_val) override {
        std::string first_int = "";
        std::string second_int = "";

        if (first == REG) {
            first_int = "000000100" + register_codes.at(second_val).to_bus_ +  "00\n";
        } else if (first == IMM) {
            first_int = "0000001000000001\n" + SCompiler::htos(second_val) + "\n";
        }

        if (second == REG) {
            second_int = "001000000" + register_codes.at(first_val).to_bus_ + "00";
        } else if (second == IMM) {
            second_int = "0010000000000001\n" + SCompiler::htos(first_val) + "\n";
        }

        return first_int + second_int;
    }

    virtual ~SUB_Inst() = default;
};

struct JMP_Inst : public InstructionCode {
    std::string get_code(const Types first, const Types second, const std::string& first_val, const std::string& second_val) override {
        std::string code = "";

        if (first == REG) {
            code = "000000110" + register_codes.at(first_val).to_bus_ + "00\n";
        } else if (first == IMM) {
            code = "0000001100000001\n" + SCompiler::htos(first_val) + "\n";
        }

        return code;
    }

    virtual ~JMP_Inst() = default;
};

struct COMP_Inst : public InstructionCode {
    std::string get_code(const Types first, const Types second, const std::string& first_val, const std::string& second_val) override {
        std::string first_int = "";
        std::string second_int = "";

        if (second == REG) {
            std::cout << "first is reg" << std::endl;
            first_int = "000000100" + register_codes.at(second_val).to_bus_ +  "00\n";
        } else if (second == IMM) {
            first_int = "0000001000000001\n" + SCompiler::htos(second_val) + "\n";
        }

        if (first == REG) {
            second_int = "001100000" + register_codes.at(first_val).to_bus_ + "00\n";
        } else if (first == IMM) {
            second_int = "0011000000000001\n" + SCompiler::htos(first_val) + "\n";
        }

        return first_int + second_int;
    }

    virtual ~COMP_Inst() = default;
};

struct JE_Inst : public InstructionCode {
    std::string get_code(const Types first, const Types second, const std::string& first_val, const std::string& second_val) override {
        std::string code = "";

        if (first == REG) {
            code = "000011111" + register_codes.at(first_val).to_bus_ + "00\n";
        } else if (first == IMM) {
            code = "0000111110000001\n" + SCompiler::htos(first_val) + "\n";
        }

        return code;
    }

    virtual ~JE_Inst() = default;
};

struct JNE_Inst : public InstructionCode {
    std::string get_code(const Types first, const Types second, const std::string& first_val, const std::string& second_val) override {
        std::string code = "";

        if (first == REG) {
            code = "000011110" + register_codes.at(first_val).to_bus_ + "00\n";
        } else if (first == IMM) {
            code = "0000111100000001\n" + SCompiler::htos(first_val) + "\n";
        }

        return code;
    }

    virtual ~JNE_Inst() = default;
};