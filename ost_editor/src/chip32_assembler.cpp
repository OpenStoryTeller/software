#include "chip32_assembler.h"

#include <sstream>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstdint>

// =============================================================================
// GLOBAL UTILITY FUNCTIONS
// =============================================================================
static const char* ws = " \t\n\r\f\v";

// trim from end of string (right)
static inline std::string& rtrim(std::string& s, const char* t = ws)
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from beginning of string (left)
static inline std::string& ltrim(std::string& s, const char* t = ws)
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from both ends of string (right then left)
static inline std::string& trim(std::string& s, const char* t = ws)
{
    return ltrim(rtrim(s, t), t);
}

static std::vector<std::string> Split(const std::string &theString)
{
    std::vector<std::string> result;
    std::istringstream iss(theString);
    for(std::string s; iss >> s; )
        result.push_back(s);
    return result;
}

static std::string ToLower(const std::string &text)
{
    std::string newText = text;
    std::transform(newText.begin(), newText.end(), newText.begin(), [](unsigned char c){ return std::tolower(c); });
    return newText;
}

static const RegNames AllRegs[] = { { R0, "r0" }, { R1, "r1" }, { R2, "r2" }, { R3, "r3" }, { R4, "r4" }, { R5, "r5" },
    { T0, "t0" }, { T1, "t1" }, { T2, "t2" }, { T3, "t3" }, { T4, "t4" }, { T5, "t5" }, { T6, "t6" }, { T7, "t7" },
    { T8, "t8" }, { T9, "t9" }, { IP, "ip" }, { BP, "bp" }, { SP, "sp" }, { RA, "ra" }
};

static const uint32_t NbRegs = sizeof(AllRegs) / sizeof(AllRegs[0]);

static bool GetRegister(const std::string &regName, uint8_t &reg)
{
    std::string lowReg = ToLower(regName);
    for (uint32_t i = 0; i < NbRegs; i++)
    {
        if (lowReg == AllRegs[i].name)
        {
            reg = AllRegs[i].reg;
            return true;
        }
    }
    return false;
}

static OpCode OpCodes[] = { { "nop", OP_NOP, 0 }, { "halt", OP_HALT, 0 }, { "syscall", OP_SYSCALL, 1 }, { "lcons", OP_LCONS, 2 },
    { "mov", OP_MOV, 2 }, { "push", OP_PUSH, 1 }, { "pop", OP_POP, 1 }, { "call", OP_CALL, 1 }, { "ret", OP_RET, 0 },
    { "store", OP_STORE, 2 }, { "load", OP_LOAD, 2 }, { "add", OP_ADD, 2 }, { "sub", OP_SUB, 2 }, { "mul", OP_MUL, 2 },
    { "div", OP_DIV, 2 }, { "shiftl", OP_SHL, 2 }, { "shiftr", OP_SHR, 2 }, { "ishiftr", OP_ISHR, 2 }, { "and", OP_AND, 2 },
    { "or", OP_OR, 2 }, { "xor", OP_XOR, 2 }, { "not", OP_NOT, 1 }, { "jump", OP_JMP, 1 }, { "jumpr", OP_JR, 1 },
    { "skipz", OP_SKIPZ, 1 }, { "skipnz", OP_SKIPNZ, 1 }
};

static const uint32_t nbOpCodes = sizeof(OpCodes) / sizeof(OpCodes[0]);

static bool IsOpCode(const std::string &label, OpCode &op)
{
    bool success = false;
    std::string lowLabel = ToLower(label);

    for (uint32_t i = 0; i < nbOpCodes; i++)
    {
        if (OpCodes[i].mnemonic == lowLabel)
        {
            success = true;
            op = OpCodes[i];
            break;
        }
    }
    return success;
}

static void GetArgs(Instr &instr, const std::string &data)
{
    std::string value;
    std::istringstream iss(data);
    while (getline(iss, value, ','))
    {
        instr.args.push_back(trim(value));
    }
}

static inline void leu32_put(std::vector<std::uint8_t> &container, uint32_t data)
{
    container.push_back(data & 0xFFU);
    container.push_back((data >> 8U) & 0xFFU);
    container.push_back((data >> 16U) & 0xFFU);
    container.push_back((data >> 24U) & 0xFFU);
}

static inline void leu16_put(std::vector<std::uint8_t> &container, uint16_t data)
{
    container.push_back(data & 0xFFU);
    container.push_back((data >> 8U) & 0xFFU);
}

#define GET_REG(name, ra) if (!GetRegister(name, ra)) {\
    std::cout << "ERROR! Bad register name: " << name << std::endl;\
    return false; }

