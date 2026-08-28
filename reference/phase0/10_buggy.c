/* Reference solution — task 0.10, with both planted bugs fixed.
 * (Read the "was:" comments — the bugs are the interesting part.) */
#include <assert.h>
#include <stdio.h>

#define N 5

static int sum_between(int lo, int hi) {
    int total = 0;
    /* was: i < hi — an off-by-one that silently dropped the last term.
     * Visible in lldb: step the loop for sum_between(2, 4) and watch it
     * exit with total == 5, never adding the 4. */
    for (int i = lo; i <= hi; i++) {
        total += i;
    }
    return total;
}

int main(void) {
    int squares[N];
    /* was: i <= N — wrote one element past the end of the array.
     * UBSan report: "index 5 out of bounds for type 'int[5]'" on the
     * squares[i] line. */
    for (int i = 0; i < N; i++) {
        squares[i] = i * i;
    }
    assert(squares[0] == 0);
    assert(squares[4] == 16);

    assert(sum_between(2, 4) == 9);
    assert(sum_between(1, 1) == 1);
    assert(sum_between(1, 10) == 55);

    printf("task 0.10: all tests passed\n");
    return 0;
}
