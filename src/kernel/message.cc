#include "message.h"
#include "array.h"
#include "list.h"
#include "dict.h"
#include "process.h"
#include "stdio.h"

struct MessageQueue {
    List<void*> messages;
    List<Process> blocked_procs;
    
    bool empty() const { return messages.empty(); }
    void* get() { return messages.popleft(); }

    void put(void* msg) {
        messages.append(msg);
        if (!blocked_procs.empty()) {
            auto proc = blocked_procs.popleft();
            process_unblock(proc);
        }
    }
};

Dict<int, MessageQueue*> queues;

MessageQueue* get_queue(int id) {
    if (!queues.has_key(id)) {
        queues.add(id, new MessageQueue);
    }
    return queues[id];
}

extern "C" void* get_message(int id, bool blocking) {
    auto q = get_queue(id);
    if (q->empty()) {
        if (blocking) {
            q->blocked_procs.append(running_proc);
            process_block();
        }
        return 0;
    } else {
        return q->get();
    }
}

extern "C" void put_message(int id, void* message) {
    auto q = get_queue(id);
    q->put(message);
}

void init_message() {
    queues.construct();
}
