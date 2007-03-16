/*
 * detector.h
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

#ifndef __DETECTOR_H__
#define __DETECTOR_H__

#include <glib.h>

void init_field (int rows, int columns);
void place_mines (int mines);

void open_field (int x, int y);
void postulate_mine (int x, int y);

int sets_equal (field_set *set1, field_set *set2);
int field_set_hash (field_set *set);

int add_field_set (field_set *set);

void make_set_bounds (field_set *set);

void build_initial_sets (void);

field_set* set_difference (field_set *set1, field_set *set2);

int yield_sets (GList *sets_list, int execute);

int sets_could_overlap (field_set *set1, field_set *set2);
int process_new_sets (void);

#ifdef DO_OUTPUT
void output_field_values (void);
void output_field_statuses (void);

void output_sets (GList *sets_list);
#endif

void delete_sets (void);

#endif
