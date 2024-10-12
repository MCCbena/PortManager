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
    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            free(entry->value);
            entry->value = strdup(value);
            return;
        }
        entry = entry->next;
    }

    // create new entry
    entry = malloc(sizeof(Entry));
    entry->key = strdup(key);
    entry->value = strdup(value);
    entry->next = hashmap->entries[index];  // for chaining
    hashmap->entries[index] = entry;
}

char *getValueFromHashMap(const HashMap *hashmap, const char *key) {
    unsigned int index = hash(key, hashmap->size);

    Entry *entry = hashmap->entries[index];
    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }

    return NULL;
}

void freeEntry(Entry *entry) {
    while (entry != NULL) {
        Entry *next = entry->next;
        free(entry->key);
        free(entry->value);
        free(entry);
        entry = next;
    }
}

void freeHashMap(HashMap *hashmap) {
    for (int i = 0; i < hashmap->size; i++) {
        freeEntry(hashmap->entries[i]);
    }
    free(hashmap->entries);
    //free(hashmap);
}

int removeFromHashMap(const HashMap *hashmap, const char *key) {
    unsigned int index = hash(key, hashmap->size);
    Entry *entry = hashmap->entries[index];

    // find the entry and its predecessor
    Entry *pred = NULL;
    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            break;
        }
        pred = entry;
        entry = entry->next;
    }

    if (entry == NULL) return -1;
    if (pred == NULL) {  // entry is the first in the list
        hashmap->entries[index] = entry->next;
    } else {  // entry is not the first in the list
        pred->next = entry->next;
    }

    free(entry->key);
    free(entry->value);
    free(entry);

    return 0;
}



#endif //PORTMANAGER_MAP_H
