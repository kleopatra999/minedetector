/*
 * data.h
 *
 * minedetector
 *
 * Copyright (C) 1997-2005 Mark Probst
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __DATA_H__
#define __DATA_H__

#include <glib.h>

#define FIELD_VALUE_MASK  0x0f
#define FIELD_STATUS_MASK 0xf0

#define VALUE_CLEAR   0x00
#define VALUE_MINE    0x0f

#define STATUS_UNKNOWN 0x00
#define STATUS_KNOWN   0x10
#define STATUS_MINE    0x20

#define FIELD_VALUE(x,y)    (fields[y][x] & FIELD_VALUE_MASK)
#define FIELD_STATUS(x,y)   (fields[y][x] & FIELD_STATUS_MASK)

#define SET_FIELD_STATUS(x,y,s)  (fields[y][x] = (FIELD_VALUE((x),(y)) | (s)))

typedef unsigned char field;

extern field **fields;
extern int field_rows;
extern int field_columns;
extern int num_mines;
extern int num_known_mines;

typedef struct
{
    int x;
    int y;
} field_pos;

#define MAX_FIELDS_IN_SET   8

#define ADD_FIELD_TO_SET(c,r,s)  { (s).fields[(s).num_fields].x = (c); \
                                   (s).fields[(s).num_fields].y = (r); \
				   ++((s).num_fields); }

typedef struct _FieldSet
{
    field_pos fields[MAX_FIELDS_IN_SET];
    int num_fields;
    int min_x;
    int max_x;
    int min_y;
    int max_y;

    int min_mines;
    int max_mines;

    struct _FieldSet *hash_next;
} field_set;

#define HASH_BUCKETS 3001

extern field_set *hash_table[];
extern GList *processed_sets_list, *new_sets_list;

#endif
