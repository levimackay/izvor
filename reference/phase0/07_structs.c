/* Reference solution — task 0.7 */
#include "07_structs.h"

Point point_add(Point a, Point b) {
    Point p = {a.x + b.x, a.y + b.y};
    return p;
}

int point_equals(Point a, Point b) {
    return a.x == b.x && a.y == b.y;
}

int rect_area(Rect r) {
    return (r.max.x - r.min.x) * (r.max.y - r.min.y);
}

int rect_contains(Rect r, Point p) {
    return p.x >= r.min.x && p.x <= r.max.x &&
           p.y >= r.min.y && p.y <= r.max.y;
}
