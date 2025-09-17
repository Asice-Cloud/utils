#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct hash_map hp;

hp *hp_new(size_t size, size_t cap, uint64_t seed0, uint64_t seed1,
           uint64_t (*hash)(const void *item, uint64_t seed0, uint64_t seed1),
           int (*cmp)(const void *a, const void *b, void *udata),
           void (*hp_free)(void *item), void *udata);

hp *hp_new_with_allocator(void *(*malloc)(size_t),
                          void *(*realloc)(void *, size_t),
                          void (*free)(void *), size_t size, size_t cap,
                          uint64_t seed0, uint64_t seed1,
                          uint64_t (*hash)(const void *item, uint64_t seed0,
                                           uint64_t seed1),
                          int (*cmp)(const void *a, const void *b, void *udata),
                          void (*hp_free)(void *item), void *udata);

void hp_free(hp *map);
void hp_clear(hp *map, bool cap2);
size_t hp_count(const hp *map);
bool hp_oom(hp *map);

const void *hp_get(const hp *map, const void *item);
const void *hp_set(hp *map, const void *item);
const void *hp_delete(hp *map, const void *item);
const void *hp_probe(hp *map, uint64_t position);
bool hp_scan(hp *map, bool (*iter)(const void *item, void *udata),
             void *udata);
bool hp_iter(hp *map, size_t *i, void **item);

uint64_t hp_sip(const void *data, size_t len, uint64_t seed0, uint64_t seed1);
uint64_t hp_murmur(const void *data, size_t len, uint64_t seed0,
                   uint64_t seed1);
uint64_t hp_xxhash3(const void *data, size_t len, uint64_t seed0,
                    uint64_t seed1);

const void *hp_get_with_hash(const hp *map, const void *key,
                             uint64_t hash);
const void *hp_delete_with_hash(hp *map, const void *key,
                                uint64_t hash);
const void *hp_set_with_hash(hp *map, const void *item,
                             uint64_t hash);
void hp_set_grow_by_power(hp *map, size_t power);
void hp_set_load_factor(hp *map, double load_factor);

// DEPRECATED: use `hp_new_with_allocator`
void hp_set_allocator(void *(*malloc)(size_t), void (*free)(void *));
