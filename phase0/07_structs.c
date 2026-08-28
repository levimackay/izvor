/* Task 0.7 — Structs.
 * Note: these functions take structs BY VALUE — the struct is copied in,
 * and returning one copies it out. (Passing a pointer instead is the other
 * option; you'll choose between them constantly in real C.)
 * Build + run:   make p0-07
 */
#include "07_structs.h"

/* Return a new Point whose coordinates are the sums. */
Point point_add(Point a, Point b) {
    (void)a; (void)b;
    Point p = {0, 0};
    return p; /* TODO */
}

/* 1 if same coordinates, else 0. (You cannot compare structs with == in C.
 * Ask yourself why not — the answer is about padding bytes.) */
int point_equals(Point a, Point b) {
    (void)a; (void)b;
    return 0; /* TODO */
}

/* Width times height. */
int rect_area(Rect r) {
    (void)r;
    return 0; /* TODO */
}

/* 1 if p is inside r (edges count), else 0. */
int rect_contains(Rect r, Point p) {
    (void)r; (void)p;
    return 0; /* TODO */
}
