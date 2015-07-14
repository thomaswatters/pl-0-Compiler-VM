//
// Created by Thomas on 7/11/2015.
//

#ifndef MODULE3_ERRORS_H
#define MODULE3_ERRORS_H

#include <stdio.h>

void error(int num) __attribute__((noreturn));
void closeFile(FILE* file);
FILE* openFile(const char* fileName, const char* mode);

#endif //MODULE3_ERRORS_H
