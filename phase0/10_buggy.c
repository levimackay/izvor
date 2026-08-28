/* Task 0.10 — Debugging practice. THIS FILE CONTAINS TWO PLANTED BUGS.
 * Your job is to find them WITH THE TOOLS, not by staring:
 *
 *   Bug 1 is a memory bug. Just run the test (`make p0-10`) — it builds
 *   with UndefinedBehaviorSanitizer, which will stop the program with a
 *   report naming the exact line and what went wrong ("index N out of
 *   bounds..."). Read the report, then fix the line it points at.
 *
 *   Bug 2 is a logic bug — the program runs cleanly but an assert fails.
 *   Find it with lldb, not printf:
 *
 *     lldb ./build/p0-10
 *     (lldb) b sum_between          # break when the function is entered
 *     (lldb) run
 *     (lldb) p lo                   # print variables
 *     (lldb) n                      # step a line at a time
 *     (lldb) p total                # watch total evolve — where does it
 *                                   # diverge from your mental math?
 *
 * Rules: you may change at most one character/operator per bug. Both bugs
 * are one-line fixes. When the tests pass, you're done with Phase 0.
 */
#include <assert.h>
#include <stdio.h>

#define N 5

/* Sum of every integer from lo to hi, INCLUSIVE. sum_between(2,4) == 9. */
static int sum_between(int lo, int hi) {
    int total = 0;
    for (int i = lo; i < hi; i++) {
        total += i;
    }
    return total;
}

int main(void) {
    /* fill squares[i] with i*i */
    int squares[N];
    for (int i = 0; i <= N; i++) {
        squares[i] = i * i;
    }
    assert(squares[0] == 0);
    assert(squares[4] == 16);

    assert(sum_between(2, 4) == 9);     /* 2 + 3 + 4 */
    assert(sum_between(1, 1) == 1);
    assert(sum_between(1, 10) == 55);

    printf("task 0.10: all tests passed\n");
    return 0;
}
