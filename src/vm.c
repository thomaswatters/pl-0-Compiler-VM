//
// Created by Thomas on 7/11/2015.
//
#include "vm.h"

#include <stdio.h>
#include <stdlib.h>

//Registers
unsigned sp = 0;
unsigned bp = 1;
unsigned pc = 0;
instruction ir;

//memory stack
int stack[MAX_STACK_HEIGHT];

//additional data to manage viewing of AR
int ar_breaks[MAX_LEXI_LEVELS];
int levels = 0;

//instruction store
instruction code[MAX_CODE_LENGTH];
int instruction_count = 0;


//Display stuff
char* OpStrings[] = {"", "lit", "opr", "lod", "sto", "cal", "inc", "jmp", "jpc", "sio"};


void readInstructionFile(char* fileName)
{
    FILE* in = fopen(fileName, "r");
    instruction_count = 0;
    if(in == NULL)
    {
        fprintf(stderr, "%s %s\n", "Unable to open file: ", fileName );
        exit(-1);
    }

    while(instruction_count < MAX_CODE_LENGTH)
    {
        if(fscanf(in, "%d %d %d", &code[instruction_count].OP, &code[instruction_count].L, &code[instruction_count].M) != 3)
            break;

        instruction_count++;
    }

    fclose(in);
}


void runProgram(char* fileName)
{

    FILE* out = fopen(fileName, "a");
   // FILE* out = stdout;
    fprintf(out, "\n\n\n%30s%7s%7s%10s\n", "pc", "bp", "sp", "stack");
    fprintf(out, "%-28s%-7d%-7d%-7d\n","Initial Values", pc, bp, sp);

    int running = 1;
    int temp_sp = 0;
    while(running)
    {
        fetch();

        fprintf(out, "%-7d%-7s%-7d%-7d", pc - 1, OpStrings[ir.OP], ir.L, ir.M);


        if(execute() != 0)
            running = 0;

        fprintf(out,"%-7d%-7d%-7d", pc, bp, sp);
        temp_sp = 1;
        int temp_lvl = 0;
        while(temp_sp <=  sp)
        {
            fprintf(out, "%d ", stack[temp_sp]);

            if(temp_lvl < MAX_LEXI_LEVELS && ar_breaks[temp_lvl] == temp_sp && temp_sp != sp )
            {
                fprintf(out, "| ");
                temp_lvl++;
            }

            temp_sp++;
        }

        fprintf(out,"\n");

    }

    fclose(out);
}



void printInstructions(char* fileName)
{

    FILE* out = fopen(fileName, "w");

    int i = 0;
    fprintf(out, "%-7s%-7s%-7s%-7s\n", "Line", "OP", "L", "M");
    for(i =0; i < instruction_count; i++)
    {
        fprintf(out, "%-7d%-7s%-7d%-7d\n", i, OpStrings[code[i].OP], code[i].L, code[i].M);
    }

    fclose(out);
}


void fetch()
{
    ir = code[pc];
    pc = pc + 1;
}


int execute()
{
    switch(ir.OP)
    {
        case 1:    // LIT
            sp = sp + 1;
            stack[sp] = ir.M;
            break;
        case 2:    //OPR
            GetOPR();
            break;
        case 3:    //LOD
            sp = sp + 1;
            stack[sp] = stack[ base(ir.L, bp) + ir.M];
            break;
        case 4:    //STO
            stack[ base(ir.L, bp) + ir.M] = stack[sp];
            sp = sp - 1;
            break;
        case 5:    //CAL
            stack[sp + 1] = 0; // return value (FV)
            stack[sp + 2] = base(ir.L, bp); // static link (SL)
            stack[sp + 3] = bp; // dynamic link (DL)
            stack[sp + 4] = pc; // return address (RA)
            bp = sp + 1;
            pc = ir.M;
            ar_breaks[levels++] = bp - 1;
            break;
        case 6:    //INC
            sp = sp + ir.M;
            break;
        case 7:    //JMP
            pc = ir.M;
            break;
        case 8:
            if( stack[sp] == 0 )  pc = ir.M;
            sp = sp - 1;
            break;
        case 9:
            return GetSIO();
        default:
            return 1;

    }

    return 0;

}

void GetOPR()
{
    switch(ir.M)
    {
        case OPR_RET: //RET
            sp = bp - 1;
            pc = stack[sp + 4];
            bp = stack[sp + 3];
            levels--;
            ar_breaks[levels] = -1;
            break;
        case OPR_NEG: //NEG
            stack[sp] = -1 * stack[sp];
            break;
        case OPR_ADD: //ADD
            sp = sp - 1;
            stack[sp] = stack[sp] + stack[sp + 1];
            break;
        case OPR_SUB: //SUB
            sp = sp - 1;
            stack[sp] = stack[sp] - stack[sp + 1];
            break;
        case OPR_MUL: //MUL
            sp = sp - 1;
            stack[sp] = stack[sp] * stack[sp + 1];
            break;
        case OPR_DIV: //DIV
            sp = sp - 1;
            stack[sp] = stack[sp] / stack[sp + 1];
            break;
        case OPR_ODD: //ODD
            stack[sp] = stack[sp] % 2;
            break;
        case OPR_MOD: // MOD
            sp = sp - 1;
            stack[sp] = stack[sp] % stack[sp + 1];
            break;
        case OPR_EQL: //EQL
            sp = sp - 1;
            stack[sp] = stack[sp] == stack[sp + 1];
            break;
        case OPR_NEQ: //NEQ
            sp = sp - 1;
            stack[sp] = stack[sp] != stack[sp + 1];
            break;
        case OPR_LSS: //LSS
            sp = sp - 1;
            stack[sp] = stack[sp] < stack[sp + 1];
            break;
        case OPR_LEQ: //LEQ
            sp = sp - 1;
            stack[sp] = stack[sp] <= stack[sp + 1];
            break;
        case OPR_GTR: //GTR
            sp = sp - 1;
            stack[sp] = stack[sp] > stack[sp + 1];
            break;
        case OPR_GEQ: //GEQ
            sp = sp - 1;
            stack[sp] = stack[sp] >= stack[sp + 1];
            break;
        default:
            exit(-1);

    }
}

int GetSIO()
{
    switch(ir.M)
    {
        case 0:
            fprintf(stdout, "SIO_PRINT: %d\n", stack[sp]);
            fflush(stdout);
            sp = sp - 1;
            break;
        case 1:
            sp = sp + 1;
            fprintf(stdout, "SIO_READ: ");
            fflush(stdout);
            fscanf(stdin, "%d", &stack[sp]);
            fflush(stdin);
            break;
        case 2:
            return 1;
        default:
            return 1;
    }

    return 0;
}

int base(int level, int b)
{
    while(level > 0)
    {
        b = stack[b + 1];
        level--;
    }

    return b;

}

