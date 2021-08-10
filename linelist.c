#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "linelist.h"
#include "helper.h"


LineNode *createLineNode()
{
    LineNode *node;
    node = malloc(sizeof(LineNode));
    node->line = NULL;
    node->next = NULL;
    node->prev = NULL;
    return node;
}

LineList *createLineList()
{
    LineList *list;
    list = malloc(sizeof(LineList));
    list->head = NULL;
    list->curr = NULL;
    list->currLineNum = 0;
    list->len = 0;
    return list;
}

LineNode *deleteCurr(LineList *list)
{
    LineNode *curr;

    if (list == NULL)
    {
        fprintf(stderr, "deleteCurr: list is NULL\n");
        exit(EXIT_FAILURE);
    }
    
    if (list->len == 0)
    {
        fprintf(stderr, "deleteCurr: list is empty\n");
        exit(EXIT_FAILURE);
    }

    curr = list->curr;

    if (curr->prev != NULL)
    {
        // reassign previous node's next pointer
        curr->prev->next = curr->next;
    }

    if (curr->next != NULL)
    {
        // reassign next node's previous pointer
        curr->next->prev = curr->prev;
        list->curr = curr->next;
    }
    else
    {
        // current node is the last node in the list, so move it back one space
        list->curr = curr->prev;
        list->currLineNum--;
    }

    if (list->head == curr)
    {
        // current node is the first node in the list, so move it forward one space
        list->head = curr->next;
    }

    list->len--;

    return curr;
}

void deleteLineList(LineList *list)
{
    if (list == NULL)
    {
        fprintf(stderr, "deleteLineList: list is NULL\n");
        exit(EXIT_FAILURE);
    }

    while (list->head != NULL)
    {
        free(deleteCurr(list));
    }

    free(list);
}

void setCurrLineNum(int lineNum, LineList *list)
{
    if (!validLineNum(lineNum, list))
    {
        fprintf(stderr, "setCurrLineNum: lineNum out of bounds\n");
        exit(EXIT_FAILURE);
    }

    list->curr = getLineNode(lineNum, list);
    list->currLineNum = lineNum;
}

void moveCurr(int lineNum, LineList *list)
{
    LineNode *node;

    if (!validLineNum(lineNum, list) && lineNum != -1)
    {
        fprintf(stderr, "moveCurr: lineNum out of bounds\n");
        exit(EXIT_FAILURE);
    }

    node = deleteCurr(list);

    if (lineNum == -1)
    {
        // put line at end of list
        setCurrLineNum(list->len, list);
        appendNodeAfterCurr(node, list);
    }
    else if (lineNum >= list->currLineNum && lineNum > 1)
    {
        setCurrLineNum(lineNum - 1, list);
        insertNodeBeforeCurr(node, list);
    }
    else
    {
        setCurrLineNum(lineNum, list);
        insertNodeBeforeCurr(node, list);
    }
}

void insertNodeBeforeCurr(LineNode *node, LineList *list)
{
    // if list is empty, replace head with new node
    if (list->curr == NULL)
    {
        list->head = node;
        list->curr = node;
        list->currLineNum = 1;
        list->len = 1;
    }
    else
    {
        node->next = list->curr;
        node->prev = list->curr->prev;
        list->curr->prev = node;
        if (node->prev != NULL)
        {
            node->prev->next = node;
        }
        else
        {
            // current node is head node; set head to new node
            list->head = node;
        }

        // move current node to be new node (line number stays the same)
        list->curr = node;

        list->len++;
    }
}

void appendNodeAfterCurr(LineNode *node, LineList *list)
{
    // if list is empty, replace head with new node
    if (list->curr == NULL)
    {
        list->head = node;
        list->curr = node;
        list->currLineNum = 1;
        list->len = 1;
    }
    else
    {
        node->next = list->curr->next;
        node->prev = list->curr;
        list->curr->next = node;
        if (node->next != NULL)
        {
            node->next->prev = node;
        }

        // move current node and line number to be new node
        list->curr = node;
        list->currLineNum++;
        
        list->len++;
    }
}

LineNode *getLineNode(int lineNum, LineList *list)
{
    int diff;
    int i;
    LineNode *currNode;

    if (!validLineNum(lineNum, list))
    {
        fprintf(stderr, "getLineNode: lineNum out of bounds\n");
        exit(EXIT_FAILURE);
    }

    diff = lineNum - list->currLineNum;
    currNode = list->curr;

    for (i = 0; i < abs(diff); i++)
    {
        if (diff < 0)
        {
            currNode = currNode->prev;
        }
        else
        {
            currNode = currNode->next;
        }
    }

    return currNode;
}


void printLine(LineNode *node)
{
    char *line;

    if (node == NULL || node->line == NULL) return;

    line = node->line;

    while (*line != '\0')
    {
        write(STDOUT_FILENO, line, 1);
        line++;
    }
}

void printLines(LineList *list)
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

void printNumberedLines(LineList *list)
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

int validLineNum(int lineNum, LineList *list)
{
    return lineNum >= 1 && lineNum <= list->len;
}
