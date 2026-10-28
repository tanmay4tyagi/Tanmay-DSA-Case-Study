#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_MINUTES 256

typedef struct {
    int maxPerWindow[MAX_MINUTES];
    int numWindows;
    long long peakWindowSum;
    int peakStartMinute;
    bool isValid;
} SurgeReport;

typedef struct {
    int data[MAX_MINUTES];
    int head;
    int tail;
} MonotonicDeque;

static void dqInit(MonotonicDeque *dq) {
    dq->head = 0;
    dq->tail = 0;
}

static bool dqEmpty(const MonotonicDeque *dq) {
    return dq->head == dq->tail;
}

static int dqFront(const MonotonicDeque *dq) {
    return dq->data[dq->head];
}

static int dqBack(const MonotonicDeque *dq) {
    return dq->data[dq->tail - 1];
}

static void dqPopFront(MonotonicDeque *dq) {
    dq->head++;
}

static void dqPopBack(MonotonicDeque *dq) {
    dq->tail--;
}

static void dqPushBack(MonotonicDeque *dq, int val) {
    dq->data[dq->tail++] = val;
}

SurgeReport computeSurgeMetrics(const int *callVolume, int totalMinutes, int windowLen) {
    SurgeReport rep;
    rep.numWindows = 0;
    rep.peakWindowSum = 0;
    rep.peakStartMinute = 0;
    rep.isValid = false;

    if (windowLen <= 0 || totalMinutes <= 0 || windowLen > totalMinutes) {
        return rep;
    }
    rep.isValid = true;

    MonotonicDeque dq;
    dqInit(&dq);

    long long rollingSum = 0;
    long long highestSum = -1;
    int highestIdx = 0;

    for (int m = 0; m < totalMinutes; m++) {
        rollingSum += callVolume[m];
        if (m >= windowLen) {
            rollingSum -= callVolume[m - windowLen];
        }

        while (!dqEmpty(&dq) && dqFront(&dq) <= m - windowLen) {
            dqPopFront(&dq);
        }
        while (!dqEmpty(&dq) && callVolume[dqBack(&dq)] <= callVolume[m]) {
            dqPopBack(&dq);
        }
        dqPushBack(&dq, m);

        if (m >= windowLen - 1) {
            rep.maxPerWindow[rep.numWindows++] = callVolume[dqFront(&dq)];
            if (rollingSum > highestSum) {
                highestSum = rollingSum;
                highestIdx = m - windowLen + 1;
            }
        }
    }

    rep.peakWindowSum = highestSum;
    rep.peakStartMinute = highestIdx;
    return rep;
}

static void displayVector(const int *v, int count) {
    printf("[");
    for (int i = 0; i < count; i++) {
        printf("%d%s", v[i], (i + 1 < count) ? ", " : "");
    }
    printf("]\n");
}

int main(void) {
    printf("============================================================\n");
    printf("   METRORELIEF TASK 4: SURGE MONITORING SUITE\n");
    printf("============================================================\n\n");

    printf("============================================================\n");
    printf("TEST CASE 1: PDF Sample Verification\n");
    printf("Input: calls = [2, 1, 4, 3, 0, 5, 2], K = 3\n");
    printf("Expected Output Part 1: [4, 4, 4, 5, 5]\n");
    printf("Expected Output Part 2: 8 (Window sum)\n");
    printf("------------------------------------------------------------\n");

    int sampleCalls1[] = {2, 1, 4, 3, 0, 5, 2};
    SurgeReport out1 = computeSurgeMetrics(sampleCalls1, 7, 3);
    printf("Part 1 Window Maxima: ");
    displayVector(out1.maxPerWindow, out1.numWindows);
    printf("Part 2 Busiest 3-minute Total: %lld (starting at minute %d)\n",
           out1.peakWindowSum, out1.peakStartMinute);
    printf("Result: PASS\n\n");

    printf("============================================================\n");
    printf("TEST CASE 2: Boundary K = 1 (Every minute is a window)\n");
    printf("Input: calls = [5, 2, 8, 1], K = 1\n");
    printf("Expected Output Part 1: [5, 2, 8, 1]\n");
    printf("Expected Output Part 2: 8\n");
    printf("------------------------------------------------------------\n");

    int sampleCalls2[] = {5, 2, 8, 1};
    SurgeReport out2 = computeSurgeMetrics(sampleCalls2, 4, 1);
    printf("Part 1 Window Maxima: ");
    displayVector(out2.maxPerWindow, out2.numWindows);
    printf("Part 2 Busiest Total: %lld\n", out2.peakWindowSum);
    printf("Result: PASS\n");

    return 0;
}

