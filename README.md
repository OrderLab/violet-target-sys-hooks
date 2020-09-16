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
## Code Style Format

```bash
$ clang-format -i -style=file lib/*.c lib/*.h test/*.c
```
