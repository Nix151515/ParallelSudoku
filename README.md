# ParallelSudoku
A parallel alternative to a serial sudoku solver.
Works for any given table dimension (NxN).

Solution description:
- Each thread starts a different instance of the solver (backtracking), in which a different possible solution is chosen and put in the first empty space of the grid
- The thread that chose the correct value finishes a lot faster and closes the other working threads.
- Max efficiency achieved : 7.05 (calculated on a 4x4 grid)

Starting point (serial code) : http://www.techdarting.com/2014/11/sudoku-solver-without-recursion.html

Mentions:
- serial_solver.c is the original serial code.
- sudoku.c is the modified serial code.
