#include <stdio.h>
#include "chess.h"

char start_position[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

void test(int expected, int actual, const char* testname) {
    if(expected == actual) {
        printf("\n%s PASSED", testname);
    } else {
        printf("\n%s FAILED expected: %d actual: %d", testname, expected, actual);
    }
}

int main() {
    test(20, perft_test2(1, start_position), "Starting position, depth 1");
}