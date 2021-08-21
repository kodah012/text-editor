#ifndef LINELIST_H
#define LINELIST_H

typedef struct LineNode
{
    char *line;
    struct LineNode *next;
    struct LineNode *prev;
}
LineNode;

typedef struct LineList
{
    LineNode *head;
    LineNode *curr;
    int len;
    int currLineNum;
}
LineList;

typedef struct BuffArr BuffArr;

LineNode *createLineNode(const char *line, int len);
LineList *createLineList();

LineNode *popCurrNode(LineList *list);
void setCurrLineNum(int lineNum, LineList *list);
LineNode *getLineNode(int lineNum, LineList *list);

void moveCurr(int lineNum, LineList *list);
void insertNodeBeforeCurr(LineNode *node, LineList *list);
void appendNodeAfterCurr(LineNode *node, LineList *list);

void printLine(LineNode *node);

int validLineNum(int lineNum, LineList *list);

void clearLineList(LineList *list);
void deleteLineList(LineList *list);
void setLines(LineList *list, BuffArr *arr);

#endif
