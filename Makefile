build:
	gcc sudoku_solver.c -lgomp -o sudoku
	
clean:
	rm sudoku

run:
	./sudoku
