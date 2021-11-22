#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

// Default header size for WAV files
int HEADER_SIZE = 45;

typedef unsigned char uchar;
typedef unsigned long ulong;

int encode_msg(char *filename, char *originPath, char *msg, char *destinationPath, char *argBits);
int encode_file(char *filename, char *originPath, char *messagePath, char *destinationPath, char *argBits);
int encode(char *filename, char *originPath, char *messagePath, char *destinationPath, char *argBits);

int decode_out(char *filename, char *originPath, char *destinationPath, char *argBits);
int decode_print(char *filename, char *originPath, char *argBits);
int decode(char *filename, char *originPath, char *argBits, ulong *outSize, uchar **outBufferPtr);

int flagErrorFunc(char *flag, char *filename);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return flagErrorFunc("n/a", argv[0]);
    }

    if (strcmp(argv[1], "-em") == 0 && argc != 6)
        return encode(argv[0], argv[2], argv[3], argv[4], argv[5]);
    if (strcmp(argv[1], "-ef") == 0 && argc != 6)
        return encode(argv[0], argv[2], argv[3], argv[4], argv[5]);
    if (strcmp(argv[1], "-df") == 0 && argc != 5)
        return decode_out(argv[0], argv[2], argv[3], argv[4]);
    if (strcmp(argv[1], "-dp") == 0 && argc == 4)
        return decode_print(argv[0], argv[2], argv[3]);

    return flagErrorFunc(argv[1], argv[0]);
}

int flagErrorFunc(char *flag, char *filename){
    if(strcmp(flag, "-em")){
        printf("usage: %s -em origin message destination <Bits per Byte>\n", filename);
        return -2;
    }
    if(strcmp(flag, "-ef")){
        printf("usage: %s -ef origin messagePath destination <Bits per Byte>\n", filename);
        return -2;
    }
    if(strcmp(flag, "-dp")){
        printf("usage: %s -dp origin <Bits per byte>\n", filename);
        return -2;
    }
    if(strcmp(flag, "-df")) {
        printf("usage: %s -df origin output <Bits per byte>\n", filename);
        return -2; 
    }

    printf("usage: \n");
    printf("%s < -ef | -em > origin < messagePath | message > destination <Bits to use in each byte (1, 2 or 4)>\n", filename);
    printf("%s < -df | -dm > origin [output] <Bits per byte>\n", filename);
    printf("Type \"%s -h\" for more information \n", filename);
    return -2;
}

int encode_msg(char *filename, char *originPath, char *msg, char *destinationPath, char *argBits) {
    return 0;
}

int encode_file(char *filename, char *originPath, char *messagePath, char *destinationPath, char *argBits) {
    int message;
    if ((message = open(messagePath, O_RDONLY)) < 0) {
        perror(messagePath);
        return -3;
    }
}

int encode(char *filename, char *originPath, char *msg, char *destinationPath, char *argBits) {
    int origin, destination;
    uchar noOfBits;

    if ((origin = open(originPath, O_RDONLY)) < 0) {
        perror(originPath);
        return -3;
    }

    // if ((message = open(messagePath, O_RDONLY)) < 0) {
    //     perror(messagePath);
    //     return -3;
    // }

    if ((destination = open(destinationPath, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
        perror(destinationPath);
        return -3;
    }

    if ((noOfBits = atoi(argBits)) && (noOfBits != 1 && noOfBits != 2 && noOfBits != 4)) {
        printf("%s: Number of bits must be 1, 2 or 4\n", filename);
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

    uchar baseShift = (8-noOfBits);
    uchar iMod = 8/noOfBits;
    uchar byteMask = 15 >> (4-noOfBits);

    uchar inputMask = (255 >> noOfBits) << noOfBits;

    if(messageSize*(8/noOfBits) > fileSize) {
        printf("%s: Message is too long for file %s\n", filename, originPath);
        return -2;
    }

    for (
        int i = 0, fileByte = HEADER_SIZE, msgByte = 0;
        fileByte < fileSize;
        i++, fileByte++
    ) {
        uchar byteWithout2LSB = (buffer[fileByte] & inputMask);
        // To get 1, 2 and 4 significant bits from the message
        // uchar msgBits = messageBuffer[msgByte] >> (7-(i%8)*1) & 1;
        // uchar msgBits = messageBuffer[msgByte] >> (6-(i%4)*2) & 3;
        // uchar msgBits = messageBuffer[msgByte] >> (4-(i%2)*4) & 15;

        printf("%c", messageBuffer[msgByte]);
        uchar msgBits = (messageBuffer[msgByte] >> (baseShift - ((i%iMod) * noOfBits))) & byteMask;
        if(i%8 == 0 && i!=0) msgByte++;   // move to next byte in message
        buffer[fileByte] = byteWithout2LSB | msgBits;
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


int decode_out(char *filename, char *originPath, char *destinationPath, char *argBits){
    int destination;
    ulong fileSize;
    
    uchar *outBuffer = NULL;
    
    int res = decode(filename, originPath, argBits, &fileSize, &outBuffer);

    if ((destination = open(destinationPath, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
        perror(destinationPath);
        return -3;
    }
    write(destination, outBuffer, fileSize);

    close(destination);
    free(outBuffer);
    return 0;
}


int decode_print(char *filename, char *originPath, char *argBits){
    ulong fileSize;
    uchar *outBuffer = NULL;

    int res = decode(filename, originPath, argBits, &fileSize, &outBuffer);
    if(res == 0) printf("%s\n", outBuffer);

    free(outBuffer);
    return res;
}

int decode(char *filename, char *originPath, char *argBits, ulong *outSize, uchar **outBufferPtr) {
    int origin, destination;
    uchar noOfBits;

    if ((origin = open(originPath, O_RDONLY)) < 0) {
        perror(originPath);
        return -3;
    }

    if ((noOfBits = atoi(argBits)) && (noOfBits != 1 && noOfBits != 2 && noOfBits != 4)) {
        printf("%s: Number of bits must be 1, 2 or 4\n", filename);
        return -2;
    }

    uchar *buffer;
    uchar *outBuffer = *outBufferPtr;
    ulong fileSize = lseek(origin, 0, SEEK_END);
    buffer = (uchar *)malloc(sizeof(uchar) * fileSize);
    *outSize = (fileSize-HEADER_SIZE)*noOfBits/8;
    outBuffer = ((uchar *)malloc(sizeof(uchar) * (*outSize)));


    lseek(origin, 0, SEEK_SET);
    read(origin, buffer, fileSize);

    uchar byteMask = 15 >> (4-noOfBits);

    uchar msgByteContent = 0;

    for (
        int i = 0, fileByte = HEADER_SIZE, msgByte = 0;
        fileByte < fileSize;
        i++, fileByte++
    ) {
        uchar LSBs = (buffer[fileByte] & byteMask);
        msgByteContent = (msgByteContent << noOfBits) | LSBs;
        if(i%8 == 0 && i!=0) {
            msgByteContent = (msgByteContent >> noOfBits);
            outBuffer[msgByte] = msgByteContent;
            if(msgByteContent == '\0') {
                *outSize = msgByte;
                break;
            };
            msgByte++;
            msgByteContent = 0;
        };
    }

    *outBufferPtr = outBuffer;
    free(buffer);
    close(origin);
    close(destination);
    return 0;
}