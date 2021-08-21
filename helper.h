#ifndef HELPER_H
#define HELPER_H

#include "linelist.h"
#include "buffarr.h"

// return number of digits in int num; will always return value greater than or equal to 1
int countDigits(int num);

// print num to stdout and return number of bytes printed
int printNum(int num);

// len should not include '\0'; return 1 if all chars in str are digits and 0 otherwise; negatives return 0
int stringIsNumber(char *str, int len);

// assuming file does not already exist, create file with name filename and add a lock (see flock)
int createFile(char *filename, int lock);

#endif
