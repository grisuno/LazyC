#include <stdio.h>   /* printf, fflush */

void go(char *args, int arglen) {
    printf("[+] Module loaded.\n");
    printf("[+] Args: %.*s\n", arglen, args);
    fflush(stdout);
}