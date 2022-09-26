#ifndef CHIP32_ASSEMBLER_H
#define CHIP32_ASSEMBLER_H

#include "chip32.h"
#include <vector>
#include <cstdint>
#include <string>
#include <map>

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

    bool isLabel{false}; //<! If true, this is a label, otherwise it is an instruction
    bool useLabel{false}; //<! If true, the instruction uses a label

    uint16_t addr{0}; //<! instruction address when assembled in program memory

    void Copy(std::vector<uint8_t> &mem)
    {
        addr = mem.size();
        mem.push_back(code.opcode);
        for (auto a : compiledArgs)
        {
            mem.push_back(a);
        }
    }
};

struct RegNames
{
    chip32_register_t reg;
    std::string name;
};


class Chip32Assembler
{
public:
    Chip32Assembler();

    void BuildBinary(std::vector<uint8_t> &program);
    void Parse(const std::string &data);

private:
    bool CompileArguments(Instr &instr);

    // label, address
    std::map<std::string, uint16_t> m_labels;

    std::vector<Instr> m_instructions;
};

#endif // CHIP32_ASSEMBLER_H
