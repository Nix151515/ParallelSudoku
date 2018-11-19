build:
	gcc serial_solver.c -lgomp -o sudoku
	
clean:
	rm sudoku

run:
	./sudoku
