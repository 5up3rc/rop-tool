/************************************************************************/
/* rop-tool - A Return Oriented Programming and binary exploitation     */
/*            tool                                                      */
/*                                                                      */
/* Copyright 2013-2015, -TOSH-                                          */
/* File coded by -TOSH-                                                 */
/*                                                                      */
/* This file is part of rop-tool.                                       */
/*                                                                      */
/* rop-tool is free software: you can redistribute it and/or modify     */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or    */
/* (at your option) any later version.                                  */
/*                                                                      */
/* rop-tool is distributed in the hope that it will be useful,          */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU General Public License for more details.                         */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with rop-tool.  If not, see <http://www.gnu.org/licenses/>     */
/************************************************************************/
#include "utils.h"

static u32 r_utils_hash(r_utils_hash_s *h, const u8 *key, size_t key_len) {
  u32 hash;
  u32 i;

  hash = 0;

  for(i = 0; i < key_len; i++) {
    hash = (hash << 1) | (hash >> 31);
    hash += key[i];
  }
  return hash % h->entries;
}

void r_utils_hash_foreach(r_utils_hash_s *h, void (*callback)(r_utils_hash_elem_s*)) {
  u32 i;
  r_utils_hash_elem_s *elem;

  assert(h != NULL);
  assert(h->elems != NULL);

  for(i = 0; i < h->entries; i++) {
    for(elem = h->elems[i]; elem; elem = elem->next) {
      callback(elem);
    }
  }
}

/* Free hashtable */
void r_utils_hash_free(r_utils_hash_s **h) {
  r_utils_hash_elem_s *e, *tmp;
  u32 i;

  assert(h != NULL && *h != NULL);
  assert((*h)->elems != NULL);

  for(i = 0; i < (*h)->entries; i++) {
    e = (*h)->elems[i];
    while(e != NULL) {
      tmp = e->next;
      if((*h)->elem_destructor)
        (*h)->elem_destructor(e->val);
      free(e);
      e = tmp;
    }
  }

  free((*h)->elems);
  free(*h);
  *h = NULL;
}

r_utils_hash_elem_s* r_utils_hash_elem_new(void *elem, u8 *key, u32 key_len) {
  r_utils_hash_elem_s *e;

  e = r_utils_malloc(sizeof(*e));

  e->val = elem;
  e->key = key;
  e->key_len = key_len;

  return e;
}

/* Allocate hashtable */
r_utils_hash_s* r_utils_hash_new(size_t entries, void(*destructor)(void*)) {
  r_utils_hash_s *h;

  h = r_utils_calloc(1, sizeof(*h));
  h->elems = r_utils_calloc(entries, sizeof(r_utils_hash_elem_s*));
  h->entries = entries;
  h->elem_destructor = destructor;

  return h;
}

/* Insert an element to the hashtable */
void r_utils_hash_insert(r_utils_hash_s *h, r_utils_hash_elem_s *elem) {
  u32 hash;

  assert(h != NULL);
  assert(elem != NULL);
  assert(h->elems != NULL);

  hash = r_utils_hash(h, elem->key, elem->key_len);

  if(h->elems[hash] != NULL)
    h->colisions++;

  elem->next = h->elems[hash];
  h->elems[hash] = elem;
  h->size++;
}


/* Return true the key match an element */
int r_utils_hash_elem_exist(r_utils_hash_s *h, u8 *key, u32 key_len) {
  r_utils_hash_elem_s *e;
  u32 hash;

  assert(h != NULL);
  assert(h->elems != NULL);
  assert(key != NULL);

  hash = r_utils_hash(h, key, key_len);

  for(e = h->elems[hash]; e; e = e->next) {
    if(e->key_len == key_len)
      if(!memcmp(e->key, key, key_len))
  return 1;
  }
  return 0;
}

/* Return the size of the hashtable (number of elements) */
u32 r_utils_hash_size(r_utils_hash_s *h) {
  assert(h != NULL);
  return h->size;
}
