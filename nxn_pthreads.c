#include <stdio.h>  
 #include <stdlib.h>  
 #include <assert.h>  
 #include <omp.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>


#define NUM_THREADS 16
   
#define GRID_SIZE 16

 struct sudoku_elem_t  
 {  
   int val;  
   int fixed;  
 };  
 typedef struct sudoku_elem_t sudoku_elem;  

 sudoku_elem **newGrid;
 int stopThreads = 0;
   
 struct triedValue_t  
 {  
   int rowPos;  
   int colPos;  
   int value;  
 };  
 typedef struct triedValue_t triedValue;  
   
 struct allTriedValues_t  
 {  
   int count;  // how many values are inserted
   int size;   // the maximum size
   triedValue *allValues;  // the values array
 };  
 typedef struct allTriedValues_t allTriedValues;  

 sudoku_elem** allocInitSudoku()  
 {  
   sudoku_elem **grid;  

   /*   Allocate grid    */
   grid = (sudoku_elem **) malloc(sizeof(sudoku_elem *)*GRID_SIZE);  
   if (grid == NULL)  
   {  
     return NULL; // Failed to allocate memory returning  
   }
   int i, j;  

   /* Allocate elements  */
   for (i = 0; i < GRID_SIZE; i++)  
   {  
     grid[i] = (sudoku_elem *) calloc(GRID_SIZE, sizeof(sudoku_elem));  
     if (grid[i] == NULL)  
     {  
       return NULL;  
     }

     /* Init the elements ('fixed' and 'val')  */
     for (j=0; j < GRID_SIZE; j++)  
     {  
       grid[i][j].fixed = 0;  
       grid[i][j].val = 0;  
     }  
   }  
   return grid;  
 }  
   
  /*   Init allTriedValues structure (hardcoded size 30)  */
 int initAllTriedValues(allTriedValues *a)  
 {  
   assert(a);  
   
   a->count = 0;  
   a->size = 30;
   a->allValues = (triedValue *) malloc(sizeof(triedValue) * a->size);  
   if (a->allValues != NULL)  
   {  
     return 1;  
   }  
   return -1;  // Alloc failed
 }  

