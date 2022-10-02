/*
The MIT License

Copyright (c) 2022 Anthony Rabine
Copyright (c) 2018 Mario Falcao

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef CHIP32_H
#define CHIP32_H

#include <stdint.h>

typedef enum
{
    // system:
    OP_NOP,  // do nothing
    OP_HALT, // halt execution
    OP_SYSCALL,  // system call handled by user-registered function, 4 arguments (R0 - R3) passed by value
    // constants:
    OP_LCONS,  // store a value in a register, e.g.: lcons r0, 0xA2 0x00 0x00 0x00

    // register operations:
    OP_MOV, // copy a value between registers, e.g.: mov r0, r2
    // stack:
    OP_PUSH, // push a register onto the stack, e.g.: push r0
    OP_POP,  // pop the first element of the stack to a register, e.g.: pop r0

    // functions
    OP_CALL, // set register RA to the next instruction and jump to subroutine, e.g.: call 0x10 0x00
    OP_RET,  // return to the address of last callee (RA), e.g.: ret

    // memory:
    OP_STORE,   // copy a value from a register to a heap address, e.g.: store 0x08 0x00, r0
    OP_LOAD,   // copy a value from a heap address to a register, e.g.: load r0, 0x08 0x00

    // arithmetic:
    OP_ADD,  // sum and store in first reg, e.g.: add r0, r2
    OP_SUB,  // subtract and store in first reg, e.g.: sub r0, r2
    OP_MUL,  // multiply and store in first reg, e.g.: mul r0, r2
    OP_DIV,  // divide and store in first reg, remain in second, e.g.: div r0, r2

    OP_SHL,  // logical shift left, e.g.: shl r0, r1
    OP_SHR,  // logical shift right, e.g.: shr r0, r1
    OP_ISHR, // arithmetic shift right (for signed values), e.g.: ishr r0, r1

    OP_AND,  // and two registers and store result in the first one, e.g.: and r0, r1
    OP_OR,   // or two registers and store result in the first one, e.g.: or r0, r1
    OP_XOR,  // xor two registers and store result in the first one, e.g.: xor r0, r1
    OP_NOT,  // not a register and store result, e.g.: not r0

    // branching:
    OP_JMP, // jump to address, e.g.: jmp 0x0A 0x00
    OP_JR,  // jump to address in register, e.g.: jr r1
    OP_SKIPZ,  // skip next instruction if zero, e.g.: skipz r0
    OP_SKIPNZ, // skip next instruction if not zero, e.g.: skipnz r2

    INSTRUCTION_COUNT
} chip32_instruction_t;


/*

| name  | number | type                             | preserved |
|-------|--------|----------------------------------|-----------|
| r0-r5 | 0-5    | general-purpose                  | Y         |
| t0-t9 | 6-15   | general-purpose (temporary/args) | N         |
| ip    | 16     | instruction pointer              | Y         |
| bp    | 17     | base pointer                     | Y         |
| sp    | 18     | stack pointer                    | Y         |
| ra    | 19     | return address                   | N         |
| ov    | 20     | Overflow register                | Y         |

*/
typedef enum
{
    // preserved across a call
    R0,
    R1,
    R2,
    R3,
    R4,
    R5,
    // temporaries - not preserved
    T0,
    T1,
    T2,
    T3,
    T4,
    T5,
    T6,
    T7,
    T8,
    T9,
    // special
    IP,
    BP,
    SP,
    RA,
    OV,
    // count
    REGISTER_COUNT
} chip32_register_t;


typedef enum
{
    VM_FINISHED,                // execution completed (i.e. got halt instruction)
    VM_PAUSED,                  // execution paused since we hit the maximum instructions
    VM_ERR_UNKNOWN_OPCODE,      // unknown opcode
    VM_ERR_UNSUPPORTED_OPCODE,  // instruction not supported on this platform
    VM_ERR_INVALID_REGISTER,    // invalid register access
    VM_ERR_UNHANDLED_INTERRUPT, // interrupt triggered without registered handler
    VM_ERR_STACK_OVERFLOW,      // stack overflow
    VM_ERR_STACK_UNDERFLOW,     // stack underflow
    VM_ERR_INVALID_ADDRESS,     // tried to access an invalid memory address
} chip32_result_t;

typedef struct {
    uint8_t opcode;
    uint8_t nbAargs; //!< Number of arguments needed in assembly
    uint8_t bytes; //!< Size of bytes arguments
} OpCode;

#define OPCODES_LIST { { OP_NOP, 0, 0 }, { OP_HALT, 0, 0 }, { OP_SYSCALL, 1, 1 }, { OP_LCONS, 2, 5 }, \
{ OP_MOV, 2, 2 }, { OP_PUSH, 1, 1 }, {OP_POP, 1, 1 }, { OP_CALL, 1, 2 }, { OP_RET, 0, 0 }, \
{ OP_STORE, 2, 3 }, { OP_LOAD, 2, 3 }, { OP_ADD, 2, 2 }, { OP_SUB, 2, 2 }, { OP_MUL, 2, 2 }, \
{ OP_DIV, 2, 2 }, { OP_SHL, 2, 2 }, { OP_SHR, 2, 2 }, { OP_ISHR, 2, 2 }, { OP_AND, 2, 2 }, \
{ OP_OR, 2, 2 }, { OP_XOR, 2, 2 }, { OP_NOT, 1 }, { OP_JMP, 1 }, { OP_JR, 1 }, \
{ OP_SKIPZ, 1 }, { OP_SKIPNZ, 1 } }

/**
  Whole memory is 64KB

  The RAM and ROM segments can be placed anywhere:
   - they must not overlap
   - there can have gaps between segments (R/W to a dummy byte if accessed)

 -----------  0xFFFF
 |         |
 |         |
 |---------|<-- stack start
 |         |
 |   RAM   |
 |_________|
 |         |
 |_________|
 |  ROM    |
 |_________|
 |         |
 -----------  0x0000
 */
typedef struct
{
    uint8_t *mem; //!< Pointer to a real memory location (ROM or RAM)
    uint16_t size; //!< Size of the real memory
    uint16_t addr; //!< Start address of the virtual memory

} virtual_mem_t;

// =======================================================================================
// VM RUN
// =======================================================================================
void chip32_initialize(virtual_mem_t *rom, virtual_mem_t *ram, uint16_t stack_size);
chip32_result_t chip32_run(uint16_t prog_size, uint32_t max_instr);

// =======================================================================================
// VM ACCESS
// =======================================================================================
uint32_t chip32_get_register(chip32_register_t reg);
void chip32_set_register(chip32_register_t reg, uint32_t val);

#endif // CHIP32_H
