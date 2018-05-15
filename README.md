# bango-sdk

## Build
### Install dependencies
```
$ sudo apt-get install libmysqlcppconn-dev # not sure if neccessary
$ sudo apt-get install libmysqlclient-dev
```
### Run CMake
```
$ mkdir -p build
$ cd build
$ cmake ..
$ make
$ ../bin/dbserver
$ ../bin/gameserver
```
### Tests & Benchmarks
```
$ cmake .. -DUSE_GTEST=ON -DUSE_BENCHMARK=ON
$ make
$ ../bin/bangonetwork_test
$ ../bin/bangonetwork_benchmark
$ ../bin/bangospace_test
$ ../bin/bangospace_benchmark
```

