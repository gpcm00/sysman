#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    if (argc != 2) {
        fprintf(stderr, "usage: %s <min>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("Printf test\n");

    
    unsigned c = 0;
    while(1) {
        c++;
        fprintf(stderr, "stderr test %d\n", c);
        fprintf(stdout, "stdout test %d\n", c);

        unsigned r = (*argv[1] - '0');
        do { 
            r = sleep(r);
        } while (r > 0);
    }


    
}