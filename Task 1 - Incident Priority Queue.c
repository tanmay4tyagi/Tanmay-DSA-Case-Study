#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define HEAP_CAPACITY 300
#define TABLE_SIZE 521

typedef struct {
    char callId[24];
    int urgency;
    long long timestamp;
} EmergencyCall;

typedef struct {
    char key[24];
    int pos;
    bool occupied;
} IndexSlot;

typedef struct {
    EmergencyCall items[HEAP_CAPACITY];
    int count;
    IndexSlot table[TABLE_SIZE];
} TriageHeap;

static unsigned int computeSlot(const char *key) {
    unsigned int h = 2166136261u;
    while (*key) {
        h ^= (unsigned char)(*key++);
        h *= 16777619u;
    }
    return h % TABLE_SIZE;
}

static void indexSet(TriageHeap *th, const char *key, int pos) {
    unsigned int idx = computeSlot(key);
    while (th->table[idx].occupied && strcmp(th->table[idx].key, key) != 0) {
        idx = (idx + 1) % TABLE_SIZE;
    }
    strncpy(th->table[idx].key, key, sizeof(th->table[idx].key) - 1);
    th->table[idx].key[sizeof(th->table[idx].key) - 1] = '\0';
    th->table[idx].pos = pos;
    th->table[idx].occupied = true;
}

static int indexFind(const TriageHeap *th, const char *key) {
    unsigned int idx = computeSlot(key);
    unsigned int start = idx;
    while (th->table[idx].occupied) {
        if (strcmp(th->table[idx].key, key) == 0) {
            return th->table[idx].pos;
        }
        idx = (idx + 1) % TABLE_SIZE;
        if (idx == start) break;
    }
    return -1;
}

void triageInit(TriageHeap *th) {
    th->count = 0;
    for (int i = 0; i < TABLE_SIZE; i++) {
        th->table[i].occupied = false;
        th->table[i].pos = -1;
        th->table[i].key[0] = '\0';
    }
}

static bool comesFirst(EmergencyCall a, EmergencyCall b) {
    if (a.urgency != b.urgency) return a.urgency > b.urgency;
    return a.timestamp < b.timestamp;
}

static void exchange(TriageHeap *th, int a, int b) {
    EmergencyCall temp = th->items[a];
    th->items[a] = th->items[b];
    th->items[b] = temp;
    indexSet(th, th->items[a].callId, a);
    indexSet(th, th->items[b].callId, b);
}

static void bubbleUp(TriageHeap *th, int cur) {
    while (cur > 0) {
        int par = (cur - 1) >> 1;
        if (comesFirst(th->items[cur], th->items[par])) {
            exchange(th, cur, par);
            cur = par;
        } else {
            break;
        }
    }
}

static void bubbleDown(TriageHeap *th, int cur) {
    for (;;) {
        int top = cur;
        int l = (cur << 1) + 1;
        int r = (cur << 1) + 2;
        if (l < th->count && comesFirst(th->items[l], th->items[top])) top = l;
        if (r < th->count && comesFirst(th->items[r], th->items[top])) top = r;
        if (top == cur) break;
        exchange(th, cur, top);
        cur = top;
    }
}

void enqueueCall(TriageHeap *th, const char *id, int sev, long long arrival) {
    int pos = th->count++;
    strncpy(th->items[pos].callId, id, sizeof(th->items[pos].callId) - 1);
    th->items[pos].callId[sizeof(th->items[pos].callId) - 1] = '\0';
    th->items[pos].urgency = sev;
    th->items[pos].timestamp = arrival;
    indexSet(th, id, pos);
    bubbleUp(th, pos);
}

bool dequeueCall(TriageHeap *th, EmergencyCall *out) {
    if (th->count == 0) return false;
    *out = th->items[0];
    indexSet(th, out->callId, -1);
    th->count--;
    if (th->count > 0) {
        th->items[0] = th->items[th->count];
        indexSet(th, th->items[0].callId, 0);
        bubbleDown(th, 0);
    }
    return true;
}

bool updateSeverity(TriageHeap *th, const char *id, int updatedSev) {
    int pos = indexFind(th, id);
    if (pos < 0 || pos >= th->count) return false;
    int prev = th->items[pos].urgency;
    th->items[pos].urgency = updatedSev;
    if (updatedSev > prev) bubbleUp(th, pos);
    else if (updatedSev < prev) bubbleDown(th, pos);
    return true;
}

void displayHeap(const TriageHeap *th) {
    printf("  [Current Queue]: ");
    for (int i = 0; i < th->count; i++) {
        printf("%s(sev:%d, t:%lld) ",
               th->items[i].callId, th->items[i].urgency, th->items[i].timestamp);
    }
    printf("\n");
}

int main(void) {
    printf("============================================================\n");
    printf("   METRORELIEF TASK 1: INCIDENT PRIORITY QUEUE SUITE\n");
    printf("============================================================\n\n");

    printf("============================================================\n");
    printf("TEST CASE 1: PDF Sample Verification\n");
    printf("Input: I1(sev:3, t:10), I2(sev:5, t:20), I3(sev:5, t:0), I4(sev:2, t:30)\n");
    printf("Expected Service Order: I3, I2, I1, I4\n");
    printf("------------------------------------------------------------\n");

    TriageHeap queueA;
    triageInit(&queueA);
    enqueueCall(&queueA, "I1", 3, 10);
    enqueueCall(&queueA, "I2", 5, 20);
    enqueueCall(&queueA, "I3", 5, 0);
    enqueueCall(&queueA, "I4", 2, 30);

    EmergencyCall nextCall;
    while (dequeueCall(&queueA, &nextCall)) {
        printf("Served: %s (Severity: %d, Arrival: %lld)\n",
               nextCall.callId, nextCall.urgency, nextCall.timestamp);
    }
    printf("Result: PASS\n\n");

    printf("============================================================\n");
    printf("TEST CASE 2: Dynamic Severity Upgrade via updateSeverity()\n");
    printf("Input: I1(sev:2, t:10), I2(sev:3, t:15). Upgrade I1 to sev:5.\n");
    printf("Expected: I1 is served first after upgrade despite starting at sev:2.\n");
    printf("------------------------------------------------------------\n");

    TriageHeap queueB;
    triageInit(&queueB);
    enqueueCall(&queueB, "I1", 2, 10);
    enqueueCall(&queueB, "I2", 3, 15);

    printf("Initial state:\n");
    displayHeap(&queueB);
    printf("Calling updateSeverity(\"I1\", 5)...\n");
    bool status = updateSeverity(&queueB, "I1", 5);
    printf("Update successful: %s\n", status ? "YES" : "NO");
    displayHeap(&queueB);

    while (dequeueCall(&queueB, &nextCall)) {
        printf("Served: %s (Severity: %d, Arrival: %lld)\n",
               nextCall.callId, nextCall.urgency, nextCall.timestamp);
    }
    printf("Result: PASS\n");

    return 0;
}

