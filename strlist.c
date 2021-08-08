#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "strlist.h"


StrNode *createStrNode()
{
    StrNode *node;
    node = malloc(sizeof(StrNode));
    node->str = NULL;
    node->next = NULL;
    node->prev = NULL;
    return node;
}

StrList *createStrList()
{
    StrList *list;
    list = malloc(sizeof(StrList));
    list->head = NULL;
    list->len = 0;
    return list;
}

void deleteStrNode(StrNode *node, StrList *list)
{
    if (node == NULL)
    {
        fprintf(stderr, "deleteStrNode: node is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (node->prev != NULL)
    {
        node->prev->next = node->next;
    }

    if (node->next != NULL)
    {
        node->next->prev = node->prev;
    }

    if (list == NULL)
    {
        fprintf(stderr, "deleteStrNode: list is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (list->head == node)
    {
        list->head = node->next;
    }

    list->len--;

    free(node);
}

void deleteStrList(StrList *list)
{
    if (list == NULL)
    {
        fprintf(stderr, "deleteStrList: list is NULL\n");
        exit(EXIT_FAILURE);
    }

    while (list->head != NULL)
    {
        deleteStrNode(list->head, list);
    }

    free(list);
}

StrNode *getStrNode(int index, StrList *list)
{
    int currIndex;
    StrNode *currNode;

    if (index < 0)
    {
        fprintf(stderr, "getStrNode: index out of bounds\n");
        exit(EXIT_FAILURE);
    }

    currIndex = 0;
    currNode = list->head;

    while (currIndex != index)
    {
        if (currNode == NULL)
        {
            fprintf(stderr, "getStrNode: index out of bounds\n");
            exit(EXIT_FAILURE);
        }

        currNode = currNode->next;
        currIndex++;
    }

    return currNode;

}


void printStrNode(StrNode *node)
{
    char *str;

    str = node->str;

    if (str == NULL)
    {
        fprintf(stderr, "printStrNode: node->str is NULL\n");
        exit(EXIT_FAILURE);
    }

    while (*str != '\0')
    {
        write(STDOUT_FILENO, str, 1);
        str++;
    }
}


