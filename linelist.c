#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "linelist.h"
#include "helper.h"
#include "buffarr.h"


LineNode *createLineNode(const char *line, int len)
{
    char *str;
    LineNode *node;

    if (line[len - 1] == '\0')
    {
        // line ends in '\0', so just copy it into str
        str = malloc(len * sizeof(char));
        strncpy(str, line, len);
    }
    else
    {
        // line does not end in '\0'; add '\0' at the end and copy into str
        str = malloc((len + 1) * sizeof(char));
        strncpy(str, line, len);
        str[len] = '\0';
    }

    node = malloc(sizeof(LineNode));
    node->next = NULL;
    node->prev = NULL;
    node->line = str;

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

LineNode *popCurrNode(LineList *list)
{
    LineNode *curr;

    if (list == NULL)
    {
        fprintf(stderr, "popCurrNode: list is NULL\n");
        exit(EXIT_FAILURE);
    }
    
    if (list->len == 0)
    {
        fprintf(stderr, "popCurrNode: list is empty\n");
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

void clearLineList(LineList *list)
{
    LineNode *node;
    
    if (list == NULL) return;

    while (list->head != NULL)
    {
        node = popCurrNode(list);
        free(node->line);
        free(node);
    }
}

void deleteLineList(LineList *list)
{
    clearLineList(list);

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

    node = popCurrNode(list);

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


int validLineNum(int lineNum, LineList *list)
{
    return lineNum >= 1 && lineNum <= list->len;
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

    // this condition should never be met assuming readFile() appended a '\n' to buffer
    if (newLineLen > 0)
    {
        fprintf(stderr, "setLines: not all characters read into lines\n");
        exit(EXIT_FAILURE);
    }

    // file does not end with '\n'; readFile() should already have handled this
    if (buffer->buf[buffer->len - 1] != '\n')
    {
        fprintf(stderr, "setLines: buffer does not end in \'\\n\'\n");
        exit(EXIT_FAILURE);
    }
}