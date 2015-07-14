//
// Created by Thomas on 7/11/2015.
//

#ifndef MODULE3_VM_H
#define MODULE3_VM_H

#define MAX_STACK_HEIGHT 2000
#define MAX_CODE_LENGTH 500
#define MAX_LEXI_LEVELS 30



typedef enum
{
    LIT=1, OPR, LOD, STO,
    CAL, INC, JMP, JPC, SIO
}OP_CODE;

typedef enum
{
    SIO_PRINT, SIO_READ, SIO_HALT
}SIO_CODE;

typedef enum
{
    OPR_RET = 0, OPR_NEG, OPR_ADD, OPR_SUB, OPR_MUL, OPR_DIV,
    OPR_ODD, OPR_MOD, OPR_EQL, OPR_NEQ, OPR_LSS, OPR_LEQ, OPR_GTR,
    OPR_GEQ
}OPR_CODES;

struct instruction
{
    unsigned OP, L, M;
}typedef instruction;



void readInstructionFile(char* fileName);
void runProgram(char* fileName);
void printInstructions(char* fileName);
void fetch();
int execute();
void GetOPR();
int GetSIO();
int base(int level, int b);



#endif //MODULE3_VM_H
