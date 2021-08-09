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
    int maxLineNum;
    int currLineNum;
}
LineList;

LineNode *createLineNode();
LineList *createLineList();

void deleteCurr(LineList *list);
void deleteLineList(LineList *list);

void moveCurr(int lineNum, LineList *list);
void appendLineAfterCurr(char *line, LineList *list);

LineNode *getLineNode(int lineNum, LineList *list);

void printLine(LineNode *node);
void printLines(LineList *list);
void printNumberedLines(LineList *list);

int validLineNum(int lineNum, LineList *list);


#endif
