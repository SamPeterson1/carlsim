#include "MagicHashTable.h"

MHTItem *createItem(uint16_t *key, uint64_t *value) {
    MHTItem *item = (MHTItem *) malloc(sizeof(MHTItem));
    item->key = key;
    item->value = (uint64_t *) malloc(sizeof(uint64_t));
    item->key = (uint16_t *) malloc(sizeof(uint16_t));

    memcpy(item->value, value, sizeof(uint64_t));
    memcpy(item->key, key, sizeof(uint16_t));

    return item;
}

MagicHashTable *createTable(int size, int bits, uint64_t magic) {
    MagicHashTable *table = (MagicHashTable *) malloc(sizeof(MagicHashTable));
    table->bits = bits;
    table->magic = magic;
    table->items = (MHTItem**) calloc (size, sizeof(MHTItem*));
    for (int i=0; i<size; i++)
        table->items[i] = NULL;
    return table;
}

void insertValue(MagicHashTable *table, uint64_t *key, uint64_t *value) {
    uint16_t hash = ((*key) * table->magic) >> (uint64_t)(64-table->bits);
    MHTItem *item = createItem(&hash, value);
    table->items[hash] = item;
}

/*
uint64_t *getValue(MagicHashTable *table, uint64_t *key) {
    uint16_t hash = ((*key) * table->magic) >> (uint64_t)(64-table->bits);
    return (table->items[hash])->value;
}
*/