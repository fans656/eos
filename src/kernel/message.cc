#include "message.h"
#include "array.h"
#include "list.h"
#include "dict.h"
#include "process.h"
#include "stdio.h"

Dict<int, MessageQueue*> queues;

MessageQueue* get_queue(int id) {
    if (!queues.has_key(id)) {
        queues.add(id, new MessageQueue);
    }
    return queues[id];
}

void* get_message(int id, bool blocking) {
    auto q = get_queue(id);
    q->empty();
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

extern "C" void replace_message(int id, void* message) {
    auto q = get_queue(id);
    q->replace(message);
}

void init_message() {
    queues.construct();
}
