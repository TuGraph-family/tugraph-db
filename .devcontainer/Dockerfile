# TuGraph-DB devcontainer - Ubuntu 24.04 base with all build dependencies
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Base build tools
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    vim \
    git \
    gdb \
    wget \
    curl \
    ca-certificates \
    gfortran \
    python3-dev \
    python3-pip \
    autoconf \
    automake \
    libtool \
    libasan8 \
    libstdc++-14-dev \
    libopenblas-dev \
    liblapacke-dev \
    locales \
    procps \
    iproute2 \
    findutils \
    tar \
    pkg-config \
    unzip \
    && rm -rf /var/lib/apt/lists/*

RUN sed -i '/en_US.UTF-8/s/^# //g' /etc/locale.gen && locale-gen
ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US:en
ENV LC_ALL=en_US.UTF-8

# googletest
RUN wget http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/gtest_v1.15.2.tar.gz -O /tmp/googletest.tar.gz && \
    cd /tmp && mkdir googletest && tar -xzf googletest.tar.gz --strip-components=1 -C googletest && cd googletest && \
    mkdir build && cd build && cmake .. && \
    make -j10 && make install && \
    cd / && rm -rf /tmp/googletest*

# gflags
RUN wget -q http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/gflags_v2.2.2.tar.gz -O /tmp/gflags.tar.gz && \
    cd /tmp && mkdir gflags && tar -xzf gflags.tar.gz --strip-components=1 -C gflags && cd gflags && \
    mkdir build && cd build && cmake .. && make -j$(nproc) && make install && \
    cd / && rm -rf /tmp/gflags*

# jemalloc
RUN wget -q http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/jemalloc_5.3.0.tar.gz -O /tmp/jemalloc.tar.gz && \
    cd /tmp && mkdir jemalloc && tar -xzf jemalloc.tar.gz --strip-components=1 -C jemalloc && cd jemalloc && \
    ./autogen.sh && ./configure && make -j$(nproc) && make install && \
    cd / && rm -rf /tmp/jemalloc*

# lz4
RUN wget -q http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/lz4_v1.10.0.tar.gz -O /tmp/lz4.tar.gz && \
    cd /tmp && mkdir lz4 && tar -xzf lz4.tar.gz --strip-components=1 -C lz4 && cd lz4 && \
    make -j$(nproc) && make install && \
    cd / && rm -rf /tmp/lz4*

# zstd
RUN wget -q http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/zstd_v1.5.6.tar.gz -O /tmp/zstd.tar.gz && \
    cd /tmp && mkdir zstd && tar -xzf zstd.tar.gz --strip-components=1 -C zstd && cd zstd && \
    make -j$(nproc) && make install && \
    cd / && rm -rf /tmp/zstd*

# rocksdb
RUN wget -q http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/rocksdb_v9.5.2.tar.gz -O /tmp/rocksdb.tar.gz && \
    cd /tmp && mkdir rocksdb && tar -xzf rocksdb.tar.gz --strip-components=1 -C rocksdb && cd rocksdb && \
    mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DWITH_GFLAGS=1 \
    -DROCKSDB_BUILD_SHARED=0 \
    -DWITH_JEMALLOC=ON \
    -DWITH_LZ4=ON \
    -DWITH_ZSTD=ON \
    -DWITH_TOOLS=OFF \
    -DWITH_CORE_TOOLS=OFF \
    -DWITH_TRACE_TOOLS=OFF \
    -DWITH_BENCHMARK_TOOLS=OFF \
    -DPORTABLE=ON .. \
    && make -j$(nproc) && make install && \
    cd / && rm -rf /tmp/rocksdb*

# spdlog
RUN wget -q http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/spdlog_v1.14.1.tar.gz -O /tmp/spdlog.tar.gz && \
    cd /tmp && mkdir spdlog && tar -xzf spdlog.tar.gz --strip-components=1 -C spdlog && cd spdlog && \
    mkdir build && cd build && cmake .. && make -j$(nproc) && make install && \
    cd / && rm -rf /tmp/spdlog*

# antlr4
RUN wget -q http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/antlr4_4.13.2.tar.gz -O /tmp/antlr4.tar.gz && \
    cd /tmp && mkdir antlr4 && tar -xzf antlr4.tar.gz --strip-components=1 -C antlr4 && cd antlr4/runtime/Cpp && \
    mkdir build && cd build && cmake -DWITH_DEMO=0 -DANTLR_BUILD_CPP_TESTS=0 -DANTLR4_INSTALL=1 -DCMAKE_CXX_STANDARD=17 .. && \
    make -j$(nproc) && make install && \
    cd / && rm -rf /tmp/antlr4*

# protobuf
RUN wget -q http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/protobuf_v3.6.0.tar.gz -O /tmp/protobuf.tar.gz && \
    cd /tmp && mkdir protobuf && tar -xzf protobuf.tar.gz --strip-components=1 -C protobuf && cd protobuf && \
    ./autogen.sh && ./configure && make -j$(nproc) && make install && \
    cd / && rm -rf /tmp/protobuf*

# boost
RUN wget -q http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/boost_1_86_0.tar.gz -O /tmp/boost.tar.gz && \
    cd /tmp && mkdir boost && tar -xzf boost.tar.gz --strip-components=1 -C boost && cd boost && \
    ./bootstrap.sh && ./b2 -j$(nproc) install --without-python && \
    cd / && rm -rf /tmp/boost*

# faiss
RUN wget -q http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/faiss_v1.9.0.tar.gz -O /tmp/faiss.tar.gz && \
    cd /tmp && mkdir faiss && tar -xzf faiss.tar.gz --strip-components=1 -C faiss && cd faiss && \
    mkdir build && cd build && cmake -DFAISS_ENABLE_GPU=OFF -DFAISS_ENABLE_PYTHON=OFF -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release -DFAISS_OPT_LEVEL=generic .. && \
    make -j$(nproc) && make install && \
    cd / && rm -rf /tmp/faiss*

# nlohmann-json
RUN wget -q http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/json_v3.11.3.tar.gz -O /tmp/nlohmann.tar.gz && \
    cd /tmp && mkdir nlohmann && tar -xzf nlohmann.tar.gz --strip-components=1 -C nlohmann && cd nlohmann && \
    cp -rf single_include/nlohmann /usr/local/include/ && \
    cd / && rm -rf /tmp/nlohmann*

# tabulate
RUN wget -q https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/deps/tabulate-3a5830.tar.gz -O /tmp/tabulate.tar.gz && \
    cd /tmp && mkdir tabulate && tar -xzf tabulate.tar.gz --strip-components=1 -C tabulate && cd tabulate && \
    cp -rf include/tabulate /usr/local/include && \
    cd / && rm -rf /tmp/tabulate*

# date library
RUN wget -q http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/date_v3.0.2.tar.gz -O /tmp/date.tar.gz && \
    cd /tmp && mkdir date && tar -xzf date.tar.gz --strip-components=1 -C date && cd date && \
    mkdir build && cd build && \
    cmake -DUSE_SYSTEM_TZ_DB=ON -DBUILD_TZ_LIB=ON .. && make -j$(nproc) && make install && \
    cd / && rm -rf /tmp/date*

# Python test deps
RUN pip3 install --break-system-packages --no-cache-dir behave neo4j pyyaml

# Install cursor cli

RUN curl https://cursor.com/install -fsS | bash
RUN echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc

# Library path
RUN echo "/usr/local/lib" >> /etc/ld.so.conf && \
    echo "/usr/local/lib64" >> /etc/ld.so.conf && \
    ldconfig

WORKDIR /workspaces/tugraph-db
