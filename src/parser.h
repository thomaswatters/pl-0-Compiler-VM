#include "scanner.h"
//
// Created by Thomas on 7/11/2015.
//

#ifndef MODULE3_PARSER_H
#define MODULE3_PARSER_H


#define MAX_SYMBOL_TABLE_SIZE 100


typedef enum
{
    constant = 1, variable, procedure
}symbol_type;


typedef struct symbol {
    symbol_type kind; // const = 1, var = 2, proc = 3
    char name[MAX_IDENTIFIER_LENGTH]; // name up to 11 chars
    int val; // number (ASCII value)
    int level; // L level
    int addr; // M address
} symbol;

typedef struct PARSER
{
    char* token_string;
    symbol symbol_table[MAX_SYMBOL_TABLE_SIZE];
    int symbols_stored;

} PARSER;


PARSER parse(const char* token_string);
void program();
void block();
void getToken();
void const_decl();
void var_decl();
void proc_decl();
void statement();
void expression();
void condition();
void term();
void factor();
symbol* enter(symbol_type type, char* ident, int val);
void emit(unsigned op, unsigned l, unsigned m);
symbol* findSymbolByLevel(char* ident);
#endif //MODULE3_PARSER_H