#define CHIP32_CHECK(instr, cond, error) if (!(cond)) { \
    std::cout << "error: " << instr.line << ": " << error << std::endl; \
    return false; } \

// =============================================================================
// ASSEMBLER CLASS
// =============================================================================
bool Chip32Assembler::CompileMnemonicArguments(Instr &instr)
{
    uint8_t ra, rb;

    switch(instr.code.opcode)
    {
    case OP_NOP:
    case OP_HALT:
    case OP_RET:
        // no arguments, just use the opcode
        break;
    case OP_SYSCALL:
        instr.compiledArgs.push_back(static_cast<uint8_t>(strtol(instr.args[0].c_str(),  NULL, 0)));
        break;
    case OP_LCONS:
        GET_REG(instr.args[0], ra);
        instr.compiledArgs.push_back(ra);
        leu32_put(instr.compiledArgs, static_cast<uint32_t>(strtol(instr.args[1].c_str(),  NULL, 0)));
        break;
    case OP_POP:
    case OP_PUSH:
        GET_REG(instr.args[0], ra);
        instr.compiledArgs.push_back(ra);
        break;
    case OP_MOV:
        GET_REG(instr.args[0], ra);
        GET_REG(instr.args[1], rb);
        instr.compiledArgs.push_back(ra);
        instr.compiledArgs.push_back(rb);
        break;
    case OP_JMP:
    case OP_CALL:
        // Reserve 2 bytes for address, it will be filled at the end
        instr.useLabel = true;
        instr.compiledArgs.push_back(0);
        instr.compiledArgs.push_back(0);
        break;
    case OP_STORE:
        leu16_put(instr.compiledArgs, static_cast<uint16_t>(strtol(instr.args[0].c_str(),  NULL, 0)));
        GET_REG(instr.args[1], ra);
        instr.compiledArgs.push_back(ra);
        break;
    case OP_LOAD:
        GET_REG(instr.args[0], ra);
        instr.compiledArgs.push_back(ra);
        leu16_put(instr.compiledArgs, static_cast<uint16_t>(strtol(instr.args[1].c_str(),  NULL, 0)));
        break;
    default:
        CHIP32_CHECK(instr, false, "Unsupported mnemonic: " << instr.code.mnemonic);
        break;
    }
    return true;
}

bool Chip32Assembler::CompileConstantArguments(Instr &instr)
{
    for (auto &a : instr.args)
    {
        // Check string
        if (a.size() > 2)
        {
            // Detected string
            if ((a[0] == '"') && (a[a.size() - 1] == '"'))
            {
                for (int i = 1; i < (a.size() - 1); i++)
                {
                    instr.compiledArgs.push_back(a[i]);
                }
                instr.compiledArgs.push_back(0);
                continue;
            }
        }

        // here, we check if the intergers are correct
        uint32_t intVal = static_cast<uint32_t>(strtol(a.c_str(),  NULL, 0));

        bool sizeOk = false;
        if (((intVal <= UINT8_MAX) && (instr.dataTypeSize == 8)) ||
            ((intVal <= UINT16_MAX) && (instr.dataTypeSize == 16)) ||
            ((intVal <= UINT32_MAX) && (instr.dataTypeSize == 32))) {
            sizeOk = true;
        }
        CHIP32_CHECK(instr, sizeOk, "integer too high: " << intVal);
        if (instr.dataTypeSize == 8) {
            instr.compiledArgs.push_back(intVal);
        } else if (instr.dataTypeSize == 16) {
            leu16_put(instr.compiledArgs, intVal);
        } else {
            leu32_put(instr.compiledArgs, intVal);
        }
    }

    return true;
}

bool Chip32Assembler::BuildBinary(std::vector<uint8_t> &program, AssemblyResult &result)
{
    result = { 0, 0, 0}; // clear stuff!

    // 1. First pass: serialize each instruction and arguments to program memory, assign address to variables (rom or ram)
    for (auto &i : m_instructions)
    {
        if (! (i.isRamData || i.isRomData)) program.push_back(i.code.opcode);

        if (i.isLabel || i.isRamData)
        {
             m_labels[i.code.mnemonic] = program.size();
             if (i.isRamData) result.ramUsageSize += i.dataLen * i.dataTypeSize/8;
        }
        else
        {
            if (i.isRomData) result.constantsSize += i.compiledArgs.size();
            std::copy (i.compiledArgs.begin(), i.compiledArgs.end(), std::back_inserter(program));
        }
    }

    // 2. Second pass: replace all label or RAM data by the real address in memory
    for (auto &i : m_instructions)
    {
        if (i.useLabel && (i.args.size() > 0))
        {
            // label is always the first argument
            std::string label = i.args[0];
            if (m_labels.count(label))
            {
                uint16_t addr = m_labels[label];
                uint16_t argsIndex = i.addr + 1;

                program[argsIndex] = addr & 0xFF;
                program[argsIndex+1] = (addr >> 8U) & 0xFF;
            }
            else
            {
                std::cout << "Error, label not found: " << label << std::endl;
            }
        }
    }
    result.romUsageSize = program.size();
    return true;
}

