all: build1 build2

build2: pthreads.c
	gcc pthreads.c -o pthreads -lpthread -lgomp
	
build1: sudoku.c
	gcc sudoku.c -lgomp -o sudoku
	
clean: sudoku pthreads
	rm sudoku
	rm pthreads
	
run1:
	./sudoku
	
run2:
	./pthreads
	
run:
	./sudoku
	./pthreads