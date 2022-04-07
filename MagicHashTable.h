
#ifndef MHT_H
#define MHT_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "MoveGenerator.h"

typedef struct MHTItem_s {
    uint16_t *key;
    uint64_t *value;
} MHTItem;

typedef struct MagicHashTable_s {
    int bits;
    uint64_t magic;
    MHTItem **items;
} MagicHashTable;

MHTItem *createItem(uint16_t *key, uint64_t *value);
MagicHashTable *createTable(int size, int bits, uint64_t magic);
void insertValue(MagicHashTable *table, uint64_t *key, uint64_t *value);
uint64_t *getValue(MagicHashTable *table, uint64_t *key);

#endif