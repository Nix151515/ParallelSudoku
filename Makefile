all: build1 build2 build3

build3: nxn_pthreads.c
	gcc nxn_pthreads.c -o nxn_pth -lpthread -lgomp

build2: nxn_omp.c
	gcc nxn_omp.c -o nxn_omp -lpthread -lgomp
	
build1: sudoku.c
	gcc sudoku.c -lgomp -o sudoku
	
clean: sudoku nxn_pth nxn_omp
	rm sudoku
	rm nxn_omp
	rm nxn_pth
	
run1:
	time ./sudoku
	
run2:
	time ./nxn_omp

run3:
	time ./nxn_pth
	
run:
	time ./sudoku
	time ./nxn_omp
	time ./nxn_pth