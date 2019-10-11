# FastEncode
Fastest encryption and hashing

Special thanks:
For Base64 SIMD implementations
https://github.com/aklomp/base64

For hardware supported crc32c hash function
https://github.com/google/crc32c

For HighwayHash
https://github.com/google/highwayhash

For the fastest hash function
http://www.sanmayce.com/Fastest_Hash/index.html

## TODO

* derive all encryption and hashing classes from base class
* move all definitions to source files
* move sources to shared VS project
* add Android/iOS support
* make Unity3d wrapper to call functions
* add C# benchmarks  

## Benchmarks:

Ryzen 7 1700

|Benchmark                                 |        Time |          CPU | Iterations | 
|------------------------------------------|------------:|-------------:|-----------:| 
|Base64_encrypt                            |  2537713 ns |   2490234 ns |        320 |
|Base64_decrypt                            |  1977678 ns |   1992754 ns |        345 |
|Base64_avx_encrypt                        |  1353243 ns |   1349147 ns |        498 |
|Base64_avx_decrypt                        |  1105749 ns |   1098633 ns |        640 |
|Base64_sse_encrypt                        |  1061225 ns |   1074219 ns |        640 |
|Base64_sse_decrypt                        |   854428 ns |    836680 ns |        747 |
|XorHash                                   |   219898 ns |    219702 ns |       2987 |
|Crc32_default                             | 15616691 ns |  15625000 ns |         41 |
|Crc32_sse                                 |   462149 ns |    467115 ns |       1338 |
|Highway_default                           |  4010927 ns |   4006410 ns |        195 |
|Fnv1aYoshimitsuTriad                      |   226045 ns |    223214 ns |       2800 |
|Fnv1aYoshimitsu                           |   221926 ns |    224609 ns |       3200 |
|XorEncryption_encrypt                     |   493955 ns |    491879 ns |       1493 |
|XorEncryption_decrypt                     |   476135 ns |    474330 ns |       1120 |
|Blowfish_default_Strong_encrypt           |  6073362 ns |   5998884 ns |        112 |
|Blowfish_default_Strong_decrypt           |  6132347 ns |   5937500 ns |        100 |
|Blowfish_default_Strong_decryptCorrupted  |   464815 ns |    460379 ns |       1120 |
|Blowfish_default_Medium_encrypt           |  4214548 ns |   4269622 ns |        172 |
|Blowfish_default_Medium_decrypt           |  4454882 ns |   4451308 ns |        172 |
|Blowfish_default_Weak_encrypt             |  2913909 ns |   2761044 ns |        249 |
|Blowfish_default_Weak_decrypt             |  2605956 ns |   2622768 ns |        280 |
|ChaCha20_encrypt                          |  1968431 ns |   1947464 ns |        345 |
|ChaCha20_decrypt                          |  2025994 ns |   2001953 ns |        320 |
|ChaCha12_encrypt                          |  1249505 ns |   1255580 ns |        560 |
|ChaCha12_decrypt                          |  1377424 ns |   1360212 ns |        448 |
|ChaCha8_encrypt                           |   972767 ns |    983099 ns |        747 |
|ChaCha8_decrypt                           |   998723 ns |   1004016 ns |        747 |
|XChaCha20_encrypt                         |  2189368 ns |   2197266 ns |        320 |
|XChaCha20_decrypt                         |  2226975 ns |   2246094 ns |        320 |
|XChaCha12_encrypt                         |  1367895 ns |   1349147 ns |        498 |
|XChaCha12_decrypt                         |  1354748 ns |   1367188 ns |        560 |
|XChaCha8_encrypt                          |  1033556 ns |   1025391 ns |        640 |
|XChaCha8_decrypt                          |   997369 ns |   1004016 ns |        747 |
 
Core i5-7600

|Benchmark                                 |        Time |          CPU | Iterations | 
|------------------------------------------|------------:|-------------:|-----------:| 
|Base64_encrypt                            |  1715506 ns |   1675603 ns |        373 |
|Base64_decrypt                            |  1499776 ns |   1443273 ns |        498 |
|Base64_avx_encrypt                        |   562126 ns |    558036 ns |       1120 |
|Base64_avx_decrypt                        |   492261 ns |    474330 ns |       1120 |
|Base64_sse_encrypt                        |   551984 ns |    558036 ns |       1120 |
|Base64_sse_decrypt                        |   551972 ns |    558036 ns |       1120 |
|XorHash                                   |   287350 ns |    288771 ns |       2489 |
|Crc32_default                             |  9415055 ns |   9375000 ns |         75 |
|Crc32_sse                                 |   131239 ns |    125558 ns |       4480 |
|Highway_default                           |  3395467 ns |   3370098 ns |        204 |
|Fnv1aYoshimitsuTriad                      |   152560 ns |    153460 ns |       4480 |
|Fnv1aYoshimitsu                           |   151697 ns |    149972 ns |       4480 |
|ChaCha20_encrypt                          |   980564 ns |    994001 ns |        896 |
|ChaCha20_decrypt                          |   973629 ns |    983099 ns |        747 |
|ChaCha12_encrypt                          |   661670 ns |    655692 ns |       1120 |
|ChaCha12_decrypt                          |   650657 ns |    645229 ns |        896 |
|ChaCha8_encrypt                           |   505214 ns |    531250 ns |       1000 |
|ChaCha8_decrypt                           |   496223 ns |    500000 ns |       1000 |
|XChaCha20_encrypt                         |   971860 ns |    983099 ns |        747 |
|XChaCha20_decrypt                         |   980723 ns |    976563 ns |        640 |
|XChaCha12_encrypt                         |   668483 ns |    655692 ns |       1120 |
|XChaCha12_decrypt                         |   659426 ns |    669643 ns |       1120 |
|XChaCha8_encrypt                          |   509314 ns |    502232 ns |       1120 |
|XChaCha8_decrypt                          |   503897 ns |    500000 ns |       1000 |