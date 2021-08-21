#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include "buffarr.h"
#include "linelist.h"
#include "helper.h"

#define TOKS_BUFSZ 64


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

int searchCmd(const char *str, LineList *lines)
{
    int lineNum;
    LineNode *node;

    lineNum = lines->currLineNum;
    node = lines->curr;

    while (node != NULL)
    {
        if (strstr(node->line, str) != NULL)
        {
            // string is found; change current line and current node
            lines->curr = node;
            lines->currLineNum = lineNum;
            return 1;
        }
        
        node = node->next;
        lineNum++;
    }

    return 0;
}

int processCmd(BuffArr *cmd, LineList *lines)
{
    int status;
    int i;
    BuffArr *buffer;
    LineNode *currNode;
    char *tokens[TOKS_BUFSZ];

    buffer = createBuffArr();
    currNode = lines->head;

    // copy all characters in lines into buffer
    while (currNode != NULL)
    {
        // copy line into buffer
        appendLine(currNode->line, buffer);
        
        // go to next line
        currNode = currNode->next;
    }

    clearLineList(lines);

    if (fork() == 0) // child side
    {
        // apply command to lines
        appendChar('\0', cmd);

        // tokenize the command
        // + 1 to get rid of the '|' at beginning of command
        tokens[0] = strtok(cmd->buf + 1, " \t\n");
        for (i = 1; i < TOKS_BUFSZ; i++)
        {
            tokens[i] = strtok(NULL, " \t\n");

            // break out of loop if run out of tokens
            if (tokens[i] == NULL)
            {
                break;
            }
            else
            {
                printf("token %d: %s\n", i, tokens[i]);
            }
        }

        // execute the command
        execvp(tokens[0], tokens);

        // this point should never be reached; parent side should be running now
        perror("exec");
        exit(EXIT_FAILURE);

    }
    else // parent side
    {
        // wait until child process finishes
        wait(&status);

        write(STDIN_FILENO, "hell-ooo\n", 9);

        // write the buffer to stdin

        // read stdin into buffer

        // copy buffer into lines
    }

    return 0;


}
