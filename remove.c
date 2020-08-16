#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define IS_DIR(mode) ((mode & S_IFMT) == S_IFDIR)

// Static variables used to save the set options (-r, -R, -i or -f).
static int isRecursive;
static int isInteractive;
static int isForced;

// Function declarations
static int getConfirmation(void);
static void parseOption(const char *sOption);
static int deleteEntry(const char *sFile);
static int parseArgs(int argc, const char *argv[]);
static int deleteDir(const char *sDir);
static int deleteEntry(const char *sDir);

/***
* Description: User code entry point.
*
* Returns:     EXIT_SUCCESS on success or EXIT_FAILURE on error
***/
int main(int argc, const char *argv[])
{
    int i = parseArgs(argc, argv); // Parse all arguments provided. This function returns the index of the first entry to be deleted.
    int returnValue = EXIT_SUCCESS;

    if (i)
    {
        // Delete every entry specified
        for (; i < argc; ++i)
        {
            int r = deleteEntry(argv[i]);
            if (r == -1 && isForced == 0)
            {
                fprintf(stderr, "remove: Failed to delete '%s'.\n", argv[i]);
                returnValue = EXIT_FAILURE;
            }
        }
    }
    else if (isForced == 0)
    {
        // "remove -f" is actually allowed, catch this case here.
        fputs("Invalid syntax.\n\nSyntax is:\n./remove [-iRr] file...\n./remove -f [-iRr] [file...]\n", stderr);
        returnValue = EXIT_FAILURE;
    }
    return returnValue;
}

/***
* Description: Checks whether the user has confirmed the current operation (with 'y' or 'Y').
*
* Returns:     1 on success or 0 on error
***/
static int getConfirmation(void)
{
    int r = 0;
    int c = getchar();

    // Test whether the user has confirmed with 'y' or 'Y'
    if (c == 'y' || c == 'Y')
        r = 1;

    // Go to the end of the line (will clear the input-buffer)
    while (c != EOF && c != '\n')
        c = getchar();

    return r;
}

/***
* Description: Parses the given option string and saves the set options into the corresponding static's
*
* Parameters:
*   sOption:   Option string to be parsed
***/
static void parseOption(const char *sOption)
{
    while (*sOption)
    {
        switch (*sOption++)
        {
        case 'f':
            isForced = 1;
            isInteractive = 0;
            break;
        case 'i':
            isInteractive = 1;
            isForced = 0;
            break;
        case 'r': // Fallthrough
        case 'R':
            isRecursive = 1;
        }
    }
}

/***
* Description: Parses the arguments given. Calls parseOption if an option block is encountered.
*              Stops on first file argument found (from left to right).
* Parameters:
*   argc:   Number of arguments
*   argv:   Array of arguments
* 
* Returns:     Index of first file argument to be deleted or 0 on error
***/
static int parseArgs(int argc, const char *argv[])
{
    // Process every option provided
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
            parseOption(argv[i]);
        else
            return i;
    }

    return 0;
}

/***
* Description: Deletes a whole directory. Called by deleteEntry.
*
* Parameters:
*   sDir:      Path to the directory to be deleted
* 
* Returns:     0 on success or -1 on error
***/
static int deleteDir(const char *sDir)
{
    int r = -1;
    DIR *pDir = opendir(sDir);

    if (pDir != NULL)
    {
        size_t lenDirPath = strlen(sDir);
        char *pPath = malloc(lenDirPath + 1); // One additional byte is needed ('/')

        if (pPath == NULL)
            return -1;

        pPath[lenDirPath] = '/';         // Add directory delimiter at the end
        memcpy(pPath, sDir, lenDirPath); // Copy rest of the name of the directory (Note: String is not nul-terminated yet) */

        // Delete every directory-entry in the current directory (sDir).
        struct dirent *pDirEntry;
        while ((pDirEntry = readdir(pDir)))
        {
            // Skip both the directory entry "." and ".."
            if (strcmp(pDirEntry->d_name, ".") != 0 && strcmp(pDirEntry->d_name, "..") != 0)
            {
                // Build the actual path of the directory entry to be deleted
                size_t lenEntry = strlen(pDirEntry->d_name);
                pPath = realloc(pPath, lenDirPath + 1 + lenEntry + 1); // Two addiontal bytes needed: For '/' and the terminating 0
                if (pPath == NULL)
                    return -1;

                memcpy(pPath + lenDirPath + 1, pDirEntry->d_name, lenEntry + 1);
                r = deleteEntry(pPath);
            }
        }

        r = rmdir(sDir); // Delete the directory itself at last

        free(pPath);
        closedir(pDir);
    }

    return r;
}

/***
* Description: Deletes a directory entry. Might call deleteDir.
*
* Parameters:
*   sDir:      Path to directory entry to be deleted
* 
* Returns:     0 on success or -1 on error
***/
static int deleteEntry(const char *sDir)
{
    int r;
    struct stat statbuf;

    // In the interactive-mode, the user has to confirm whether to delete the current directory entry.
    if (isInteractive)
    {
        fprintf(stderr, "remove: Do you really want to delete '%s' (y/N)? ", sDir);
        r = getConfirmation();
        if (r == 0)
            return 0;
    }

    // Distinguish the current directory entry between a directory (will be deleted recursively) or a regular file, symbolic link etc.
    r = lstat(sDir, &statbuf); // Get stat of the current target (Note: Symbolic links will not be followed)
    if (r == 0)
    {
        if (IS_DIR(statbuf.st_mode))
            r = isRecursive ? deleteDir(sDir) : -1; // Only delete a directory when the recursion option (-r or -R) is set.
        else
            r = unlink(sDir);
    }

    return r;
}
