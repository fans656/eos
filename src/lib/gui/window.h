#ifndef _WINDOW_H
#define _WINDOW_H

#include "rect.h"
#include "color.h"
#include "painter.h"
#include "eos.h"
#include "stdio.h"
#include "list.h"
#include "pair.h"

struct Window;
struct ServerWindow;

constexpr uint WND_BORDER = (1 << 0);
constexpr uint WND_CAPTION = (1 << 1);
constexpr uint WND_TRANSPARENT = (1 << 2);
constexpr uint WND_KEEP_INACTIVE = (1 << 3);
constexpr uint WND_KEEP_BOTTOM = (1 << 4);
constexpr uint WND_NO_CLIENT_FILL = (1 << 5);

constexpr uint WND_DEFAULT = WND_BORDER | WND_CAPTION;
constexpr uint WND_CLIENT_ONLY = 0;

constexpr int DEF_WND_WIDTH = 400;
constexpr int DEF_WND_HEIGHT = 300;
constexpr int DEF_BORDER_WIDTH = 10;
constexpr int DEF_CAPTION_HEIGHT = 25;

enum MessageType {
    CREATE = 1024,
    DESTROY,
    PAINTED,
    UPDATE,

    ON_CREATE,
    ON_DESTROY,
    ON_ACTIVATE,
    ON_DEACTIVATE,
    ON_PAINT,
    ON_MOVE,
};

struct WindowMessage {
    WindowMessage(MessageType type) : type(type) {}
    MessageType type;
};

struct WindowEvent : public WindowMessage {
    WindowEvent(MessageType type) : WindowMessage(type) {}
};

struct CreateEvent : public WindowEvent {
    CreateEvent(Bitmap* font, ServerWindow* swnd)
        : WindowEvent(ON_CREATE), font(font), swnd(swnd) {}
    ServerWindow* swnd;
    Bitmap* font;
};

struct DestroyEvent : public WindowEvent {
    DestroyEvent() : WindowEvent(ON_DESTROY) {}
};

struct ActivateEvent : public WindowEvent {
    ActivateEvent() : WindowEvent(ON_ACTIVATE) {}
};

struct DeactivateEvent : public WindowEvent {
    DeactivateEvent() : WindowEvent(ON_DEACTIVATE) {}
};

struct PaintEvent : public WindowEvent {
    PaintEvent() : WindowEvent(ON_PAINT) {}
};

struct MoveEvent : public WindowEvent {
    MoveEvent(int x, int y) : WindowEvent(ON_MOVE), x(x), y(y) {}
    int x, y;
};

struct WindowRequest : public WindowMessage {
    WindowRequest(MessageType type, Window* wnd) : WindowMessage(type), wnd(wnd) {}
    Window* wnd;
};

struct CreateRequest : public WindowRequest {
    CreateRequest(Window* wnd) : WindowRequest(CREATE, wnd) {}
};

struct DestroyRequest : public WindowRequest {
    DestroyRequest(Window* wnd) : WindowRequest(DESTROY, wnd) {}
};

struct PaintedRequest : public WindowRequest {
    PaintedRequest(Window* wnd) : WindowRequest(PAINTED, wnd) {}
};

struct UpdateRequest : public WindowRequest {
    UpdateRequest(Rect rc, Window* wnd) : WindowRequest(UPDATE, wnd), rc(rc) {}
    Rect rc;
};

struct BaseWindow {
    void init(int x, int y, int width, int height, uint attr,
            int border_width, int caption_height) {
        x_ = x;
        y_ = y;
        width_ = width;
        height_ = height;
        attr_ = attr;
        border_width_ = attr & WND_BORDER ? border_width : 0;
        caption_height_ = attr & WND_CAPTION ? caption_height : 0;
        active_ = false;
    }
    
    int x() const { return x_; }
    int y() const { return y_; }
    int width() const { return width_; }
    int height() const { return height_; }
    Rect rect() const { return Rect(0, 0, width_, height_); }
    
