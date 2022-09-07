#include <stdio.h>
#include <stdint.h>

#define RAM_SIZE    (4*1024)
#define PROG_SIZE    (4*1024)


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <time.h>

extern uint8_t waitinput(uint8_t *keys)
{
    // stub for now
    return 0;
}

enum {
    W        = 64,
    H        = 32,
    SCALE    = 10,
    FONTADDR = 0,
    HZ       = 500,
    NS       = (int)1E9,
    TIMERHZ  = 60,
};

typedef struct {
    uint16_t mask;
    uint16_t inst;
    void   (*func)(void);
} Inst;

char     disp[H][W];
uint8_t  V[0x10];
uint16_t PC;
uint16_t stack[0x10];
uint8_t  SP;
uint16_t I;
uint8_t  DT;
uint8_t  ST;
uint8_t  ram[0x1000] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,
    0x20, 0x60, 0x20, 0x20, 0x70,
    0xF0, 0x10, 0xF0, 0x80, 0xF0,
    0xF0, 0x10, 0xF0, 0x10, 0xF0,
    0x90, 0x90, 0xF0, 0x10, 0x10,
    0xF0, 0x80, 0xF0, 0x10, 0xF0,
    0xF0, 0x80, 0xF0, 0x90, 0xF0,
    0xF0, 0x10, 0x20, 0x40, 0x40,
    0xF0, 0x90, 0xF0, 0x90, 0xF0,
    0xF0, 0x90, 0xF0, 0x10, 0xF0,
    0xF0, 0x90, 0xF0, 0x90, 0x90,
    0xE0, 0x90, 0xE0, 0x90, 0xE0,
    0xF0, 0x80, 0x80, 0x80, 0xF0,
    0xE0, 0x90, 0x90, 0x90, 0xE0,
    0xF0, 0x80, 0xF0, 0x80, 0xF0,
    0xF0, 0x80, 0xF0, 0x80, 0x80,
};

uint8_t  keys[0x10];
uint8_t  timer;
uint8_t  quit;

uint16_t inst;
uint16_t nnn;
uint8_t  n;
uint8_t  x;
uint8_t  y;
uint8_t  kk;

void cls  (void) { memset(disp, 0, sizeof(disp)); }
void ret  (void) { PC = stack[--SP]; }
void jp   (void) { PC = nnn; }
void call (void) { stack[SP++] = PC; PC = nnn; }
void sei  (void) { if (V[x] == kk) PC += 2; }
void snei (void) { if (V[x] != kk) PC += 2; }
void se   (void) { if (V[x] == V[y]) PC += 2; }
void ldi  (void) { V[x] = kk; }
void addi (void) { V[x] += kk; }
void ld   (void) { V[x] = V[y]; }
void or   (void) { V[x] |= V[y]; }
void and  (void) { V[x] &= V[y]; }
void xor  (void) { V[x] ^= V[y]; }
void add  (void) { V[x] += V[y]; V[0xF] = V[x] < V[y]; }
void sub  (void) { V[0xF] = V[x] >= V[y]; V[x] -= V[y]; }
void shr  (void) { V[0xF] = V[x] & 1; V[x] >>= 1; }
void subn (void) { V[0xF] = V[y] >= V[x]; V[x] = V[y] - V[x]; }
void shl  (void) { V[0xF] = V[x] >> 7; V[x] <<= 1; }
void sne  (void) { if (V[x] != V[y]) PC += 2; }
void ldIi (void) { I = nnn; }
void jpv0 (void) { PC = V[0] + nnn; }
void rnd  (void) { V[x] = rand() & kk; }
void skp  (void) { if (keys[V[x]]) PC += 2; }
void sknp (void) { if (!keys[V[x]]) PC += 2; }
void ldfDT(void) { V[x] = DT; }
void ldfK (void) { V[x] = waitinput(keys); if (V[x] == 0xFF) quit = 1;}
void ldDT (void) { DT = V[x]; }
void ldST (void) { ST = V[x]; }
void addI (void) { I = (I + V[x]) % 0xFFF; V[0xF] = I < V[x]; }
void ldF  (void) { I = FONTADDR + V[x]*5; }
void ldB  (void) { ram[I] = V[x]/100; ram[I+1] = V[x]/10%10; ram[I+2] = V[x]%10; }
void stoV (void) { for (uint8_t i = 0; i <= x; i++) ram[I+i] = V[i]; }
void ldV  (void) { for (uint8_t i = 0; i <= x; i++) V[i] = ram[I+i]; }
void nop  (void) {}

