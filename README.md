# Cache Simulation
Cache simulator written in C.

flags
-
- <b>m</b> : cache size <br>
- <b>s</b> : cache sets <br>
- <b>e</b> : direct mapped cache lines(0), multi-way cache lines (>0) <br>
- <b>b</b> : cache memory size <br>
- <b>i</b> : file name { address01, address02, address03 } <br>
- <b>r</b> : algorithm { lru, fifo } <br>

Least Recently Used Algorithm for cache replacement:

```./cachelab -m 32 -s 2 -e 0 -b 2 -i address03 -r lru```

```
30	M
31	H
42	M
55	M
32	M
E0	M
41	M
57	H
[result] hits: 2 misses: 6 miss rate: 75% total running time: 608 cycle
```

First In First Out (FIFO) Algorithm for cache replacement:

```./cachelab -m 32 -s 2 -e 0 -b 2 -i address03 -r fifo```

```
10	M
20	M
22	H
18	M
E10	M
210	M
12	M
[result] hits: 1 misses: 6 miss rate: 85% total running time: 607 cycle
```
