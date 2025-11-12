#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct thing {
    int name[4];
    int age;
} __attribute__((packed));


int main(void) {
    struct thing x;
    x.age = 0;
    
    for (int i = 0; i < 4; i++) {
        printf(">> ");
        scanf(" %lld", &x.name[i]);
    }

    if (x.age > 0) { 
        printf("shouldn't u be zero not %d?\n", x.age);
    }

    return 0;
}
