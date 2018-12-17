all: build1 build2 build3

build3: nxn_pthreads.c
	gcc nxn_pthreads.c -o nxn_pth -lpthread -lgomp

build2: pthreads.c
	gcc pthreads.c -o pthreads -lpthread -lgomp
	
build1: sudoku.c
	gcc sudoku.c -lgomp -o sudoku
	
clean: sudoku pthreads
	rm sudoku
	rm pthreads
	rm nxn_pth
	
run1:
	time ./sudoku
	
run2:
	time ./pthreads

run3:
	time ./nxn_pth
	
run:
	time ./sudoku
	time ./pthreads
	time ./nxn_pth