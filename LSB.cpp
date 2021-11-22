#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

typedef unsigned char uchar;
typedef unsigned long ulong;

int main(int argc, char *argv[]) {
    int origin, message, destination;

    if (argc != 4) {
        printf("usage: %s origin message destination\n", argv[0]);
        return -2;
    }

    if ((origin = open(argv[1], O_RDONLY)) < 0) {
        perror(argv[1]);
        return -3;
    }

    if ((message = open(argv[2], O_RDONLY)) < 0) {
        perror(argv[2]);
        return -3;
    }

    if ((destination = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
        perror(argv[3]);
        return -3;
    }

    int bytes;
    uchar *buffer;
    uchar *messageBuffer;

    ulong fileSize = lseek(origin, 0, SEEK_END);
    buffer = (uchar *)malloc(sizeof(uchar) * fileSize);


    ulong messageSize = lseek(message, 0, SEEK_END);
    messageBuffer = (uchar *)malloc(sizeof(uchar) * messageSize);

    lseek(origin, 0, SEEK_SET);
    read(origin, buffer, fileSize);

    lseek(message, 0, SEEK_SET);
    read(message, messageBuffer, messageSize);

    for (int i = 0, j = 0; i < fileSize; i++) {
        uchar byteWithout2LSB = (buffer[i+45] & 248);
        // use 1 lsb
        // uchar msgBit = messageBuffer[j] >> (7-i%8) & 1;
        // use 2 lsb
        // uchar msgBits = messageBuffer[j] >> (6-(i%4)*2) & 3;
        // use 4 lsb
        uchar msgBits = messageBuffer[j] >> (4-(i%2)*4) & 7;
        if(i%8 == 0) j++;   // move to next byte in message
        buffer[i+45] = byteWithout2LSB | msgBits;
    }

    write(destination, buffer, fileSize);
    printf("done\n");

    free(buffer);
    free(messageBuffer);
    close(origin);
    close(message);
    close(destination);
    return 0;
}