void
drw(void)
{

}

void
badi(void)
{
    fprintf(stderr, "bad instruction: inst:0x%04X PC:0x%04X\n", inst, PC-2);
    exit(1);
}

Inst insts[] = {
    { 0xFFFF, 0x00E0, cls   },
    { 0xFFFF, 0x00EE, ret   },
    { 0xF000, 0x1000, jp    },
    { 0xF000, 0x2000, call  },
    { 0xF000, 0x3000, sei   },
    { 0xF000, 0x4000, snei  },
    { 0xF000, 0x5000, se    },
    { 0xF000, 0x6000, ldi   },
    { 0xF000, 0x7000, addi  },
    { 0xF00F, 0x8000, ld    },
    { 0xF00F, 0x8001, or    },
    { 0xF00F, 0x8002, and   },
    { 0xF00F, 0x8003, xor   },
    { 0xF00F, 0x8004, add   },
    { 0xF00F, 0x8005, sub   },
    { 0xF00F, 0x8006, shr   },
    { 0xF00F, 0x8007, subn  },
    { 0xF00F, 0x800E, shl   },
    { 0xF00F, 0x9000, sne   },
    { 0xF000, 0xA000, ldIi  },
    { 0xF000, 0xB000, jpv0  },
    { 0xF000, 0xC000, rnd   },
    { 0xF000, 0xD000, drw   },
    { 0xF0FF, 0xE09E, skp   },
    { 0xF0FF, 0xE0A1, sknp  },
    { 0xF0FF, 0xF007, ldfDT },
    { 0xF0FF, 0xF00A, ldfK  },
    { 0xF0FF, 0xF015, ldDT  },
    { 0xF0FF, 0xF018, ldST  },
    { 0xF0FF, 0xF01E, addI  },
    { 0xF0FF, 0xF029, ldF   },
    { 0xF0FF, 0xF033, ldB   },
    { 0xF0FF, 0xF055, stoV  },
    { 0xF0FF, 0xF065, ldV   },
    { 0xF000, 0x0000, nop   },
    { 0x0000, 0x0000, badi  },
};

long
tsdiff(struct timespec new, struct timespec old)
{
    if (new.tv_nsec < old.tv_nsec) {
        new.tv_nsec += 1E9;
        new.tv_sec--;
    }
    return (new.tv_sec - old.tv_sec)*1E9 + new.tv_nsec - old.tv_nsec;
}

void
tick(struct timespec ts)
{
    static struct timespec lastdec;
    if (tsdiff(ts, lastdec) >= NS/TIMERHZ) {
        if (DT)
            --DT;
        if (ST) {
            /* TODO: make noise */
            --ST;
        }
        lastdec = ts;
    }

    inst = ram[PC]<<8 | ram[PC+1];
    nnn  =  inst & 0x0FFF;
    n    =  inst & 0x000F;
    x    = (inst & 0x0F00) >> 8;
    y    = (inst & 0x00F0) >> 4;
    kk   =  inst & 0x00FF;

    PC += 2;

    Inst *p;
    for (p = insts; (inst & p->mask) != p->inst; p++)
        ;
    p->func();
}

int chip8_run(void)
{
    int c;
    PC = 0x200;
    while ((c = getchar()) != EOF)
        ram[PC++] = c;
    PC = 0x200;

    do {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        tick(ts);
//        draw(disp);
        nanosleep(&(struct timespec){.tv_nsec = 1E9/HZ}, 0);
    } while (!quit);

    return 0;
}

