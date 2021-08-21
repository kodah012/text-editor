/**
 * Author:  Khoa Hoang
 * Created: 08.21.2021
 * 
 * This file contains functions that handle all the commands the user can run while in the editor.
 **/
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
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
    int i;
    int currLineNum;
    int status;
    int pid;

    int pipe1[2];
    int pipe2[2];

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

    // pipe1[0] is read end of pipe; pipe1[1] is write end of pipe
    pipe(pipe1);
    pipe(pipe2);

    pid = fork();

    if (pid == -1)
    {
        perror("processCmd: fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) // child side
    {
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
        }

        // close the write end of pipe1
        close(pipe1[1]);
        // close the read end of pipe2
        close(pipe2[0]);

        // replace stdin with the read end of pipe1
        close(STDIN_FILENO);
        dup(pipe1[0]);

        // replace stdout with the write end of pipe2
        close(STDOUT_FILENO);
        dup(pipe2[1]);

        // execute the command
        // input should come from read end of pipe1
        // output should go to write end of pipe2
        execvp(tokens[0], tokens);

        // if this line is reached, it means the command was invalid

        deleteBuffArr(buffer);

        // close remaining file descriptors (deletes the pipes)
        close(pipe1[0]);
        close(pipe2[1]);

        exit(EXIT_FAILURE);

    }
    else // parent side
    {
        // close the read end of pipe1
        close(pipe1[0]);
        // close the write end of pipe2
        close(pipe2[1]);

        // write the buffer to the write end of pipe1
        for (i = 0; i < buffer->len; i++)
        {
            write(pipe1[1], buffer->buf + i, 1);
        }

        // send EOF to child process by widowing pipe1 (close the write end)
        close(pipe1[1]);

        // wait until child process terminates
        wait(&status);

        // start executing code here after child process ended

        if (WEXITSTATUS(status) != EXIT_FAILURE)
        {
            // read the read end of pipe2 into buffer
            clearBuf(buffer);
            readFile(pipe2[0], buffer);

            // copy buffer into lines
            currLineNum = lines->currLineNum;
            clearLineList(lines);
            setLines(lines, buffer);

            if (currLineNum > lines->len)
            {
                currLineNum = lines->len;
            }
            setCurrLineNum(currLineNum, lines);
        }

        deleteBuffArr(buffer);

        // close remaining file descriptor (deletes the pipes)
        close(pipe2[0]);
    }

    // return false if command failed execution, true otherwise
    return WEXITSTATUS(status) == EXIT_FAILURE ? 0 : 1;
}
