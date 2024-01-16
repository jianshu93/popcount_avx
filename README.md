### popcount avx2 and avx512 benchmark
your CPU must support avx2 and avx512f. I was using GCC 12.
##how to run:
```bash
g++ -mavx2 -mavx512f -o ./avx_popcount ./avx_popcount.cpp
./avx_popcount
```
change line 104 and 105 to see different sketch and bbit size. With sketchsize64 128000, bbit 64, I have the following results:

Scalar Result: 6784000, Elapsed time: 20810 microseconds
AVX2 Result: 6784000, Elapsed time: 16547 microseconds
AVX512 Result: 6784000, Elapsed time: 14187 microseconds


On average, I have 30% to 40% improvement for avx2, 40% to 50% for avx512. Not sure whehter to include into bindash yet becase we will lose portability if using SIMD. 
