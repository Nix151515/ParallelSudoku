Sudoku Solver
	Parallel Bois (Anghelus Dragos-Anton, State Nicolae; 343C5)
	
	Commands:
	- make = Compile each algorithm version (serial, omp, pthreads)
	- make build1 = Compile serial algorithm
	- make build2 = Compile omp algorithm
	- make build3 = Compile pthreads algorithm
	- make run1 = Run serial algorithm
	- make run2 = Run omp algorithm
	- make run3 = Run pthreads algorithm
	- make clean = Delete algorithms
	
	Edit NUM_THREADS and GRID_SIZE inside the file to change the parameters.
	Other testing sudoku grids can be found in grids.txt
	
	To submit a job to a queue:
		qsub -q <queue> -b y ./<exe>
		
	To submit a job to a queue measuring the time:
		qsub -q <queue> -b y time ./<exe>
		
	To delete a submitted job:
		qdel -f <jobID>