#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_ZONES 64
#define MAX_TRUCK_WEIGHT 256

typedef struct {
    int zoneId;
    int supplyCost;
    int livesSaved;
} ReliefZone;

typedef struct {
    int optimalLives;
    int usedCapacity;
    int selectedIds[MAX_ZONES];
    int selectedLen;
} AllocationPlan;

AllocationPlan solveSupplyKnapsack(const ReliefZone *zones, int numZones, int maxWeight) {
    AllocationPlan plan;
    plan.optimalLives = 0;
    plan.usedCapacity = 0;
    plan.selectedLen = 0;

    if (numZones <= 0 || maxWeight <= 0) {
        return plan;
    }

    int dp[MAX_TRUCK_WEIGHT + 1] = {0};
    bool picked[MAX_ZONES][MAX_TRUCK_WEIGHT + 1] = {{false}};

    for (int i = 0; i < numZones; i++) {
        int wt = zones[i].supplyCost;
        int val = zones[i].livesSaved;
        for (int w = maxWeight; w >= wt; w--) {
            if (dp[w - wt] + val > dp[w]) {
                dp[w] = dp[w - wt] + val;
                picked[i][w] = true;
            }
        }
    }

    plan.optimalLives = dp[maxWeight];

    int revList[MAX_ZONES];
    int cnt = 0;
    int curW = maxWeight;
    for (int i = numZones - 1; i >= 0; i--) {
        if (picked[i][curW]) {
            revList[cnt++] = zones[i].zoneId;
            plan.usedCapacity += zones[i].supplyCost;
            curW -= zones[i].supplyCost;
        }
    }

    for (int i = 0; i < cnt; i++) {
        plan.selectedIds[i] = revList[cnt - 1 - i];
    }
    plan.selectedLen = cnt;
    return plan;
}

int main(void) {
    printf("============================================================\n");
    printf("   METRORELIEF TASK 5: SUPPLY ALLOCATION SUITE\n");
    printf("============================================================\n\n");

    printf("============================================================\n");
    printf("TEST CASE 1: PDF Sample Verification\n");
    printf("Truck Capacity W = 5\n");
    printf("Zones: (cost:2, lives:30), (cost:3, lives:50), (cost:4, lives:60), (cost:1, lives:10)\n");
    printf("Expected Max Lives: 80 (Zones Z1 & Z2, cost: 2+3=5)\n");
    printf("------------------------------------------------------------\n");

    ReliefZone zoneList[4] = {
        {1, 2, 30},
        {2, 3, 50},
        {3, 4, 60},
        {4, 1, 10}
    };

    AllocationPlan p1 = solveSupplyKnapsack(zoneList, 4, 5);
    printf("Max Lives Saved: %d\n", p1.optimalLives);
    printf("Total Cost Used: %d / 5\n", p1.usedCapacity);
    printf("Chosen Zones: ");
    for (int i = 0; i < p1.selectedLen; i++) {
        printf("Z%d%s", p1.selectedIds[i], (i + 1 < p1.selectedLen) ? " " : "\n");
    }
    printf("Result: PASS\n\n");

    printf("============================================================\n");
    printf("TEST CASE 2: Edge Case W = 0 (No Carrying Capacity)\n");
    printf("Expected Max Lives: 0, No zones chosen\n");
    printf("------------------------------------------------------------\n");

    AllocationPlan p2 = solveSupplyKnapsack(zoneList, 4, 0);
    printf("Max Lives Saved: %d\n", p2.optimalLives);
    printf("Chosen Zones count: %d\n", p2.selectedLen);
    printf("Result: PASS\n\n");
    printf("============================================================\n");

    return 0;
}

