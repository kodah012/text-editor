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
#include "commands.h"

enum Mode
{
    NORMAL,
    INSERT,
    APPEND,
    SEARCH
};

enum Mode currMode = NORMAL;
int fileDesc;
char *filename;

void setLines(LineList*, BuffArr*);
void runEditor(LineList*);

int main(int argc, char *argv[])
{
    BuffArr *buffer;
    LineList *lines;

    if (argc != 2)
    {
        fprintf(stderr, "usage: edit file\n");
        return EXIT_FAILURE;
    }

    buffer = createBuffArr();
    lines = createLineList();

    filename = argv[1];
    // open and read file into buffer and lines if file exists
    fileDesc = open(filename, O_RDWR);
    if (fileDesc != -1)
    {
        flock(fileDesc, LOCK_EX);
        readFile(fileDesc, buffer);
        setLines(lines, buffer);
    }

    deleteBuffArr(buffer);

    // start text editor loop
    runEditor(lines);
    
    // any code after this point should never be reached
 
    fprintf(stderr, "editor loop exited unexpectedly\n");
    return EXIT_FAILURE;
}


int validCommand(BuffArr *cmd, LineList *lines)
{
    if (currMode != NORMAL)
    {
        return 1;
    }

    if (stringIsNumber(cmd->buf, cmd->len))
    {
        return validLineNum(atoi(cmd->buf), lines);
    }

    switch (*cmd->buf)
    {
        case 'p':
            return cmd->len == 1;
        case 'n':
            return cmd->len == 1;
        case 'd':
            return cmd->len == 1 && lines->head != NULL;
        case 'm':
            if (stringIsNumber(cmd->buf + 1, cmd->len - 1))
            {
                return validLineNum(atoi(cmd->buf + 1), lines);
            }
            return cmd->buf[1] == '$' && cmd->len == 2;
        case 'i':
            return cmd->len == 1;
        case 'a':
            return cmd->len == 1;
        case 'w':
            return cmd->len == 1;
        case 's':
            return cmd->len == 1;
        case 'q':
            return cmd->len == 1;
    }

    return 0;
}

int runCommand(BuffArr *cmd, LineList *lines)
{
    LineNode *node;

    if (currMode == INSERT || currMode == APPEND)
    {
        if (*cmd->buf == '.' && cmd->len == 1)
        {
            currMode = NORMAL;
        }
        else
        {
            appendChar('\n', cmd);
            node = createLineNode(cmd->buf, cmd->len);

            if (currMode == INSERT)
            {
                insertNodeBeforeCurr(node, lines);
                currMode = APPEND;
            }
            else if (currMode == APPEND)
            {
                appendNodeAfterCurr(node, lines);
            }
        }
        return 1;
    }

    if (currMode == SEARCH)
    {
        currMode = NORMAL;
        // append '\0' for use with strstr() in searchCmd()
        appendChar('\0', cmd);
        write(STDOUT_FILENO, cmd->buf, cmd->len);
        return searchCmd(cmd->buf, lines);
    }

    if (isdigit(*cmd->buf))
    {
        setCurrLineNum(atoi(cmd->buf), lines);
        return 1;
    }

    switch (*cmd->buf)
    {
        case 'p':
            printCmd(lines);
            break;
        case 'n':
            numberedPrintCmd(lines);
            break;
        case 'd':
            deleteCmd(lines);
            break;
        case 'm':
            moveCmd(cmd, lines);
            break;
        case 'i':
            currMode = INSERT;
            break;
        case 'a':
            currMode = APPEND;
            break;
        case 'w':
            writeCmd(fileDesc, filename, lines);
            break;
        case 's':
            currMode = SEARCH;
            break;
        case 'q':
            if (fileDesc != -1)
            {
                flock(fileDesc, LOCK_UN);
                close(fileDesc);
            }

            deleteLineList(lines);
            exit(EXIT_SUCCESS);
            break;
    }

    return 1;
}

void runEditor(LineList *lines)
{
    int i;
    int cmdWasValid;
    char c;
    BuffArr *cmd;

    lines->currLineNum = lines->len > 0 ? 1 : 0;
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

        // cmd now contains only characters entered (no '\n' or '\0')

        if ((cmdWasValid = validCommand(cmd, lines)))
        {
            cmdWasValid = runCommand(cmd, lines);
        }
    } 
}

void setLines(LineList *lines, BuffArr *buffer)
{
    int i;
    int newLineLen;
    char *currLine;
    char *newLine;
    LineNode *node;

    newLineLen = 0;
    currLine = buffer->buf;

    for (i = 0; i < buffer->len; i++)
    {
        newLineLen++;

        if (buffer->buf[i] == '\n')
        {
            node = createLineNode(currLine, newLineLen);
            appendNodeAfterCurr(node, lines);

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
        node = createLineNode(currLine, newLineLen);
        appendNodeAfterCurr(node, lines);
    }
}


