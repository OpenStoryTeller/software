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

#define _NEXT_BYTE g_rom->mem[++g_registers[IP]]
#define _NEXT_SHORT ({ g_registers[IP] += 2; g_rom->mem[g_registers[IP]-1]\
                     | g_rom->mem[g_registers[IP]] << 8; })
#define _NEXT_INT ({                                                                               \
    g_registers[IP] += 4;                                                                     \
    g_rom->mem[g_registers[IP] - 3] | g_rom->mem[g_registers[IP] - 2] << 8 |       \
        g_rom->mem[g_registers[IP] - 1] << 16 | g_rom->mem[g_registers[IP]] << 24; \
})

#ifndef VM_DISABLE_CHECKS
#define _CHECK_ADDR_VALID(a) \
    if ((a < g_rom->addr) || (a >= (g_rom->addr + g_rom->size))) \
        return VM_ERR_INVALID_ADDRESS;
#define _CHECK_BYTES_AVAIL(n) \
    _CHECK_ADDR_VALID(g_registers[IP] + n)
#define _CHECK_REGISTER_VALID(r) \
    if (r >= REGISTER_COUNT)     \
        return VM_ERR_INVALID_REGISTER;
#define _CHECK_CAN_PUSH(n)                                              \
    if (g_registers[SP] - (n * sizeof(uint32_t)) > g_ram->addr) \
        return VM_ERR_STACK_OVERFLOW;
#define _CHECK_CAN_POP(n)                                               \
    if (g_registers[SP] + (n * sizeof(uint32_t)) > (g_ram->addr + g_ram->size) \
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

static virtual_mem_t *g_ram = NULL;
static virtual_mem_t *g_rom = NULL;
static uint32_t g_registers[REGISTER_COUNT] = {0};
static uint16_t g_stack_size;

bool (*g_interrupt_callback)(uint8_t) = nullptr;


void chip32_initialize(virtual_mem_t *rom, virtual_mem_t *ram, uint16_t stack_size)
{
    g_ram = ram;
    g_rom = rom;
    g_stack_size = stack_size;

    memset(g_ram->mem, 0, g_ram->size);
    memset(g_registers, 0, REGISTER_COUNT * sizeof(uint32_t));

    g_registers[SP] = g_ram->size;
}

#define MEM_ACCESS(addr, vmem) if ((addr >= vmem->addr) && ((addr + vmem->size) < vmem->size))\
{\
    addr -= vmem->addr;\
    return &vmem->mem[addr];\
}

uint8_t *chip32_memory(uint16_t addr)
{
    static uint8_t dummy = 0;
    // Beware, can provoke memory overflow
    MEM_ACCESS(addr, g_rom);
    MEM_ACCESS(addr, g_ram);

    return g_ram->mem; //!< Defaut memory to RAM location if address out of segment.
}

static void chip32_on_interrupt(bool (*callback)(uint8_t))
{
    g_interrupt_callback = callback;
}

uint32_t chip32_stack_count()
{
    return g_ram->size - g_registers[SP];
}

void chip32_stack_push(uint32_t value)
{
    g_registers[SP] -= 4;
    memcpy(chip32_memory(g_registers[SP]), &value, sizeof(uint32_t));
}

uint32_t chip32_stack_pop()
{
    uint32_t val = 0;
    memcpy(&val, chip32_memory(g_registers[SP]), sizeof(uint32_t));
    g_registers[SP] += 4;
    return val;
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
        const uint8_t instr = g_rom->mem[g_registers[IP]];
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
