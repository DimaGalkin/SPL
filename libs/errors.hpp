#pragma once

enum errors {
    ok = 0,
    invalid_instruction = 1,
    invalid_instruction_structure = 2,
    invalid_register = 3,
    invalid_immediate = 4,
    invalid_label = 5,
    invalid_label_reference = 6,
    invalid_calculation = 7,
    integer_overflow = 8,
    incomplete_code = 9,
    integer_underflow = 10,
    file_not_open = 11,
    invalid_token = 12,
};