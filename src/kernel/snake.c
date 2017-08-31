#include <stdbool.h>
#include "interrupt.h"
#include "snake.h"
#include "math.h"
#include "io.h"

void snake_init(Snake* snake) {
    snake->direction.x = 1;
    snake->direction.y = 0;

    snake->n_body_parts = 2;

    snake->points[0].y = 25 / 2;
    snake->points[0].x = 80 / 2;

    snake->points[1].y = snake->points[0].y;
    snake->points[1].x = snake->points[0].x - INITIAL_LENGTH;
}

int snake_length(Snake* snake) {
    int n = 0;
    for (int i = 0; i < snake->n_body_parts - 1; ++i) {
        n += abs(snake->points[i].x - snake->points[i + 1].x);
        n += abs(snake->points[i].y - snake->points[i + 1].y);
    }
    return n;
}

void draw(Snake* snake) {
    clear_screen();
    for (int i = 0; i < snake->n_body_parts - 1; ++i) {
        Point* pt1 = &snake->points[i];
        Point* pt2 = &snake->points[i + 1];

        int diff_x = pt2->x - pt1->x;
        int diff_y = pt2->y - pt1->y;
        int dx = 0;
        if (diff_x) {
            dx = diff_x > 0 ? 1 : -1;
        }
        int dy = 0;
        if (diff_y) {
            dy = diff_y > 0 ? 1 : -1;
        }
        for (int x = pt1->x, y = pt1->y; x != pt2->x || y != pt2->y; x += dx, y+= dy) {
            set_cursor_pos(y, x);
            put_char('*');
        }
    }
    set_cursor_pos(0, 0);
    print_int(snake_length(snake));
}

int unitilized(int val) {
    return val ? (val > 0 ? 1 : -1) : 0;
}

bool same_point(Point* pt1, Point* pt2) {
    return pt1->x == pt2->x && pt1->y == pt2->y;
}

bool point_on_line(int x, int y, Point* beg, Point* end) {
    if (x == beg->x && x == end->x) {
        return beg->y <= y && y < end->y;
    }
    if (y == beg->y && y == end->y) {
        return beg->x <= x && x < end->x;
    }
    return false;
}

void forward_tail(Snake* snake) {
    Point* pt2 = &snake->points[0] + snake->n_body_parts - 1;
    Point* pt1 = pt2 - 1;
    pt2->x += unitilized(pt1->x - pt2->x);
    pt2->y += unitilized(pt1->y - pt2->y);
    if (same_point(pt1, pt2)) {
        --snake->n_body_parts;
    }
}

bool same_direction(Snake* snake, int dx, int dy) {
    return snake->direction.x == dx && snake->direction.y == dy;
}

void forward_head(Snake* snake, int dx, int dy) {
    snake->points[0].x += dx;
    snake->points[0].y += dy;
    snake->direction.x = dx;
    snake->direction.y = dy;
}

void offset_body(Snake* snake) {
    for (int i = snake->n_body_parts; i > 0; --i) {
        snake->points[i] = snake->points[i - 1];
    }
}

void new_body_part(Snake* snake) {
    snake->n_body_parts += 1;
}

void snake_move(Snake* snake, int dx, int dy, int* food_x, int* food_y) {
    if (same_direction(snake, dx, dy)) {
        forward_head(snake, dx, dy);
    } else {
        offset_body(snake);
        forward_head(snake, dx, dy);
        new_body_part(snake);
    }
    bool eaten = false;
    if (snake->points[0].x == *food_x && snake->points[0].y == *food_y) {
        eaten = true;
        *food_x = -1;
    }
    if (!eaten) {
        forward_tail(snake);
    }
}

int snake_dead(Snake* snake) {
    int x = snake->points[0].x;
    int y = snake->points[0].y;
    if (x < 0 || y < 0 || x > 79 || y > 24) {
        return true;
    }
    for (int i = 1; i < snake->n_body_parts - 1; ++i) {
        if (point_on_line(x, y, &snake->points[i], &snake->points[i + 1])) {
            return true;
        }
    }
    return false;
}

void get_direction_from_user(int* dx, int* dy) {
    int key = get_char_nonblocking();
    int prev_dx = *dx, prev_dy = *dy;
    switch (key) {
        case KEY_UP:
            *dx = 0; *dy = -1;
            break;
        case KEY_DOWN:
            *dx = 0; *dy = 1;
            break;
        case KEY_LEFT:
            *dx = -1; *dy = 0;
            break;
        case KEY_RIGHT:
            *dx = 1; *dy = 0;
            break;
    }
    if (*dx == -prev_dx && *dy == -prev_dy) {
        *dx = prev_dx;
        *dy = prev_dy;
    }
}

void draw_food(int x, int y) {
    if (x >= 0) {
        set_cursor_pos(x, y);
        put_char('o');
        set_cursor_pos(x, y);
    }
}

void gen_food(Snake* snake, int* x, int* y) {
    *x = 10;
    *y = 10;
}

void snake_game() {
    while (1) {
        Snake snake;
        snake_init(&snake);
        draw(&snake);

        set_cursor_pos(snake.points[0].y + 2, 29);
        print_str("Press any key to start");
        set_cursor_pos(snake.points[0].y, snake.points[0].x + 1);
        print_str(">");
        get_char();

        int dx = 1, dy = 0;
        int food_x = -1, food_y = -1;
        gen_food(&snake, &food_x, &food_y);
        while (!snake_dead(&snake)) {
            draw(&snake);
            draw_food(food_x, food_y);

            get_direction_from_user(&dx, &dy);
            snake_move(&snake, dx, dy, &food_x, &food_y);
            if (food_x < 0) {
                gen_food(&snake, &food_x, &food_y);
            }
            sleep(1);
        }
        set_cursor_pos(10, 30);
        print_str("  YOU ARE DEAD!!!  ");
        sleep(20);
        get_char();
        sleep(5);
    }
}
