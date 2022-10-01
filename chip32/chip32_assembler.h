#ifndef CHIP32_ASSEMBLER_H
#define CHIP32_ASSEMBLER_H

#include "chip32.h"
#include <vector>
#include <cstdint>
#include <string>
#include <map>
#include <iostream>

struct OpCode {
    std::string mnemonic;
    uint8_t opcode;
    int nbAargs; //!< Number of arguments needed in assembly
};

// Complete tokenized instruction
struct Instr {
    uint16_t line{0};
    std::vector<std::string> args;
    std::vector<uint8_t> compiledArgs;
    OpCode code;
    uint16_t dataTypeSize{0};
    uint16_t dataLen{0};

    bool isLabel{false}; //!< If true, this is a label, otherwise it is an instruction
    bool useLabel{false}; //!< If true, the instruction uses a label
    bool isRomData{false}; //!< True is constant data in program
    bool isRamData{false}; //!< True is constant data in program

    uint16_t addr{0}; //!< instruction address when assembled in program memory
};

struct RegNames
{
    chip32_register_t reg;
    std::string name;
};

struct AssemblyResult
{
    int ramUsageSize{0};
    int romUsageSize{0};
    int constantsSize{0};

    void Print()
    {
        std::cout << "RAM usage: " << ramUsageSize << " bytes\n"
                  << "IMAGE size: " << romUsageSize << " bytes\n"
                  << "   -> ROM DATA: " << constantsSize << " bytes\n"
                  << "   -> ROM CODE: " << romUsageSize - constantsSize << "\n"
                  << std::endl;

    }
};


class Chip32Assembler
{
public:
    // Separated parser to allow only code check
    bool Parse(const std::string &data);
    // Generate the executable binary after the parse pass
    bool BuildBinary(std::vector<uint8_t> &program, AssemblyResult &result);

    void Clear() {
        m_labels.clear();
        m_instructions.clear();
    }

private:
    bool CompileMnemonicArguments(Instr &instr);

    // label, address
    std::map<std::string, uint16_t> m_labels;

    std::vector<Instr> m_instructions;
    bool CompileConstantArguments(Instr &instr);
};

#endif // CHIP32_ASSEMBLER_H
