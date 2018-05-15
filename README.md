# bango-sdk

### Build
Install dependiences
```
$ sudo apt-get install libmysqlcppconn-dev
$ sudo apt-get install libmysqlclient-dev
```
Generate make. If you want to build with tests just add ```-DUSE_GTEST=ON``` at the end, similarly for benchamrks ```-DUSE_BENCHMARK=ON```
```
$ cmake -H. -Bbuild
```
Make

```
$ cd build
$ make
$ ./bin/dbserver
```

### Run tests
```
$ ./bin/bangonetwork_test
$ ./bin/bangospace_test

...
```
