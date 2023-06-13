#include "./utils.h"

int period;
int x_size, y_size;
int sock;
int spd, period;

void work(char gardener, int y, int x, struct sockaddr_in* sendAddr) {
    char buffer[16];
    char rcvBuff[1];
    int bytesRcvd;
    struct { int x; int y; int gardener; } send_values;
    send_values.x = x;
    send_values.y = y;
    send_values.gardener = gardener;
    buffer[0] = GARDENER_START;
    memcpy(&(buffer[1]), &send_values, sizeof(send_values));

    do {
        if (sendto(sock, buffer, 13, 0, (struct sockaddr *) sendAddr, sizeof(*sendAddr)) != 13) {
            DieWithError("send() start failed");
        }
        if ((bytesRcvd = recvfrom(sock, rcvBuff, 1, 0, NULL, 0)) < 0) {
            DieWithError("recvfrom() failed");
        }
        if (bytesRcvd != 1) {
            DieWithError("recv() wrong data");
        }
        usleep(10 * 1000);
    }while (rcvBuff[0] == PROCESS);

    if (rcvBuff[0] == EMPTY) {
        usleep(spd * period * 1000);
    }
    buffer[0] = GARDENER_FINISH;
    if (sendto(sock, buffer, 13, 0, (struct sockaddr *) sendAddr, sizeof(*sendAddr)) != 13) {
        DieWithError("send() finish failed");
    }
    usleep(period * 1000);
}

void gardener1(struct sockaddr_in* sendAddr) {
    for (int y = 0; y < y_size; ++y) {
        for (int x = y % 2 ? x_size - 1 : 0; y % 2 ? x >= 0 : x < x_size;
             y % 2 ? --x : ++x) {
            printf("1 work with y=%d; x=%d\n", y, x);
            work(FIRST_GARDENER, y, x, sendAddr);
        }
    }
    printf("1 all\n");
}

void gardener2(struct sockaddr_in* sendAddr) {
    for (int x = x_size - 1; x >= 0; --x) {
        for (int y = (x_size - 1 - x) % 2 ? 0 : y_size - 1;
             (x_size - 1 - x) % 2 ? y < y_size : y >= 0;
             (x_size - 1 - x) % 2 ? ++y : --y) {
            printf("2 work with y=%d; x=%d\n", y, x);
            work(SECOND_GARDENER, y, x, sendAddr);
        }
    }
    printf("2 all\n");
}

int main(int argc, char *argv[]) {
    unsigned short recvServPort;
    char *servIP;
    char rcvBuff[13];
    char gardener;
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <Server IP> <Port> <N> <Speed>\n", argv[0]);
        exit(1);
    }

    servIP = argv[1];
    recvServPort = atoi(argv[2]);
    gardener = atoi(argv[3]);
    spd = atoi(argv[4]);
    if (gardener == 1) {
        gardener = FIRST_GARDENER;
    } else if (gardener == 2) {
        gardener = SECOND_GARDENER;
    } else {
        DieWithError("Uncorrect garder number");
    }

    struct sockaddr_in sendAddr;

    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    memset(&sendAddr, 0, sizeof(sendAddr));
    sendAddr.sin_family         = AF_INET;
    sendAddr.sin_addr.s_addr    = inet_addr(servIP);
    sendAddr.sin_port           = htons(recvServPort);

    char c = ADD_GARDENER;

    if (sendto(sock, &c, sizeof(c), 0, (struct sockaddr *) &sendAddr, sizeof(sendAddr)) != sizeof(c)) {
        DieWithError("send() add failed");
    }

    if (recvfrom(sock, rcvBuff, 12, 0, NULL, 0) < 0) {
        DieWithError("recvfrom() failed");
    }

    struct { int x_size; int y_size; int period; } init_values;
    memcpy (&init_values, rcvBuff, 12);
    x_size = init_values.x_size;
    y_size = init_values.y_size;
    period = init_values.period;

    if (gardener == FIRST_GARDENER) {
        gardener1(&sendAddr);
    } else if (gardener == SECOND_GARDENER) {
        gardener2(&sendAddr);
    }

    char sendBuff[2];
    sendBuff[0] = END_GARDENER;
    sendBuff[1] = gardener;
    if (sendto(sock, sendBuff, 2, 0, (struct sockaddr *) &sendAddr, sizeof(sendAddr)) != 2) {
        DieWithError("send() add failed");
    }

    close(sock);
}