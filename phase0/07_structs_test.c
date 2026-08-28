/* Test for task 0.7 — do not edit; make these pass. */
#include <assert.h>
#include <stdio.h>
#include "07_structs.h"

int main(void) {
    Point a = {1, 2};
    Point b = {3, 4};

    Point c = point_add(a, b);
    assert(c.x == 4 && c.y == 6);
    assert(a.x == 1);                 /* a was copied, not modified */

    assert(point_equals(a, a) == 1);
    assert(point_equals(a, b) == 0);
    Point a2 = {1, 2};
    assert(point_equals(a, a2) == 1); /* equal by VALUE, not identity */

    Rect r = {{0, 0}, {4, 3}};
    assert(rect_area(r) == 12);
    Rect flat = {{2, 2}, {2, 5}};
    assert(rect_area(flat) == 0);

    Point inside = {2, 1};
    Point edge   = {4, 3};
    Point outside = {5, 1};
    assert(rect_contains(r, inside) == 1);
    assert(rect_contains(r, edge) == 1);
    assert(rect_contains(r, outside) == 0);

    printf("task 0.7: all tests passed\n");
    return 0;
}
