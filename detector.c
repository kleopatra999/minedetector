/*
 * detector.c
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

#include <stdlib.h>
#include <assert.h>
#include <glib.h>

#include "data.h"

#ifndef MIN
#define MIN(a,b)      ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b)      ((a)>(b)?(a):(b))
#endif

field **fields;
field_set *hash_table[HASH_BUCKETS];
int field_rows,
    field_columns,
    num_mines,
    num_known_mines;
GList *processed_sets_list = 0,
    *new_sets_list = 0;

void
init_field (int rows, int columns)
{
    int i, x, y;

    field_rows = rows;
    field_columns = columns;

    fields = ((field**)malloc((field_rows + 2) * sizeof(field*))) + 1;
    for (i = -1; i <= field_rows; ++i)
	fields[i] = ((field*)malloc((field_columns + 2) * sizeof(field))) + 1;

    for (y = 0; y < field_rows; ++y)
    {
	for (x = 0; x < field_columns; ++x)
	    fields[y][x] = 0;
	fields[y][-1] = STATUS_KNOWN;
	fields[y][field_columns] = STATUS_KNOWN;
    }
    for (x = -1; x <= field_columns; ++x)
    {
	fields[-1][x] = STATUS_KNOWN;
	fields[field_rows][x] = STATUS_KNOWN;
    }
}

void
place_mines (int mines)
{
    int i, x, y;

    num_mines = mines;
    num_known_mines = 0;

    for (i = 0; i < num_mines; ++i)
    {
	while (1)
	{
	    x = random() % field_columns;
	    y = random() % field_rows;

	    if (fields[y][x] != VALUE_MINE)
	    {
		fields[y][x] = VALUE_MINE;
		break;
	    }
	}

	if (x > 0)
	{
	    if (fields[y][x - 1] != VALUE_MINE) ++fields[y][x - 1];
	    if (y > 0)
		if (fields[y - 1][x - 1] != VALUE_MINE) ++fields[y - 1][x - 1];
	    if (y < field_rows - 1)
		if (fields[y + 1][x - 1] != VALUE_MINE) ++fields[y + 1][x - 1];
	}
	if (x < field_columns - 1)
	{
	    if (fields[y][x + 1] != VALUE_MINE) ++fields[y][x + 1];
	    if (y > 0)
		if (fields[y - 1][x + 1] != VALUE_MINE) ++fields[y - 1][x + 1];
	    if (y < field_rows - 1)
		if (fields[y + 1][x + 1] != VALUE_MINE) ++fields[y + 1][x + 1];
	}
	if (y > 0)
	    if (fields[y - 1][x] != VALUE_MINE) ++fields[y - 1][x];
	if (y < field_rows - 1)
	    if (fields[y + 1][x] != VALUE_MINE) ++fields[y + 1][x];
    }

    for (i = 0; i < HASH_BUCKETS; ++i)
	hash_table[i] = 0;
}

#ifdef DO_OUTPUT
void
output_field_values (void)
{
    int x,
	y;

    for (y = 0; y < field_rows; ++y)
    {
	for (x = 0; x < field_columns; ++x)
	{
	    int value = FIELD_VALUE(x, y);
	    
	    if (value == VALUE_MINE)
		printf("X ");
	    else if (value == 0)
		printf("  ");
	    else
		printf("%d ", value);
	}

	printf("\n");
    }
    printf("\n");
}

void
output_field_statuses (void)
{
    int x,
	y;

    for (y = 0; y < field_rows; ++y)
    {
	for (x = 0; x < field_columns; ++x)
	{
	    int value = FIELD_VALUE(x, y),
		status = FIELD_STATUS(x, y);
	    
	    if (status == STATUS_UNKNOWN)
		printf("# ");
	    else if (status == STATUS_MINE)
		printf("X ");
	    else
	    {
		if (value == 0)
		    printf("  ");
		else
		    printf("%d ", value);
	    }
	}

	printf("\n");
    }
    printf("\n");
}
#endif

void
open_field (int x, int y)
{
    if (FIELD_STATUS(x, y) != STATUS_UNKNOWN)
	return;

    if (FIELD_VALUE(x, y) == VALUE_MINE)
    {
	printf("oops, there is a mine on (%d,%d) : you lost (mines left: %d)!\n",
	       x, y, num_mines - num_known_mines);
	exit(0);
    }

    SET_FIELD_STATUS(x, y, STATUS_KNOWN);

    if (FIELD_VALUE(x, y) == VALUE_CLEAR)
    {
	if (x > 0)
	{
	    open_field(x - 1, y);
	    if (y > 0)
		open_field(x - 1, y - 1);
	    if (y < field_rows - 1)
		open_field(x - 1, y + 1);
	}
	if (x < field_columns - 1)
	{
	    open_field(x + 1, y);
	    if (y > 0)
		open_field(x + 1, y - 1);
	    if (y < field_rows - 1)
		open_field(x + 1, y + 1);
	}
	if (y > 0)
	    open_field(x, y - 1);
	if (y < field_rows - 1)
	    open_field(x, y + 1);
    }
}

void
postulate_mine (int x, int y)
{
    assert(FIELD_VALUE(x, y) == VALUE_MINE);

    if (FIELD_STATUS(x, y) == STATUS_MINE)
	return;

    SET_FIELD_STATUS(x, y, STATUS_MINE);

    if (++num_known_mines == num_mines)
    {
	printf("congrats, you won!\n");
	exit(0);
    }
}

int
sets_equal (field_set *set1, field_set *set2)
{
    int i;

    if (set1->num_fields != set2->num_fields)
	return 0;

    for (i = 0; i < set1->num_fields; ++i)
    {
	int j,
	    found = 0;

	for (j = 0; j < set2->num_fields; ++j)
	    if (set1->fields[i].x == set2->fields[j].x && set1->fields[i].y == set2->fields[j].y)
	    {
		found = 1;
		break;
	    }

	if (!found)
	    return 0;
    }

    return 1;
}

int
field_set_hash (field_set *set)
{
    unsigned int hash = 0;
    int i;

    for (i = 0; i < set->num_fields; ++i)
    {
	int remainder = (hash & 0xe0000000) >> 29;

	hash = ((hash << 3) | remainder) + (set->fields[i].x << 16) + set->fields[i].y;
    }

    return hash % HASH_BUCKETS;
}

int
field_pos_comparator (const void *p1, const void *p2)
{
    field_pos *fp1 = (field_pos*)p1, *fp2 = (field_pos*)p2;

    if (fp1->y < fp2->y)
	return -1;
    else if (fp1->y == fp2->y)
	return fp1->x - fp2->x;
    else
	return 1;
}

void
canonize_set (field_set *set)
{
    qsort(set->fields, set->num_fields, sizeof(field_pos), field_pos_comparator);
}

field_set*
add_field_set (field_set *set)
{
    int hash = field_set_hash(set);
    field_set *a_set = hash_table[hash];

    while (a_set != 0)
    {
	if (sets_equal(set, a_set))
	{
	    if (set->min_mines > a_set->min_mines
		|| set->max_mines < a_set->max_mines)
	    {
		a_set->min_mines = MAX(set->min_mines, a_set->min_mines);
		a_set->max_mines = MIN(set->max_mines, a_set->max_mines);

#ifdef DO_OUTPUT
		printf("updated set\n");
#endif

		return a_set;
	    }
	    else
		return 0;
	}

	a_set = a_set->hash_next;
    }

    set->hash_next = hash_table[hash];
    hash_table[hash] = set;

    return set;
}

void
make_set_bounds (field_set *set)
{
    int i;

    set->min_x = set->max_x = set->fields[0].x;
    set->min_y = set->max_y = set->fields[0].y;

    for (i = 1; i < set->num_fields; ++i)
    {
	if (set->fields[i].x < set->min_x) set->min_x = set->fields[i].x;
	if (set->fields[i].x > set->max_x) set->max_x = set->fields[i].x;

	if (set->fields[i].y < set->min_y) set->min_y = set->fields[i].y;
	if (set->fields[i].y > set->max_y) set->max_y = set->fields[i].y;
    }
}

void
build_initial_sets (void)
{
    int x,
	y;

    for (y = 0; y < field_rows; ++y)
	for (x = 0; x < field_columns; ++x)
	    if (FIELD_STATUS(x, y) == STATUS_KNOWN && FIELD_VALUE(x, y) != VALUE_CLEAR)
	    {
		field_set *set = (field_set*)malloc(sizeof(field_set));

		set->num_fields = 0;
		set->max_mines = FIELD_VALUE(x, y);

		if (FIELD_STATUS(x-1,y) == STATUS_UNKNOWN) ADD_FIELD_TO_SET(x-1,y,*set);
		if (FIELD_STATUS(x-1,y) == STATUS_MINE) --set->max_mines;

		if (FIELD_STATUS(x-1,y-1) == STATUS_UNKNOWN) ADD_FIELD_TO_SET(x-1,y-1,*set);
		if (FIELD_STATUS(x-1,y-1) == STATUS_MINE) --set->max_mines;

		if (FIELD_STATUS(x-1,y+1) == STATUS_UNKNOWN) ADD_FIELD_TO_SET(x-1,y+1,*set);
		if (FIELD_STATUS(x-1,y+1) == STATUS_MINE) --set->max_mines;

		if (FIELD_STATUS(x+1,y) == STATUS_UNKNOWN) ADD_FIELD_TO_SET(x+1,y,*set);
		if (FIELD_STATUS(x+1,y) == STATUS_MINE) --set->max_mines;

		if (FIELD_STATUS(x+1,y-1) == STATUS_UNKNOWN) ADD_FIELD_TO_SET(x+1,y-1,*set);
		if (FIELD_STATUS(x+1,y-1) == STATUS_MINE) --set->max_mines;

		if (FIELD_STATUS(x+1,y+1) == STATUS_UNKNOWN) ADD_FIELD_TO_SET(x+1,y+1,*set);
		if (FIELD_STATUS(x+1,y+1) == STATUS_MINE) --set->max_mines;

		if (FIELD_STATUS(x,y-1) == STATUS_UNKNOWN) ADD_FIELD_TO_SET(x,y-1,*set);
		if (FIELD_STATUS(x,y-1) == STATUS_MINE) --set->max_mines;

		if (FIELD_STATUS(x,y+1) == STATUS_UNKNOWN) ADD_FIELD_TO_SET(x,y+1,*set);
		if (FIELD_STATUS(x,y+1) == STATUS_MINE) --set->max_mines;

		set->min_mines = set->max_mines;

		if (set->num_fields == 0)
		    free(set);
		else
		{
		    field_set *added_set;

		    make_set_bounds(set);
		    canonize_set(set);
		    if ((added_set = add_field_set(set)) != 0)
		    {
			assert(added_set == set);

			new_sets_list = g_list_prepend(new_sets_list, set);
		    }
		    else
			free(set);
		}
	    }
}

field_set*
set_difference (field_set *set1, field_set *set2)
{
    field_set *difference = (field_set*)malloc(sizeof(field_set));
    int i,
	set1_alone_fields,
	set2_alone_fields,
	intersect_fields,
	intersect_min_mines,
	intersect_max_mines;

    difference->num_fields = 0;

    for (i = 0; i < set1->num_fields; ++i)
    {
	int j,
	    found = 0;

	for (j = 0; j < set2->num_fields; ++j)
	    if (set1->fields[i].x == set2->fields[j].x && set1->fields[i].y == set2->fields[j].y)
	    {
		found = 1;
		break;
	    }

	if (!found)
	    difference->fields[difference->num_fields++] = set1->fields[i];
    }

    if (difference->num_fields == 0)
    {
	free(difference);
	return 0;
    }

    intersect_fields = set1->num_fields - difference->num_fields;
    set1_alone_fields = set1->num_fields - intersect_fields;
    set2_alone_fields = set2->num_fields - intersect_fields;

    intersect_min_mines = MAX(0, MAX(set1->min_mines - set1_alone_fields,
				   set2->min_mines - set2_alone_fields));
    difference->max_mines = MIN(difference->num_fields, set1->max_mines - intersect_min_mines);

    intersect_max_mines = MIN(intersect_fields, MIN(set1->max_mines, set2->max_mines));
    difference->min_mines = MAX(0, set1->min_mines - intersect_max_mines);

    make_set_bounds(difference);
    canonize_set(difference);

    return difference;
}

int
yield_sets (GList *sets_list, int execute)
{
    int done_something = 0;

    while (sets_list != 0)
    {
	field_set *set = (field_set*)sets_list->data;

	if (set->max_mines == 0)
	{
	    int i;

	    for (i = 0; i < set->num_fields; ++i)
	    {
		if (FIELD_STATUS(set->fields[i].x, set->fields[i].y) == STATUS_UNKNOWN)
		{
#ifdef DO_OUTPUT
		    printf("opening (%d,%d)\n", set->fields[i].x, set->fields[i].y);
#endif
		    if (execute)
			open_field(set->fields[i].x, set->fields[i].y);
		}
		done_something = 1;
	    }
	}
	else if (set->num_fields == set->min_mines)
	{
	    int i;

	    for (i = 0; i < set->num_fields; ++i)
	    {
		if (FIELD_STATUS(set->fields[i].x, set->fields[i].y) == STATUS_UNKNOWN)
		{
#ifdef DO_OUTPUT
		    printf("postulating mine at (%d,%d)\n", set->fields[0].x, set->fields[0].y);
#endif
		    if (execute)
			postulate_mine(set->fields[0].x, set->fields[0].y);
		    done_something = 1;
		}
	    }
	}

	sets_list = g_list_next(sets_list);
    }

    return done_something;
}

int
sets_could_overlap (field_set *set1, field_set *set2)
{
    if (set1->max_x < set2->min_x || set1->min_x > set2->max_x
	|| set1->max_y < set2->min_y || set1->min_y > set2->max_y)
	return 0;
    return 1;
}

int
process_new_sets (void)
{
    int done_something = 0;
    GList *current_sets_list;

    current_sets_list = g_list_first(new_sets_list);
    new_sets_list = 0;
    while (current_sets_list != 0)
    {
	field_set *current_set = (field_set*)current_sets_list->data;
	GList *sets_list;

	assert(current_set != 0);

	sets_list = g_list_first(processed_sets_list);
	while (sets_list != 0)
	{
	    field_set *set = (field_set*)sets_list->data;

	    assert(set != 0);

	    if (sets_could_overlap(set, current_set))
	    {
		field_set *difference;

		difference = set_difference(set, current_set);
		if (difference != 0)
		{
		    field_set *added_set;

		    if ((added_set = add_field_set(difference)) != 0)
		    {
			if (added_set != difference)
			{
			    free(difference);
			    difference = added_set;
			}
			new_sets_list = g_list_prepend(new_sets_list, difference);
			done_something = 1;
		    }
		    else
			free(difference);
		}

		difference = set_difference(current_set, set);
		if (difference != 0)
		{
		    field_set *added_set;

		    if ((added_set = add_field_set(difference)) != 0)
		    {
			if (added_set != difference)
			{
			    free(difference);
			    difference = added_set;
			}
			new_sets_list = g_list_prepend(new_sets_list, difference);
			done_something = 1;
		    }
		    else
			free(difference);
		}
	    }

	    sets_list = g_list_next(sets_list);
	}
	
	processed_sets_list = g_list_prepend(processed_sets_list, current_set);
	current_sets_list = g_list_next(current_sets_list);
    }

    g_list_free(current_sets_list);

    return done_something;
}

#ifdef DO_OUTPUT
void
output_sets (GList *sets_list)
{
    int i;

    while (sets_list != 0)
    {
	field_set *set = (field_set*)sets_list->data;

	printf("{ ");
	for (i = 0; i < set->num_fields; ++i)
	    printf("(%d,%d) ", set->fields[i].x, set->fields[i].y);
	printf("} : %d - %d\n", set->min_mines, set->max_mines);

	sets_list = g_list_next(sets_list);
    }
}
#endif

void
free_field_set (gpointer p, gpointer user_data)
{
    if (user_data != 0)
	if (g_list_find((GList*)user_data, p) != 0)
	    return;
    free(p);
}

void
delete_sets (void)
{
    field_set *set;
    int i;

    /*
    g_list_foreach(new_sets_list, free_field_set, processed_sets_list);
    g_list_free(new_sets_list);
    */
    new_sets_list = 0;

    /*
    g_list_foreach(processed_sets_list, free_field_set, 0);
    g_list_free(processed_sets_list);
    */
    processed_sets_list = 0;

    for (i = 0; i < HASH_BUCKETS; ++i)
	hash_table[i] = 0;
}
