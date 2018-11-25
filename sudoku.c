#include <stdio.h>  
 #include <stdlib.h>  
 #include <assert.h>  
 #include <omp.h>  
   
 #define GRID_SIZE 9  
   
 struct sudoku_elem_t  
 {  
   int val;  
   int fixed;  
 };  
 typedef struct sudoku_elem_t sudoku_elem;  
   
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
     grid[i] = (sudoku_elem *) calloc(9, sizeof(sudoku_elem));  
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
   // Check if the given number exists in the row or the column  
   for (i = 0; i < GRID_SIZE; i++)  
   {  
     if (grid[rowPos][i].val == valToCheck)  
     {  
       return -1;  
     }  
     if (grid[i][colPos].val == valToCheck)  
     {  
       return -1;  
     }  
   }  
   
   // Check if the same number exists in the 3x3 tile of rowPos and colPos  
   int bound3x3TopLeft_x = (rowPos / 3) * 3;  
   int bound3x3TopLeft_y = (colPos / 3) * 3;  
   int bound3x3BottomRight_x = ((rowPos / 3) + 1) * 3;  
   int bound3x3BottomRight_y = ((colPos / 3) + 1) * 3;  
   
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
     rowPos = rand() % 9;  
     colPos = rand() % 9;  
     if (grid[rowPos][colPos].val == 0)  
     {  
       break;  
     }  
   }  
   
   int i;  
   /*   Trying to find a value for the selected box */
   for (i = 1; i <=9; i++)
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
   for (i = 0; i < 9; i++)  
   {  
     for (j = 0; j < 9; j++)  
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
   
 // All the magic happens here.  
 void sudokuSolver(sudoku_elem **grid)  
 {  
   assert(grid);  
   /*  Stack with the previously selected values (from all boxes) */
   allTriedValues allValuesStack;  
   int ret = initAllTriedValues(&allValuesStack);
   int row = 0, col = 0;  
   if (ret == -1)  
   {  
     printf("Failed to allocate memory returning\n");  
     return;  
   }  
   int temp_cnt = 0;  
   
   /*   while there are empty boxes  */
   while (getUnfilledPosition(grid, &row, &col) == 0)  
   { 
     int startVal = 1;

    /* Search for a valid value and insert it into the tried values
                                                  for that box.
      Also, temporarely place the value on the grid  */
     for (startVal = 1; startVal <= 9; startVal++)  
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
           for (inc_val = inc_val; inc_val <= 9; inc_val++)  
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
         if (tempcol == 8)  
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
         int backtrackPos = backtrack_row*9 + backtrack_col;
         int startPos = start_row*9 + start_col;

         /* Bad, re-iterate the second while  */
         if (startPos > backtrackPos)  
         {  
           shouldBacktrack = 1;  
           continue;  
         }

        /* Fill the route emptied by going back */
         while (startPos <= backtrackPos)  
         {  
           start_row = startPos / 9;  
           start_col = startPos % 9;  
           if (grid[start_row][start_col].fixed == 0)  
           {  
             grid[start_row][start_col].val = 0;  
             for (startVal = 1; startVal <= 9; startVal++)  
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
   free(allValuesStack.allValues);  
 }  
   
 void printSudokuGrid(sudoku_elem **grid)  
 {  
   int i, j;  
   for (i = 0; i < GRID_SIZE; i++)  
   {  
     printf("\n");  
     for (j = 0; j < GRID_SIZE; j++ )  
       printf("%d-%d ", grid[i][j].val, grid[i][j].fixed);  
   }  
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
           return 0;  
         }  
       }  
     }  
   }  
   
   return 1;  
 }  
   
 int main()  
 {  
   int i, j;  
   
   sudoku_elem **newGrid = allocInitSudoku();  
   //sudoku_elem newGrid[GRID_SIZE][GRID_SIZE];  
   char *sudoku = "........3..1..56...9..4..7......9.5.7.......8.5.4.2....8..2..9...35..1..6........";  
   // Some of the other extreme Sudoku puzzles, you can try this out.  
   //".2..5.7..4..1....68....3...2....8..3.4..2.5.....6...1...2.9.....9......57.4...9..";  
   // ".......12........3..23..4....18....5.6..7.8.......9.....85.....9...4.5..47...6...";  
   // "..............3.85..1.2.......5.7.....4...1...9.......5......73..2.1........4...9";  
   
   for (i=0; i < GRID_SIZE; i++)  
   {  
     for (j=0; j < GRID_SIZE; j++)  
     {  
       if (sudoku[i*GRID_SIZE + j] != '.')  
       {  
         newGrid[i][j].fixed = 1;  
         newGrid[i][j].val = sudoku[i*GRID_SIZE + j] - '0';  
       }  
     }  
   }  
   
   int ret = isValidSudoku(newGrid);  
   if (ret == 0)  
   {  
     printf("Invalid sudoku exiting.....\n");  
     return -1;  
   }  
   printf("\n -0 ==> initially empty cell\n -1 ==> fixed cell, i.e., initially filled cell\n");  
   printSudokuGrid(newGrid);  
   
   double startime = omp_get_wtime();  
   sudokuSolver(newGrid);  
   double endtime = omp_get_wtime();  
   printf("\n\n Solved Sudoku ");  
   
   printSudokuGrid(newGrid);  
   printf("\n\nSolving time = %lf\n", endtime - startime);  
 }  