#include <stdlib.h>
#include <stdio.h>

int main(void) {
    unsigned int a;
    while(scanf("%2x ", &a) == 1) {
	putchar((int)a);
    }
    exit(0);
}
