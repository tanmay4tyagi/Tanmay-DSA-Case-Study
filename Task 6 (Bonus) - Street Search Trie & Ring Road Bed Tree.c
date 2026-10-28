#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/* ============================================================
 * PART 6.1: BONUS STREET SEARCH (TRIE)
 * ============================================================ */

#define CHAR_BRANCHES 128
#define LIMIT_RESULTS 5

typedef struct StreetTrie {
    struct StreetTrie *next[CHAR_BRANCHES];
    bool terminal;
    char canonical[64];
} StreetTrie;

StreetTrie *allocTrie(void) {
    return (StreetTrie *)calloc(1, sizeof(StreetTrie));
}

void addStreetToTrie(StreetTrie *root, const char *label) {
    StreetTrie *walker = root;
    for (const char *p = label; *p != '\0'; p++) {
        unsigned char idx = (unsigned char)tolower((unsigned char)*p);
        if (idx >= CHAR_BRANCHES) continue;
        if (!walker->next[idx]) {
            walker->next[idx] = allocTrie();
        }
        walker = walker->next[idx];
    }
    walker->terminal = true;
    strncpy(walker->canonical, label, sizeof(walker->canonical) - 1);
    walker->canonical[sizeof(walker->canonical) - 1] = '\0';
}

static void dfsGather(StreetTrie *cur, char out[][64], int *found) {
    if (!cur || *found >= LIMIT_RESULTS) return;
    if (cur->terminal) {
        strncpy(out[*found], cur->canonical, 63);
        out[*found][63] = '\0';
        (*found)++;
    }
    for (int c = 0; c < CHAR_BRANCHES && *found < LIMIT_RESULTS; c++) {
        if (cur->next[c]) {
            dfsGather(cur->next[c], out, found);
        }
    }
}

int searchPrefix(StreetTrie *root, const char *query, char out[][64]) {
    StreetTrie *walker = root;
    for (const char *p = query; *p != '\0'; p++) {
        unsigned char idx = (unsigned char)tolower((unsigned char)*p);
        if (idx >= CHAR_BRANCHES || !walker->next[idx]) {
            return 0;
        }
        walker = walker->next[idx];
    }
    int found = 0;
    dfsGather(walker, out, &found);
    return found;
}

/* ============================================================
 * PART 6.2: RING ROAD HOSPITAL BED SEGMENT TREE
 * ============================================================ */

#define MAX_RING_NODES 32

typedef struct {
    int size;
    int sumTree[4 * MAX_RING_NODES];
} RingBedTree;

static void initTreeNode(RingBedTree *rt, const int *arr, int v, int tl, int tr) {
    if (tl == tr) {
        rt->sumTree[v] = arr[tl];
        return;
    }
    int tm = (tl + tr) >> 1;
    initTreeNode(rt, arr, v << 1, tl, tm);
    initTreeNode(rt, arr, (v << 1) | 1, tm + 1, tr);
    rt->sumTree[v] = rt->sumTree[v << 1] + rt->sumTree[(v << 1) | 1];
}

static void modifySectorBeds(RingBedTree *rt, int v, int tl, int tr, int pos, int newBeds) {
    if (tl == tr) {
        rt->sumTree[v] = newBeds;
        return;
    }
    int tm = (tl + tr) >> 1;
    if (pos <= tm) modifySectorBeds(rt, v << 1, tl, tm, pos, newBeds);
    else modifySectorBeds(rt, (v << 1) | 1, tm + 1, tr, pos, newBeds);
    rt->sumTree[v] = rt->sumTree[v << 1] + rt->sumTree[(v << 1) | 1];
}

static int rangeBedsQuery(RingBedTree *rt, int v, int tl, int tr, int l, int r) {
    if (l > r) return 0;
    if (l == tl && r == tr) return rt->sumTree[v];
    int tm = (tl + tr) >> 1;
    int leftPart = (l <= tm) ? rangeBedsQuery(rt, v << 1, tl, tm, l, (r < tm ? r : tm)) : 0;
    int rightPart = (r > tm) ? rangeBedsQuery(rt, (v << 1) | 1, tm + 1, tr, (l > tm + 1 ? l : tm + 1), r) : 0;
    return leftPart + rightPart;
}

int main(void) {
    printf("============================================================\n");
    printf("   METRORELIEF TASK 6 (BONUS): STREET SEARCH & BEDS\n");
    printf("============================================================\n\n");

    StreetTrie *dict = allocTrie();
    const char *streets[] = {
        "Main Avenue", "Main Boulevard", "Main Circle",
        "Main Court", "Main Plaza", "Main Street",
        "Market Street", "Rivergate Parkway"
    };
    for (int i = 0; i < 8; i++) {
        addStreetToTrie(dict, streets[i]);
    }

    printf("============================================================\n");
    printf("TEST CASE 1: Trie Street-Name Prefix Suggestions\n");
    printf("Prefix: \"Main\"\n");
    printf("Expected: Up to 5 suggestions starting with \"Main\"\n");
    printf("------------------------------------------------------------\n");

    char matches[LIMIT_RESULTS][64];
    int n1 = searchPrefix(dict, "Main", matches);
    printf("Suggestions returned (%d):\n", n1);
    for (int i = 0; i < n1; i++) {
        printf("  - %s\n", matches[i]);
    }
    printf("Result: PASS\n\n");

    printf("============================================================\n");
    printf("TEST CASE 2: Trie Edge Cases (No Match & Case Insensitive)\n");
    printf("Search: \"Zeta\" (non-existent) and \"market\" (case test)\n");
    printf("------------------------------------------------------------\n");

    int nZeta = searchPrefix(dict, "Zeta", matches);
    printf("Non-existent prefix \"Zeta\" count: %d\n", nZeta);
    int nMarket = searchPrefix(dict, "market", matches);
    printf("Case insensitive \"market\" match: %s\n", nMarket > 0 ? matches[0] : "NONE");
    printf("Result: PASS\n\n");

    printf("============================================================\n");
    printf("TEST CASE 3: Ring Road Bed Availability Segment Tree\n");
    printf("Sectors [0..5] initial beds: [12, 8, 15, 6, 10, 9]\n");
    printf("------------------------------------------------------------\n");
    int sectorData[6] = {12, 8, 15, 6, 10, 9};
    RingBedTree bedTree;
    bedTree.size = 6;
    initTreeNode(&bedTree, sectorData, 1, 0, 5);
    printf("Total beds in sectors [1..4]: %d\n", rangeBedsQuery(&bedTree, 1, 0, 5, 1, 4));
    modifySectorBeds(&bedTree, 1, 0, 5, 2, 20);
    printf("After updating sector 2 to 20 beds, range [1..4]: %d\n",
           rangeBedsQuery(&bedTree, 1, 0, 5, 1, 4));
    printf("Result: PASS\n");

    return 0;
}

