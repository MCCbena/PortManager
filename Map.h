//
// Created by shuta on 24/09/28.
//

#ifndef PORTMANAGER_MAP_H
#define PORTMANAGER_MAP_H

#include <stdlib.h>
#include <string.h>

typedef struct Entry Entry;
struct Entry {
    char *key;
    char *value;
    Entry *next;  // for chaining
};

typedef struct HashMap HashMap;
struct HashMap {
    Entry **entries;
    int size;  // size of entries
};

HashMap *newHashMap(int size) {
    HashMap *hashmap = malloc(sizeof(HashMap));
    hashmap->size = size;

    // initialize entries
    hashmap->entries = malloc(sizeof(Entry) * size);
    for (int i = 0; i < size; i++) {
        hashmap->entries[i] = NULL;
    }

    return hashmap;
}

unsigned int hash(const char *key, const int size) {
    unsigned int hash = 0;
    for (int i = 0; key[i] != '\0'; i++) {
        hash = 31 * hash + key[i];
    }
    return hash % size;
}

void insertToHashMap(const HashMap *hashmap, const char *key,
                     const char *value) {
    unsigned int index = hash(key, hashmap->size);

    // check if key already exists
    Entry *entry = hashmap->entries[index];
    if (entry != NULL) {
        return;
    }
    // create new entry
    entry = malloc(sizeof(Entry));
    entry->key = strdup(key);
    entry->value = strdup(value);
    hashmap->entries[index] = entry;
}

char *getValueFromHashMap(const HashMap *hashmap, const char *key) {
    unsigned int index = hash(key, hashmap->size);

    Entry *entry = hashmap->entries[index];
    // check if key exists
    if (entry == NULL) {
        return NULL;
    }

    return entry->value;
}

void destroyHashMap(HashMap *hashmap) {
    for(int i = 0; i < hashmap->size; i++){
        Entry *entry = hashmap->entries[i];
        if(hashmap->entries[i] != NULL) {
            free(entry->value);
            free(entry->key);
            free(entry);
        }
    }
}

#endif //PORTMANAGER_MAP_H
