//
// Created by Thomas on 7/11/2015.
//

#ifndef MODULE2_SCANNER_H
#define MODULE2_SCANNER_H


#define MAX_IDENTIFIER_LENGTH 11
#define MAX_NUMBER_LENGTH 5



//Prototypes
int cleanFile(char *in_file_name);
void stripExtraSpaces(char *str);
int removeComments(char *str);
int getLine(char *buffer, char *seek);
void lexicalParser(char *in_file_name);


// DFA
typedef enum {
    nulsym = 1, identsym, numbersym, plussym, minussym,
    multsym, slashsym, oddsym, eqsym, neqsym, lessym, leqsym,
    gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
    periodsym, becomessym, beginsym, endsym, ifsym, thensym,
    whilesym, dosym, callsym, constsym, varsym, procsym, writesym,
    readsym, elsesym, returnsym
} token_type;

struct {
    token_type type;
    char lexeme[12];
    char* lexeme_str;
} typedef token;



#endif //MODULE2_SCANNER_H
