# Minedetector - A solver for Minesweeper

Solving a Minesweeper configuration, meaning given a configuration,
finding out which fields are certain to be mines and which are certain
not to be, is really very easy. Here's an algorithm to do it:

* Put together a list of all unvisited fields which have at least one
  visited neighbour.

* For these fields, try out all combinations of assigning each field
  either bomb status or no-bomb status. Eliminate all combinations
  where the resulting configurations contain trivial contradictions
  (i.e., where visited fields have either too many or too few
  neighbouring bombs).

* For all the remaining combinations (the ones without
  contradictions), find out which fields have the same statuses in all
  combinations. If in all combinations a field has bomb status, it's
  sure to contain a bomb. If it has non-bomb status in all
  configurations, it's sure not to contain a bomb. Of all the other
  fields, no certain statement can be made.

This algorithm will solve all solvable Minesweeper
configurations. Unfortunately, it's also very inefficient. For
example, take this configuration:

	-----------
	-8-8-8-8-8-
	-----------
	-----------
	-8-8-8-8-8-
	-----------
	-----------
	-8-8-8-8-8-
	-----------

The `-` characters are fields which are not yet visited, the `8`s are
visited fields with 8 mines in their neighbourhood. Although it's
obvious that all unvisited fields contain mines, the above algorithm
would have to try 2^84 combinations to find that out. Assuming you
have a 10 GHz Pentium 6 and your implementation can generate and check
each combination in only 10 cycles, you'll have to wait about 613
million years for the solution.

Even worse, it could be that there is no algorithm for solving a
Minesweeper configuration which has better worst-case asymptotic
performance than the one above. Richard Kaye has proven that
determining whether a Minesweeper configuration is consistent, i.e.,
whether it could arise during normal play and does not contradict
itself, is NP-complete. Note that that does not mean that solving a
configuration is requires exponential time, but it does at least make
it somewhat likely.

It's not surprising, then, that my solver doesn't solve all solvable
configurations. For those that it does solve, it's reasonably
efficient, though. It can solve an expert board (99 mines in 30x16
fields) from start to finish in less than one tenth of a second, if
it's successful.

The solver works by making deductions about minimum and maximum
numbers of mines in sets of fields, very similar to how I usually
think when I play the game. Here's an example (a `+` signifies a known
mine):

    +33+
    ---+

Via the left `3` we know that the three unvisited fields contain
exactly two mines, while via the right `3` we know that the two right
unvisited fields contain exactly one mine. If we combine that
information, we can conclude that the left unvisited field contains a
mine, although we cannot say which one of the other two contains a
mine.

In general, let's say we have a set N with |N| fields, about which we
know that it contains at least N~l mines and at most N~u mines. We also
have a set M with |M| fields, containing at least M~l mines and at most
M~u mines.

