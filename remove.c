#include <stdio.h>
#include <stdlib.h>

void usage()
{
    printf("Usage: ./remove <file_name>\n");
}

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        usage();
        return EXIT_FAILURE;
    }

    if (!remove(argv[1]))
        printf("File deleted successfully!\n");
    else
    {
        perror("Error");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
