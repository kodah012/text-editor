#include <unistd.h>
#include <stdio.h>
#include <sys/file.h>


int countDigits(int num)
{
    int count = 0;
    do
    {
        num /= 10;
        count++;
    }
    while (num);

    return count;
}

int stringIsNumber(char *str, int len)
{
    int i;

    for (i = 0; i < len; i++)
    {
        if (str[i] < '0' || str[i] > '9')
        {
            return 0;
        }
    }

    return 1;
}

int printNum(int num)
{
    int n;
    char buf[128];
    n = sprintf(buf, "%d", num);
    write(STDOUT_FILENO, buf, n);
    return n;
}

int createFile(char *filename, int lock)
{
    int fileDesc;
    fileDesc = open(filename, O_RDWR|O_CREAT, 0644);
    flock(fileDesc, lock);
    return fileDesc;
}
