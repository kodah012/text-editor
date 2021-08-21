#ifndef COMMANDS_H
#define COMMANDS_H

#include "linelist.h"
#include "buffarr.h"

void printCmd(LineList *lines);
void numberedPrintCmd(LineList *lines);
void deleteCmd(LineList *lines);
void moveCmd(BuffArr *cmd, LineList *lines);
void writeCmd(int fileDesc, char *filename, LineList *lines);
int searchCmd(const char *str, LineList *lines);
int processCmd(BuffArr *cmd, LineList *lines);

#endif
