# Violet target system hooks

This repository contains the common utilities to be used for a target system to 
be analyzed by Violet.

## Compilation

```bash
$ export S2EDIR=/path/to/s2e_root_dir

Or

$ source /path/to/violet/workspace/s2e_activate

$ mkdir {build, dist}
$ cd build
$ cmake .. -DCMAKE_INSTALL_PREFIX=`pwd`/../dist
$ make && make install
```

## Testing


### Normal environment testing


```bash
$ unset S2EDIR
$ rm -rf build
$ mkdir build && cd build
$ cmake -DCMAKE_BUILD_TYPE=DEBUG  -DCMAKE_C_FLAGS_DEBUG="-g -O0" -DCMAKE_CXX_FLAGS_DEBUG="-g -O0" ..
$ make -j4
$ cd test
```


1. Test generating single request based on a sample workload template file:

```bash
$ ./test_workload ../../test/template.json
```

2. Test generating multiple requests based on workload option file and workload template file:

```bash
$ ./test_workload ../../test/options.json ../../test/template.json 
```

### S2E environment testing

TBA

## Code Style Format

```bash
$ clang-format -i -style=file lib/*.c lib/*.h test/*.c
```
