#include "io.h"
#include "util.h"
#include "draw.h"
#include "interrupt.h"
#include "filesystem.h"
#include "malloc.h"

void main();

void kernel_entry() {
    clear_screen();
    setup_idt();
    init_filesystem();
    init_memory();
    main();
    hlt();
}

typedef struct Node {
    int val;
    struct Node* next;
} Node;

Node* build_list(int a[], int n) {
    Node* head = malloc(sizeof(Node));
    Node* tail = head;
    for (int i = 0; i < n; ++i) {
        Node* node = (Node*)malloc(sizeof(Node));
        node->val = a[i];
        node->next = 0;
        tail->next = node;
        tail = node;
    }
    return head->next;
}

void main() {
    int a[] = {1,2,3,4,5};
    Node* p = build_list(a, 5);
    while (p) {
        printf("%d\n", p->val);
        p = p->next;
    }
}
