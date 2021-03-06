/**
 * Author:  Khoa Hoang
 * Created: 08.21.2021
 * 
 * This file contains functions for the BuffArr datatype.
 * BuffArr is a dynamic/resizable char array.
 * Use it whenever you need a buffer of characters with an undefined length.
 **/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/file.h>
#include "buffarr.h"


#define INIT_BUFFARR_CAP 128


BuffArr *createBuffArr()
{
    BuffArr *arr;
    arr = malloc(sizeof(BuffArr));
    arr->cap = INIT_BUFFARR_CAP;
    arr->len = 0;
    arr->buf = malloc(arr->cap * sizeof(char));
    return arr;
}

void deleteBuffArr(BuffArr *arr)
{
    if (arr == NULL) return;
    if (arr->buf != NULL)
    {
        free(arr->buf);
    }
    free(arr);
}

void expandBuf(BuffArr *arr)
{
    if (arr->len > arr->cap)
    {
        fprintf(stderr, "expandBuf: arr length is greater than capacity\n");
        exit(EXIT_FAILURE);
    }

    if (arr->len == arr->cap)
    {
        arr->cap *= 2;
        arr->buf = realloc(arr->buf, arr->cap);
    }
}

void clearBuf(BuffArr *arr)
{
    memset(arr->buf, '\0', arr->len);
    arr->len = 0;
}

void appendChar(char c, BuffArr *arr)
{
    expandBuf(arr);
    arr->buf[arr->len] = c;
    arr->len++;
}

void appendLine(char *line, BuffArr *arr)
{
    char *str;

    str = line;

    while (*str != '\0')
    {
        appendChar(*str, arr);
        str++;
    }
}

void readFile(int fileDesc, BuffArr *arr)
{
    int numChars;

    // read all characters in file into buffer
    while ((numChars = read(fileDesc, arr->buf + arr->len, arr->cap - arr->len)) > 0)
    {
        arr->len += numChars;
        expandBuf(arr);
    }

    // add a '\n' at the end if it is not present
    if (arr->buf[arr->len - 1] != '\n')
    {
        appendChar('\n', arr);
    }

}