We'll first consider the intersection I of the two sets, which
contains all fields that are both in N as well as in M. How many mines
does this set contain at a minimum? Assume N contains its minimum
amount of mines, N~l. Also assume that as many of those mines as
possible are in those fields that are not in the intersection set I,
namely |N| - |I| (that's the number of fields in N but not in I). So, I
must contain at least N~l - (|N| - |I|) mines. We can use the same argument
for the set M and arrive at another lower bound of M~l - (|M| - |I|)
mines. Note that both of these numbers could be zero, so our third
lower bound is 0. The best lower bound we can give, therefore, is the
largest of these three numbers.

Next we'll see what we can say about the maximum number of mines in
the set I. One upper bound is the maximum number of mines in N,
N~u. The other one is M~u. Of course, I cannot contain more mines than
it contains fields, so the third upper bound is |I|. The best upper
bound we can give is the lowest of these three numbers.

Having established lower and upper mine bounds I~l and I~u for the
intersection set I, we'll now consider the difference set D = N - M, which
contains all fields in N which are not in M.

To establish its lower bound, assume that N contains as few mines as
possible (N~l), and that as many of N's mines as possible are in the
intersection I (that's the part of N that is not in D). We know this
latter number, too, it's Iu, the maximum number of mines in I. That
gives us a lower bound of N~l - I~u, which can, unfortunately, be
negative, so the second lower bound is 0.

We can establish the upper bound very similarly: Assume that N
contains as many mines as possible (N~u), and that as few mines as
possible are in the intersection I (I~l), giving an upper bound of
N~u - I~l, with the other upper bound being |D|, the number of fields in
the difference set.

The algorithm my solver uses is this:

* Given a configuration, for each visited field which neighbours on at
  least one non-visited field, build a set containing all neighbouring
  non-visited fields. The lower and upper bounds of this set is the
  number of mines the visited field still misses in its neighbourhood,
  i.e., the number on the field minus the number of already identified
  mines in its neighbourhood. Put all those sets into the list Q.

* Set D to be the empty list.

* Repeat the following until Q is empty: Take one set N out of Q and
  for each set M in D do: Build the difference set N - M according to
  the description above. If there is no set in D containing the same
  fields as this difference set, add it to D. If there is, and its
  bounds are at least as good as those of our new N - M, do nothing. If
  there is, but at least one of the bounds of N - M is better than the
  old bound, take the old set out of D, set its bounds to the improved
  values (if only one new bound is better, than only that bound is
  improved, i.e., if one of the new bounds is worse than the old one,
  it is not put in), and put the set into Q. Then do the same for the
  set M - N as well.

When the algorithm terminates, the list D will contain zero or more
sets with lower and upper bounds. Of real interest are only those sets
whose lower and upper bounds are equal and are also equal to the
number of fields in the set, or equal to zero. If both bounds are
equal to the number of fields in the set, we know that all fields
contain mines. If both bounds are zero, none of them do.

The lower and upper bounds of the other sets can at best be
interpreted as approximate probabilities for the presence of mines. If
a set containing one field has a lower bound of 0 and an upper bound
of 1, it is wrong to conclude that it has a 50% chance of containing a
mine. An exhaustive algorithm might find that that field must contain
a mine, for example. In fact, a program for finding the exact
probabilities in all configurations must be at least as inefficient as
a program for solving all solvable configurations (after all, solving
a configuration is nothing else than finding the probabilities for
only those fields which are certain to contain mines and for those
which are certain not to, which are 100% and 0%, respectively).

To illustrate this point a bit further, here's a configuration my
solver cannot solve:

    ++4-2
    --++-
    +6--3
    ++22+

After finishing its algorithm, my solver proclaims that the second
field in the second row (the one directly above the `6`) contains at
least 0 mines and at most 1. While this is certainly true (the bounds
my solver produces are always accurate, though not necessarily the
tightest ones possible), the probability that that field contains a
mine is nowhere near 50%. The exact probability is in fact 100%,
meaning that field must contain a mine. You can easily verify this by
examining the consequences of there being no mine in that field.

Obviously, there is room for improvement in my solver. One such
improvement which would probably take care of most cases arising
during solving random mine fields would be, in case the above
algorithm fails to unearth some certain mines or clear fields, to go
through all unvisited fields neighbouring at least one visited field
and try what the consequences would be if there were a mine in that
field, and what if there were not. If one of these two scenarios
resulted in a trivial contradiction, the other scenario would
inevitably reflect the truth (assuming, of course, that the whole
configuration does not in itself contain a conflict). Note that if
none of the two possibilities turn out to result in contradiction, no
conclusion can be made.

Of course, that new solver would still not be able to solve all
configurations, although it would take care of the example above.

## Compiling

Simply type

    make

By default, `minedetector` gives very verbose output.  To disable this,
comment the first line in the `Makefile` and compile again.

## Usage

Minedetector can be used either with prefabricated problems or on
random boards.

To generate and try to solve a random board, use this syntax:

    ./minedetector --random WIDTH HEIGHT NUM-MINES

To solve a problem, use this syntax:

    cat FILENAME | ./minedetector --read WIDTH HEIGHT

Where the problem contains the board.  See the `problem*` files for
examples.  `-` stands for unvisited fields, `+` for known mines.

## License

This program is licenced under the GNU General Public License.  See
the file `COPYING` for details.

---
Mark Probst <mark.probst@gmail.com>
