//
// Created by Thomas on 7/11/2015.
//

#include <stdlib.h>
#include <stdio.h>
#include "errors.h"

char* error_statements[33] =
        {
                "Unknown error",                                          //0
                "Use = instead of :=.",                                   //1
                "= must be followed by a number.",                        //2
                "Identifier must be followed by =",                       //3
                "const, var, procedure must be followed by identifier.",  //4
                "Semicolon or comma missing.",                            //5
                "Incorrect symbol after procedure declaration.",          //6
                "Statement expected.",                                    //7
                "Incorrect symbol after statement part in block.",        //8
                "Period expected.",                                       //9
                "Semicolon between statements missing.",                  //10
                "Undeclared identifier.",                                 //11
                "Assignment to constant or procedure is not allowed.",    //12
                "Assignment operator expected.",                          //13
                "call must be followed by an identifier.",                //14
                "Call of a constant or variable is meaningless.",         //15
                "then expected.",                                         //16
                "Semicolon or } expected.",                               //17
                "do expected.",                                           //18
                "Incorrect symbol following statement.",                  //19
                "Relational operator expected.",                          //20
                "Expression must not contain a procedure identifier.",    //21
                "Right parenthesis missing.",                             //22
                "The preceding factor cannot begin with this symbol.",    //23
                "An expression cannot begin with this symbol.",           //24
                "This number is too large.",                              //25
                "begin must be closed with end",                          //26
                "Lexical error",                                          //27
                "Duplicate identifier within scope",                      //28
                "Use of semicolon after statement when not within begin/end", //29
                "else not contained within an if/then statement",         //30
                "end of file reach unexpectedly",                          //31
                "unable to open file"   //32

        };


typedef struct open_file_list
{
        FILE* file;
        struct open_file_list* next;

}open_file_list;
open_file_list* file_list;

FILE* openFile(const char* fileName, const char* mode)
{
        FILE* newFile = fopen(fileName, mode);
        if(newFile == NULL)
        {
                printf("Unable to open file: %s", fileName);
                error(32);
        }
        open_file_list* node = malloc(sizeof(open_file_list));
        node->file = newFile;
        node->next = NULL;

        if(file_list == NULL)
                file_list = node;
        else
        {
                open_file_list* temp = file_list;
                while(temp->next != NULL)
                        temp = temp->next;

                temp->next = node;
        }


        return newFile;
}


void closeFile(FILE* file)
{
        fflush(file);
        open_file_list* temp = file_list;
        open_file_list* prev = NULL;
        while(temp != NULL && temp->file != file)
        {
                prev = temp;
                temp = temp->next;
        }

        if(temp != NULL)
        {
                FILE* closeThisFile = temp->file;
                if(prev == NULL)
                        file_list = temp->next;
                else
                        prev->next = temp->next;

                fclose(closeThisFile);
        }

}

void cleanupFiles()
{
        open_file_list* temp = file_list;
        open_file_list* freeThis = temp;
        while(temp != NULL)
        {
                temp = temp->next;
                closeFile(freeThis->file);
                free(freeThis);
                freeThis = temp;
        }

}



void error(int num)
{

        cleanupFiles();
        fprintf(stderr, "Error number %d: %s\nCompile Process Failed. Exiting\n\n", num, error_statements[num]);
        fflush(stderr);

        exit(EXIT_FAILURE);
}
