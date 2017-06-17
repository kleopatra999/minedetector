# Minedetector - A solver for Minesweeper

For a description of what this program does, see [my "old stuff" page](http://www.complang.tuwien.ac.at/schani/oldstuff/).

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