    int margin_left() const { return border_width(); }
    int margin_right() const { return border_width(); }
    int margin_top() const { return border_width() + caption_height(); }
    int margin_bottom() const { return border_width(); }

    int window_width() const { return width() + margin_left() + margin_right(); }
    int window_height() const { return height() + margin_top() + margin_bottom(); }

    uint attr() const { return attr_; }
    int border_width() const { return border_width_; }
    int caption_height() const { return caption_height_; }
    
    Rect clip_top() const {
        return Rect(x(), y(), window_width(), margin_top());
    }
    
    Rect clip_top_border() const {
        return Rect(x(), y(), window_width(), border_width());
    }
    
    Rect clip_top_caption() const {
        return Rect(x(), y() + border_width(), window_width(), caption_height());
    }

    Rect clip_bottom() const {
        return Rect(x(), y() + window_height() - margin_bottom(),
                window_width(), margin_bottom());
    }

    Rect clip_left() const {
        return Rect(x(), y() + margin_top(), margin_left(), height());
    }

    Rect clip_right() const {
        return Rect(x() + window_width() - margin_right(), y() + margin_top(),
                margin_right(), height());
    }
    
    Rect client_rect_in_window_coord() const {
        return Rect(margin_left(), margin_top(), width(), height());
    }
    
    Rect client_rect_in_screen_coord() const {
        return Rect(x() + margin_left(), y() + margin_top(), width(), height());
    }
    
    Rect window_rect_in_window_coord() const {
        return Rect(0, 0, window_width(), window_height());
    }
    
    Rect window_rect_in_screen_coord() const {
        return Rect(x(), y(), window_width(), window_height());
    }
    
    Rect caption_rect_in_window_coord() const {
        return Rect(border_width(), border_width(), width(), caption_height());
    }

    bool transparent() const { return attr_ & WND_TRANSPARENT; }
    bool has_margin() const { return has_border() || has_caption(); }
    bool has_border() const { return attr_ & WND_BORDER; }
    bool has_caption() const { return attr_ & WND_CAPTION; }
    bool keep_bottom() const { return attr_ & WND_KEEP_BOTTOM; }
    bool keep_inactive() const { return attr_ & WND_KEEP_INACTIVE; }
    bool no_client_fill() const { return attr_ & WND_NO_CLIENT_FILL; }
    bool active() const { return active_; }
    
    void resize(int width, int height) {
        width_ = width;
        height_ = height;
    }
    
    void move(int x, int y) {
        x_ = x;
        y_ = y;
    }
    
    virtual void activate() { active_ = true; }
    virtual void deactivate() { active_ = false; }

    int x_;
    int y_;
    int width_;
    int height_;
    uint attr_;
    int border_width_;
    int caption_height_;
    bool active_;
    Bitmap* bitmap;
};

struct Window : public BaseWindow {
    Window(int x = 0, int y = 0,
            int width = DEF_WND_WIDTH, int height = DEF_WND_HEIGHT,
            uint attr = WND_DEFAULT) {
        init(x, y, width, height, attr, DEF_BORDER_WIDTH, DEF_CAPTION_HEIGHT);
    }
    
    virtual void on_create(CreateEvent* ev) {
        swnd = ev->swnd;
        Painter::init(ev->font);
    }

    virtual void on_destroy(DestroyEvent* ev) {
        destroyed_ = true;
    }
    virtual void on_activate(ActivateEvent* ev) {}
    virtual void on_deactivate(DeactivateEvent* ev) {}
    virtual void on_paint(PaintEvent* ev) {}
    virtual void on_timer(TimerEvent* ev) {}
    virtual void on_mouse(MouseEvent* ev) {
        if (ev->buttons == 2) {
            put_message(new DestroyRequest(this));
        }
    }
    virtual void on_key(KeyEvent* ev) {
        if (ev->up) {
            on_key_release(ev);
        } else {
            on_key_press(ev);
        }
    }
    virtual void on_key_press(KeyEvent* ev) {}
    virtual void on_key_release(KeyEvent* ev) {}
    
