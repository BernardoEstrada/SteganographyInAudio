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

int embed_msg(
    char *filename,
    char *originPath,
    char *msg,
    char *destinationPath,
    char *argBits
);
int embed_file(
    char *filename,
    char *originPath,
    char *messagePath,
    char *destinationPath,
    char *argBits
);
int embed(
    char *filename,
    char *originPath,
    uchar *messageBuffer,
    ulong messageSize,
    char *destinationPath,
    char *argBits
);

int extract_out(
    char *filename,
    char *originPath,
    char *destinationPath,
    char *argBits
);
int extract_print(
    char *filename,
    char *originPath,
    char *argBits
);
int extract(
    char *filename,
    char *originPath,
    char *argBits,
    ulong *outSize,
    uchar **outBufferPtr
);

int flagErrorFunc(char *flag, char *filename);
int helpFunc(char *filename);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return flagErrorFunc("n/a", argv[0]);
    }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
        return helpFunc(argv[0]);
    if (strcmp(argv[1], "-em") == 0 && argc == 6)
        return embed_msg(argv[0], argv[2], argv[3], argv[4], argv[5]);
    if (strcmp(argv[1], "-ef") == 0 && argc == 6)
        return embed_file(argv[0], argv[2], argv[3], argv[4], argv[5]);
    if (strcmp(argv[1], "-xf") == 0 && argc == 5)
        return extract_out(argv[0], argv[2], argv[3], argv[4]);
    if (strcmp(argv[1], "-xp") == 0 && argc == 4)
        return extract_print(argv[0], argv[2], argv[3]);

    return flagErrorFunc(argv[1], argv[0]);
}

int flagErrorFunc(char *flag, char *filename){
    if(strcmp(flag, "-em") == 0){
        printf("usage: %s -em origin message destination <Bits per Byte>\n", filename);
        return -2;
    }
    if(strcmp(flag, "-ef") == 0){
        printf("usage: %s -ef origin messagePath destination <Bits per Byte>\n", filename);
        return -2;
    }
    if(strcmp(flag, "-xp") == 0){
        printf("usage: %s -xp origin <Bits per byte>\n", filename);
        return -2;
    }
    if(strcmp(flag, "-xf") == 0) {
        printf("usage: %s -xf origin output <Bits per byte>\n", filename);
        return -2;
    }

    printf("unknown option: %s\n", flag);
    printf("usage: %s ", filename);
    printf("{-ef | -em} origin {messagePath | message} destination <Bits per byte>\n");
    printf("%*c{-xf | -xp} origin [output] <Bits per byte>\n", (int)strlen(filename)+8, ' ');
    printf("Type \"%s -h\" for more information \n", filename);
    return -2;
}

int helpFunc(char *filename){
    printf("usage: ");
    printf("%s { -em | -ef | -xp | -xf | -h } [origin] [{messagePath|message}] [destination] [<Bits per byte>]\n", filename);

    printf("\nFlags:\n");
    printf("\t-em\tEmbed into wav from text input\n");
    printf("\t-ef\tEmbed into wav from file\n");
    printf("\t-xp\tExtract from wav and print to stdout\n");
    printf("\t-xf\tExtract from wav and output to file\n");
    printf("\t-h\tPrint this help message\n");

    printf("\nAttributes:\n");
    printf("\torigin\t\tPath to the wav file to be embedded/extracted\n");
    printf("\tmessagePath\tPath to the text file to be embedded\n");
    printf("\tmessage\t\tText to be embedded\n");
    printf("\tdestination\tPath to the wav file to be embedded/extracted\n");
    printf("\tBits per byte\tBits per byte to use in the encoding/decoding (1, 2 or 4)\n");
    
    printf("\nExamples:\n");
    printf("\t%s -em ./test.wav \"Hello World\" ./test_out.wav 4\n", filename);
    printf("\t%s -ef ./test.wav ./test_msg.txt ./test_out.wav 4\n", filename);
    printf("\t%s -xp ./test.wav 4\n", filename);
    printf("\t%s -xf ./test.wav ./test_out.wav 4\n", filename);

    return 0;
}

int embed_msg(char *filename, char *originPath, char *msg, char *destinationPath, char *argBits) {
    uchar *messageBuffer;
    ulong messageSize;

    messageSize = strlen(msg);
    messageBuffer = (uchar *)malloc(sizeof(uchar) * messageSize);

    memccpy(messageBuffer, msg, 0, messageSize);

    printf("%s\n", messageBuffer);
    int res = embed(filename, originPath, messageBuffer, messageSize, destinationPath, argBits);

    free(messageBuffer);

    return res;
}

int embed_file(char *filename, char *originPath, char *messagePath, char *destinationPath, char *argBits) {
    int message;
    uchar *messageBuffer;
    ulong messageSize;

    if ((message = open(messagePath, O_RDONLY)) < 0) {
        perror(messagePath);
        return -3;
    }

    messageSize = lseek(message, 0, SEEK_END);
    messageBuffer = (uchar *)malloc(sizeof(uchar) * messageSize);

    lseek(message, 0, SEEK_SET);
    read(message, messageBuffer, messageSize);

    printf("%s\n", messageBuffer);
    int res = embed(filename, originPath, messageBuffer, messageSize, destinationPath, argBits);

    free(messageBuffer);
    close(message);

    return res;
}

int embed(char *filename, char *originPath, uchar *messageBuffer, ulong messageSize, char *destinationPath, char *argBits) {
    int origin, destination;
    uchar noOfBits;

    if ((origin = open(originPath, O_RDONLY)) < 0) {
        perror(originPath);
        return -3;
    }

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

    ulong fileSize = lseek(origin, 0, SEEK_END);
    buffer = (uchar *)malloc(sizeof(uchar) * fileSize);

    lseek(origin, 0, SEEK_SET);
    read(origin, buffer, fileSize);

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

        uchar msgBits = (messageBuffer[msgByte] >> (baseShift - ((i%iMod) * noOfBits))) & byteMask;
        if(i%8 == 0 && i!=0) msgByte++;   // move to next byte in message
        buffer[fileByte] = byteWithout2LSB | msgBits;
    }

    write(destination, buffer, fileSize);
    printf("done\n");

    free(buffer);
    close(origin);
    close(destination);
    return 0;
}


int extract_out(char *filename, char *originPath, char *destinationPath, char *argBits){
    int destination;
    ulong fileSize;

    uchar *outBuffer = NULL;

    int res = extract(filename, originPath, argBits, &fileSize, &outBuffer);

    if(res == 0) {
        if ((destination = open(destinationPath, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
            perror(destinationPath);
            return -3;
        }
        write(destination, outBuffer, fileSize);
    }

    close(destination);
    free(outBuffer);
    return res;
}


int extract_print(char *filename, char *originPath, char *argBits){
    ulong fileSize;
    uchar *outBuffer = NULL;

    int res = extract(filename, originPath, argBits, &fileSize, &outBuffer);
    if(res == 0) printf("%s\n", outBuffer);

    free(outBuffer);
    return res;
}

int extract(char *filename, char *originPath, char *argBits, ulong *outSize, uchar **outBufferPtr) {
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
        if(i%8 == 0 && i!=0) {
            outBuffer[msgByte] = msgByteContent;
            if(msgByteContent == '\0') {
                *outSize = msgByte;
                break;
            };
            msgByte++;
            msgByteContent = 0;
        };
        uchar LSBs = (buffer[fileByte] & byteMask);
        msgByteContent = (msgByteContent << noOfBits) | LSBs;
    }

    *outBufferPtr = outBuffer;
    free(buffer);
    close(origin);
    close(destination);
    return 0;
}