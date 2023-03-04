
#ifndef MHT_H
#define MHT_H

#include "Defines.h"
#include "Types.h"
#include "Log.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "MoveGenerator.h"

MHTItem *createItem(uint16_t *key, uint64_t *value);
MagicHashTable *createTable(int size, int bits, uint64_t magic);
void insertValue(MagicHashTable *table, uint64_t *key, uint64_t *value);
//uint64_t *getValue(MagicHashTable *table, uint64_t *key);

#endif