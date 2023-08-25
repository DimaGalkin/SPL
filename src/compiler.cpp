#include "compiler.hpp"

bool SCompiler::set_output(std::string format) {
    if (format == "D8") {
        output_type_ = HALF_DUAL_WORD;
        return true;
    } else if (format == "S8") {
        output_type_ = HALF_SINGLE_WORD;
        return true;
    } else if (format == "S16") {
        output_type_ = FULL_SINGLE_WORD;
        return true;
    }
    
    return false;
}

void SCompiler::split(std::string str, std::string const delim, std::vector<std::string>& out) {
    const size_t delim_len = delim.length();
    size_t pos = 0;

    while ((pos = str.find(delim)) != std::string::npos) {
        out.push_back(str.substr(0, pos));
        str.erase(0, pos + delim_len);
    }

    if (str.size() > 0) {
        out.push_back(str);
    }
}

Errors SCompiler::load_file(const std::string file_name) {
    std::ifstream file (file_name);

    if (!file.is_open()) {
        std::cout << "File not found: " << file_name << '\n';
        return file_not_found;
    }

    std::string line;

    // remove blank lines
    while (std::getline(file, line)) {
        if (line.length() > 0 && line[0] != '\r') {
            if (!line.empty() && line[line.size() - 1] == '\r') {
                line.erase(line.size() - 1);
            }

            lines_.push_back(line);
        }
    }

    file.close();

    return OK;
}

Errors SCompiler::locate_instruction(const std::string inst_name) {
    for (const auto& [name, _] : instructions_) {
        if (name == inst_name) {
            return OK;
        }
    }

    return invalid_instruction;
}

bool SCompiler::is_valid_reg(const std::string reg) {
    for (const auto& [name, _] : registers_) {
        if (name == reg) {
            return true;
        }
    }

    return false;
}

bool SCompiler::is_valid_imm(const std::string imm) {
    if (imm[0] != '0' || imm[1] != 'x') {
        std::cout << "Immediate value must be in hex format (starting with 0x)" << '\n';
        return false;
    }

    for (const char& digit : imm.substr(2)) {
        if (!isxdigit(digit)) {
            return false;
        }
    }

    if (std::stoi(imm, 0, 16) > 0xFFFF){
        std::cout << "Immediate value out of range: " << imm << " for uint16" << '\n';

        return false;
    }

    return true;
}

TKCptr SCompiler::make_token(Instruction inst, const std::string arg, const std::string inst_name, const int token_num) {
    if (inst.child_types_.at(token_num) == REG) {
        if (is_valid_reg(arg)) {
            return std::make_shared<Register>(REG, arg);
        } else {
            std::cout << "Invalid register: " << arg << ", for instruction " << inst_name << '\n';
            return nullptr;
        }
    } else if (inst.child_types_.at(token_num) == IMM) {
        if (is_valid_imm(arg)) {
            return std::make_shared<Immediate>(IMM, arg);
        } else {
            std::cout << "Invalid immidate: " << arg << ", for instruction " << inst_name << '\n';
            return nullptr;
        }
    } else {
        bool valid_reg = is_valid_reg(arg);
        bool valid_imm = false;

        if (!valid_reg) {
            valid_imm = is_valid_imm(arg);
        }

        if ((valid_imm || valid_reg) && inst.child_types_.at(token_num) == REG_IMM) {
            if (valid_reg) {
                return std::make_shared<Register>(REG, arg);
            } else {
                return std::make_shared<Immediate>(IMM, arg);
            }
        } else {
            std::cout << "Invalid register/immidate: " << arg << ", for instruction " << inst_name << '\n';
            return nullptr;
        }
    }
}

std::string SCompiler::htos(const std::string hex_value) {
    return std::bitset<16>(std::stoi(hex_value, 0, 16)).to_string();
}

InstPtr SCompiler::get_instruction_struct(const Instructions& inst) {
    switch (inst) {
        case(MOV):
            return std::make_shared<MOV_Inst>();
        case(HLT):
            return std::make_shared<HLT_Inst>();
        case(WR):
            return std::make_shared<WR_Inst>();
        case(RD):
            return std::make_shared<RD_Inst>();
        case(ADD):
            return std::make_shared<ADD_Inst>();
        case(SUB):
            return std::make_shared<SUB_Inst>();
        case(CMP):
            return std::make_shared<COMP_Inst>();
        case(JE):
            return std::make_shared<JE_Inst>();
        case(JNE):
            return std::make_shared<JNE_Inst>();
        case(JMP):
            return std::make_shared<JMP_Inst>();
        case(EXEC):
            return std::make_shared<EXEC_Inst>();
        default:
            return nullptr;
    };
}

