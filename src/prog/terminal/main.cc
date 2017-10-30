#include "stdio.h"
#include "stdlib.h"
#include "gui.h"
#include "eos.h"
#include "unistd.h"
#include "algorithm.h"
#include "time.h"

static constexpr int CHAR_WIDTH = 8;
static constexpr int CHAR_HEIGHT = 18;
static constexpr int TERM_COLS = 80;
static constexpr int TERM_ROWS = 25;

struct Wnd : public Window {
    Wnd() : cursor_rc(0, 2, CHAR_WIDTH, CHAR_HEIGHT - 2) {
        move(500, 200);
        resize(TERM_COLS * CHAR_WIDTH, TERM_ROWS * CHAR_HEIGHT);
    }
    
    void on_create(CreateEvent* ev) {
        Window::on_create(ev);
        set_timer(600, (uint)this);
        clear();
        print("$ help");
    }
    
    void on_timer(TimerEvent* ev) {
        cursor_on = !cursor_on;
        update();
    }
    
    void on_paint(PaintEvent* ev) {
        Painter painter(this);
        painter.fill_rect(rect(), 0xdd000000);
        for (int i = 0; i < TERM_ROWS; ++i) {
            for (int j = 0; j < TERM_COLS; ++j) {
                painter.draw_char(j * CHAR_WIDTH, i * CHAR_HEIGHT,
                        buffer[i * TERM_COLS + j]);
            }
        }
        if (active() && cursor_on) {
            cursor_rc.move_to(cursor_col * CHAR_WIDTH, cursor_row * CHAR_HEIGHT + 2);
            painter.fill_rect(cursor_rc, GhostWhite);
        }
    }
    
    void on_key_press(KeyEvent* ev) {
        uint key = ev->key;
        print(key, true);
    }
    
    void clear() {
        for (int i = 0; i < TERM_ROWS * TERM_COLS; ++i) {
            buffer[i] = ' ';
        }
    }

    void print(char ch, bool update = false) {
        char* q = buffer + cursor_row * TERM_COLS + cursor_col;
        *q = ch;
        ++cursor_col;
        if (update) {
            this->update();
        }
    }
    
    void print(const char* s, bool update = true) {
        for (const char* p = s; *p; ++p) {
            print(*p);
        }
        if (update) {
            this->update();
        }
    }
    
    bool cursor_on = false;
    Rect cursor_rc;
    char buffer[TERM_ROWS * TERM_COLS];
    int cursor_row = 0;
    int cursor_col = 0;
};

int main() {
    sleep(1000);
    Wnd* wnd = new Wnd();
    wnd->exec();
}