bool Chip32Assembler::Parse(const std::string &data)
{
    std::stringstream data_stream(data);
    std::string line;

    Clear();

    int lineNum = 0;
    while(std::getline(data_stream, line))
    {
        lineNum++;
        Instr instr;
        instr.line = lineNum;

        line = trim(line);

        int pos = line.find_first_of(";");
        if (pos != std::string::npos) {
            line.erase(pos);
            std::cout << "Removed comment at line: " << lineNum << std::endl;
        }

        if (line.length() <= 0) continue;

        // Split the line
        std::vector<std::string> lineParts = Split(line);

        CHIP32_CHECK(instr, (lineParts.size() > 0), " not a valid line");

        // Ok until now
        std::string opcode = lineParts[0];

        // =======================================================================================
        // LABEL
        // =======================================================================================
        if (opcode[0] == '.')
        {
            CHIP32_CHECK(instr, (opcode[opcode.length() - 1] == ':') && (lineParts.size() == 1), "label must end with ':'");
            // Label
            std::cout << "Detected label line: " << lineNum << std::endl;
            opcode.pop_back(); // remove the colon character
            instr.code.mnemonic = opcode;
            instr.isLabel = true;
            CHIP32_CHECK(instr, m_labels.count(opcode) == 0, "duplicated label : " << opcode);
            m_labels[opcode] = 0; // will be filled during the build binary phase
            m_instructions.push_back(instr);
        }

        // =======================================================================================
        // INSTRUCTIONS
        // =======================================================================================
        else if (IsOpCode(opcode, instr.code))
        {
            std::cout << "Found potential opcode line: " << lineNum << std::endl;

            bool nbArgsSuccess = false;
            // Test nedded arguments
            if ((instr.code.nbAargs == 0) && (lineParts.size() == 1))
            {
                nbArgsSuccess = true; // no arguments, solo mnemonic
            }
            else if ((instr.code.nbAargs > 0) && (lineParts.size() >= 2))
            {
                // Compute arguments
                for (int i = 1; i < lineParts.size(); i++)
                {
                    GetArgs(instr, lineParts[i]);
                }

                CHIP32_CHECK(instr, instr.args.size() == instr.code.nbAargs,
                             "Bad number of parameters. Required: " << instr.code.nbAargs << ", got: " << instr.args.size());
                nbArgsSuccess = true;
            }
            else
            {
                CHIP32_CHECK(instr, false, "Bad number of parameters");
            }

            if (nbArgsSuccess)
            {
                CompileMnemonicArguments(instr);
                m_instructions.push_back(instr);
            }
        }
        // =======================================================================================
        // CONSTANTS IN ROM OR RAM (eg: $yourLabel  DC8 "a string", 5, 4, 8  (DV32 for RAM
        // =======================================================================================
        else if (opcode[0] == '$')
        {
            std::cout << "Found Data line: " << lineNum << std::endl;

            instr.code.mnemonic = opcode;
            CHIP32_CHECK(instr, (lineParts.size() >= 3), "bad number of parameters");

            std::string type = lineParts[1];

            CHIP32_CHECK(instr, (type.size() >= 3), "bad data type size");
            CHIP32_CHECK(instr, (type[0] == 'D') && ((type[1] == 'C') || (type[1] == 'V')), "bad data type (must be DCxx or DVxx");
            CHIP32_CHECK(instr, m_labels.count(opcode) == 0, "duplicated label : " << opcode);
            m_labels[opcode] = 0; // will be filled during the build binary phase

            instr.isRomData = type[1] == 'C' ? true : false;
            instr.isRamData = type[1] == 'V' ? true : false;
            type.erase(0, 2);
            instr.dataTypeSize = static_cast<uint32_t>(strtol(type.c_str(),  NULL, 0));

            if (instr.isRomData)
            {
                for (int i = 2; i < lineParts.size(); i++)
                {
                    GetArgs(instr, lineParts[i]);
                }
                CHIP32_CHECK(instr, CompileConstantArguments(instr), "Compile error, stopping.");
            }
            else
            {
                instr.dataLen = static_cast<uint16_t>(strtol(lineParts[2].c_str(),  NULL, 0));
            }
            m_instructions.push_back(instr);
        }
    }
    return true;
}
