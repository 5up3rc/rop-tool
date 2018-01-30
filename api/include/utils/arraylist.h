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

#ifndef DEF_API_UTILS_ARRAYLIST_H
#define DEF_API_UTILS_ARRAYLIST_H

typedef struct {
  void **list;
  size_t num;
  size_t head;
}r_utils_arraylist_s;

void r_utils_arraylist_init(r_utils_arraylist_s *l, size_t nelem);
void r_utils_arraylist_push(r_utils_arraylist_s *l, void *e);
void* r_utils_arraylist_pop(r_utils_arraylist_s *l);
void *r_utils_arraylist_access(r_utils_arraylist_s *l, size_t i);
size_t r_utils_arraylist_size(r_utils_arraylist_s *l);
void r_utils_arraylist_free(r_utils_arraylist_s *l, void (*free_cb)(void*));
void r_utils_arraylist_sort(r_utils_arraylist_s *l, int (*cmp)(const void*, const void*));
void r_utils_arraylist_foreach(r_utils_arraylist_s *l, void (*cb)(void*));


#endif
