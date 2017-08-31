#ifndef SNAKE_H
#define SNAKE_H

#include <stdint.h>
#include "util.h"

#define INITIAL_LENGTH 3

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point direction;
    Point points[1000];
    int n_body_parts;
} Snake;

void snake_game();

#endif
