Cache Simulator - CSF Assignment 3

Best Cache Configuration Experiments

Test 1: 256 4 16 write-allocate write-back lru
Baseline 4-way set-associative cache with LRU and write-back to compare against other configs.

gcc.trace:
Total loads: 318197
Total stores: 197486
Load hits: 314798
Load misses: 3399
Store hits: 188250
Store misses: 9236
Total cycles: 9344483

swim.trace:
Total loads: 220668
Total stores: 82525
Load hits: 219507
Load misses: 1161
Store hits: 71956
Store misses: 10569
Total cycles: 9009593

Test 2: 256 4 16 no-write-allocate write-through lru
Tests write-through policy which writes every store to memory, increasing memory traffic.

gcc.trace:
Total loads: 318197
Total stores: 197486
Load hits: 311613
Load misses: 6584
Store hits: 164819
Store misses: 32667
Total cycles: 22897883

swim.trace:
Total loads: 220668
Total stores: 82525
Load hits: 218072
Load misses: 2596
Store hits: 58030
Store misses: 24495
Total cycles: 9594093

Test 3: 256 4 16 write-allocate write-back fifo
Same as Test 1 but with FIFO eviction to compare against LRU.

gcc.trace:
Total loads: 318197
Total stores: 197486
Load hits: 314171
Load misses: 4026
Store hits: 188047
Store misses: 9439
Total cycles: 9845283

swim.trace:
Total loads: 220668
Total stores: 82525
Load hits: 218357
Load misses: 2311
Store hits: 71787
Store misses: 10738
Total cycles: 9655593

Test 4: 1 1024 16 write-allocate write-back lru
Fully associative cache to minimize conflict misses at the cost of larger set search.

gcc.trace:
Total loads: 318197
Total stores: 197486
Load hits: 314973
Load misses: 3224
Store hits: 188300
Store misses: 9186
Total cycles: 9226883

swim.trace:
Total loads: 220668
Total stores: 82525
Load hits: 219570
Load misses: 1098
Store hits: 71969
Store misses: 10556
Total cycles: 8965993

Test 5: 1024 1 16 write-allocate write-back lru
Direct-mapped cache to show the effect of conflict misses with no associativity.

gcc.trace:
Total loads: 318197
Total stores: 197486
Load hits: 312238
Load misses: 5959
Store hits: 187502
Store misses: 9984
Total cycles: 11127283

swim.trace:
Total loads: 220668
Total stores: 82525
Load hits: 217735
Load misses: 2933
Store hits: 71538
Store misses: 10987
Total cycles: 10171593

Test 6: 256 4 64 write-allocate write-back lru
Larger block size to exploit spatial locality at the cost of higher miss penalty.

gcc.trace:
Total loads: 318197
Total stores: 197486
Load hits: 316955
Load misses: 1242
Store hits: 195056
Store misses: 2430
Total cycles: 9707683

swim.trace:
Total loads: 220668
Total stores: 82525
Load hits: 220217
Load misses: 451
Store hits: 79454
Store misses: 3071
Total cycles: 9591193

Test 7: 256 4 128 write-allocate write-back lru
Even larger block size to further test spatial locality benefits vs miss penalty tradeoff.

gcc.trace:
Total loads: 318197
Total stores: 197486
Load hits: 317433
Load misses: 764
Store hits: 196178
Store misses: 1308
Total cycles: 9587683

swim.trace:
Total loads: 220668
Total stores: 82525
Load hits: 220374
Load misses: 294
Store hits: 80967
Store misses: 1558
Total cycles: 8757593

Contributions
Completed individually.
