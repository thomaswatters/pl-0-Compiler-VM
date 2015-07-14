//
// Created by Thomas on 7/11/2015.
//

#include "parser.h"
#include "vm.h"
#include "errors.h"

#include <string.h>
#include <stdlib.h>

#define CODE_SIZE 10000

PARSER parser_struct;
token current_token;

int level, mod; //memory address stuff

instruction code[CODE_SIZE];
int cx; // code index

PARSER parse(const char* token_string)
{
    parser_struct.token_string = malloc(sizeof(char)*(strlen(token_string) + 1));
    strcpy(parser_struct.token_string, token_string);
    parser_struct.symbols_stored = 0;
    level = -1;
    mod=0;
    cx = 0;
    program();

    free(parser_struct.token_string);

    FILE* f = openFile("mcode.txt", "w");
    int i = 0;
    for(i = 0; i < cx; i++)
    {
        fprintf(f, "%d %d %d\n", code[i].OP, code[i].L, code[i].M );
    }

    fflush(f);
    closeFile(f);

    return parser_struct;

}


void getToken() {
    static int first_call = 1;
    if (first_call) {
        current_token.lexeme_str = strtok(parser_struct.token_string, " ");
        first_call = 0;
    }
    else
        current_token.lexeme_str = strtok(NULL, " ");

    if (current_token.lexeme_str == NULL)
        error(31);


    current_token.type = atoi(current_token.lexeme_str);
}

symbol* enter(symbol_type type, char* ident, int val)
{
    symbol* sym = findSymbolByLevel(ident);
    if(sym != NULL && sym->level == level) {
        fprintf(stderr, "%s in level %d already defined\n", ident, level);
        error(28);
    }

    int idx = parser_struct.symbols_stored;
    symbol* entry = &parser_struct.symbol_table[idx];

    entry->kind = type;
    strcpy(entry->name, ident);
    entry->level = level;
    entry->val = val;
    if(type == procedure)
        entry->addr = cx;
    else if( type == variable)
        entry->addr = mod++;

    parser_struct.symbols_stored++;

    return entry;

}

void program()
{
    getToken();
    block();
    if(current_token.type != periodsym)
        error(9);

    //Emit code for halting the machine
    emit(SIO, 0, SIO_HALT);


}
void block()
{
    int prev_mod = mod;
    level++;
    mod = 4;
    if(current_token.type == constsym)  const_decl();
    if(current_token.type == varsym) var_decl();
    if(current_token.type == procsym) proc_decl();


    emit(INC, 0,mod);

    statement();
    level--;
    mod = prev_mod;

}

void const_decl()
{
    char* identifier;
    int value;
    do
    {
        getToken();
        if(current_token.type != identsym)
            error(4);

        identifier = strtok(NULL, " ");

        getToken();
        if(current_token.type != eqsym)
            error(3);

        getToken();
        if(current_token.type != numbersym)
            error(2);

        value = atoi(strtok(NULL, " "));

        enter(constant, identifier, value);

        getToken();

    }while (current_token.type == commasym);
    if(current_token.type != semicolonsym)
        error(5);


    getToken();
}

void var_decl()
{
    char* identifier;
    int count = 0;
    do
    {
        getToken();
        if(current_token.type != identsym)
            error(4);

        identifier = strtok(NULL, " ");

        enter(variable, identifier, 0);
        count++;
        getToken();

    }while (current_token.type == commasym);
    if(current_token.type != semicolonsym)
        error(5);

    getToken();

}


void proc_decl()
{
    char* identifier;

    int tempc = cx;
    emit(JMP, 0, 0);

    getToken();
    if(current_token.type != identsym)
        error(6);

    identifier = strtok(NULL, " ");
    enter(procedure, identifier, 0);

    getToken();
    if(current_token.type != semicolonsym)
        error(17);

    getToken();
    block();
    if(current_token.type != semicolonsym)
        error(17);

    getToken();

    if(current_token.type == procsym)
        proc_decl();


    emit(OPR, 0, 0);
    code[tempc].M = cx;



}



