//
//  cachelab.h - header file for cachelab
//  : defining global variabls and functions
//

#ifndef cachelab_h
#define cachelab_h

#define HIT_TIME 1          // hit time fixed for calculating running time
#define MISS_PENALTY 100    // miss penalty fixed for calculating running time

//
// printResult
// : providing a standard way for your cache simulator to display its following result
//  - hits: number of hits
//  - misses: number of misses
//  - missRate: miss rate in percentage (=hits/(hits+misses))
//  - rumTime: total running time in cycle
//

void printResult(int hits, int misses, int missRate, int runTime);


#endif /* cachelab_h */
