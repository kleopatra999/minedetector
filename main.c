/*
 * main.c
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
#include <time.h>

#include "data.h"
#include "detector.h"

extern int num_known_mines;
extern field **fields;

void
usage (void)
{
    printf("usage: detector --random <width> <height> <mines>\n"
	   "       detector --read <width> <height>\n");
    exit(1);
}

int
solver_iteration (int execute)
{
    int done_something;

    build_initial_sets();
    done_something = yield_sets(new_sets_list, execute);

    if (!done_something)
    {
	while (process_new_sets())
	    ;

#ifdef DO_OUTPUT
	printf("done some extra processing\n");
	output_sets(processed_sets_list);
#endif
	done_something = yield_sets(processed_sets_list, execute);
    }
    delete_sets();

    return done_something;
}
    
int
main (int argc, char *argv[])
{
    srandom(time(0));

    if (argc < 2)
	usage();

    if (strcmp(argv[1], "--random") == 0)
    {
	int width, height, mines;
	int done_something;

	if (argc != 5)
	    usage();

	width = atoi(argv[2]);
	height = atoi(argv[3]);
	mines = atoi(argv[4]);

	init_field(height, width);
	place_mines(mines);

#ifdef DO_OUTPUT
	output_field_values();
#endif

	while (1)
	{
	    int row = random() % height,
		col = random() % width;

	    if (FIELD_STATUS(col,row) == STATUS_UNKNOWN)
	    {
		printf("opening random field (%d,%d)\n", col, row);
		open_field(col, row);

		do
		{
#ifdef DO_OUTPUT
		    output_field_statuses();
#endif
		    done_something = solver_iteration(1);
		} while (done_something);
	    }
	}
    }
    else if (strcmp(argv[1], "--read") == 0)
    {
	int width, height;
	int y, x;

	if (argc != 4)
	    usage();

	width = atoi(argv[2]);
	height = atoi(argv[3]);

	init_field(height, width);

	for (y = 0; y < height; ++y)
	    for (x = 0; x < width; ++x)
	    {
		char c;

		do
		{
		    c = getchar();
		} while (strchr("012345678+-", c) == 0);

		if (isdigit(c))
		    fields[y][x] = (c - '0') | STATUS_KNOWN;
		else if (c == '+')
		    fields[y][x] = VALUE_MINE | STATUS_MINE;
		else
		    fields[y][x] = 9 | STATUS_UNKNOWN;
	    }

	solver_iteration(1);
#ifdef DO_OUTPUT
	output_field_statuses();
#endif
    }
    else
	usage();

    return 0;
}