    void move(int x, int y) {
        x_ = x;
        y_ = y;
    }
    
    void resize(int width, int height) {
        width_ = width;
        height_ = height;
    }
    
    void create() {
        put_message(new CreateRequest(this));
    }
    
    void update() {
        put_message(new UpdateRequest(window_rect_in_screen_coord(), this));
    }
    
    void update(Rect rc) {
        put_message(new UpdateRequest(rc, this));
    }
    
    void exec() {
        create();
        while (!destroyed()) {
            auto ev = (WindowEvent*)get_message((uint)this);
            switch (ev->type) {
                case MOUSE_EVENT:
                    on_mouse((MouseEvent*)ev);
                    break;
                case KEY_EVENT:
                    on_key((KeyEvent*)ev);
                    break;
                case TIMER_EVENT:
                    on_timer((TimerEvent*)ev);
                    break;
                case ON_CREATE:
                    on_create((CreateEvent*)ev);
                    break;
                case ON_DESTROY:
                    on_destroy((DestroyEvent*)ev);
                    break;
                case ON_ACTIVATE:
                    activate();
                    on_activate((ActivateEvent*)ev);
                    break;
                case ON_DEACTIVATE:
                    deactivate();
                    on_deactivate((DeactivateEvent*)ev);
                    break;
                case ON_PAINT:
                    on_system_paint((PaintEvent*)ev);
                    on_paint((PaintEvent*)ev);
                    put_message(new PaintedRequest(this));
                    break;
            }
            delete ev;
        }
    }
    
    void on_system_paint(PaintEvent* ev) {
        Painter painter(bitmap, window_rect_in_window_coord());
        uint caption_color = active() ? SteelBlue : LightSteelBlue;
        caption_color &= active() ? 0xddffffff : 0xccffffff;
        if (has_border()) {
            draw_border(painter, caption_color);
        }
        if (has_caption()) {
            draw_caption(painter, caption_color);
        }
        if (!no_client_fill()) {
            painter._fill_rect_solid(client_rect_in_window_coord(), 0);
        }
    }
    
    void draw_border(Painter& painter, uint caption_color) {
        Rect rc(0, 0, window_width(), window_height());
        auto max_alpha = active() ? 40 : 10;
        auto dalpha = max_alpha / DEF_BORDER_WIDTH;
        for (int i = 0; i < DEF_BORDER_WIDTH - 1; ++i) {
            painter.set_pen_color(i * dalpha << 24);
            painter.draw_rect(rc);
            rc.adjust(1, 1, -1, -1);
        }
        painter.set_pen_color(caption_color);
        painter.draw_rect(rc);
    }
    
    void draw_caption(Painter& painter, uint caption_color) {
        auto rc = caption_rect_in_window_coord();
        painter.fill_rect(rc, caption_color);
        //painter.draw_text(rc.left() + 8, rc.top() + (rc.height() - 18) / 2, "hello");
    }
    
    void put_message(WindowRequest* request) {
        ::put_message(QUEUE_ID_GUI, request);
    }
    
    bool destroyed() const { return destroyed_; }
    
    ServerWindow* swnd;
    bool destroyed_ = false;
};

struct ServerWindow : public BaseWindow {
    ServerWindow(Window* wnd) : wnd(wnd) {
        init(wnd->x(), wnd->y(), wnd->width(), wnd->height(), wnd->attr(),
                wnd->border_width(), wnd->caption_height());
        bitmap = wnd->bitmap = new Bitmap(wnd->window_width(), wnd->window_height());
    }
    
    void create(Bitmap* font) {
        put_message(new CreateEvent(font, this));
    }
    
