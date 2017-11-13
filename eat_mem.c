#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MEGABYTE (1024*1024)

int main(int argc, char *argv[])
{
    void *block = NULL;
    int count = 0;
    int i;

    while (1)
    {
        block = (void *) malloc(MEGABYTE);
        if (!block) 
        {
            break;
        }

        for(i = 0;i < MEGABYTE / 4;i ++)	
        {
            ((unsigned int*)block)[i]	= i;
        }
        count++;
        printf("%d \n",count);
        sleep(2);
    }
    exit(0);
}