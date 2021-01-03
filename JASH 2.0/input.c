#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "headers.h"

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

/************************************
Custom Shell: Reading a User's Input
************************************/
char *lsh_read_line(void) {
    char *line;
    ssize_t bufsize = 0;
    getline(&line, &bufsize, stdin);
    return line;
}


/************************************
Custom Shell: Spiltting I/P Commands
************************************/
char **lsh_split_line(char *line, char delim) {
    int count = 0;
    char *temp = malloc(1001 * sizeof(char));
    char **tokens = malloc(101 * sizeof(char *));
    strcpy(temp, line);
    char *temp_t = strtok(temp, &delim);

    while(temp_t != NULL) {
        tokens[count++] = temp_t;
        temp_t = strtok(NULL, &delim);
    }

    tokens[count] = NULL;
    return tokens;
}


/************************************
Custom Shell: Parsing Input Commands
************************************/
int lsh_parse_line(char *line, char *args[], int *type) {
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char *token;
    token = line;

    if(!token) {
        fprintf(stderr, "lsh_parse_line: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while(*token != '\0') {
        if(*token != ' ' && *token != '&') {
            ++position;
            *args = token;
            ++args;
        }

        while(*token != ' ' && *token != '\t' && *token != '\0' && *token != '\n') {
            if(*token == '&') {
                *type = 1;
            }
            ++token;
        }

        while(*token == ' ' || *token == '\t' || *token == '\n') {
            *token = '\0';
            ++token;
        }
    }

        *args = '\0';
        return position;
}