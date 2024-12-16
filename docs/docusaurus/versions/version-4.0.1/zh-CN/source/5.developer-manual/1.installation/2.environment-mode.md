# 环境分类

> 此文档主要介绍 TuGraph 涉及的三种环境。

## 1.分类

根据环境所承载功能的不同，区分为编译环境，运行环境，以及精简运行环境。
* 编译环境，具备TuGraph编译的所有依赖库，包含运行环境的所有依赖，并且能够编译TuGraph源码，但不包含预编译好的TuGraph可执行文件和库文件，供开发者编译源码使用。
* 运行环境，具备GCC/Java/Python环境，能够运行TuGraph的所有功能，并且能承载全文索引，java client，c++源码上传为plugin，以及python plugin的完整功能，内置TuGraph预编译好的可执行文件和库文件，供客户直接安装使用，无需编译源码。
* 精简运行环境，约等于裸系统加预编译TuGraph，仅能运行TuGraph的基本功能，无C++ plugin编译运行，仅so上传，无全文索引，无python plugin，供快速搭建试用。

TuGraph编译后，会把所有的依赖库以.a的形式打包在一起，因此原则上运行不需要的其他的依赖库。但TuGraph支持存储过程，即在服务端编译C++代码，因此在环境中依然需要涉及的编译器。

## 2.依赖系统库

针对三种环境，除去TuGraph的运行包，所需要的系统库如下：
* 编译环境，包括gcc、python、java等编译器，也包含antlr4、pybind11等，具体参见tugraph-db源码目录 ci/images/tugraph-compile-*-Dockerfile。
* 运行环境，主要由存储过程引入，包括gcc、boost、cmake等，具体参见tugraph-db源码目录 ci/images/tugraph-runtime-*-Dockerfile。
* 精简运行环境，无，可以参见tugraph-db源码目录 ci/images/ tugraph-mini-runtime-*-Dockerfile。



