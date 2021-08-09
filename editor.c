#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/file.h>
#include "helper.h"
#include "linelist.h"
#include "buffarr.h"

void setLines(LineList *lines, BuffArr *buffer);
void runEditor(int, LineList*);

int main(int argc, char *argv[])
{
    int fileDesc;
    BuffArr *buffer;
    LineList *lines;

    if (argc != 2)
    {
        fprintf(stderr, "usage: edit file\n");
        return EXIT_FAILURE;
    }

    buffer = createBuffArr();
    lines = createLineList();

    // open and read file into buffer and lines if file exists
    fileDesc = open(argv[1], O_RDWR);
    if (fileDesc != -1)
    {
        flock(fileDesc, LOCK_EX);
        readFile(fileDesc, buffer);
        setLines(lines, buffer);
    }

    deleteBuffArr(buffer);

    // start text editor loop
    runEditor(fileDesc, lines);
    
    // any code after this point should never be reached
 
    fprintf(stderr, "editor loop exited unexpectedly\n");
    return EXIT_FAILURE;
}


int validCommand(BuffArr *cmd, LineList *lines)
{
    int cmdIsValid = 0;

    if (stringIsNumber(cmd->buf, cmd->len))
    {
        cmdIsValid = validLineNum(atoi(cmd->buf), lines);
    }
    else if (*cmd->buf == 'p' && cmd->len == 1) cmdIsValid = 1;
    else if (*cmd->buf == 'n' && cmd->len == 1) cmdIsValid = 1;
    else if (*cmd->buf == 'd' && cmd->len == 1)
    {
        // valid if list is not empty
        cmdIsValid = lines->head != NULL;
    }
    else if (*cmd->buf == 'q' && cmd->len == 1) cmdIsValid = 1;

    return cmdIsValid;
}

void runCommand(BuffArr *cmd, LineList *lines, int fileDesc)
{
    int n;

    if (isdigit(*cmd->buf))
    {
        n = atoi(cmd->buf);
        moveCurr(n, lines);
    }
    else if (*cmd->buf == 'p')
    {
        printLines(lines);
    }
    else if (*cmd->buf == 'n')
    {
        printNumberedLines(lines);
    }
    else if (*cmd->buf == 'd')
    {
        deleteCurr(lines);
    }
    else if (*cmd->buf == 'q')
    {
        if (fileDesc != -1)
        {
            flock(fileDesc, LOCK_UN);
            close(fileDesc);
        }

        deleteLineList(lines);
        exit(EXIT_SUCCESS);
    }
}

void runEditor(int fileDesc, LineList *lines)
{
    int i;
    int cmdWasValid;
    char c;
    BuffArr *cmd;

    lines->currLineNum = 1;
    lines->curr = lines->head;

    cmdWasValid = 1;
    cmd = createBuffArr();

    while (1)
    {
        printLine(lines->curr);
        printNum(lines->currLineNum);

        if (cmdWasValid)
        {
            write(STDOUT_FILENO, " -> ", 4);
        }
        else
        {
            write (STDOUT_FILENO, " !> ", 4);
        }

        clearBuf(cmd);

        while ((c = getchar()) != '\n')
        {
            appendChar(c, cmd);
        }

        if ((cmdWasValid = validCommand(cmd, lines)))
        {
            runCommand(cmd, lines, fileDesc);
        }
    } 
}

void setLines(LineList *lines, BuffArr *buffer)
{
    int i;
    int newLineLen;
    char *currLine;
    char *newLine;

    newLineLen = 0;
    currLine = buffer->buf;

    for (i = 0; i < buffer->len; i++)
    {
        newLineLen++;

        if (buffer->buf[i] == '\n')
        {
            newLine = malloc((newLineLen + 1) * sizeof(char));
            strncpy(newLine, currLine, newLineLen);
            newLine[newLineLen] = '\0';

            appendLineAfterCurr(newLine, lines);

            newLineLen = 0;
            
            // set currLine to start at character after '\n' if possible
            if (i + 1 < buffer->len)
            {
                currLine = buffer->buf + i + 1;
            }
        }
    }

    if (newLineLen > 0 && buffer->buf[buffer->len - 1] != '\n')
    {
        // file does not end with '\n'; insert into list anyways
        newLine = malloc(newLineLen * sizeof(char));
        strncpy(newLine, currLine, newLineLen);

        appendLineAfterCurr(newLine, lines);
    }
}


