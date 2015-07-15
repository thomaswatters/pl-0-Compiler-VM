
#include <stdbool.h>


#ifndef MODULE3_VM_H
#define MODULE3_VM_H

#define DEFAULT_MAX_STACK_HEIGHT 2000
#define DEFAULT_MAX_CODE_LENGTH 500
#define DEFAULT_MAX_LEXI_LEVELS 30

typedef struct
{
    unsigned max_stack_height;
    unsigned max_code_length;
    unsigned max_lexi_levels;
    bool output_stacktrace_to_file;
    bool output_detailed_instructions_to_file;

}VM_SETTINGS;



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


void startVM(char* assembly_file, const VM_SETTINGS* settings);

#endif //MODULE3_VM_H
