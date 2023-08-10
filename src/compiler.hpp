#include <iostream>

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <fstream>
#include <sstream>
#include <bitset>

#include "types.hpp"
#include "structs.hpp"

struct RegisterCode {
    std::string name;
    std::string from_bus;
    std::string to_bus;

    RegisterCode(std::string name, std::string from_bus, std::string to_bus) {
        this->name = name;
        this->from_bus = from_bus;
        this->to_bus = to_bus;
    }
};

struct Instruction {
    std::string name;

    int children;
    std::vector<Types> child_types;
    Instructions type;

    Instruction(std::string name, int children, std::vector<Types> child_types, Instructions type) {
        this->name = name;
        this->children = children;
        this->child_types = child_types;
        this->type = type;
    }

    virtual ~Instruction() = default;
};

const std::map<std::string, RegisterCode> register_codes = {
    {"a", RegisterCode("a", "00001", "00001")},
    {"b", RegisterCode("b", "00011", "00010")},
    {"c", RegisterCode("c", "01000", "00111")},
    {"acc", RegisterCode("acc", "N_ALWD", "00100")},
    {"flgs", RegisterCode("flgs", "N_ALWD", "00101")},
    {"lgc", RegisterCode("lgc", "N_ALWD", "01000")},
};

class SCompiler {
    private:
        const std::map<std::string, Instruction> instructions = {
            {"mov", Instruction("mov", 2, {REG, REG_IMM}, MOV)},
            {"wr", Instruction("wr", 2, {REG_IMM, REG}, WR)},
            {"rd", Instruction("wr", 2, {REG_IMM, REG}, RD)},
            {"add", Instruction("add", 2, {REG_IMM, REG_IMM}, ADD)},
            {"sub", Instruction("sub", 2, {REG_IMM, REG_IMM}, SUB)},
            {"cmp", Instruction("cmp", 2, {REG_IMM, REG_IMM}, CMP)},
            {"jmp", Instruction("jmp", 1, {REG_IMM}, JMP)},
            {"je", Instruction("je", 1, {REG_IMM}, JE)},
            {"jne", Instruction("jne", 1, {REG_IMM}, JNE)},
            {"hlt", Instruction("hlt", 0, {}, HLT)}
        };

        const std::map<std::string, Register> registers = {
            {"a", Register(REG, "a")},
            {"b", Register(REG, "b")},
            {"c", Register(REG, "c")},
            {"acc", Register(REG, "acc")},
            {"flgs", Register(REG, "flgs")},
            {"lgc", Register(REG, "lgc")},
        };

        std::vector<std::string> lines;
        std::vector<TKptr> tokens;
        std::string output;
        OutputTypes output_type = HALF_DUAL_WORD;

    public:

        SCompiler() = default;

        Errors load_file(std::string const file_name);
        Errors parse_file();
        Errors parse_tree();
        Errors write_file();
        Errors locate_instruction(std::string const inst_name);

        bool set_output(std::string const format);
        InstPtr get_instruction_struct(Instructions const inst);
        void split (std::string str, std::string const delim, std::vector<std::string>& out);
        int compile(std::string const file_name);
        bool is_valid_reg(std::string const reg);
        bool is_valid_imm(std::string const imm);
        static std::string htos(std::string const hex_value);
        TKCptr make_token(Instruction inst, std::string const reg, std::string const imm, int const token_num);

        ~SCompiler() = default;
};

struct MOV_Inst : public InstructionCode {
public:
    std::string get_code(const Types first, const Types second, const std::string first_val, const std::string second_val) override {
        if (first == REG && second == REG) {
            return "0000" + register_codes.at(first_val).from_bus + register_codes.at(second_val).to_bus + "00\n";
        } else if (first == REG && second == IMM) {
            return "0000" + register_codes.at(first_val).from_bus + "0000001\n" + SCompiler::htos(second_val) + "\n";
        }

        return "error";
    }

    virtual ~MOV_Inst() = default;
};

struct HLT_Inst : public InstructionCode {
public:
    std::string get_code(const Types first, const Types second, const std::string first_val, const std::string second_val) override {
        return SCompiler::htos("0x7C") + "\n";
    }

    virtual ~HLT_Inst() = default;
};

struct WR_Inst : public InstructionCode {
public:
    std::string get_code(const Types first, const Types second, const std::string first_val, const std::string second_val) override {
        // load address into mar (from imm or from reg)
        std::string mar_code = "";
        std::string wr_code = "";

        if (first == REG) {
            mar_code = "000000111" + register_codes.at(first_val).to_bus + "00\n";
        } else if (first == IMM) {
            mar_code = "0000001110000001\n" + SCompiler::htos(first_val) + "\n";
        }

        wr_code = "100000010" + register_codes.at(second_val).to_bus + "11\n";
        
        return mar_code + wr_code;
    }

    virtual ~WR_Inst() = default;
};