/*

Memory Map:
+---------------+= 0xFFF (4095) End of Chip-8 RAM
|               |
|               |
|               |
|               |
|               |
| 0x200 to 0xFFF|
|     Chip-8    |
| Program / Data|
|     Space     |
|               |
|               |
|               |
+- - - - - - - -+= 0x600 (1536) Start of ETI 660 Chip-8 programs
|               |
|               |
|               |
+---------------+= 0x200 (512) Start of most Chip-8 programs
| 0x000 to 0x1FF|
| Reserved for  |
|  interpreter  |
+---------------+= 0x000 (0) Start of Chip-8 RAM


2.2 - Registers           [TOC]

Chip-8 has 16 general purpose 8-bit registers, usually referred to as Vx, where x is a hexadecimal digit (0 through F). There is also a 16-bit register called I. This register is generally used to store memory addresses, so only the lowest (rightmost) 12 bits are usually used.
The VF register should not be used by any program, as it is used as a flag by some instructions. See section 3.0, Instructions for details.
Chip-8 also has two special purpose 8-bit registers, for the delay and sound timers. When these registers are non-zero, they are automatically decremented at a rate of 60Hz. See the section 2.5, Timers & Sound, for more information on these.
There are also some "pseudo-registers" which are not accessable from Chip-8 programs. The program counter (PC) should be 16-bit, and is used to store the currently executing address. The stack pointer (SP) can be 8-bit, it is used to point to the topmost level of the stack.
The stack is an array of 16 16-bit values, used to store the address that the interpreter shoud return to when finished with a subroutine. Chip-8 allows for up to 16 levels of nested subroutines.


2.3 - Keyboard           [TOC]

The computers which originally used the Chip-8 Language had a 16-key hexadecimal keypad with the following layout:

1	2	3	C
4	5	6	D
7	8	9	E
A	0	B	F

This layout must be mapped into various other configurations to fit the keyboards of today's platforms.


 3.0 - Chip-8 Instructions           [TOC]

The original implementation of the Chip-8 language includes 36 different instructions, including math, graphics, and flow control functions.
Super Chip-48 added an additional 10 instructions, for a total of 46.
All instructions are 2 bytes long and are stored most-significant-byte first. In memory, the first byte of each instruction should be located at an even addresses. If a program includes sprite data, it should be padded so any instructions following it will be properly situated in RAM.
This document does not yet contain descriptions of the Super Chip-48 instructions. They are, however, listed below.

In these listings, the following variables are used:

nnn or addr - A 12-bit value, the lowest 12 bits of the instruction
n or nibble - A 4-bit value, the lowest 4 bits of the instruction
x - A 4-bit value, the lower 4 bits of the high byte of the instruction
y - A 4-bit value, the upper 4 bits of the low byte of the instruction
kk or byte - An 8-bit value, the lowest 8 bits of the instruction


3.1 - Standard Chip-8 Instructions           [TOC]

0nnn - SYS addr
Jump to a machine code routine at nnn.

This instruction is only used on the old computers on which Chip-8 was originally implemented. It is ignored by modern interpreters.


00E0 - CLS
Clear the display.


00EE - RET
Return from a subroutine.

The interpreter sets the program counter to the address at the top of the stack, then subtracts 1 from the stack pointer.


1nnn - JP addr
Jump to location nnn.

The interpreter sets the program counter to nnn.


2nnn - CALL addr
Call subroutine at nnn.

The interpreter increments the stack pointer, then puts the current PC on the top of the stack. The PC is then set to nnn.


3xkk - SE Vx, byte
Skip next instruction if Vx = kk.

The interpreter compares register Vx to kk, and if they are equal, increments the program counter by 2.


4xkk - SNE Vx, byte
Skip next instruction if Vx != kk.

The interpreter compares register Vx to kk, and if they are not equal, increments the program counter by 2.


5xy0 - SE Vx, Vy
Skip next instruction if Vx = Vy.

The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.


6xkk - LD Vx, byte
Set Vx = kk.

The interpreter puts the value kk into register Vx.


7xkk - ADD Vx, byte
Set Vx = Vx + kk.

Adds the value kk to the value of register Vx, then stores the result in Vx.

8xy0 - LD Vx, Vy
Set Vx = Vy.

Stores the value of register Vy in register Vx.


8xy1 - OR Vx, Vy
Set Vx = Vx OR Vy.

Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx. A bitwise OR compares the corrseponding bits from two values, and if either bit is 1, then the same bit in the result is also 1. Otherwise, it is 0.


8xy2 - AND Vx, Vy
Set Vx = Vx AND Vy.

Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx. A bitwise AND compares the corrseponding bits from two values, and if both bits are 1, then the same bit in the result is also 1. Otherwise, it is 0.


8xy3 - XOR Vx, Vy
Set Vx = Vx XOR Vy.

Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx. An exclusive OR compares the corrseponding bits from two values, and if the bits are not both the same, then the corresponding bit in the result is set to 1. Otherwise, it is 0.


8xy4 - ADD Vx, Vy
Set Vx = Vx + Vy, set VF = carry.

The values of Vx and Vy are added together. If the result is greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0. Only the lowest 8 bits of the result are kept, and stored in Vx.


8xy5 - SUB Vx, Vy
Set Vx = Vx - Vy, set VF = NOT borrow.

If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and the results stored in Vx.


8xy6 - SHR Vx {, Vy}
Set Vx = Vx SHR 1.

If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.


8xy7 - SUBN Vx, Vy
Set Vx = Vy - Vx, set VF = NOT borrow.

If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.


8xyE - SHL Vx {, Vy}
Set Vx = Vx SHL 1.

If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.


9xy0 - SNE Vx, Vy
Skip next instruction if Vx != Vy.

The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.


Annn - LD I, addr
Set I = nnn.

The value of register I is set to nnn.


Bnnn - JP V0, addr
Jump to location nnn + V0.

The program counter is set to nnn plus the value of V0.


Cxkk - RND Vx, byte
Set Vx = random byte AND kk.

The interpreter generates a random number from 0 to 255, which is then ANDed with the value kk. The results are stored in Vx. See instruction 8xy2 for more information on AND.


Dxyn - DRW Vx, Vy, nibble
Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.

The interpreter reads n bytes from memory, starting at the address stored in I. These bytes are then displayed as sprites on screen at coordinates (Vx, Vy). Sprites are XORed onto the existing screen. If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0. If the sprite is positioned so part of it is outside the coordinates of the display, it wraps around to the opposite side of the screen. See instruction 8xy3 for more information on XOR, and section 2.4, Display, for more information on the Chip-8 screen and sprites.


Ex9E - SKP Vx
Skip next instruction if key with the value of Vx is pressed.

Checks the keyboard, and if the key corresponding to the value of Vx is currently in the down position, PC is increased by 2.


ExA1 - SKNP Vx
Skip next instruction if key with the value of Vx is not pressed.

Checks the keyboard, and if the key corresponding to the value of Vx is currently in the up position, PC is increased by 2.


Fx07 - LD Vx, DT
Set Vx = delay timer value.

The value of DT is placed into Vx.


Fx0A - LD Vx, K
Wait for a key press, store the value of the key in Vx.

All execution stops until a key is pressed, then the value of that key is stored in Vx.


Fx15 - LD DT, Vx
Set delay timer = Vx.

DT is set equal to the value of Vx.


Fx18 - LD ST, Vx
Set sound timer = Vx.

ST is set equal to the value of Vx.


Fx1E - ADD I, Vx
Set I = I + Vx.

The values of I and Vx are added, and the results are stored in I.


Fx29 - LD F, Vx
Set I = location of sprite for digit Vx.

The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx. See section 2.4, Display, for more information on the Chip-8 hexadecimal font.


Fx33 - LD B, Vx
Store BCD representation of Vx in memory locations I, I+1, and I+2.

The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.


Fx55 - LD [I], Vx
Store registers V0 through Vx in memory starting at location I.

The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.


Fx65 - LD Vx, [I]
Read registers V0 through Vx from memory starting at location I.

The interpreter reads values from memory starting at location I into registers V0 through Vx.


*/

