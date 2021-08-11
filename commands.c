#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include "buffarr.h"
#include "linelist.h"
#include "helper.h"


void printCmd(LineList *list)
{
    LineNode *node;

    node = list->head;

    while (node != NULL)
    {
        printLine(node);
        node = node->next;
    }
}

void printNumLeftAligned(int num, LineList *list)
{
    int i;
    int numBytes;
    int maxDigits;

    numBytes = printNum(num);
    maxDigits = countDigits(list->len);

    for (i = 0; i < maxDigits - numBytes; i++)
    {
        write(STDOUT_FILENO, " ", 1);
    }
}

void numberedPrintCmd(LineList *list)
{
    int n;
    LineNode *node;

    node = list->head;

    for (n = 1; n <= list->len; n++)
    {
        printNumLeftAligned(n, list);
        write(STDOUT_FILENO, " | ", 3);
        printLine(node);

        if (n < list->len)
        {
            node = node->next;
        }
    }


    if (node == NULL) return;
    
    n = strlen(node->line);
    if (n == 0 || node->line[n - 1] != '\n')
    {
        // list is empty OR last line in list does not end with '\n'; print one anyways
        write(STDOUT_FILENO, "\n", 1);
    }
}

void deleteCmd(LineList *lines)
{
    LineNode *node;
    node = popCurrNode(lines);
    free(node->line);
    free(node);
}

void moveCmd(BuffArr *cmd, LineList *lines)
{
    int lineNum;

    if (isdigit(cmd->buf[1]))
    {
        lineNum = atoi(cmd->buf + 1);
        moveCurr(lineNum, lines);
    }
    else if (cmd->buf[1] == '$')
    {
        // move line to end of file
        moveCurr(-1, lines);
    }
}

void writeCmd(int fileDesc, char *filename, LineList *lines)
{
    int fileSize;
    BuffArr *buffer;
    LineNode *node;

    fileSize = 0;
    node = lines->head;
    buffer = createBuffArr();

    while (node != NULL)
    {
        appendLine(node->line, buffer);
        fileSize += strlen(node->line);
        node = node->next;
    }

    if (fileDesc == -1)
    {
        fileDesc = createFile(filename, LOCK_EX);
    }

    if (ftruncate(fileDesc, fileSize) == -1)
    {
        perror("runCommand: ftruncate");
        exit(EXIT_FAILURE);
    }

    if (pwrite(fileDesc, buffer->buf, fileSize, 0) == -1)
    {
        perror("runCommand: write");
        exit(EXIT_FAILURE);
    }

    deleteBuffArr(buffer);

}