/*  Insert a 'tried value' (row, col, val) into the allValues array  */
 int insertIntoTriedValues(allTriedValues *a, int row, int col, int value)  
 {  
   assert(a);  
   
   /*   If there are more elements than the allocated size, realloc   */
   if (a->count == a->size)  
   {  
     a->size *= 2;  
     a->allValues = realloc(a->allValues, sizeof(triedValue)*a->size);  
     if (a->allValues == NULL)  
     {  
       return -1; // Reallocation failed no memory  
     }  
   } 
   a->allValues[a->count].rowPos = row;  
   a->allValues[a->count].colPos = col;  
   a->allValues[a->count].value = value;  
   a->count++;  
   return 1;  
 }  
   
 /* Returns 1: if no duplicates found in the sudoku constraints */  
 int verifyRules(sudoku_elem** grid, int rowPos, int colPos, int valToCheck)  
 {  
   int i, j;  
   int sqrtGrid = (int)floor(sqrt(GRID_SIZE));
   // Check if the given number exists in the row or the column
   for (i = 0; i < GRID_SIZE; i++)  
   {  
     if (grid[rowPos][i].val == valToCheck)  
     {  
       return -1;  
     }  
     if (grid[i][colPos].val == valToCheck)  
     {  
       // printf("grid[%d][%d] = %d \n",i,colPos,valToCheck);
       return -1;  
     }  
   }  
   
   // Check if the same number exists in the 3x3 tile of rowPos and colPos  
   int bound3x3TopLeft_x = (rowPos / sqrtGrid) * sqrtGrid;  
   int bound3x3TopLeft_y = (colPos / sqrtGrid) * sqrtGrid;  
   int bound3x3BottomRight_x = ((rowPos / sqrtGrid) + 1) * sqrtGrid;  
   int bound3x3BottomRight_y = ((colPos / sqrtGrid) + 1) * sqrtGrid;  
   
   for (i = bound3x3TopLeft_x; i < bound3x3BottomRight_x; i++)  
   {  
     for (j = bound3x3TopLeft_y; j < bound3x3BottomRight_y; j++)  
     {  
       if (grid[i][j].val == valToCheck)  
       {  

         return -1;  
       }  
     }  
   }  
   return 1;  
 }  

 /*   Insert a valid value in a random selected place  */
 int insertValuesInSudokuGrid(sudoku_elem **grid)  
 {  
   assert(grid);  
   int rowPos = 0;  
   int colPos = 0;  
   
   /* Randomly select the row and column  */
   while (1)  
   {  
     rowPos = rand() % GRID_SIZE;  
     colPos = rand() % GRID_SIZE;  
     if (grid[rowPos][colPos].val == 0)  
     {  
       break;  
     }  
   }  
   
   int i;  
   /*   Trying to find a value for the selected box */
   for (i = 1; i <=GRID_SIZE; i++)
   {  
     int retVal = verifyRules(grid, rowPos, colPos, i); 
     /*  If no duplicates found for the value (i)  */ 
     if (retVal == 1)
     { 
       /*  Insert the value and fix it  */ 
       grid[rowPos][colPos].val = i; 
       grid[rowPos][colPos].fixed = 1;  
       return 1;  
     }  
   }  
   return 1;  
 }  
  
  /*  Set the row and col with the first empty box  */
 int getUnfilledPosition(sudoku_elem **grid, int *row, int *col)  
 {  
   int i, j;  
   for (i = 0; i < GRID_SIZE; i++)  
   {  
     for (j = 0; j < GRID_SIZE; j++)  
     {  
       if (grid[i][j].val == 0 && grid[i][j].fixed != 1)  
       {  
         *row = i;  
         *col = j;  
         return 0;  
       }  
     }  
   }  
   return 1; // All the positions are filled appropriately  
 }  
   


   
 void printSudokuGrid(sudoku_elem **grid)  
 {  
   int i, j;  
   for (i = 0; i < GRID_SIZE; i++)  
   {  
      if(i % (int)floor(sqrt(GRID_SIZE)) == 0)
      {
          printf("\n---------------------------------------------------------\n");
      }
      else
        printf("\n");  
      for (j = 0; j < GRID_SIZE; j++ )
      {
        if(j % (int)floor(sqrt(GRID_SIZE)) == 0)
        {
          printf("| ");
        }
        if(grid[i][j].val < 10)
          printf("%d  ", grid[i][j].val);
        else
          printf("%d ", grid[i][j].val);
      }
      printf("| ");
   }
  printf("\n---------------------------------------------------------\n");
 }  
   
 int isValidSudoku(sudoku_elem **grid)  
 {  
   int i, j;  
   for (i=0; i < GRID_SIZE; i++)  
   {  
     for (j=0; j < GRID_SIZE; j++)  
     {  
       if (grid[i][j].val != 0)  
       {  
         int temp = grid[i][j].val;  
         grid[i][j].val = 0;  
         int retval = verifyRules(grid, i, j, temp);  
         grid[i][j].val = temp;  
         if (retval == -1)  
         {
            printf("Not valid at (%d, %d), val %d \n",i ,j, temp); 
           return 0;  
         }  
       }  
     }  
   }  
   return 1;  
 }  




