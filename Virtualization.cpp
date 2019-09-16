//
// Created by Aidan Milligan on 2019-09-14.
//

#include "Virtualization.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>

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

/* Memory Mapped Registers */
enum
{
    MR_KBSR = 0xFE00, /* keyboard status */
    MR_KBDR = 0xFE02  /* keyboard data */
};

/* TRAP Codes */
enum
{
    TRAP_GETC = 0x20,  /* get character from keyboard, not echoed onto the terminal */
    TRAP_OUT = 0x21,   /* output a character */
    TRAP_PUTS = 0x22,  /* output a word string */
    TRAP_IN = 0x23,    /* get character from keyboard, echoed onto the terminal */
    TRAP_PUTSP = 0x24, /* output a byte string */
    TRAP_HALT = 0x25   /* halt the program */
};


/* Initialise Virtual Memory */
uint16_t virtualMemory[UINT16_MAX];

/* Initialise Register Storage */
uint16_t virtualRegisters[RegisterCount];

using namespace std;

/* CPU Functions/Instructions */
/* Sign Extended */
uint16_t SignExtended(uint16_t x, int bit_count)
{
    if(( x >> (bit_count - 1) & 1 ))
    {
        x |= (0xFFFF << bit_count);
    } return x;
}

/* Swap lower 8 bits for upper 8 bits */
uint16_t Swap16(uint16_t x)
{

    return (x << 8) | (x >> 8);
}

/* Update Flags */
void update_flags(uint16_t r)
{
    if (virtualRegisters[r] == 0)
    {
        virtualRegisters[RegisterCond] = FlagZero;
    }
    else if (virtualRegisters[r] >> 15) /* a 1 in the left-most bit indicates negative */
    {
        virtualRegisters[RegisterCond] = FlagNegative;
    }
    else
    {
        virtualRegisters[RegisterCond] = FlagPositive;
    }
}

/* Read Image File */
void read_image_file(FILE* file)
{
    /* the origin tells us where in memory to place the image */
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    origin = Swap16(origin);

    /* we know the maximum file size so we only need one fread */
    uint16_t max_read = UINT16_MAX - origin;
    uint16_t* p = virtualMemory + origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    /* swap to little endian */
    while (read -- > 0)
    {
        *p = Swap16(*p);
        ++p;
    }
}

/* Read Image */
int read_image(const char* image_path)
{
    FILE* file = fopen(image_path, "rb");
    if (!file) { return 0; };
    read_image_file(file);
    fclose(file);
    return 1;
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

/* Input Buffering */
struct termios original_tio;

void restore_input_buffering()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

void disable_input_buffering()
{
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

/* Handle Interrupt */
void handle_interrupt(int signal)
{
    restore_input_buffering();
    printf("\n");
    exit(-2);
}


template <unsigned op>
void execIns(uint16_t instruction)
{
    uint16_t r0, r1, r2, imm5, imm_flag;
    uint16_t pc_plus_off, base_plus_off;

    uint16_t opbit = ( 1 << op );

    if(0x4EEE & opbit ) { r0 = (instruction >> 9) & 0x7; }
    if(0x12E3 & opbit ) { r1 = (instruction >> 6) & 0x7; }
    if(0x0022 & opbit )
    {
        r2 = instruction & 0x7;
        imm_flag = (instruction >> 5) & 0x1;
        imm5 = SignExtended((instruction) & 0x1F, 5);
    }
    if(0x00C0 & opbit )
    {
        base_plus_off = virtualRegisters[r1] + SignExtended(instruction & 0x3f, 6);
    }
    if(0x4C0D & opbit )
    {
        // Indirect address
        pc_plus_off = virtualRegisters[RegisterPC] + SignExtended(instruction & 0x1ff, 9);
    }
    if(0x0001 & opbit )
    {
        // BR
        uint16_t cond = (instruction >> 9) & 0x7;
        if (cond & virtualRegisters[RegisterCond]) { virtualRegisters[RegisterPC] = pc_plus_off; }
    }

    //ADD Operation (r0 = r1+r2)
    if(0x0002 & opbit )
    {
        if (imm_flag) {
            virtualRegisters[r0] = virtualRegisters[r1] + imm5;
        } else {
            virtualRegisters[r0] = virtualRegisters[r1] + virtualRegisters[r2];
        }
    }

    //AND Operation (r0 = r1&r2)
    if(0x0020 & opbit)
    {
        if(imm_flag)
        {
            virtualRegisters[r0] = virtualRegisters[r1] & imm5;
        } else {
            virtualRegisters[r0] = virtualRegisters[r1] & virtualRegisters[r2];
        }

    }

    //NOT Operation (r0 = ~r1)
    if(0x0200 & opbit)
    {
        virtualRegisters[r0] = ~virtualRegisters[r1];
    }

    //Jump Operation (PC = r1)
    if(0x1000 & opbit){
        virtualRegisters[RegisterPC] = virtualRegisters[r1];
    }

    if (0x0010 & opbit)  // JSR
    {
        uint16_t long_flag = (instruction >> 11) & 1;
        pc_plus_off = virtualRegisters[RegisterPC] + SignExtended(instruction & 0x7ff, 11);
        virtualRegisters[RegisterPC] = virtualRegisters[RegisterPC];
        if (long_flag) {
            virtualRegisters[RegisterPC] = pc_plus_off;
        } else {
            virtualRegisters[RegisterPC] = virtualRegisters[r1];
        }
    }
}


/* Op Table */
static void (*op_table[16])(uint16_t) = {
        execIns<0>, execIns<1>, execIns<2>, execIns<3>,
        execIns<4>, execIns<5>, execIns<6>, execIns<7>,
        NULL, execIns<9>, execIns<10>, execIns<11>,
        execIns<12>, NULL, execIns<14>, execIns<15>
};


int VirtualMachine::StartVM(bool*continueProcess)
{

    signal(SIGINT, handle_interrupt);
    disable_input_buffering();


    //set the program counter register
    enum { PC_START = 0x3000 };
    virtualRegisters[RegisterPC] = PC_START;


    while(*continueProcess)
    {
        //read mem pointed by program counter
        uint16_t currentInstruction = ReadFromMemory(virtualRegisters[RegisterPC]);

        //increment program counter
        virtualRegisters[RegisterPC]++;

        //4 bits for op code
        uint16_t currentOperation = currentInstruction >> 12;

        op_table[currentOperation](currentInstruction);
    }
}
