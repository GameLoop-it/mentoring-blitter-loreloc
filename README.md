
# SoftwareBlitter

## Benchmark

Compilator: GCC 7.2.0 x86-64   (MINGW64)  
Optimization flags: -O2  
Processor: Intel i5 4460 3.20Ghz  
Number of blits: 2000000  

|                      | Elapsed seconds |
|:--------------------:|:---------------:|
| Blitter without simd |      27.87      |
|   Blitter with simd  |      10.25      |  

The blitter that uses SSE2 instructions is about 2.7 times faster.  


