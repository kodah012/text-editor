#ifndef BUFFARR_H
#define BUFFARR_H

typedef struct BuffArr
{
    char *buf;
    int len;
    int cap;
}
BuffArr;

BuffArr *createBuffArr();
void deleteBuffArr(BuffArr *arr);
void clearBuf(BuffArr *arr);
void appendChar(char c, BuffArr *arr);
void appendLine(char *line, BuffArr *arr);
void readFile(int fileDesc, BuffArr *arr);


#endif
