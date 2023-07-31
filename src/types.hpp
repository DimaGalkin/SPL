#pragma once

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
    ADD,
    SUB,
    JMP,
    CMP,
    JE,
    JNE,
    HLT
};