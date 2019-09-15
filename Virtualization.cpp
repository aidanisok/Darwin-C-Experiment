//
// Created by Aidan Milligan on 2019-09-14.
//

#include "Virtualization.h"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

/* CPU Registers
 * 8 General Purpose Registers */
enum
{
    RegisterR0 = 0,
    RegisterR1,
    RegisterR2,
    RegisterR3,
    RegisterR4,
    RegisterR5,
    RegisterR6,
    RegisterR7,
    RegisterPC, //Program Counting Register
    RegisterCond,
    RegisterCount,
};

enum
{
    FlagPositive = 1 << 0,
    FlagZero = 1 << 1,
    FlagNegative = 1 << 2
};


enum {
    OP_BR = 0, /* branch */
    OP_ADD,    /* add  */
    OP_LD,     /* load */
    OP_ST,     /* store */
    OP_JSR,    /* jump register */
    OP_AND,    /* bitwise and */
    OP_LDR,    /* load register */
    OP_STR,    /* store register */
    OP_RTI,    /* unused */
    OP_NOT,    /* bitwise not */
    OP_LDI,    /* load indirect */
    OP_STI,    /* store indirect */
    OP_JMP,    /* jump */
    OP_RES,    /* reserved (unused) */
    OP_LEA,    /* load effective address */
    OP_TRAP    /* execute trap */
};


/* Initialise Virtual Memory */
uint16_t virtualMemory[UINT16_MAX];

/* Initialise Register Storage */
uint16_t virtualRegisters[RegisterCount];


/* CPU Functions/Instructions */
/* Sign Extended */
uint16_t SignExtended(uint16_t x, int bit_count)
{
    if((x>>(bit_count-1) & 1))
    {
        x |= (0xFFFF << bit_count);
    } return x;
}

/* Swap lower 8 bits for upper 8 bits */
uint16_t Swap16(uint16_t x)
{

    return (x << 8) | (x >> 8);
}

/* Read from memory function */
uint16_t ReadFromMemory(uint16_t address)
{
    return virtualMemory[address];
}

/* Write to memory function */
uint16_t WriteToMemory(uint16_t address, uint16_t value)
{
    virtualMemory[address] = value;
    return virtualMemory[address];
}

template <unsigned op>
void ExecuteInstruction(uint16_t instruction)
{

    uint16_t opbit = ( 1 << op );

    if( 0x0002 & opbit ) //ADD r0 = r1 + r2
    {
        virtualRegisters[RegisterR0] = virtualRegisters[RegisterR1] + virtualRegisters[RegisterR2];
    }

}


int init()
{
    while(true)
    {
        //read mem pointed by program counter
        uint16_t currentInstruction = ReadFromMemory(virtualRegisters[RegisterPC]);

        //increment program counter
        virtualRegisters[RegisterPC]++;

        //4 bits for op code
        uint16_t currentOperation = currentInstruction >> 12;
    }
}
