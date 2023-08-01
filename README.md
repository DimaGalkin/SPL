# SPL / SPLC - A Custom Built Assembly Like Language For A Custom CPU

**SPL (Simple Programming Language) is a custom programming language created for my CPU project. SC (Simple Compiler) is the complier which turns SPL into hex for the cpu**

## SPLC - Section (About the compiler)

### Building SPLC 
>The compiler should be built with the make file by running `make` in the root of the project

### Using SPLC
>Once the compiler is built it can be used by running `./splc <filename>` or `./splc -h` for additional options

### SPLC Output Formats

>For now SPLC can only output hex in the `v2.0 raw` format. However it can do so in multiple ways.
>
>To load instructions into the CPU an EEPROM is needed, as this is a language targeted to a 16 bit CPU an EEPROM with a width of 16 bits is needed to load instructions properley, however these seem to not exist so there are two work arounds: having two EEPROMS, each 8 bit or having one dual line 8 bit EEPROM.
>
> Both of these work arounds require diffrent output formats, so the compiler supports outputing:
> -  Two Hex files, one with low bytes and the other one with high bytes, each file to be loaded into a different EEPROM
> - One Hex file with the commands split into consecutive bytes so one command gets spilt over two 8 bit space (one for each line)
> - One Hex file with full 16 bit commands not spilt up, this can be usefull if runing in an emulator which has a EEPROM with a 16 bit width.
>
> <br />
>

## SPL - Section (About the language)

### Commands
**SPL is syntactically very similar to assembly language, the language supports a range of commands such as `mov`, `cmp`, `add`, `sub` and some more. Here is the full list of commands and their uses, (commands written in uppercase for clarity, in file lowercase must be used):**
> **`MOV [REG16, REG16/IMM16]`** - moves contents of second value in to the first value

> **`WR [REG16/IMM16, REG16]`** - moves the contents of a register (second) into the memory address held at the register / immediate (first)

> **`RD [REG16/IMM16, REG16]`** - reads the value at the memory address in the register or immediate (first), and writes it in to the register at (second)

> **`ADD [REG16/IMM16, REG16/IMM16]`** - adds the two register or immediate values together and stores them in the **`acc`** register

> **`SUB [REG16/IMM16, REG16/IMM16]`** - subtracts the (first) value from the second value and stores them in the **`acc`** register

> **`CMP [REG16/IMM16, REG16/IMM16]`** - compares two values, **`JE or JNE`** can be used after

> **`JMP [REG16/IMM16]`** - jumps to the memory address in the register or the immediate

> **`JE [REG16/IMM16]`** - jumps to the memory address in the register or the immediate if the previous **`CMP`** instruction was equal

> **`JNE [REG16/IMM16]`** - jumps to the memory address in the register or the immediate if the previous **`CMP`** instruction was not equal

> **`HLT`** - stops the clock, the cpu stops

### Registers

**There are 9 registers in the target CPU but not all of them are accessible, registers must also be lowercase in file.**

>### Accessible Registers
>
>>**`A`** - stores a 16 bit integer
>
>>**`B`** - stores a 16 bit integer
>
>>**`C`** - stores a 16 bit integer
>
>>**`FLGS`** - stores flags relating to calculations **`READ ONLY`**
>
>>**`LGC`** - stores flags relating to logic operations **`READ ONLY`**
>
>>**`ACC`** - stores the result of calculations **`READ ONLY`**
>  
> <br />

>### Non-accessible Registers
>
>>**`CBUS_CACHE`** - when an immediate value is loaded from memory it the instruction has to be cached from the control bus until the next clock cycle
>
>>**`NUMBR`** - one of the values sent into the ALU is from this register, values are copied into here behind the scenes
>
>>**`MAR`** - stores the memory address to jump to, write to or read from, values are copied into here for you.
>  
> <br />

### About SPL
**SPL was created as I needed a programing language to make the proccess of CPU design easier, SPL code will not run on anything other than Custom CPU it was designed for.**
