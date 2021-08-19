#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include "buffarr.h"


int countDigits(int num)
{
    int count = 0;
    do
    {
        num /= 10;
        count++;
    }
    while (num);

    return count;
}

int stringIsNumber(char *str, int len)
{
    int i;

    for (i = 0; i < len; i++)
    {
        if (str[i] < '0' || str[i] > '9')
        {
            return 0;
        }
    }

    return 1;
}

int printNum(int num)
{
    int n;
    char buf[128];
    n = sprintf(buf, "%d", num);
    write(STDOUT_FILENO, buf, n);
    return n;
}

int createFile(char *filename, int lock)
{
    int i;
    int len;
    int fileDesc;
    char *start;
    BuffArr *dir;

    dir = createBuffArr();
    clearBuf(dir);


    if (*filename == '/')
    {
        // absolute path
        appendChar('/', dir);
        filename++;
    }
    else if (*filename == '~')
    {
        // home directory path
        start = getenv("HOME");
        if (start == NULL)
        {
            fprintf(stderr, "createFile: home directory not found\n");
            exit(EXIT_FAILURE);
        }

        // copy home directory path into dir
        while (*start != '\0')
        {
            appendChar(*start, dir);
            start++;
        }
        filename++;
        
        if (*filename != '/')
        {
            fprintf(stderr, "createFile: invalid filepath (missing '/' after '~')\n");
            exit(EXIT_FAILURE);
        }
        appendChar('/', dir);
        filename++;

        // do not free start (according to documentation)


    }
    else
    {
        // relative path
        i = 0;
        start = (char*)malloc(512 * sizeof(char));
        memset(start, '\0', 512);
        getcwd(start, 512);
        if (start == NULL)
        {
            fprintf(stderr, "createFile: current working directory not found\n");
            exit(EXIT_FAILURE);
        }

        // copy current working directory into dir
        while (start[i] != '\0')
        {
            appendChar(start[i], dir);
            i++;
        }
        appendChar('/', dir);
        free(start);

        write(STDOUT_FILENO, dir->buf, dir->len);
        write(STDOUT_FILENO, "\n", 1);
    }

    printf("filename: %s\n", filename);


    // create all nonexistent directories
    i = 0;
    len = 0;
    start = filename;
    while (filename[i] != '\0')
    {
        len++;
        appendChar(filename[i], dir);

        if (filename[i] == '/' && filename[i + 1] != '\0')
        {
            mkdir(dir->buf, 0755);
            
            start = filename + i + 1;
            
            if (*start == '/')
            {
                fprintf(stderr, "createFile: invalid filepath (//)\n");
                exit(EXIT_FAILURE);
            }

            len = 0;
        }
        i++;
    }

    write(STDOUT_FILENO, dir->buf, dir->len);
    write(STDOUT_FILENO, "\n", 1);
    printf("filename: %s\n", filename);

    if (filename[i - 1] == '/')
    {
        fprintf(stderr, "createFile: cannot edit a directory\n");
        exit(EXIT_FAILURE);
    }

    appendChar('\0', dir);

    printf("%s\n", dir->buf);

    // create the file
    fileDesc = open(dir->buf, O_RDWR|O_CREAT, 0644);
    flock(fileDesc, lock);

    deleteBuffArr(dir);

    return fileDesc;
}
