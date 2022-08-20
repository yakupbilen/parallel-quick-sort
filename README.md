## Parallel Quick Sort Implementation

##### The OpenMP library was used.

## Usage
#### The source code must be compiled with _**gcc 10.2.0**_.

### To run random_pivots.c

### ``gcc random.pivots.c -o out -fopenmp`` 

### To run optimum_pivots.c

### ``gcc optimum.pivots.c -o out -fopenmp`` 

## Result
#### Both versions were run 10 times with 4 threads and 800,000 numbers.

### Random pivots(Average) : 0.115289 second 
### Optimum pivots(Average) : 0.052082 second 
