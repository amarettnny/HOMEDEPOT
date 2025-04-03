Group members:
    Siyang Sun
    Tianji Li

Contributions:
MS1:
    Siyang wrote the Makefile, Tianji started the main.cpp

MS2:
    Siyang wrote the main.cpp and storing in cache
    Tianji wrote the cache_sim.cpp

MS3:
    Siyang wrote the storing for fifo
    Tianji wrote the loading for fifo

-----------------------------------------------------------------
Best cache report:
To conduct the best cache testing and find the best over effectiveness, we considers the following 
properties for simulation: hit rates, miss penalties, total cache size. These properties are calculated
using the following formula:
    hit rate = 100.0 * (load_hits + store_hits) / (total_loads + total_stores)

    miss penalties = (total_cycles - (total_loads + total_stores)) / (load_misses + store_misses)

    total cache size = num_sets * blocks per set * (block size in bytes + overhead in bytes per block)

To test for which cache configuration is the most effective (Direct Mapping / Fully Associative / Set 
Associative), and how parameters of set size / block size impact the performance, we keep the total 
cache size constant for all the following tests, which helps to better evaluate other parameters. We chose
the total cache size to be 32,768 bytes (32KB), and we need num_sets * block_per_set * block_size = 32KB. 

---------------------------------
Direct Mapping (1 block per set) -> eviction policy wouldn't cause difference:
./csim 512 1 64 write-allocate write-back lru <gcc.trace
Total loads: 318197
Total stores: 197486
Load hits: 314446
Load misses: 3751
Store hits: 194631
Store misses: 2855
Total cycles: 16483683
Hit Rate: 98.719%
Average Miss Penalty: 2417.2 cycles
Total Cache Size (including overhead): 34304 bytes

./csim 512 1 64 write-allocate write-through lru <gcc.trace
Total loads: 318197
Total stores: 197486
Load hits: 314446
Load misses: 3751
Store hits: 194631
Store misses: 2855
Total cycles: 30833883
Hit Rate: 98.719%
Average Miss Penalty: 4589.49 cycles
Total Cache Size (including overhead): 34304 bytes

./csim 512 1 64 no-write-allocate write-through lru <gcc.trace
Total loads: 318197
Total stores: 197486
Load hits: 313272
Load misses: 4925
Store hits: 171717
Store misses: 25769
Total cycles: 28144283
Hit Rate: 94.0479%
Average Miss Penalty: 900.13 cycles
Total Cache Size (including overhead): 34304 bytes

Section Conclusion: write-allocate and write-back is the best combination for now in terms of hit rate and
                    total cycles. 

---------------------------------
Fully Associative (1 num_sets):
./csim 1 512 64 write-allocate write-back lru <gcc.trace
Total loads: 318197
Total stores: 197486
Load hits: 316780
Load misses: 1417
Store hits: 195027
Store misses: 2459
Total cycles: 10838883
Hit Rate: 99.2484%
Average Miss Penalty: 2663.36 cycles
Total Cache Size (including overhead): 34816 bytes

./csim 1 512 64 write-allocate write-back fifo <gcc.trace
Total loads: 318197
Total stores: 197486
Load hits: 316457
Load misses: 1740
Store hits: 194974
Store misses: 2512
Total cycles: 11672483
Hit Rate: 99.1755%
Average Miss Penalty: 2623.89 cycles
Total Cache Size (including overhead): 34816 bytes

./csim 1 512 64 write-allocate write-through lru <gcc.trace
Total loads: 318197
Total stores: 197486
Load hits: 316780
Load misses: 1417
Store hits: 195027
Store misses: 2459
Total cycles: 26465883
Hit Rate: 99.2484%
Average Miss Penalty: 6695.1 cycles
Total Cache Size (including overhead): 34816 bytes

./csim 1 512 64 write-allocate write-through fifo <gcc.trace
Total loads: 318197
Total stores: 197486
Load hits: 316457
Load misses: 1740
Store hits: 194974
Store misses: 2512
Total cycles: 27067483
Hit Rate: 99.1755%
Average Miss Penalty: 6244.54 cycles
Total Cache Size (including overhead): 34816 bytes

./csim 1 512 64 no-write-allocate write-through lru <gcc.trace
Total loads: 318197
Total stores: 197486
Load hits: 315637
Load misses: 2560
Store hits: 173404
Store misses: 24082
Total cycles: 24360283
Hit Rate: 94.8336%
Average Miss Penalty: 895 cycles
Total Cache Size (including overhead): 34816 bytes

./csim 1 512 64 no-write-allocate write-through fifo <gcc.trace
Total loads: 318197
Total stores: 197486
Load hits: 315349
Load misses: 2848
Store hits: 173247
Store misses: 24239
Total cycles: 24821083
Hit Rate: 94.7474%
Average Miss Penalty: 897.309 cycles
Total Cache Size (including overhead): 34816 bytes

Section Conclusion: write-allocate and write-back is the best combination for now in terms of hit rate and
                    total cycles. Comparing the hit rate, lru is better than fifo for eviction policy. 

---------------------------------
Set Associative (fixed write-allocate & write-back):
2-way:
./csim 256 2 64 write-allocate write-back lru <gcc.trace
Total loads: 318197
Total stores: 197486
Load hits: 316409
Load misses: 1788
Store hits: 194945
Store misses: 2541
Total cycles: 11786083
Hit Rate: 99.1605%
Average Miss Penalty: 2603.47 cycles
Total Cache Size (including overhead): 34304 bytes

4-way:
./csim 128 4 64 write-allocate write-back lru <gcc.trace
Total loads: 318197
Total stores: 197486
Load hits: 316672
Load misses: 1525
Store hits: 194981
Store misses: 2505
Total cycles: 11197283
Hit Rate: 99.2185%
Average Miss Penalty: 2650.52 cycles
Total Cache Size (including overhead): 34304 bytes

8-way:
./csim 64 8 64 write-allocate write-back lru <gcc.trace
Total loads: 318197
Total stores: 197486
Load hits: 316734
Load misses: 1463
Store hits: 194999
Store misses: 2487
Total cycles: 11018083
Hit Rate: 99.234%
Average Miss Penalty: 2658.84 cycles
Total Cache Size (including overhead): 34304 bytes

16-way:
./csim 32 16 64 write-allocate write-back lru <gcc.trace
Total loads: 318197
Total stores: 197486
Load hits: 316764
Load misses: 1433
Store hits: 195010
Store misses: 2476
Total cycles: 10917283
Hit Rate: 99.242%
Average Miss Penalty: 2660.94 cycles
Total Cache Size (including overhead): 34304 bytes

Section Conclusion: with an increasing block_per_set number, we found an increasing hit rate (decreasing
                    miss rate), but also an increasing average miss penalty. 

Based on the experiment, the properties show that for a direct mapping cache, the eviction policy doesn't
influence performance, and the combination of write-allocate with write-back yields a notably lower 
total cycle count and high hit rate compare to other proper combination. In fully associative 
configuration, LRU slightly outperforms FIFO in terms of hit rate and total cycles, reinforcing that 
using write allocate with write-back is most effective.When using set-associative caches, as 
associativity increases from 2-way up to 16-way, the hit rate improves because more slots decrease the
missed possibility. 

Comparing fully-associative and set-associative, their performances in hit rate / average miss penalty are
similar. However in practice, set-associative caches are preferred because they reduce hardware complexity
and the power to compare tags parallely. 

Overall, these experiments shows that a cache design employing write-allocate and write-back with LRU
policy, and increasing high associativity, generally leads to the best performance in terms of both hit
rate and total cycles. 