void *TryTheNumber(void *threadID)
{

  /*  Starting with 0 in main, incrementing   */
  int threadid = *((int*)(&threadID)) + 1;

  pthread_detach(pthread_self());
  printf("Hello from thread %d \n",(int) threadid);
 


  allTriedValues allValuesStack;  
     int a = 0,b = 0, c=0, d=0;
  int ret = initAllTriedValues(&allValuesStack);
  int row = 0, col = 0;  
  if (ret == -1)  
  {  
    printf("Failed to allocate memory returning\n");  
    
  }

  int temp_cnt = 0;


  int i,j;
  sudoku_elem **grid;
  grid = allocInitSudoku();
  for (i=0; i < GRID_SIZE; i++)  
   {  
     for (j=0; j < GRID_SIZE; j++)  
     {  
         grid[i][j].fixed = newGrid[i][j].fixed;  
         grid[i][j].val = newGrid[i][j].val;   
     }  
   }  



   double startime = omp_get_wtime();
  if(getUnfilledPosition(grid, &row, &col) == 0)
  {
      int retVal = verifyRules(grid, row, col, threadid);  
       if (retVal == 1)  
       {  
         grid[row][col].val = threadid;
         insertIntoTriedValues(&allValuesStack, row, col, threadid);
       }
  }
   
   /*   while there are empty boxes  */
   while (getUnfilledPosition(grid, &row, &col) == 0)  
   {
     if(stopThreads == 1)
     {
        printf("Interrupting thread (point1) %d\n",threadid);
        pthread_exit(threadID);
     }
     int startVal = 1;

    /* Search for a valid value and insert it into the tried values
                                                  for that box.
      Also, temporarely place the value on the grid  */
     for (startVal = 1; startVal <= GRID_SIZE; startVal++)  
     {  
       int retVal = verifyRules(grid, row, col, startVal);  
       if (retVal == 1)  
       {  
         grid[row][col].val = startVal;


         insertIntoTriedValues(&allValuesStack, row, col, startVal);  
          break;
       }  
     }

     /* If duplicates were found for that box
        (no value assigned above) = should go back or try another value */

     if (grid[row][col].val == 0)  
     {  
       int shouldBacktrack = 1;  
       int backtrack_level;  
       int backtrack_row = row;  
       int backtrack_col = col;  
       int temprow = row;  
       int tempcol = col;  
   
       while (grid[backtrack_row][backtrack_col].val == 0)  
       {
          if(stopThreads == 1)
         {
            printf("Interrupting thread (point2) %d\n",threadid);
            //return;
            pthread_exit(threadID);
         }


         if (shouldBacktrack && (allValuesStack.count > 0))  
         { 
          /* Go a level back  */
           allValuesStack.count--; 
           backtrack_level = allValuesStack.count;  
           temprow = allValuesStack.allValues[backtrack_level].rowPos;  
           tempcol = allValuesStack.allValues[backtrack_level].colPos;

           /* Get the next possible value  */  
           int inc_val = grid[temprow][tempcol].val + 1;  
   
           grid[temprow][tempcol].val = 0;  
           /*  Check from the next possible value to 9  */
           for (inc_val = inc_val; inc_val <= GRID_SIZE; inc_val++)  
           {  
             int retVal = verifyRules(grid, temprow, tempcol, inc_val);

             /*  If we found a suitable value, don't go back further */
             if (retVal == 1)  
             {  
               grid[temprow][tempcol].val = inc_val;  
               shouldBacktrack = 0;  


               insertIntoTriedValues(&allValuesStack, temprow, tempcol, inc_val);  
               break;  
             }  
           }
           /*  If another possible value does not exist, go deeper
                (repeat the second while)  */  
           if (grid[temprow][tempcol].val == 0)  
           {  
             shouldBacktrack = 1;  
             continue;  
           }  
         }
         /* All good, take the next box */
         int start_row, start_col;  
         if (tempcol == GRID_SIZE - 1)  
         {  
           start_col = 0;  
           start_row = temprow + 1;  
         }  
         else  
         {  
           start_col = tempcol + 1;  
           start_row = temprow;  
         }

        /* Calculate the box number */
         int backtrackPos = backtrack_row* GRID_SIZE + backtrack_col;
         int startPos = start_row* GRID_SIZE + start_col;

         /* Bad, re-iterate the second while  */
         if (startPos > backtrackPos)  
         {  
           shouldBacktrack = 1;  
           continue;  
         }

        /* Fill the route emptied by going back */
         while (startPos <= backtrackPos)  
         {
           if(stopThreads == 1)
           {
              printf("Interrupting thread (point3) %d\n",threadid);
              pthread_exit(threadID);
           }
           start_row = startPos / GRID_SIZE;  
           start_col = startPos % GRID_SIZE;  
           if (grid[start_row][start_col].fixed == 0)  
           {  
             grid[start_row][start_col].val = 0;  
             for (startVal = 1; startVal <= GRID_SIZE; startVal++)  
             {  
               int retVal = verifyRules(grid, start_row, start_col, startVal);  
               if (retVal == 1)  
               {  
                 grid[start_row][start_col].val = startVal;

                 insertIntoTriedValues(&allValuesStack, start_row, start_col, startVal);  
                 break;  
               }  
             }  
             if (grid[start_row][start_col].val == 0)  
             {  
               shouldBacktrack = 1;  
               break;  
             }  
             else  
             {  
               shouldBacktrack = 0;  
             }  
           }  
           startPos++;  
         }

           
       }  // 2-nd while ends
     } // if ends  
   } // first while ends
   double endtime = omp_get_wtime();
   printf("Thread %d finished in %lf\n", threadid, endtime - startime);
   printSudokuGrid(grid);  
   free(allValuesStack.allValues);

  printf("StopThreads \n");
   stopThreads = 1;

}




   
 int main()  
 {  
   int i, j;  
   
   newGrid = allocInitSudoku();    
   char *sudoku = 
   ".g.........e375...e2..cbf..9..8.f.....28a.7.d..c..3........5a.e21.45.7.2b..c....b7.fd...32.g41..c..9..b.7......e.e.8.1.g5..4........2..7c.d.e.a.g......c.e..9..7..cde.a1...72.45....g..d4.a.6f.18b.4c........a..5..3.b.48g.....9.2..3..fe6..54...6f78.........d.";

   for (i=0; i < GRID_SIZE; i++)  
   {  
     for (j=0; j < GRID_SIZE; j++)  
     {  
       if (sudoku[i*GRID_SIZE + j] != '.')
       {
          if(sudoku[i*GRID_SIZE + j] >= 'a' && sudoku[i*GRID_SIZE + j] <= 'g')
          {
 
           newGrid[i][j].fixed = 1;  
           newGrid[i][j].val = sudoku[i*GRID_SIZE + j] - '0'- 39;  
          }
          else  
          {  
           newGrid[i][j].fixed = 1;  
           newGrid[i][j].val = sudoku[i*GRID_SIZE + j] - '0';  
          }
        }  
     }  
   }  
   
   int ret = isValidSudoku(newGrid);  
   if (ret == 0)  
   {  
     printf("Invalid sudoku exiting.....\n");  
     printSudokuGrid(newGrid);
     return -1;  
   }  
   printf("\n -0 ==> initially empty cell\n -1 ==> fixed cell, i.e., initially filled cell\n");  
   printSudokuGrid(newGrid);  

   pthread_t threads[NUM_THREADS];
   int t, rc, count = 0, r, c;

   getUnfilledPosition(newGrid, &r, &c);
   printf("Starting threads on (%d, %d) \n", r,c);

    for(t = 0 ; t < GRID_SIZE ; t++) {
       if(count >= NUM_THREADS)
       {
        printf("Count reached %d, exiting\n", count);
        break;
       }

      ret =  verifyRules(newGrid, r, c, t+1);
      if(ret == 1)
      {
        count ++;
        printf("Count is %d, thread %d started\n",count,t+1);
        rc = pthread_create(&threads[t], NULL, TryTheNumber, (void *)t);
        
        
      }
    }  
    pthread_exit(NULL);
   
   return 0;  
 }  