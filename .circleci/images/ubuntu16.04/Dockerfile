# build for lgraph
FROM ubuntu:16.04

COPY sources.list /etc/apt

RUN apt-get update && apt-get install -y \
    git \
    ssh \
    tar \
    gzip \
    ca-certificates \
    g++ \
    pkg-config \
    uuid-dev \
    libssl-dev \
    lcov \
    curl \
    libleveldb-dev \
    unzip \
    bzip2 \
    python \
    autoconf \
    automake \
    libtool

# install python3
RUN wget https://www.python.org/ftp/python/3.6.9/Python-3.6.9.tgz \
    && tar zxf Python-3.6.9.tgz && cd Python-3.6.9 && ./configure --prefix=/usr/local \
    && make -j4 && make install && python3 -m pip install pexpect && rm -rf /Python*
# install cmake-3.13.4
RUN wget https://cmake.org/files/v3.13/cmake-3.13.4.tar.gz \
    && tar xzf cmake-3.13.4.tar.gz && cd cmake-3.13.4 \
    && ./bootstrap && make -j4 && make install && rm -rf /cmake-3.*
# install boost-1.68
RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.68.0/source/boost_1_68_0.tar.gz \
    && tar zxf boost_1_68_0.tar.gz && cd boost_1_68_0 \
    && ./bootstrap.sh --with-libraries=system,random,thread,filesystem,chrono,atomic,date_time,regex,stacktrace \
    && ./b2 -j4 cxxflags="-std=c++14 -fPIC" install && rm -rf /boost_*
# install protobuf
RUN wget https://github.com/protocolbuffers/protobuf/archive/v3.6.0.tar.gz -O protobuf-3.6.0.tar.gz \
    && tar xzf protobuf-3.6.0.tar.gz && cd protobuf-3.6.0 \
    && ./autogen.sh && ./configure CFLAGS='-fPIC' CXXFLAGS='-fPIC' --prefix=/usr/local \
    && make -j4 && make install && ldconfig && rm -rf /protobuf-*
# install snappy
RUN git clone https://github.com/google/snappy.git \
    && cd snappy && git checkout 1.1.9 && git submodule update --init && mkdir build && cd build \
    && cmake -DCMAKE_CXX_FLAGS=-fPIC .. && make -j4 && make install && rm -rf /snappy*
# install gflags
RUN wget https://github.com/gflags/gflags/archive/v2.2.1.tar.gz -O gflags-2.2.1.tar.gz \
    && tar xzf gflags-2.2.1.tar.gz && cd gflags-2.2.1 \
    && mkdir build && cd build && cmake -DCMAKE_CXX_FLAGS=-fPIC .. \
    && make -j4 && make install && rm -rf /gflags-*
# install node.js
RUN wget https://npm.taobao.org/mirrors/node/v10.13.0/node-v10.13.0.tar.gz \
    && tar xvf node-v10.13.0.tar.gz && cd node-v10.13.0/ && ./configure \
    && make -j4 && make install && npm config set registry https://registry.npm.taobao.org && rm -rf /node-*
ENV LD_LIBRARY_PATH=/usr/local/lib64:$LD_LIBRARY_PATH \
    PYTHONPATH=/usr/local/lib64:$PYTHONPATH