struct RD_Inst : public InstructionCode {
public:
    std::string get_code(const Types first, const Types second, const std::string first_val, const std::string second_val) override {
        // load address into mar (from imm or from reg)
        std::string mar_code = "";
        std::string wr_code = "";

        if (first == REG) {
            mar_code = "000000111" + register_codes.at(first_val).to_bus + "00\n";
        } else if (first == IMM) {
            mar_code = "0000001110000001\n" + SCompiler::htos(first_val) + "\n";
        }

        wr_code = "1000" + register_codes.at(second_val).to_bus + "00000" + "01\n";
        
        return mar_code + wr_code;
    }

    virtual ~RD_Inst() = default;
};

struct ADD_Inst : public InstructionCode {
public:
    std::string get_code(const Types first, const Types second, const std::string first_val, const std::string second_val) override {
        std::string first_int = "";
        std::string second_int = "";

        if (first == REG) {
            first_int = "000000100" + register_codes.at(first_val).to_bus +  "00\n";
        } else if (first == IMM) {
            first_int = "0000001000000001\n" + SCompiler::htos(first_val) + "\n";
        }

        if (second == REG) {
            second_int = "000100000" + register_codes.at(second_val).to_bus + "00\n";
        } else if (second == IMM) {
            second_int = "0001000000000001\n" + SCompiler::htos(second_val) + "\n";
        }

        return first_int + second_int;
    }

    virtual ~ADD_Inst() = default;
};

struct SUB_Inst : public InstructionCode {
public:
    std::string get_code(const Types first, const Types second, const std::string first_val, const std::string second_val) override {
        std::string first_int = "";
        std::string second_int = "";

        if (first == REG) {
            first_int = "000000100" + register_codes.at(second_val).to_bus +  "00\n";
        } else if (first == IMM) {
            first_int = "0000001000000001\n" + SCompiler::htos(second_val) + "\n";
        }

        if (second == REG) {
            second_int = "001000000" + register_codes.at(first_val).to_bus + "00";
        } else if (second == IMM) {
            second_int = "0010000000000001\n" + SCompiler::htos(first_val) + "\n";
        }

        return first_int + second_int;
    }

    virtual ~SUB_Inst() = default;
};

struct JMP_Inst : public InstructionCode {
public:
    std::string get_code(const Types first, const Types second, const std::string first_val, const std::string second_val) override {
        std::string code = "";

        if (first == REG) {
            code = "000000110" + register_codes.at(first_val).to_bus + "00\n";
        } else if (first == IMM) {
            code = "0000001100000001\n" + SCompiler::htos(first_val) + "\n";
        }

        return code;
    }

    virtual ~JMP_Inst() = default;
};

struct COMP_Inst : public InstructionCode {
public:
    std::string get_code(const Types first, const Types second, const std::string first_val, const std::string second_val) override {
        std::string first_int = "";
        std::string second_int = "";

        if (second == REG) {
            std::cout << "first is reg" << std::endl;
            first_int = "000000100" + register_codes.at(second_val).to_bus +  "00\n";
        } else if (second == IMM) {
            first_int = "0000001000000001\n" + SCompiler::htos(second_val) + "\n";
        }

        if (first == REG) {
            second_int = "001100000" + register_codes.at(first_val).to_bus + "00\n";
        } else if (first == IMM) {
            second_int = "0011000000000001\n" + SCompiler::htos(first_val) + "\n";
        }

        return first_int + second_int;
    }

    virtual ~COMP_Inst() = default;
};

struct JE_Inst : public InstructionCode {
public:
    std::string get_code(const Types first, const Types second, const std::string first_val, const std::string second_val) override {
        std::string code = "";

        if (first == REG) {
            code = "000011111" + register_codes.at(first_val).to_bus + "00\n";
        } else if (first == IMM) {
            code = "0000111110000001\n" + SCompiler::htos(first_val) + "\n";
        }

        return code;
    }

    virtual ~JE_Inst() = default;
};

struct JNE_Inst : public InstructionCode {
public:
    std::string get_code(const Types first, const Types second, const std::string first_val, const std::string second_val) override {
        std::string code = "";

        if (first == REG) {
            code = "000011110" + register_codes.at(first_val).to_bus + "00\n";
        } else if (first == IMM) {
            code = "0000111100000001\n" + SCompiler::htos(first_val) + "\n";
        }

        return code;
    }

    virtual ~JNE_Inst() = default;
};

struct AND_Inst : public InstructionCode {
public:
    std::string get_code(const Types first, const Types second, const std::string first_val, const std::string second_val) override {
        std::string code = "";

        if (first == REG) {
            code = "000011110" + register_codes.at(first_val).to_bus + "00\n";
        } else if (first == IMM) {
            code = "0000111100000001\n" + SCompiler::htos(first_val) + "\n";
        }

        return code;
    }

    virtual ~AND_Inst() = default;
};
