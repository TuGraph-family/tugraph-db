# Benchmark

This is a very simple (yet effective) benchmark for graph databases. 

Each vertex has two string fields, "no" and "name", each having fixed length of 10. "no" acts like the primary key in relational databases, i.e. all the operations use "no" as the entry.

The operations tested in this benchmark include the following:

- Batch and single insertion of vertices, which tests the sequential insertion performance ("no" of inserted vertices are in ascending order).
- Batch and single insertion of edges, which tests the random insertion performance (the "no" of source and destination vertices of inserted edges are uniformly random).
- Neighborhood lookup, which tests the random read performance.
- Pairwise shortest path, which tests the random read performance.

To observe the performance at different magnitudes, the benchmark consists of multiple phases, and each phase does the following:

- Insert *N* vertices in batches.
- Insert *N \* 10* edges in batches.
- Insert *C* vertices.
- Insert *C \* 10* edges.
- Complete *N* neighborhood lookups.
- Compute *N* pairwise shortest paths.

According to the above growing pattern, the last ~1/2 vertices will have 5 edges on average, the last ~1/4 vertices before these (last 1/2) vertices will have 10 edges on average, the last ~1/8 vertices before these (last 1/2 + 1/4) vertices will have 15 edges on average, ...
And the average degree of all vertices will be 10.

*N* is set to 100000 initially and is doubled after each phase finishes. *C* is set to 10000.

## Environment

CPU: 1 * Intel(R) Xeon(R) CPU E5-2620 v4

Memory: 64 GB

Disk: Intel SSDPEDMD020T4D HHHL NVMe 2 TB

## Results

Unit: Operations per second

"-": The number is unknown as the test takes too much time.

### Neo4j

JVM Heap Size: 8 GB

Worker Threads: 16

Insertion Batch: 500

|   \|V\|   | Vertex Insertion (Batch) | Edge Insertion (Batch) | Vertex Insertion | Edge Insertion | (1-Hop) Neighborhood Lookup | (3-Hop) Pairwise Shortest Path |
| :-------: | :----------------------: | :--------------------: | :--------------: | :------------: | :-------------------------: | :----------------------------: |
|  110000   |          21758           |         16544          |       805        |      5014      |            64602            |             30885              |
|  220000   |          56721           |         17411          |       981        |      5525      |           116461            |             56649              |
|  430000   |          57356           |         13911          |       989        |      5513      |           117970            |             55107              |
|  840000   |          65392           |         13203          |       1160       |      5430      |           129357            |             61333              |
|  1650000  |          66308           |         13531          |       1026       |      4862      |           123061            |             52457              |
|  3260000  |          51146           |         13049          |       937        |      4941      |           119166            |             49772              |
|  6470000  |          51498           |         13345          |       973        |      4835      |           105820            |             43575              |
| 12880000  |          52877           |         12545          |       840        |      4365      |           108351            |             41693              |
| 25690000  |          50758           |         11858          |       845        |      4539      |           104985            |             40895              |
| 51300000  |          49345           |         14030          |       963        |      5607      |           102485            |             40866              |
| 102510000 |          49031           |         12684          |       888        |      5203      |            85928            |             36653              |
| 204920000 |          47184           |          7896          |       865        |      3375      |            10346            |              1518              |
| 409720000 |          44123           |           -            |        -         |       -        |              -              |               -                |

#### Notes

- Using 8 threads and 32 threads both yield worse read performance.
- Neo4j recommends a heap size between 8 GB and 16 GB. The test uses 8 GB so that more space can be used for page cache.

### TuGraph

Worker Threads: 100

Insertion Batch: 20000

|   \|V\|   | Vertex Insertion (Batch) | Edge Insertion (Batch) | Vertex Insertion | Edge Insertion | (1-Hop) Neighborhood Lookup | (3-Hop) Pairwise Shortest Path |
| :-------: | :----------------------: | :--------------------: | :--------------: | :------------: | :-------------------------: | :----------------------------: |
|  110000   |          312261          |         131843         |       8877       |      8436      |           877087            |             294641             |
|  220000   |          368720          |         79165          |      10723       |      9829      |           1114740           |             303593             |
|  430000   |          336220          |         62852          |      10357       |      9669      |           876554            |             297832             |
|  840000   |          409569          |         54122          |       9700       |      8947      |           818778            |             300218             |
|  1650000  |          389312          |         48062          |       8504       |      8530      |           757130            |             304114             |
|  3260000  |          419935          |         44338          |       9921       |      8723      |           656944            |             264643             |
|  6470000  |          399891          |         41122          |       8928       |      9037      |           581386            |             256772             |
| 12880000  |          401556          |         37949          |       8621       |      8121      |           512254            |             248873             |
| 25690000  |          397384          |         34690          |       9151       |      8046      |           474045            |             242652             |
| 51300000  |          390508          |         30735          |       8145       |      7963      |           457671            |             239702             |
| 102510000 |          360493          |         20243          |       7055       |      6712      |           434898            |             234048             |
| 204920000 |          247563          |         19865          |       7517       |      4242      |           154713            |             149004             |
| 409720000 |          373830          |          6811          |       6604       |      2129      |            58072            |              68882             |
| 819340000 |          245184          |          3446          |       6851       |      1611      |            40679            |              48995             | 

Worker Threads: 240

Insertion Batch: 20000

|   \|V\|   | Vertex Insertion (Batch) | Edge Insertion (Batch) | Vertex Insertion | Edge Insertion | (1-Hop) Neighborhood Lookup | (3-Hop) Pairwise Shortest Path |
| :-------: | :----------------------: | :--------------------: | :--------------: | :------------: | :-------------------------: | :----------------------------: |
|  110000   |          315081          |         131536         |      12241       |     11625      |           1066360           |             244826             |
|  220000   |          362746          |         79796          |      10914       |      9957      |           1180980           |             285814             |
|  430000   |          379109          |         63774          |       9019       |      8888      |           567759            |             272874             |
|  840000   |          365245          |         53088          |       9001       |      8745      |           625378            |             283248             |
|  1650000  |          390920          |         47352          |      10646       |      8520      |           714893            |             236846             |
|  3260000  |          398702          |         43626          |       8552       |      8558      |           538829            |             196282             |
|  6470000  |          385274          |         39757          |      11061       |      8250      |           472987            |             189287             |
| 12880000  |          379560          |         36586          |       8334       |      7424      |           447343            |             176400             |
| 25690000  |          363715          |         33445          |       7400       |      7565      |           422511            |             172009             |
| 51300000  |          391800          |         30320          |       7625       |      7509      |           414918            |             169438             |
| 102510000 |          390222          |         20011          |       7704       |      6758      |           396959            |             165505             |
| 204920000 |          381615          |         19808          |       6642       |      4361      |           150756            |             102245             |
| 409720000 |          381279          |          6967          |       7136       |      2233      |            58796            |             59953              |
