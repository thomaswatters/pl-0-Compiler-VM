#include <stdlib.h>

#include "vm.h"
#include "errors.h"


//prototypes
void stopVM();
void internal_error(int num);
void loadInstructions(char* assembly_code);
void executeVM();
void printInstructions(char* fileName);
void fetch();
int execute();
void opr();
int sio();
int base(int level, int b);
void stacktraceToFile();



//Registers
unsigned sp = 0;
unsigned bp = 1;
unsigned pc = 0;
unsigned mp = 1;
instruction ir;

//memory stack
int* stack;

//additional data to manage viewing of AR
int* ar_breaks;
int levels = 0;

//instruction store
instruction* code;
int instruction_count = 0;

char file_buffer[4096];
int file_buffer_length;

FILE* out;


VM_SETTINGS default_settings = (VM_SETTINGS)
        {
                .max_code_length = DEFAULT_MAX_CODE_LENGTH,
                .max_lexi_levels = DEFAULT_MAX_LEXI_LEVELS,
                .max_stack_height = DEFAULT_MAX_STACK_HEIGHT,
                .output_stacktrace_to_file = false,
                .output_detailed_instructions_to_file = false
        };

VM_SETTINGS vm_settings;


//Display stuff
char* OpStrings[] = {"", "lit", "opr", "lod", "sto", "cal", "inc", "jmp", "jpc", "sio", "mst"};


//Output file
char* outputfile = "stacktrace.out";


void startVM(char* assembly_code, const VM_SETTINGS* settings)
{
    if(assembly_code == NULL)
        internal_error(0);

    if(settings == NULL)
        settings = &default_settings;

    vm_settings = *settings;

    code = (instruction*)calloc(vm_settings.max_code_length,sizeof(instruction));
    if(code == NULL)
        internal_error(0);


    stack = (int*)calloc(vm_settings.max_stack_height,sizeof(int));
    if(stack == NULL)
        internal_error(0);

    ar_breaks = (int*)calloc(vm_settings.max_lexi_levels,sizeof(int));

    loadInstructions(assembly_code);

    if(vm_settings.output_detailed_instructions_to_file)
    {
        printInstructions(outputfile);
    }

    executeVM();
    stopVM();

}


void stopVM()
{
    if(code != NULL)
        free(code);

    if(stack != NULL)
        free(stack);

    if(ar_breaks != NULL)
        free(ar_breaks);

    if(file_buffer_length != 0 && out != NULL) {
        fprintf(out, "%s", file_buffer);
        closeFile(out);
    }

}


void internal_error(int num)
{
    stopVM();
    error(num);

}


/*
 * Runs the code through the vm
 */
void executeVM()
{

    out = NULL;
    if(vm_settings.output_stacktrace_to_file)
    {
        out = openFile(outputfile, "a");
        if(out == NULL)
            internal_error(0);

        fprintf(out, "\n\n\n%30s%7s%7s%10s\n", "pc", "bp", "sp", "stack");
        fprintf(out, "%-28s%-7d%-7d%-7d\n","Initial Values", pc, bp, sp);
    }


    bool running = true;
    while(running)
    {
        fetch();

        running = execute() == 0 ? true : false;

        if(vm_settings.output_stacktrace_to_file)
            stacktraceToFile();

    }


}

void stacktraceToFile()
{

    if(out == NULL)
        internal_error(0);

    file_buffer_length += sprintf(&file_buffer[file_buffer_length], "%-7d%-7s%-7d%-7d", pc - 1, OpStrings[ir.OP], ir.L, ir.M);

    file_buffer_length += sprintf(&file_buffer[file_buffer_length], "%-7d%-7d%-7d", pc, bp, sp);
    int temp_sp = 1;
    int temp_lvl = 0;
    while (temp_sp <= sp)
    {
        file_buffer_length += sprintf(&file_buffer[file_buffer_length], "%d ", stack[temp_sp]);

        if (temp_lvl < vm_settings.max_lexi_levels &&
            ar_breaks[temp_lvl] == temp_sp &&
            temp_sp != sp)
        {
            file_buffer_length += sprintf(&file_buffer[file_buffer_length], "| ");
            temp_lvl++;
        }

        temp_sp++;
    }

    file_buffer_length += sprintf(&file_buffer[file_buffer_length], "\n");

    if(file_buffer_length >= 2048)
    {
        fprintf(out, "%s", file_buffer);
        file_buffer_length = 0;
    }

}



