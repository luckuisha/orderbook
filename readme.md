### Custom Orderbook

I am trying to use modern c++ to create a modern version of what I think an orderbook is. 

## Requirements

Fast


## To implement

Perhaps a ui with a ticker price
multiple threads creating orders
real time data?


## Building

`make`

the resulting build files/executables exist within the build directory thats generated. 

run `clean` if something breaks lol hope not

## testing

`make test` for running tests. uses doctest for its speed of compile and simplicity, based off of catch2. 

## benchmarking

run `make bench` to build and run the bench marking tests. 

Here are some basic results:

Benchmark Report BENCH INSERTION
Total Samples: 100000
Total Time: 775357094 ns
Ops per second: 128972
Median: 6041 ns
0.99: 25250 ns
0.999: 30167 ns
0.99999: 852042 ns

Benchmark Report BENCH CANCEL
Total Samples: 100000
Total Time: 1369384988 ns
Ops per second: 73025
Median: 11792 ns
0.99: 41000 ns
0.999: 409292 ns
0.99999: 885000 ns

Benchmark Report BENCH MATCH
Total Samples: 100000
Total Time: 1369384988 ns
Ops per second: 73025
Median: 11792 ns
0.99: 41000 ns
0.999: 409292 ns
0.99999: 885000 ns