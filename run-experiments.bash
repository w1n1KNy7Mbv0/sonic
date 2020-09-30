#!/bin/bash

if [ ! -d "build" ]
then
    mkdir "build"
fi
if [ ! -d "results" ]
then
    mkdir "results"
fi
cd "build"
cmake ..
make clean
make -j
./SONIC --benchmark_filter=Build --benchmark_out_format=json --benchmark_out=../results/buildResults.json
./SONIC --benchmark_filter=Point_Lookup --benchmark_out_format=json --benchmark_out=../results/pointLookupResults.json
./SONIC --benchmark_filter=Prefix_Lookup --benchmark_out_format=json --benchmark_out=../results/prefixLookupResults.json
./SONIC --benchmark_filter=Bucket_Size --benchmark_out_format=json --benchmark_out=../results/bucketResults.json
./SONIC --benchmark_filter=String_Key_B --benchmark_out_format=json --benchmark_out=../results/stringKeyBuildResults.json
./SONIC --benchmark_filter=String_Key_Pr_Lookup --benchmark_out_format=json --benchmark_out=../results/stringKeyPrefixLookupResults.json
./WCOJ --benchmark_filter=Join --benchmark_out_format=json --benchmark_out=../results/joinResults.json
./WCOJ --benchmark_filter=TwoColumn_Counting --benchmark_out_format=json --benchmark_out=../results/countingResults.json