/*
 * Loads pm/0 instructions from a file named assembly_code
 * into code_head array
 *
 * Throws an error if the file is not able to be opened
 *
 */
void loadInstructions(char* assembly_code)
{

    FILE* in_file = openFile(assembly_code, "r");
    if(in_file == NULL)
    {
        fprintf(stderr, "Unable to access file: %s\n", assembly_code);
        internal_error(0);
    }

    instruction_count = 0;
    while(fscanf(in_file, "%d %d %d",
                 &code[instruction_count].OP,
                 &code[instruction_count].L,
                 &code[instruction_count].M) == 3)
    {
        instruction_count++;
        if(instruction_count >= vm_settings.max_code_length)
        {
            closeFile(in_file);
            internal_error(0);
        }

    }

    closeFile(in_file);

}


void printInstructions(char* fileName)
{

    FILE* out = openFile(fileName, "w");
    if(out == NULL)
        internal_error(0);

    int i = 0;
    fprintf(out, "%-7s%-7s%-7s%-7s\n", "Line", "OP", "L", "M");
    for(i =0; i < instruction_count; i++)
    {
        fprintf(out, "%-7d%-7s%-7d%-7d\n", i, OpStrings[code[i].OP], code[i].L, code[i].M);
    }

    closeFile(out);

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
        case LIT:    // LIT
            sp = sp + 1;
            if(sp >= vm_settings.max_stack_height)
                internal_error(0);

            stack[sp] = ir.M;
            break;
        case OPR:    //OPR
            opr();
            break;
        case LOD:    //LOD
            sp = sp + 1;

            if(sp >= vm_settings.max_stack_height)
                internal_error(0);

            stack[sp] = stack[ base(ir.L, bp) + ir.M];
            break;
        case STO:    //STO
            stack[ base(ir.L, bp) + ir.M] = stack[sp];
            sp = sp - 1;
            break;
        case MST:
            if(sp + 4 >= vm_settings.max_stack_height)
                internal_error(0);

            stack[sp + 1] = 0; // return value (FV)
            stack[sp + 2] = base(ir.L, bp); // static link (SL)
            stack[sp + 3] = bp; // dynamic link (DL)

            mp = sp + 1;
            sp = sp + 4;

            if(vm_settings.output_stacktrace_to_file)
                ar_breaks[levels++] = mp - 1;


            if(sp + 4 >= vm_settings.max_stack_height)
                internal_error(0);

            break;
        case CAL:    //CAL
            bp = mp;
            stack[bp + 3] = pc; // return address (RA)
            pc = ir.M;
            break;
        case INC:    //INC
            sp = sp + ir.M;
            if(sp + 4 >= vm_settings.max_stack_height)
                internal_error(0);

            break;
        case JMP:    //JMP
            pc = ir.M;
            break;
        case JPC:
            if( stack[sp] == 0 )  pc = ir.M;
            sp = sp - 1;
            break;
        case SIO:
            return sio();

        default:
            internal_error(0);

    }

    return 0;

}

void opr()
{
    switch(ir.M)
    {
        case OPR_RET: //RET
            sp = bp - 1;
            pc = stack[sp + 4];
            bp = stack[sp + 3];
            if(vm_settings.output_stacktrace_to_file) {
                levels--;
                ar_breaks[levels] = -1;
            }
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
            internal_error(0);

    }
}

int sio()
{
    switch(ir.M)
    {
        case SIO_PRINT:
            fprintf(stdout, "SIO_PRINT: %d\n", stack[sp]);
            fflush(stdout);
            sp = sp - 1;
            break;
        case SIO_READ:
            sp = sp + 1;
            fprintf(stdout, "SIO_READ: ");
            fflush(stdout);

            fflush(stdin);
            fscanf(stdin, "%d", &stack[sp]);
            break;
        case SIO_HALT:
            return 1;
        default:
            internal_error(0);
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

