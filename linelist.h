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

LineNode *createLineNode();
LineList *createLineList();

LineNode *deleteCurr(LineList *list);
void deleteLineList(LineList *list);

void setCurrLineNum(int lineNum, LineList *list);

void moveCurr(int lineNum, LineList *list);
void insertNodeBeforeCurr(LineNode *node, LineList *list);
void appendNodeAfterCurr(LineNode *node, LineList *list);

LineNode *getLineNode(int lineNum, LineList *list);

void printLine(LineNode *node);
void printLines(LineList *list);
void printNumberedLines(LineList *list);

int validLineNum(int lineNum, LineList *list);


#endif
