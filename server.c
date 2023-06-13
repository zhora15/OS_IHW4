#include "utils.h"

int servSock;
int period;
int x_size, y_size;
cell *field;


void stop() {
    close(servSock);
    free(field);
    printf("Deallocated\n");
}

void handle_sigint(int sig) {
    stop();
    exit(0);
}

void HandleUDPClient() {
    char rcvBuff[16];
    int recvMsgSize;
    char outpMsg[1001];
    int init_values[3] = {x_size, y_size, period};
    struct sockaddr_in echoClntAddr;
    unsigned int cliAddrLen = sizeof(echoClntAddr);

    if ((recvMsgSize = recvfrom(servSock, rcvBuff, 13, 0, (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0) {
        DieWithError("recv() loop failed");
    }
    if (recvMsgSize == 1 && rcvBuff[0] == ADD_GARDENER) {
        snprintf(outpMsg, 1000, "Gardener connected\n");
        if (sendto(servSock, init_values, sizeof(init_values), 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(init_values)) {
            DieWithError("send() init failed");
        }
    } else if (recvMsgSize == 13 && rcvBuff[0] == GARDENER_START) {
        char res = EMPTY;
        struct {
          int x;
          int y;
          int gardener;
        } recv_values;
        memcpy(&recv_values, rcvBuff + 1, sizeof(recv_values));
        
        if (field[recv_values.y * x_size + recv_values.x].state == EMPTY) {
            field[recv_values.y * x_size + recv_values.x].state = PROCESS;
        } else {
            res = field[recv_values.y * x_size + recv_values.x].state;
        }
        if (res != PROCESS){
            snprintf(outpMsg, 1000, "%d work with y=%d; x=%d\n", recv_values.gardener == FIRST_GARDENER ? 1 : 2, recv_values.y, recv_values.x);
        } else {
            outpMsg[0] = '\0'; 
        }
        if (sendto(servSock, &res, 1, 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != 1) {
            DieWithError("send() start failed");
        }
    } else if (recvMsgSize == 13 && rcvBuff[0] == GARDENER_FINISH) {
        output_field(outpMsg, y_size, x_size, field);
        struct {
          int x;
          int y;
          int gardener;
        } recv_values;
        memcpy(&recv_values, rcvBuff + 1, sizeof(recv_values));
        field[recv_values.y * x_size + recv_values.x].state = recv_values.gardener;
    } else if ((recvMsgSize == 2) && (rcvBuff[0] == END_GARDENER)) {
        char gardener = rcvBuff[1];
        snprintf(outpMsg, 1000, "%d gardener finished working", gardener == FIRST_GARDENER ? 1 : 2);
        puts(outpMsg);
        exit(0);
    }
    if (outpMsg[0]){
        puts(outpMsg);
    }
}


void SIGIOHandler(int signalType) {
    HandleUDPClient();
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_sigint);
    srand(time(NULL));
    if (argc != 2) {
        fprintf(stderr, "Usage:  %s <SERVER PORT>\n", argv[0]);
        exit(1);
    }
    unsigned short port = atoi(argv[1]);

    printf("Введите сколько мс будет принято за единицу отсчета: ");
    scanf("%d", &period);
    if (period < 1) {
        DieWithError("Incorrect input. Finished...");
    }

    printf("Введите размер поля по двум координатам: ");
    scanf("%d %d", &x_size, &y_size);
    if ((x_size < 1) || (y_size < 1)) {
        DieWithError("Incorrect input. Finished...");
    }

    struct sockaddr_in servAddr;

    if ((servSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family         = AF_INET;
    servAddr.sin_addr.s_addr    = htonl(INADDR_ANY);
    servAddr.sin_port           = htons(port);

    if (bind(servSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        DieWithError("bind() failed");

    field = malloc(x_size * y_size * sizeof(cell));

    for (int y = 0; y < y_size; ++y) {
        for (int x = 0; x < x_size; ++x) {
            field[y * x_size + x].state = EMPTY;
        }
    }

    int amount_obst = (int) ((0.1 * x_size * y_size + rand() % x_size * y_size * 0.2));
    while (amount_obst > 0) {
        int x = rand() % x_size;
        int y = rand() % y_size;
        if (field[y * x_size + x].state == EMPTY) {
            field[y * x_size + x].state = OBSTACLE;
            --amount_obst;
        }
    }

    struct sigaction handler;
    handler.sa_handler = SIGIOHandler;
    if (sigfillset(&handler.sa_mask) < 0) 
        DieWithError("sigfillset() failed");
    handler.sa_flags = 0;

    if (sigaction(SIGIO, &handler, 0) < 0)
        DieWithError("sigaction() failed for SIGIO");

    if (fcntl(servSock, F_SETOWN, getpid()) < 0)
        DieWithError("Unable to set process owner to us");

    if (fcntl(servSock, F_SETFL, O_NONBLOCK | FASYNC) < 0)
        DieWithError("Unable to put client sock into non-blocking/async mode");


    for(;;){

    }
    stop();
    exit(0);
}