    void destroy() {
        put_message(new DestroyEvent);
    }
    
    void paint() {
        put_message(new PaintEvent);
    }
    
    void move(int x, int y) {
        BaseWindow::move(x, y);
        put_message(new MoveEvent(x, y));
    }
    
    void activate() {
        BaseWindow::activate();
        put_message(new ActivateEvent);
    }
    
    void deactivate() {
        BaseWindow::deactivate();
        put_message(new DeactivateEvent);
    }
    
    void mouse_press(MouseEvent* ev) {
        auto rc = client_rect_in_screen_coord();
        auto ev_ = new MouseEvent(ev->x - rc.left(), ev->y - rc.top(), ev->buttons);
        put_message((WindowEvent*)ev_);
    }
    
    void mouse_release(MouseEvent* ev) {
        auto rc = client_rect_in_screen_coord();
        auto ev_ = new MouseEvent(ev->x - rc.left(), ev->y - rc.top(), ev->buttons);
        put_message((WindowEvent*)ev_);
    }
    
    void keyevent(KeyEvent* ev) {
        put_message((WindowEvent*)new KeyEvent(ev->key, ev->up));
    }
    
    bool hit_test_activate(int x, int y) {
        return window_rect_in_screen_coord().contains(x, y);
    }
    
    bool hit_test_drag(int x, int y) {
        return active() && window_rect_in_screen_coord().contains(x, y);
    }
    
    bool clip(List<Rect>& rcs) {
        if (no_client_fill()) {
            for (auto& rc: rcs) {
                clips.append(make_pair(rc, false));
            }
            return true;
        } else {
            for (auto& rc: rcs) {
                auto wnd_rc = window_rect_in_screen_coord().intersected(rc);
                if (!wnd_rc.empty()) {
                    clips.append(make_pair(wnd_rc, true));
                }
            }
            return true;
        }
        //bool to_draw = false;
        //auto size = rcs.size();
        //for (auto i = 0; i < size; ++i) {
        //    auto invalid_rc = rcs.popleft();
        //    auto wnd_rc = window_rect_in_screen_coord().intersected(invalid_rc);
        //    if (wnd_rc.empty()) {
        //        rcs.append(invalid_rc);
        //        continue;
        //    }
        //    auto client_rc = client_rect_in_screen_coord().intersected(wnd_rc);
        //    if (!client_rc.empty()) {
        //        auto alpha = transparent();
        //        clips.append(make_pair(client_rc, alpha));
        //        to_draw = true;
        //        if (alpha) {
        //            rcs.append(invalid_rc);
        //        } else {
        //            rcs.extend(invalid_rc - client_rc);
        //        }
        //    } else {
        //        rcs.append(invalid_rc);
        //    }
        //    if (has_margin()) {
        //        clip_margin(clip_top(), wnd_rc, to_draw);
        //        clip_margin(clip_bottom(), wnd_rc, to_draw);
        //        clip_margin(clip_left(), wnd_rc, to_draw);
        //        clip_margin(clip_right(), wnd_rc, to_draw);
        //    }
        //}
        //return to_draw;
    }
    
    void clip_margin(const Rect& margin_rc, const Rect& wnd_rc, bool& to_draw) {
        auto draw_rc = margin_rc.intersected(wnd_rc);
        if (!draw_rc.empty()) {
            clips.append(make_pair(draw_rc, true));
            to_draw = true;
        }
    }
    
    void blit(Painter& painter) {
        for (auto pair: clips) {
            auto rc = pair.first;
            auto alpha = pair.second;
            painter.set_alpha_blending(alpha);
            painter.draw_bitmap(rc, bitmap, rc.translated(-x(), -y()));
        }
        clips.clear();
    }
    
    void put_message(WindowEvent* event) {
        ::put_message((uint)wnd, event);
    }
    
    Window* wnd;
    List<Pair<Rect, bool>> clips;
};

#endif
