#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define SIZE 1024

void usage()
{
    printf("Usage: ./copy <source_file> <destination_file>\n");
}

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        usage();
        return EXIT_FAILURE;
    }

    if (access(argv[1], F_OK) != 0)
    {
        perror("Error");
        return EXIT_FAILURE;
    }

    int src = open(argv[1], O_RDONLY);
    int dest = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC);

    if (src == -1 || dest == -1)
    {
        perror("Error");
        printf("\n");
        usage();
        return EXIT_FAILURE;
    }

    char *buffer[SIZE];
    int readBuffer;

    while ((readBuffer = read(src, buffer, SIZE)) > 0)
        if (write(dest, buffer, readBuffer) != readBuffer)
        {
            printf("Error in writing data to \n");
            return EXIT_FAILURE;
        }

    if (close(src) == -1)
    {
        printf("Error in closing source file\n");
        return EXIT_FAILURE;
    }

    if (close(dest) == -1)
    {
        printf("Error in closing destination file\n");
        return EXIT_FAILURE;
    }

    printf("File copied successfully!\n");

    return EXIT_SUCCESS;
}