#ifndef GUITYPES_H
#define GUITYPES_H

#include "../common/def.h"
#include "list.h"

constexpr int DEFAULT_WINDOW_WIDTH = 320;
constexpr int DEFAULT_WINDOW_HEIGHT = 240;

////////////////////////////////////////////////////////// enum

enum EventType {
    KeyboardEventType, PaintEventType,
};

enum PaintState {
    Unpainted, Painting, Painted,
};

////////////////////////////////////////////////////////// event

struct Event {
    EventType type;
};

struct PaintEvent : Event {
    PaintEvent() { type = PaintEventType; }
};

struct KeyboardEvent : Event {
    uint key;
    bool up;
    KeyboardEvent(uint key, bool up) : key(key), up(up) { type = KeyboardEventType; }
};

////////////////////////////////////////////////////////// window

typedef struct _Process* Process;

struct Window {
    int width;
    int height;
    int x;
    int y;
    Process proc;
    List<Event*> events;
    PaintState paint_state;
};

typedef uint (*WndProc)(Event*);

#endif