void statement()
{
    if(current_token.type == identsym)
    {
        char* identifier = strtok(NULL, " "); // get name
        symbol* sym = findSymbolByLevel(identifier);
        if(sym == NULL)
            error(11);

        getToken();
        if(current_token.type != becomessym)
            error(13);


        getToken();
        if(sym->kind != variable)
            error(12);



        expression();
        emit(STO, level - sym->level, sym->addr);
    }
    else if( current_token.type == callsym)
    {
        getToken();
        if(current_token.type != identsym)
            error(14);

        symbol* sym = findSymbolByLevel(strtok(NULL, " ")); // get procedure symbol
        if(sym == NULL)
            error(11);
        if(sym->kind != procedure)
            error(15);


        emit(CAL, level - sym->level, sym->addr);

        getToken();

    }
    else if( current_token.type == beginsym )
    {
        getToken();
        statement();
        while( current_token.type == semicolonsym)
        {
            getToken();
            statement();
        }

        if(current_token.type != endsym)
            error(10);

        getToken();
    }
    else if( current_token.type == ifsym)
    {
        getToken();
        condition();
        if(current_token.type != thensym)
            error(16);

        getToken();
        int ctemp = cx;
        emit(JPC, 0, 0);
        statement();
        code[ctemp].M = cx;

        if(current_token.type == elsesym)
        {
            int temp2 = cx;
            emit(JMP, 0, 0);
            code[ctemp].M = cx;
            getToken();
            statement();
            code[temp2].M = cx;

        }

    }
    else if(current_token.type == elsesym)
    {

    }

    else if(current_token.type == whilesym)
    {
        int cx1 = cx;
        getToken();
        condition();
        int cx2 = cx;
        emit(JPC,0,0);

        if(current_token.type != dosym)
            error(18);
        getToken();
        statement();

        emit(JMP,0,cx1);
        code[cx2].M = cx;

    }
    else if( current_token.type == readsym)
    {
        getToken();
        if(current_token.type != identsym)
            error(0);

        char* ident = strtok(NULL, " ");

        symbol* sym= findSymbolByLevel(ident);
        if(sym == NULL)
            error(11);

        emit(SIO, 0, SIO_READ);
        emit(STO, level - sym->level, sym->addr);

        getToken();
        //TODO: implement read from input
    }
    else if( current_token.type == writesym)
    {
        getToken();
        if(current_token.type != identsym)
            error(0);

        char* ident = strtok(NULL, " ");

        symbol* sym= findSymbolByLevel(ident);
        if(sym == NULL)
            error(11);

        if(sym->kind == variable)
            emit(LOD, level - sym->level, sym->addr);
        else if(sym->kind == constant)
            emit(LIT, 0, sym->val);
        else
            error(0);

        emit(SIO, 0, SIO_PRINT);
        //TODO: implement write to output

        getToken();
    }

}


void expression()
{
    int addop = 0;
    if(current_token.type == plussym || current_token.type == minussym)
    {
        addop = current_token.type;
        getToken();
        term();
        if(addop == minussym);
            emit(OPR, 0, OPR_NEG);
    }
    else
    {
        term();
    }

    while(current_token.type == plussym || current_token.type == minussym)
    {
        addop = current_token.type;
        getToken();
        term();
        if(addop == plussym)
            emit(OPR, 0, OPR_ADD);
        else
            emit(OPR, 0, OPR_SUB);
    }

}

int isRelationOp(token_type type)
{
    switch(type)
    {
        case eqsym:
        case geqsym:
        case leqsym:
        case neqsym:
        case lessym:
        case gtrsym:
            return 1;
        default:
            return 0;

    }
}


OPR_CODES getOPRCode(token_type type)
{
    switch(type)
    {
        case eqsym:
            return OPR_EQL;
        case geqsym:
            return OPR_GEQ;
        case leqsym:
            return OPR_LEQ;
        case neqsym:
            return OPR_NEQ;
        case lessym:
            return OPR_LSS;
        case gtrsym:
            return OPR_GTR;
        default:
            return 0;

    }
}



void condition()
{
    if(current_token.type == oddsym )
    {
        getToken();
        expression();

        emit(OPR, 0, OPR_ODD);
    }
    else
    {
        expression();
        if(!isRelationOp(current_token.type))
            error(20);

        OPR_CODES opr_code = getOPRCode(current_token.type);


        getToken();
        expression();
        emit(OPR, 0, opr_code);

    }

}


void term()
{
    int mulop;
    factor();
    while(current_token.type == multsym || current_token.type == slashsym)
    {
        mulop = current_token.type;
        getToken();
        factor();
        if(mulop == multsym)
            emit(OPR, 0, OPR_MUL);
        else
            emit(OPR, 0, OPR_DIV);
    }
}


void factor()
{
    if(current_token.type == identsym)
    {
        char* identifier = strtok(NULL, " ");   //Get the identifier name
        symbol* sym = findSymbolByLevel(identifier);
        if(sym == NULL)
            error(11);
        if(sym->kind == procedure)
            error(21);

        if(sym->kind == variable)
            emit(LOD, level - sym->level, sym->addr);
        else if(sym->kind == constant)
            emit(LIT, 0, sym->val);

        getToken();

    }
    else if( current_token.type == numbersym)
    {
        int number = atoi(strtok(NULL, " ")); // clear number from token string
        getToken();
        emit(LIT, 0, number);
    }
    else if( current_token.type == lparentsym)
    {
        getToken();
        expression();
        if(current_token.type != rparentsym)
            error(22);

        getToken();

    }
    else
    {
        error(23);
    }


}

symbol* findSymbolByLevel(char* ident)
{
    int i = 0;
    symbol* res = NULL;
    symbol* temp;
    for(i = 0; i < parser_struct.symbols_stored; i++ )
    {
        if(strcmp(ident, parser_struct.symbol_table[i].name) == 0)
        {
            temp = &parser_struct.symbol_table[i];
            if(temp->level <= level) {
                if (res == NULL)
                    res = temp;
                else if (res->level < temp->level) {
                    res = temp;
                }

            }
        }

    }

    return res;
}

void emit(unsigned op, unsigned l, unsigned  m)
{
    if( cx > CODE_SIZE )
        error(25);
    else
    {

        if(op == INC && cx >= 1 && code[cx - 1].OP == INC)
        {
                code[cx - 1].M += m;
        }
        else {
            code[cx].OP = op;
            code[cx].L = l;
            code[cx].M = m;
            cx++;
        }
    }

}