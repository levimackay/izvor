/* Task 0.7 — shared type definitions.
 * This is your first header file: both 07_structs.c and the test include
 * it so they agree on what a Point and Rect ARE. Task 0.9 explains
 * headers properly (including the #ifndef "include guard" below). */
#ifndef PHASE0_07_STRUCTS_H
#define PHASE0_07_STRUCTS_H

/* A struct is several variables that travel together as one value. */
typedef struct {
    int x;
    int y;
} Point;

/* Structs can contain structs. */
typedef struct {
    Point min;   /* bottom-left corner  */
    Point max;   /* top-right corner (assume max.x >= min.x, max.y >= min.y) */
} Rect;

Point point_add(Point a, Point b);
int   point_equals(Point a, Point b);
int   rect_area(Rect r);
int   rect_contains(Rect r, Point p);   /* edges count as inside */

#endif
