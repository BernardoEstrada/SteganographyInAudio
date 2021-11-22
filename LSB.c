#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

typedef unsigned char uchar;
typedef unsigned long ulong;

int main(int argc, char *argv[]) {
    int origin, message, destination;
    uchar noOfBits;

    if (argc != 5) {
        printf("usage: %s origin message destination [No. of Bits to use in each byte (1, 2 or 4)]\n", argv[0]);
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

    if (noOfBits = atoi(argv[4]) && (noOfBits != 1 && noOfBits != 2 && noOfBits != 4)) {
        printf("%s: Number of bits must be 1, 2 or 4\n", argv[0]);
        return -2;
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

    uchar baseShift = (7-noOfBits/2);
    uchar iMod = 8/noOfBits;
    uchar byteMask = 15 >> (4-noOfBits);

    uchar inputMask = (255 >> noOfBits) << noOfBits;

    for (int i = 0, j = 0; i < fileSize; i++) {
        uchar byteWithout2LSB = (buffer[i+45] & 248);
        // To get 1, 2 and 4 significant bits from the message
        // uchar msgBits = messageBuffer[j] >> (7-(i%8)*1) & 1;
        // uchar msgBits = messageBuffer[j] >> (6-(i%4)*2) & 3;
        // uchar msgBits = messageBuffer[j] >> (4-(i%2)*4) & 15;

        uchar msgBits = (messageBuffer[j] >> (baseShift - ((i%iMod) * noOfBits))) & byteMask;
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