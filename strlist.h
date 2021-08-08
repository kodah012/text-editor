#ifndef STRLIST_H
#define STRLIST_H

typedef struct StrNode
{
    char *str;
    struct StrNode *next;
    struct StrNode *prev;
}
StrNode;

typedef struct StrList
{
    StrNode *head;
    int len;
}
StrList;

StrNode *createStrNode();
StrList *createStrList();
void deleteStrNode(StrNode *node, StrList *list);
void deleteStrList(StrList *list);
StrNode *getStrNode(int index, StrList *list);
void printStrNode(StrNode *node);


#endif
