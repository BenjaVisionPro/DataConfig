# Benchmark

We've integrated two benchmarks fixtures from [JSON for modern C++][1] project. In the benchmark we deserialize JSON into C++ structs, then serialize them back to JSON. Then we convert JSON to MsgPack and do identical process.

* [canada.json]({{RepoRoot}}DataConfig/Tests/LargeFixtures/canada.json)
* [corpus.ndjson]({{RepoRoot}}DataConfig/Tests/LargeFixtures/corpus.ndjson)

See [here for instructions](./Automation.md#running-the-benchmarks) to build and run the benchmark.

Here're the results:

* [DcBenchmarkFixture1.cpp]({{SrcRoot}}DataConfigExtra/Private/DataConfig/Extra/Benchmark/DcBenchmarkFixture1.cpp)

On a AMD Ryzen 9 5950X 16-Core Processor 3.40 GHz:

```
Corpus Json Deserialize: [Shipping] Bandwidth: 94.951(MB/s), Mean: 58.097(ms), Median:58.061(ms), Deviation:0.997
Corpus Json Serialize: [Shipping] Bandwidth: 117.859(MB/s), Mean: 46.805(ms), Median:46.086(ms), Deviation:1.818
Corpus MsgPack Deserialize: [Shipping] Bandwidth: 103.436(MB/s), Mean: 50.531(ms), Median:50.453(ms), Deviation:0.469
Corpus MsgPack Serialize: [Shipping] Bandwidth: 103.586(MB/s), Mean: 50.457(ms), Median:50.342(ms), Deviation:0.614

Canada Json Deserialize: [Shipping] Bandwidth: 73.581(MB/s), Mean: 29.176(ms), Median:29.171(ms), Deviation:0.132
Canada Json Serialize: [Shipping] Bandwidth: 56.439(MB/s), Mean: 38.037(ms), Median:37.882(ms), Deviation:1.050
Canada MsgPack Serialize: [Shipping] Bandwidth: 131.555(MB/s), Mean: 4.441(ms), Median:4.432(ms), Deviation:0.030
Canada MsgPack Deserialize: [Shipping] Bandwidth: 100.450(MB/s), Mean: 5.816(ms), Median:5.816(ms), Deviation:0.024
```

Some insights on the results:

- Benchmark in `Shipping` build configuration, otherwise it doesn't make much sense.

- Recall that [runtime performance isn't our top priority](../Design.md#manifesto). We opted for a classic inheritance based API for `FDcReader/FDcWriter` 
  which means that each read/write step result in a virtual dispatch. This by design would result in mediocre performance metrics.
  The bandwidth should be in the range of `10~100(MB/s)` on common PC setup, no matter how simple the format is.

- MsgPack and JSON has similar bandwidth numbers in the benchmark. However MsgPack has far more tight layout when dealing with 
  numeric data. Note in the `Canada` fixture MsgPack only takes around 10ms, as this fixture is mostly float number coordinates.


[1]:https://json.nlohmann.me "JSON for Modern C++"
