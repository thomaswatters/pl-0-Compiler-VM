#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "vm.h"
#include "errors.h"


int option_show_lexeme = 0;
int option_show_assembly = 0;
int option_show_stacktrace = 0;
char* program_name;
char* source_code_file;

void usageStatement()
{

    printf("Usage:\n\t%s [optional flags] [input file]\n", program_name);
    printf("\tOptional Flags:\n\t-l Print the list of lexemes/tokens to the screen\n");
    printf("\t-a Print the generated assebly code\n");
    printf("\t-v Print the virtual machine\n\n");
    printf("\t Input file - file containing source code of program to process");

}

void processParams(char* str)
{
    if(*str == '-')
    {
        //process a flag
        switch(*(str+1))
        {
            case 'l':
                option_show_lexeme = 1;
                break;
            case 'a':
                option_show_assembly = 1;
                break;
            case 'v':
                option_show_stacktrace = 1;
                break;
            default:
                printf("Not a valid flag: %c\n\n", *(str+1));
                usageStatement();
                exit(0);

        }
    }
    else
    {
        if(source_code_file == NULL)
            source_code_file = str;
        else
        {
            printf("Invalid parameters given\n\n");
            usageStatement();
            exit(0);
        }
    }

}

void printLexeme(FILE* outfile)
{

    FILE *f = openFile("lexemelist.txt", "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    closeFile(f);

    string[fsize] = 0;


    fprintf(outfile, "\nLexeme List:\n\n");
    fwrite(string, fsize, 1, outfile);

    free(string);

}


void printInputFile(char* file_name, FILE* outfile)
{
    FILE *f = openFile(file_name, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    closeFile(f);

    string[fsize] = 0;

    fprintf(outfile, "\nInput:\n\n");
    fwrite(string, fsize, 1, outfile);

    free(string);

}
void printAssembly(FILE* outfile)
{
    FILE *f = openFile("mcode.txt", "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    closeFile(f);

    string[fsize] = 0;

    fprintf(outfile, "\nAssembly Code:\n\n");
    fwrite(string, fsize, 1, outfile);

    free(string);
}


void printStackTrace(FILE* outfile)
{
    FILE *f = openFile("stacktrace.txt", "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    closeFile(f);

    string[fsize] = 0;


    fprintf(outfile, "\nStack Trace:\n\n");
    fwrite(string, fsize, 1, outfile);

    free(string);
}

int main(int argc, char** argv)
{
    program_name = argv[0];
    int i = 1;
    for(i = 1; i < argc; i++)
    {
        processParams(argv[i]);
    }

    if(source_code_file == NULL)
        source_code_file = "input.txt";

    lexicalParser(source_code_file);

    // Load file into memory store in string
    FILE *f = openFile("lexemelist.txt", "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    closeFile(f);

    string[fsize] = 0;

    parse(string);
    free(string);

    fprintf(stdout, "Program is syntactically correct\n\n");


    fprintf(stdout, "Program Running\n");
    fprintf(stdout, "Console:\n");

    VM_SETTINGS vm_settings =
            {
                    .max_stack_height = 1000,
                    .output_detailed_instructions_to_file = true,
                    .output_stacktrace_to_file = true,
                    .max_code_length = 1000,
                    .max_lexi_levels = 200
            };

    startVM("mcode.txt", &vm_settings);

    fprintf(stdout, "Program finished successfully\n\n");


//
//    if(option_show_lexeme)
//        printLexeme(stdout);
//    if(option_show_assembly)
//        printAssembly(stdout);
//    if(option_show_stacktrace)
//        printStackTrace(stdout);
//
//    FILE* output = openFile("output.txt", "w");
//    printInputFile(source_code_file, output);
//    fprintf(output, "\n\nProgram is syntactically correct\n\n");
//    printAssembly(output);
//    printStackTrace(output);
//    closeFile(output);



    return 0;
}