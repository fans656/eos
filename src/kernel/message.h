#ifndef MESSAGE_H
#define MESSAGE_H

#include "def.h"
#include "list.h"
#include "process.h"

struct MessageQueue {
    List<void*> messages;
    List<Process> blocked_procs;
    
    bool empty() const { return messages.empty(); }
    void* get() { return messages.popleft(); }

    void put(void* msg) {
        messages.append(msg);
        if (!blocked_procs.empty()) {
            auto proc = blocked_procs.peekleft();
            process_unblock(proc);
        }
    }
    
    void replace(void* msg) {
        while (!messages.empty()) {
            messages.pop();
        }
        messages.append(msg);
        if (!blocked_procs.empty()) {
            auto proc = blocked_procs.peekleft();
            process_unblock(proc);
        }
    }
};

extern "C" void* get_message(int id, bool blocking);
extern "C" void put_message(int id, void* message);
extern "C" void replace_message(int id, void* message);
void init_message();

#endif
