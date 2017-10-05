#ifndef GUITYPES_H
#define GUITYPES_H

#include "../common/def.h"
#include "list.h"

////////////////////////////////////////////////////////// enum

typedef enum {
    KeyboardEventType, PaintEventType,
} EventType;

typedef enum {
    Unpainted, Painting, Painted,
} PaintState;

////////////////////////////////////////////////////////// event

typedef struct _Event {
    EventType type;
} _Event;
typedef _Event* Event;

typedef struct _PaintEvent {
    EventType type;
} _PaintEvent;
typedef _PaintEvent* PaintEvent;

typedef struct _KeyboardEvent {
    EventType type;
    uint key;
    bool up;
} _KeyboardEvent;
typedef _KeyboardEvent* KeyboardEvent;

////////////////////////////////////////////////////////// window

typedef struct _Process* Process;

typedef struct _Window {
    Process proc;
    int width;
    int height;
    // add these cause page fault, why?
    //int x;
    //int y;

    List events;
    PaintState paint_state;
} _Window;
typedef struct _Window* Window;

typedef uint (*WndProc)(Event);

#endif
