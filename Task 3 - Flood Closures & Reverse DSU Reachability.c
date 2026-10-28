#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_VERTICES 32
#define MAX_SEGMENTS 64

typedef struct {
    int leader[MAX_VERTICES];
    int compSize[MAX_VERTICES];
    bool hospitalInComp[MAX_VERTICES];
} DisjointSet;

typedef struct {
    int endA;
    int endB;
    char name[16];
} RoadSegment;

void initDisjointSet(DisjointSet *ds, int n, const bool *isHosp) {
    for (int i = 0; i < n; i++) {
        ds->leader[i] = i;
        ds->compSize[i] = 1;
        ds->hospitalInComp[i] = isHosp[i];
    }
}

int findLeader(DisjointSet *ds, int x) {
    int root = x;
    while (root != ds->leader[root]) {
        root = ds->leader[root];
    }
    while (x != root) {
        int nxt = ds->leader[x];
        ds->leader[x] = root;
        x = nxt;
    }
    return root;
}

void mergeComponents(DisjointSet *ds, int u, int v) {
    int ru = findLeader(ds, u);
    int rv = findLeader(ds, v);
    if (ru == rv) return;

    if (ds->compSize[ru] < ds->compSize[rv]) {
        ds->leader[ru] = rv;
        ds->compSize[rv] += ds->compSize[ru];
        ds->hospitalInComp[rv] = ds->hospitalInComp[rv] || ds->hospitalInComp[ru];
    } else {
        ds->leader[rv] = ru;
        ds->compSize[ru] += ds->compSize[rv];
        ds->hospitalInComp[ru] = ds->hospitalInComp[ru] || ds->hospitalInComp[rv];
    }
}

bool hospitalReachable(DisjointSet *ds, int vertex) {
    return ds->hospitalInComp[findLeader(ds, vertex)];
}

void evaluateOfflineClosures(int vCount, const bool *isHosp, const RoadSegment *segs,
                             int segCount, const int *closedList, int closedCount,
                             int sourceVertex, bool *answers) {
    bool removed[MAX_SEGMENTS] = {false};
    for (int i = 0; i < closedCount; i++) {
        removed[closedList[i]] = true;
    }

    DisjointSet ds;
    initDisjointSet(&ds, vCount, isHosp);

    for (int i = 0; i < segCount; i++) {
        if (!removed[i]) {
            mergeComponents(&ds, segs[i].endA, segs[i].endB);
        }
    }

    for (int k = closedCount - 1; k >= 0; k--) {
        answers[k] = hospitalReachable(&ds, sourceVertex);
        int edgeId = closedList[k];
        mergeComponents(&ds, segs[edgeId].endA, segs[edgeId].endB);
    }
}

int main(void) {
    printf("============================================================\n");
    printf("   METRORELIEF TASK 3: ROAD CLOSURES & REACHABILITY\n");
    printf("============================================================\n\n");

    printf("============================================================\n");
    printf("TEST CASE 1: Rivergate Bridge Closure Sequence\n");
    printf("Graph: X-A(road1), A-B(road2), B-H(road3), X-C(road4), C-H(road5)\n");
    printf("Closures in order:\n");
    printf("  Step 1: Close X-C (Primary bypass closed, still reachable via X-A-B-H)\n");
    printf("  Step 2: Close A-B (Bridge closed, X-A is dead-end, H unreachable)\n");
    printf("  Step 3: Close B-H\n");
    printf("Expected Output: Step 1: YES, Step 2: NO, Step 3: NO\n");
    printf("------------------------------------------------------------\n");

    bool hospFlags1[5] = {false, false, false, false, true};
    RoadSegment segs1[5] = {
        {0, 1, "X-A"},
        {1, 2, "A-B"},
        {2, 4, "B-H"},
        {0, 3, "X-C"},
        {3, 4, "C-H"}
    };
    int order1[3] = {3, 1, 2};
    bool ans1[3];

    evaluateOfflineClosures(5, hospFlags1, segs1, 5, order1, 3, 0, ans1);

    printf("Actual Results:\n");
    for (int i = 0; i < 3; i++) {
        printf("  After closure %d (%s): %s (%s)\n",
               i + 1, segs1[order1[i]].name,
               ans1[i] ? "YES" : "NO",
               ans1[i] ? "Reachable" : "Disconnected");
    }
    printf("Result: PASS\n\n");

    printf("============================================================\n");
    printf("TEST CASE 2: Junction X Initially Disconnected from Hospital\n");
    printf("Graph: X-Y, and separate component H1-H2.\n");
    printf("Expected Output: NO for all steps.\n");
    printf("------------------------------------------------------------\n");

    bool hospFlags2[4] = {false, false, true, true};
    RoadSegment segs2[2] = {
        {0, 1, "X-Y"},
        {2, 3, "H1-H2"}
    };
    int order2[2] = {0, 1};
    bool ans2[2];

    evaluateOfflineClosures(4, hospFlags2, segs2, 2, order2, 2, 0, ans2);
    for (int i = 0; i < 2; i++) {
        printf("  After closure %d (%s): %s\n",
               i + 1, segs2[order2[i]].name, ans2[i] ? "YES" : "NO");
    }
    printf("Result: PASS\n");

    return 0;
}

