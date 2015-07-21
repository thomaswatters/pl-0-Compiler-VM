#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "scanner.h"
#include "errors.h"

char *clean_input_name = "cleaninput.txt";

//Prototypes

int cleanFile(char *in_file_name);
void stripExtraSpaces(char *str);
int removeComments(char *str);
int getLine(char *buffer, char *seek);




/*
 * Parses the file having name in_file_name
 *
 */
void lexicalParser(char *in_file_name)
{
    cleanFile(in_file_name);
    // Load file into memory store in string
    FILE *f = openFile(clean_input_name, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    closeFile(f);

    string[fsize] = 0; // terminate string

    stripExtraSpaces(string);   // Remove spaces that dont contribute

    FILE *lexemeTable = openFile("lexemetable.txt", "w");
    FILE *lexemeList = openFile("lexemelist.txt", "w");


    fprintf(lexemeTable, "%-15s %-15s\n", "lexeme", "token type");

    char *cur_char;     // This will be our pointer that steps through stirng
    cur_char = string;
    int fault_detected = 0; // Flag for if a fault detected. If fault dected parsing halts
    int line_count = 1;
    token tok;
    tok.type = 0;
    tok.lexeme_str = "";

    while (*cur_char != '\0')
    {
        memset(&tok, 0, sizeof(token)); // Reset token data to 0s

        if (isalpha(*cur_char))  // Ident or Reserved Word
        {
            tok.type = identsym;
            tok.lexeme[strlen(tok.lexeme)] = *cur_char;
            cur_char++;
            while (*cur_char != '\0')
            {
                if (isalnum(*cur_char))  // It is ok for identfiers to have numbers
                {
                    if (strlen(tok.lexeme) >= MAX_IDENTIFIER_LENGTH)
                    {
                        fprintf(stderr, "Name too long\n");
                        fault_detected = 1;
                        break;
                    }
                    else
                    {
                        tok.lexeme[strlen(tok.lexeme)] = *cur_char;
                        cur_char++;
                    }
                }
                else
                {
                    break;
                }

            }   // End of identifier loop


            // Compare to reserved words
            if (strcmp("null", tok.lexeme) == 0) tok.type = nulsym;
            else if (strcmp("const", tok.lexeme) == 0) tok.type = constsym;
            else if (strcmp("var", tok.lexeme) == 0) tok.type = varsym;
            else if (strcmp("procedure", tok.lexeme) == 0) tok.type = procsym;
            else if (strcmp("call", tok.lexeme) == 0) tok.type = callsym;
            else if (strcmp("begin", tok.lexeme) == 0) tok.type = beginsym;
            else if (strcmp("end", tok.lexeme) == 0) tok.type = endsym;
            else if (strcmp("if", tok.lexeme) == 0) tok.type = ifsym;
            else if (strcmp("then", tok.lexeme) == 0) tok.type = thensym;
            else if (strcmp("else", tok.lexeme) == 0) tok.type = elsesym;
            else if (strcmp("while", tok.lexeme) == 0) tok.type = whilesym;
            else if (strcmp("do", tok.lexeme) == 0) tok.type = dosym;
            else if (strcmp("read", tok.lexeme) == 0) tok.type = readsym;
            else if (strcmp("write", tok.lexeme) == 0) tok.type = writesym;
            else if (strcmp("odd", tok.lexeme) == 0) tok.type = oddsym;
            else if (strcmp("return", tok.lexeme) == 0) tok.type = returnsym;
            else if (strcmp("param", tok.lexeme) == 0) tok.type = paramsym;


        }
        else if (isdigit(*cur_char))
        {
            tok.type = numbersym;   // Assume number
            tok.lexeme[strlen(tok.lexeme)] = *cur_char;
            cur_char++;
            while (*cur_char != '\0')
            {
                if (isdigit(*cur_char))
                {
                    if (strlen(tok.lexeme) >= MAX_NUMBER_LENGTH)
                    {
                        fprintf(stderr, "Name too long\n");
                        fault_detected = 1;
                        break;
                    }
                    else
                    {
                        tok.lexeme[strlen(tok.lexeme)] = *cur_char;
                        cur_char++;
                    }
                }
                else if (isalpha(*cur_char)) // This is a invalid identifier
                {
                    fprintf(stderr, "Variable does not start with a letter\n");
                    fault_detected = 1;
                    break;
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            tok.lexeme[strlen(tok.lexeme)] = *cur_char;
            switch (*cur_char)
            {
                case '+': tok.type = plussym;
                    break;
                case '-': tok.type = minussym;
                    break;
                case '*': tok.type = multsym;
                    break;
                case '/': tok.type = slashsym;
                    break;
                case '(': tok.type = lparentsym;
                    break;
                case ')': tok.type = rparentsym;
                    break;
                case '.': tok.type = periodsym;
                    break;
                case ';': tok.type = semicolonsym;
                    break;
                case ',': tok.type = commasym;
                    break;
                case '=': tok.type = eqsym;
                    break;
                case '<':
                {
                    cur_char++;
                    if (*cur_char == '>')
                    {  // '<>' not equal
                        tok.type = neqsym;
                        tok.lexeme[strlen(tok.lexeme)] = *cur_char;
                    }
                    else if (*cur_char == '=')
                    { // <= less than or equal
                        tok.type = leqsym;
                        tok.lexeme[strlen(tok.lexeme)] = *cur_char;
                    }
                    else
                    {
                        cur_char--;     // we need to rewind the pointer
                        tok.type = lessym;
                    }
                    break;
                }
                case '>':
                {
                    cur_char++;
                    if (*cur_char == '=')
                    {  // '>=' greater or equal to
                        tok.type = geqsym;
                        tok.lexeme[strlen(tok.lexeme)] = *cur_char;
                    }
                    else
                    {
                        cur_char--;     // reqind character
                        tok.type = gtrsym;
                    }
                    break;
                }
                case ':':
                {
                    cur_char++;
                    if (*cur_char == '=')
                    {      // ':='
                        tok.type = becomessym;
                        tok.lexeme[strlen(tok.lexeme)] = *cur_char;
                    }
                    else
                    {
                        fprintf(stderr, "Invalid symbol\n");
                        fault_detected = 1;
                    }
                    break;
                }
                default:
                {
                    if (iscntrl(*cur_char) || isspace(*cur_char))
                    {
                        if (*cur_char == '\n')
                        {
                            line_count++;
                        }
                        break;
                    }

                    fprintf(stderr, "Invalid symbol\n");
                    fault_detected = 1;
                    break;
                }

            }

            cur_char++;

        }

        if (fault_detected == 1)
        {
            fprintf(stderr, "Fault on line %d", line_count);
            error(27);
        }

        if (tok.type != 0)
        {

            fprintf(lexemeTable, "%-15s %-15d\n", tok.lexeme, tok.type);
            fprintf(lexemeList, "%d ", tok.type);
            if (tok.type == identsym || tok.type == numbersym)
            {
                fprintf(lexemeList, "%s ", tok.lexeme);
            }

        }
    }

    closeFile(lexemeList);
    closeFile(lexemeTable);

    free(string);
}



/**
 * Removes all comments
 * Clears whitespace down to just one space between
 * either a blank space or newline char
 *
 */
int cleanFile(char *in_file_name)
{
    FILE *f = openFile(in_file_name, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    closeFile(f);

    string[fsize] = 0;

    //stripExtraSpaces(string);

    int comments_removed;
    do
    {
        comments_removed = removeComments(string);
        if (comments_removed == -1)
        {
            return -1;
        }

    } while (comments_removed == 1);

    FILE *out = openFile(clean_input_name, "wb");
    fwrite(string, sizeof(char), strlen(string), out);
    fflush(out);
    closeFile(out);
    free(string);

    return 0;

}

/**
 * Remove white space preserving new line
 */
void stripExtraSpaces(char *str)
{
    int i, x;
    int len = strlen(str);
    for (i = x = 1; i < len; i++)
    {
        if (!isspace(str[i]) || str[i] == '\n' || (i > 0 && !isspace(str[i - 1])))
        {
            str[x++] = str[i];
        }
    }
    str[x] = '\0';
}

int removeComments(char *str)
{
    char *comment_start, *comment_stop;
    int flag = 0;

    char *beg_comment = "/*";
    char *end_comment = "*/";

    comment_start = strstr(str, beg_comment);
    if (comment_start != NULL)
    {
        flag = 1;

        comment_stop = strstr(comment_start, end_comment);
        if (comment_stop == NULL)
        {
            fprintf(stderr, "Invalid use of comment started on line: %d\n", getLine(str, comment_start));
            return -1;
        }


        char *temp_ptr = comment_stop + 2;
        *comment_start = ' ';
        *(comment_start + 1) = 0;

        memmove(comment_start+1, temp_ptr, strlen(temp_ptr) + 1);
      //  strcat(str, temp_ptr);
    }

    return flag;
}


int getLine(char *buffer, char *seek)
{
    int lines = 1;
    while (buffer != 0 && buffer != seek)
    {
        if (*buffer == '\n')
        {
            lines++;
        }

        buffer++;
    }

    return lines;
}
