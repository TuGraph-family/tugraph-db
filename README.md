# TuGraph

TuGraph is a graph database that supports fast lookup and batch update.

## Compile the project with GCC:
1. `deps/build_deps.sh` or `SKIP_WEB=1 deps/build_deps.sh` to skip building web interface
2. `cmake .. -DOURSYSTEM=centos` or `cmake .. -DOURSYSTEM=ubuntu`
3. If suport shell lgraph_cypher, use `-DENABLE_PREDOWNLOAD_DEPENDS_PACKAGE=1`
4. `make`
5. `make package` or `cpack --config CPackConfig.cmake`

## Compile the project with Clang on macOS:
1. `deps/build_deps.sh` or `SKIP_WEB=1 deps/build_deps.sh` to skip building web interface
2. `cmake ..`
3. `make`

## Release notice:
1. Use gcc-5.4.0 or gcc-7.5.0
2. Use CMAKE_BUILD_TYPE `Release`
3. Check the package's directory tree (especially `include`)
4. Make sure the front-end updated

## Code Style
### Automatic cpplint check
Execute script `cpplint/git-hooks-init.sh` to configure your local git hooks
When run `git push`, the hook script is automatically triggered to check the cpp source files of this push with cpplint.
### Manual cpplint check
Before `git push`, use `cpplint` to check the c++ source files involved in this commit.
`cpplint` is just a python file that has been placed in the code repository, which path is `./cpplint/cpplint.py`.
For example, if you modify the two files `src/algo/algo.cpp` and `src/algo/algo.h`, execute the following command
```
./cpplint/cpplint.py src/algo/algo.cpp src/algo/algo.h
```
If `cpplint.py` reports some warnings, follow the prompts to modify until it is passed
You can also use the `./cpplint/check_code_style.sh` file, this script will check the source files involved in the latest git commit

If want to check all the cpp files in the `src` directory, use the following command
```
./cpplint/cpplint.py --recursive src
```
For more details, see [cpplint](https://github.com/cpplint/cpplint) or `./cpplint/cpplint.py --help`
### clang-format
You can use clang-format to format the code, the command is as follows:
```
clang-format -i src/algo/algo.cpp
```
clang-format tool needs to be installed on your own computer by yourself.
clang-format will look for the default configuration file `.clang-format` from the current path, which has been placed in the code repository and describes Google's cpp style.
For more Google's cpp style details, see [English Version](https://google.github.io/styleguide/cppguide.html#Forward_Declarations) or [中文版](https://google-styleguide.readthedocs.io/zh_CN/latest/google-cpp-styleguide/contents.html)
