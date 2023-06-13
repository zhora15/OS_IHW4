#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define OBSTACLE '.'
#define EMPTY '0'
#define FIRST_GARDENER 'a'
#define SECOND_GARDENER 'b'
#define PROCESS 'p'

typedef struct {
  char state;
} cell;

enum COMMANDS {
  GARDENER_START = 1, GARDENER_FINISH = 14, ADD_GARDENER = 127, END_GARDENER = 33,
};

void output_field(char *outpMsg, int y_size, int x_size, cell *field) {
    outpMsg += sprintf(outpMsg, "Field:\n");
    for (int y = 0; y < y_size; ++y) {
        for (int x = 0; x < x_size; ++x) {
            outpMsg += sprintf(outpMsg, "%c ", field[y * x_size + x].state);
        }
        outpMsg += sprintf(outpMsg, "\n");
    }
}

void DieWithError(char *errorMessage) {
    perror(errorMessage);
    exit(1);
}