Errors SCompiler::parse_file() {
    for (const std::string& line : lines_) {
        std::vector<std::string> line_tokens;
        std::vector<std::string> first_part;

        split(line, ",", line_tokens);

        split(line_tokens.at(0), " ", first_part);

        std::string inst_name = first_part.at(0);

        if (first_part.size() > 1) {
            line_tokens.at(0) = first_part.at(1);
        }

        if (!inst_name.empty()) {
            std::string name;

            if (locate_instruction(inst_name) != OK) {
                std::cout << "Invalid instruction: " << inst_name << '\n';
                return invalid_instruction;
            }

            Instruction inst = instructions_.at(inst_name);

            if (inst.children_ != line_tokens.size() && !(inst.children_ == 0 && line_tokens.size() == 1)) {
                std::cout << "Invalid number of arguments for instruction: " << inst_name << '\n';
                return invalid_instruction;
            }

            TKptr inst_token = std::make_shared<Token>(inst.type_, std::vector<TKCptr>());

            if (inst.children_ == 0) {
                tokens_.push_back(inst_token);
                continue;
            }

            int token_num = 0;

            for (std::string& arg : line_tokens) {

                arg.erase(std::remove(arg.begin(), arg.end(), ' '), arg.end());

                TKCptr token = make_token(inst, arg, inst_name, token_num);

                if (token == nullptr) {
                    return invalid_instruction;
                }

                inst_token->children_.push_back(token);

                ++token_num;
            }

            tokens_.push_back(inst_token);
        }
    }

    return OK;
}

Errors SCompiler::parse_tree() {
    for (const auto& tree_parent : tokens_) {
        int child_num = 0;
        InstPtr inst = get_instruction_struct(tree_parent->type_);

        Types first;
        Types second;

        std::string first_value;
        std::string second_value;

        for (const auto& tree_child: tree_parent->children_) {
            if (tree_child->type_ == REG) {
                if (child_num == 0) {
                    first = REG;
                    first_value = tree_child->name_;
                } else {
                    second = REG;
                    second_value = tree_child->name_;
                }
            } else if (tree_child->type_ == IMM) {
                if (child_num == 0) {
                    first = IMM;
                    first_value = tree_child->value_;
                } else {
                    second = IMM;
                    second_value = tree_child->value_;
                }
            }

            ++child_num;
        }

        std::string binary_string = inst->get_code(first, second, first_value, second_value);
        
        output_ += binary_string;
    }

    return OK;
}

Errors SCompiler::write_file() {
    std::string header = "v2.0 raw\n";

    std::stringstream file_data;
    std::stringstream high_data;
    std::stringstream low_data;

    std::vector<std::string> binary_strings;
    split(output_, "\n", binary_strings);

    for(const auto& line : binary_strings) {
        std::bitset<8> high_byte {line.substr(0, 8)};
        std::bitset<8> low_byte {line.substr(8, 8)};

        if (output_type_ == HALF_SINGLE_WORD) {
            high_data << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << high_byte.to_ulong();
            high_data << " ";
            low_data << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << low_byte.to_ulong();
            low_data << " ";
        } else {
            file_data << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << high_byte.to_ulong();
            
            if (output_type_ == HALF_DUAL_WORD) {
                file_data << " ";
            }

            file_data << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << low_byte.to_ulong();
            file_data << " ";
        }
    }

    if (output_type_ == HALF_SINGLE_WORD) {
        std::ofstream high {"high.hex"};
        std::ofstream low {"low.hex"};

        if (!high.is_open() || !low.is_open()) {
            std::cout << "Failed to open output files" << '\n';
            return file_write_error;
        }

        high << header;
        high << high_data.str();
        high.close();

        low << header;
        low << low_data.str();
        low.close();
    } else {
        std::ofstream file {"out.hex"};

        if (!file.is_open()) {
            std::cout << "Failed to open output file" << '\n';
            return file_write_error;
        }

        file << header;
        file << file_data.str();
        file.close();
    }

    return OK;
}

int SCompiler::compile(const std::string filename) {
    if (load_file(filename) != OK) {
        return 1;
    }

    if (parse_file() != OK) {
        return 1;
    }

    if (parse_tree() != OK) {
        return 1;
    }

    if (write_file() != OK) {
        return 1;
    }

    return 0;
}