#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/file.h>
#include "helper.h"

#define CMD_BUFSZ 512
#define INIT_BUFFER_CAP 128
#define INIT_LINES_CAP 8

typedef struct Buffer
{
    char *ptr;
    int cap;
    int len;
    int endsInNewline;
}
Buffer;

typedef struct Lines
{
    char **ptr;
    int cap;
    int len;
}
Lines;

void readFile(int, Buffer*);
void setLines(Lines*, Buffer*);
void runEditor(int, Buffer*, Lines*);


int main(int argc, char *argv[])
{
    int fileDesc;
    Buffer buffer;
    Lines lines;

    if (argc != 2)
    {
        fprintf(stderr, "usage: edit file\n");
        return EXIT_FAILURE;
    }

    // initialize buffer
    buffer.cap = INIT_BUFFER_CAP;
    buffer.len = 0;
    buffer.ptr = malloc(buffer.cap * sizeof(char));
    buffer.endsInNewline = 0;

    // initialize lines
    lines.cap = INIT_LINES_CAP;
    lines.len = 0;
    lines.ptr = malloc(lines.cap * sizeof(char*));

    // open and read file into buffer and lines if file exists
    fileDesc = open(argv[1], O_RDWR);
    if (fileDesc != -1)
    {
        readFile(fileDesc, &buffer);
        setLines(&lines, &buffer);
    }

    // start text editor loop
    runEditor(fileDesc, &buffer, &lines);
    
    // any code after this point should never be reached
 
    fprintf(stderr, "editor loop exited unexpectedly\n");
    return EXIT_FAILURE;
}

char *getLine(const Lines *lines, int lineNum)
{
    // if line number is invalid, return null
    if (lineNum - 1 < 0 || lineNum > lines->len) return NULL;
    return lines->ptr[lineNum - 1];
}

void printLine(int lineNum, Lines *lines)
{
    int charsPrinted;
    char *line;

    charsPrinted = 0;
    line = getLine(lines, lineNum);

    if (lines->len <= 1)
    {
        write(STDOUT_FILENO, "\n", 1);
        charsPrinted++;
    }
    else
    {
        // print characters in line until newline is reached
        for (charsPrinted = 0; line[charsPrinted] != '\n'; charsPrinted++)
        {
            write(STDOUT_FILENO, line + charsPrinted, 1);
        }

        // print newline at the end of line
        write(STDOUT_FILENO, line + charsPrinted, 1);
        charsPrinted++;
    }
}

int validCommand(char *cmd, int cmdLen, Lines *lines)
{
    int cmdIsValid = 0;

    if (stringIsNumber(cmd, cmdLen))
    {
        cmdIsValid = getLine(lines, atoi(cmd)) != NULL;
    }
    else if (*cmd == 'p' && cmdLen == 1) cmdIsValid = 1;
    else if (*cmd == 'n' && cmdLen == 1) cmdIsValid = 1;
    else if (*cmd == 'q' && cmdLen == 1) cmdIsValid = 1;

    return cmdIsValid;
}

void printNumLeftAligned(int num, Lines *lines)
{
    int i;
    int numBytes;
    int maxDigits;

    numBytes = printNum(num);
    maxDigits = countDigits(lines->len);

    for (i = 0; i < maxDigits - numBytes; i++)
    {
        write(STDOUT_FILENO, " ", 1);
    }
}

void runCommand(char *cmd, int fileDesc, Buffer *buffer, Lines *lines, int *currLine)
{
    int lineNum;    

    if (isdigit(*cmd))
    {
        lineNum = atoi(cmd);
        *currLine = lineNum;
    }
    else if (*cmd == 'p')
    {
        for (lineNum = 1; lineNum <= lines->len; lineNum++)
        {
            printLine(lineNum, lines);
        }
    }
    else if (*cmd == 'n')
    {
        for (lineNum = 1; lineNum <= lines->len; lineNum++)
        {
            printNumLeftAligned(lineNum, lines);
            write(STDOUT_FILENO, " | ", 3);
            printLine(lineNum, lines);
        }
    }
    else if (*cmd == 'q')
    {
        if (fileDesc != -1)
        {
            flock(fileDesc, LOCK_UN);
            close(fileDesc);
        }

        free(buffer->ptr);
        free(lines->ptr);
        exit(EXIT_SUCCESS);
    }
}

void runEditor(int fileDesc, Buffer *buffer, Lines *lines)
{
    int i;
    int currLine;
    int cmdLen;
    int cmdWasValid;
    char cmd[CMD_BUFSZ];

    currLine = 1;
    cmdWasValid = 1;

    while (1)
    {
        printLine(currLine, lines);

        printNum(currLine);
        if (cmdWasValid)
        {
            write(STDOUT_FILENO, " -> ", 4);
        }
        else
        {
            write (STDOUT_FILENO, " !> ", 4);
        }

        // - 1 to get rid of newline at the end of stdin
        cmdLen = read(STDIN_FILENO, cmd, CMD_BUFSZ) - 1;


        if ((cmdWasValid = validCommand(cmd, cmdLen, lines)))
        {
            runCommand(cmd, fileDesc, buffer, lines, &currLine);
        }
    } 
}


void expandBuffer(Buffer *buffer)
{
    // if there is less than 32 bytes left in buffer, double buffer's capacity
    if (buffer->cap - buffer->len < 32)
    {
        buffer->cap *= 2;
        buffer->ptr = realloc(buffer->ptr, buffer->cap);
    }
}

void readFile(int fileDesc, Buffer *buffer)
{
    int n;

    // prevent anyone else from reading or writing
    flock(fileDesc, LOCK_EX);

    while ((n = read(fileDesc, buffer->ptr + buffer->len, buffer->cap - buffer->len)) > 0)
    {
        buffer->len += n;
        expandBuffer(buffer);
    }
    
    buffer->endsInNewline = buffer->ptr[buffer->len - 1] == '\n';

    // if last character of buffer is not a newline, add a newline to the buffer
    if (!buffer->endsInNewline)
    {
        buffer->endsInNewline = 0;
        buffer->len += 1;
        expandBuffer(buffer);
        buffer->ptr[buffer->len - 1] = '\n';
    }
}

void setLines(Lines *lines, Buffer *buffer)
{
    int i;
    char *currLine;

    currLine = buffer->ptr;

    for (i = 0; i < buffer->len; i++)
    {
        if (buffer->ptr[i] == '\n')
        {
            lines->ptr[lines->len] = currLine;
            lines->len++;

            // set currLine to start at character after newline if possible
            if (i + 1 < buffer->len)
            {
                currLine = buffer->ptr + i + 1;
            }

            // if lines buffer has been filled to capacity, double its capacity
            if (lines->len >= lines->cap)
            {
                lines->cap *= 2;
                lines->ptr = realloc(lines->ptr, lines->cap * sizeof(char*));
            }
        }

    }
}


