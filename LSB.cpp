#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

typedef unsigned char uchar;
typedef unsigned long ulong;

int main(int argc, char *argv[]) {
    int origin, destination;

    if (argc != 3) {
        printf("usage: %s origin destination\n", argv[0]);
        return -2;
    }

    if ((origin = open(argv[1], O_RDONLY)) < 0) {
        perror(argv[0]);
        return -3;
    }

    // Si no exitse lo creas y lo truncas
    if ((destination = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
        perror(argv[0]);
        return -3;
    }

    int bytes;
    uchar *buffer;
    ulong size = lseek(origin, 0, SEEK_END);
    buffer = (uchar *)malloc(sizeof(uchar) * size);

    lseek(origin, 0, SEEK_SET);
    read(origin, buffer, size);

    for (int i = 0; i < size; i++) {
        uchar temp1 = (buffer[i] & 240) >> 4;
        uchar temp2 = (buffer[i] & 15) << 4;
        buffer[i] = temp1 | temp2;
    }

    write(destination, buffer, size);
    printf("done\n");

    free(buffer);
    close(origin);
    close(destination);
    return 0;
}