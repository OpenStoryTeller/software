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

#include "chip32.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define _NEXT_BYTE g_memory[++g_registers[IP]]
#define _NEXT_SHORT ({ g_registers[IP] += 2; g_memory[g_registers[IP]-1]\
                     | g_memory[g_registers[IP]] << 8; })
#define _NEXT_INT ({                                                                               \
    g_registers[IP] += 4;                                                                     \
    g_memory[g_registers[IP] - 3] | g_memory[g_registers[IP] - 2] << 8 |       \
        g_memory[g_registers[IP] - 1] << 16 | g_memory[g_registers[IP]] << 24; \
})

#ifndef VM_DISABLE_CHECKS
#define _CHECK_ADDR_VALID(a) \
    if (a >= g_memory_size) \
        return VM_ERR_INVALID_ADDRESS;
#define _CHECK_BYTES_AVAIL(n) \
    _CHECK_ADDR_VALID(g_registers[IP] + n)
#define _CHECK_REGISTER_VALID(r) \
    if (r >= REGISTER_COUNT)     \
        return VM_ERR_INVALID_REGISTER;
#define _CHECK_CAN_PUSH(n)                                              \
    if (g_registers[SP] - (n * sizeof(uint32_t)) < prog_size) \
        return VM_ERR_STACK_OVERFLOW;
#define _CHECK_CAN_POP(n)                                               \
    if (g_registers[SP] + (n * sizeof(uint32_t)) > g_memory_size) \
        return VM_ERR_STACK_UNDERFLOW;                      \
    if (g_registers[SP] < prog_size)                          \
        return VM_ERR_STACK_OVERFLOW;
#else
#define _CHECK_ADDR_VALID(a)
#define _CHECK_BYTES_AVAIL(n)
#define _CHECK_REGISTER_VALID(r)
#define _CHECK_CAN_PUSH(n)
#define _CHECK_CAN_POP(n)
#endif

static uint8_t *g_memory = NULL;
static uint32_t g_registers[REGISTER_COUNT] = {0};
static uint16_t g_memory_size;
static uint16_t g_stack_size;


bool (*g_interrupt_callback)(uint8_t) = nullptr;


void chip32_initialize(uint8_t *memory, uint32_t mem_size, uint32_t stack_size)
{
    g_memory = memory;
    g_memory_size = mem_size;
    g_stack_size = stack_size;

    memset(g_registers, 0, REGISTER_COUNT * sizeof(uint32_t));
    g_registers[SP] = mem_size;
}

static void chip32_on_interrupt(bool (*callback)(uint8_t))
{
    g_interrupt_callback = callback;
}

uint32_t chip32_stack_count()
{
    return g_memory_size - g_registers[SP];
}

void chip32_stack_push(uint32_t value)
{
    g_registers[SP] -= 4;
    memcpy(&g_memory[g_registers[SP]], &value, sizeof(uint32_t));
}

uint32_t chip32_stack_pop()
{
    uint32_t val = 0;
    memcpy(&val, &g_memory[g_registers[SP]], sizeof(uint32_t));
    g_registers[SP] += 4;
    return val;
}

uint8_t *chip32_memory(uint16_t addr)
{
    return &g_memory[addr];
}

uint32_t chip32_get_register(chip32_register_t reg)
{
    return g_registers[reg];
}

void chip32_set_register(chip32_register_t reg, uint32_t val)
{
    g_registers[reg] = val;
}

chip32_result_t chip32_run(uint16_t prog_size, uint32_t max_instr)
{
    uint32_t instrCount = 0;

    while ((max_instr == 0) || (instrCount < max_instr))
    {
        _CHECK_ADDR_VALID(g_registers[IP])
        const uint8_t instr = g_memory[g_registers[IP]];
        if (instr >= INSTRUCTION_COUNT)
            return VM_ERR_UNKNOWN_OPCODE;

        switch (instr)
        {
        case OP_NOP:
        {
            break;
        }
        case OP_HALT:
        {
            return VM_FINISHED;
        }
        case OP_SYSCALL:
        {
            _CHECK_BYTES_AVAIL(1)
            const uint8_t code = _NEXT_BYTE;

            if (g_interrupt_callback == nullptr)
                return VM_ERR_UNHANDLED_INTERRUPT;
            if (!g_interrupt_callback(code))
                return VM_FINISHED;
            break;
        }
        case OP_MOV:
        {
            _CHECK_BYTES_AVAIL(2)
            const uint8_t reg1 = _NEXT_BYTE;
            const uint8_t reg2 = _NEXT_BYTE;
            _CHECK_REGISTER_VALID(reg1)
            _CHECK_REGISTER_VALID(reg2)
            g_registers[reg1] = g_registers[reg2];
            break;
        }
        case OP_LCONS:
        {
            _CHECK_BYTES_AVAIL(5)
            const uint8_t reg = _NEXT_BYTE;
            _CHECK_REGISTER_VALID(reg)
            g_registers[reg] = _NEXT_INT;
            break;
        }
        case OP_LCONSW:
        {
            _CHECK_BYTES_AVAIL(3)
            const uint8_t reg = _NEXT_BYTE;
            _CHECK_REGISTER_VALID(reg)
            g_registers[reg] = _NEXT_SHORT;
            break;
        }
        case OP_LCONSB:
        {
            _CHECK_BYTES_AVAIL(2)
            const uint8_t reg = _NEXT_BYTE;
            _CHECK_REGISTER_VALID(reg)
            g_registers[reg] = _NEXT_BYTE;
            break;
        }

        case OP_JMP:
        {
            _CHECK_BYTES_AVAIL(2)
            g_registers[IP] = _NEXT_SHORT - 1;
            break;
        }
        }

        g_registers[IP]++;
        instrCount++;
    }

    return VM_PAUSED;
}
