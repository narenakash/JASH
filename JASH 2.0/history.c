#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "headers.h"

#define HISTORY_MAX_SIZE 20

extern char *history[HISTORY_MAX_SIZE];
extern unsigned int hCount;

void add_history(char *command) {
    if(hCount < HISTORY_MAX_SIZE) {
        history[hCount++] = strdup(command);
    } else {
        free(history[0]);
        for(int i = 0; i < HISTORY_MAX_SIZE; i++) {
            history[i-1] = history[i];
        }
        history[HISTORY_MAX_SIZE-1] = strdup(command);
    }
}