## your CPU must support avx2 and avx512f
##how to run:
```bash
g++ -mavx2 -mavx512f -o ./avx_popcount ./avx_popcount.cpp
./avx_popcount
```
change line 104 and 105 to see different sketch and bbit size. With skethsize64 128000, bbit 64, I have the following results:

Scalar Result: 6784000, Elapsed time: 20810 microseconds
AVX2 Result: 6784000, Elapsed time: 16547 microseconds
AVX512 Result: 6784000, Elapsed time: 14187 microseconds
