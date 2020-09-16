# Test

```bash

# at root directory

$ unset S2EDIR
$ mkdir build && cd build
$ cmake -DCMAKE_BUILD_TYPE=DEBUG  -DCMAKE_C_FLAGS_DEBUG="-g -O0" -DCMAKE_CXX_FLAGS_DEBUG="-g -O0" ..
$ make -j4
$ cd test

# test generate single request based on workload template file
$ ./test_workload ../../test/template.json

# test generate multiple requests based on workload option file and workload template file
$ ./test_workload ../../test/options.json ../../test/template.json 
```

