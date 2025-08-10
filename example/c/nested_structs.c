#include <stdio.h>

struct baz {
    int values[5];
};

struct bar {
    int alpha;
    int beta;
    struct baz gamma;
};

struct foo {
    int first;
    int second;
    struct bar third;
};

int main() {
    struct baz further_inner = {{1, 2, 3, 4, 5}};
    struct bar inner = {5, 6, further_inner};
    struct foo result = {7, 8, inner};

    printf("%d\n", result.third.gamma.values[0]);
}
