FROM centos:centos7.9.2009

RUN cp -rf /etc/yum.repos.d /etc/yum.repos.d.bak && cd /etc/yum.repos.d && \
    sed -i '/mirrorlist=/d' CentOS-Base.repo CentOS-fasttrack.repo CentOS-x86_64-kernel.repo && \
    sed -i 's|#baseurl=http://mirror\.centos\.org|baseurl=https://mirrors.aliyun.com|g' CentOS-Base.repo CentOS-fasttrack.repo CentOS-x86_64-kernel.repo && \
    sed -i 's|http://mirror\.centos\.org|https://mirrors.aliyun.com|g' CentOS-CR.repo && \
    yum clean all && yum makecache && \
    yum -y install centos-release-scl && \
    sed -i '/mirrorlist=/d' CentOS-SCLo-scl-rh.repo CentOS-SCLo-scl.repo && \
    sed -i 's|#baseurl=http://mirror\.centos\.org|baseurl=https://mirrors.aliyun.com|g' CentOS-SCLo-scl-rh.repo && \
    sed -i 's|# baseurl=http://mirror\.centos\.org|baseurl=https://mirrors.aliyun.com|g' CentOS-SCLo-scl.repo && \
    yum clean all && yum makecache

RUN yum -y install devtoolset-10 devtoolset-10-libasan-devel devtoolset-10-libstdc++-devel vim python3-devel passwd gfortran openssh-server rsync openssl-static git procps iproute findutils tar wget which autoconf automake libtool openblas-devel

RUN echo "source /opt/rh/devtoolset-10/enable" >> /etc/bashrc
SHELL ["/bin/bash", "--login", "-c"]

RUN wget https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/deps/cmake-3.25.2.tar.gz -O /tmp/cmake.tar.gz && \
    cd /tmp && mkdir cmake && tar -xzf cmake.tar.gz --strip-components=1 -C cmake && cd cmake && \
    ./bootstrap && make -j10 && make install &&  \
    cd / && rm -rf /tmp/cmake*

RUN wget http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/gtest_v1.15.2.tar.gz -O /tmp/googletest.tar.gz && \
    cd /tmp && mkdir googletest && tar -xzf googletest.tar.gz --strip-components=1 -C googletest && cd googletest && \
    mkdir build && cd build && cmake .. && \
    make -j10 && make install && \
    cd / && rm -rf /tmp/googletest*

RUN wget http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/gflags_v2.2.2.tar.gz -O /tmp/gflags.tar.gz && \
    cd /tmp && mkdir gflags && tar -xzf gflags.tar.gz --strip-components=1 -C gflags && cd gflags && \
    mkdir build && cd build && cmake .. && \
    make -j10 && make install && \
    cd / && rm -rf /tmp/gflags*

RUN wget http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/jemalloc_5.3.0.tar.gz -O /tmp/jemalloc.tar.gz && \
    cd /tmp && mkdir jemalloc && tar -xzf jemalloc.tar.gz --strip-components=1 -C jemalloc && cd jemalloc && \
    ./autogen.sh && ./configure && \
    make -j10 && make install && \
    cd / && rm -rf /tmp/jemalloc*

RUN wget http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/lz4_v1.10.0.tar.gz -O /tmp/lz4.tar.gz && \
    cd /tmp && mkdir lz4 && tar -xzf lz4.tar.gz --strip-components=1 -C lz4 && cd lz4 && \
    make -j10 && make install && \
    cd / && rm -rf /tmp/lz4*

RUN wget http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/zstd_v1.5.6.tar.gz -O /tmp/zstd.tar.gz && \
    cd /tmp && mkdir zstd && tar -xzf zstd.tar.gz --strip-components=1 -C zstd && cd zstd && \
    make -j10 && make install && \
    cd / && rm -rf /tmp/zstd*

RUN wget http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/rocksdb_v9.5.2.tar.gz -O /tmp/rocksdb.tar.gz && \
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
    && make -j10 && make install && \
    cd / && rm -rf /tmp/rocksdb*

RUN wget http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/spdlog_v1.14.1.tar.gz -O /tmp/spdlog.tar.gz && \
    cd /tmp && mkdir spdlog && tar -xzf spdlog.tar.gz --strip-components=1 -C spdlog && cd spdlog && \
    mkdir build && cd build && cmake .. && \
    make -j10 && make install && \
    cd / && rm -rf /tmp/spdlog*

RUN wget http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/antlr4_4.13.2.tar.gz -O /tmp/antlr4.tar.gz && \
    cd /tmp && mkdir antlr4 && tar -xzf antlr4.tar.gz --strip-components=1 -C antlr4 && cd antlr4/runtime/Cpp && \
    mkdir build && cd build && cmake -DWITH_DEMO=0 -DANTLR_BUILD_CPP_TESTS=0 -DANTLR4_INSTALL=1 -DCMAKE_CXX_STANDARD=17 .. && \
    make -j10 && make install && \
    cd / && rm -rf /tmp/antlr4*

RUN wget http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/protobuf_v3.6.0.tar.gz -O /tmp/protobuf.tar.gz && \
    cd /tmp && mkdir protobuf && tar -xzf protobuf.tar.gz --strip-components=1 -C protobuf && cd protobuf && \
    ./autogen.sh && ./configure && \
    make -j10 && make install && \
    cd / && rm -rf /tmp/protobuf*

RUN wget http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/boost_1_86_0.tar.gz -O /tmp/boost.tar.gz && \
    cd /tmp && mkdir boost && tar -xzf boost.tar.gz --strip-components=1 -C boost && cd boost && \
    ./bootstrap.sh && \
    ./b2 -j10 install --without-python && \
    cd / && rm -rf /tmp/boost*

#RUN yum install -y yum-utils zlib-devel gfortran python3-devel libomp-devel libomp ca-certificates && \
#    yum-config-manager --add-repo https://yum.repos.intel.com/mkl/setup/intel-mkl.repo && \
#    rpm --import https://yum.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB && \
#    yum install -y intel-mkl-64bit-2020.0-088

RUN wget http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/vsag_v0.11.5.tar.gz -O /tmp/vsag.tar.gz && \
    cd /tmp && mkdir vsag && tar -xzf vsag.tar.gz --strip-components=1 -C vsag && cd vsag && \
    mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_INTEL_MKL=OFF -DDISABLE_AVX2_FORCE=ON -DDISABLE_AVX512_FORCE=ON .. && \
    make -j10 && make install && \
    cd / && rm -rf /tmp/vsag*

RUN wget http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/faiss_v1.9.0.tar.gz -O /tmp/faiss.tar.gz && \
    cd /tmp && mkdir faiss && tar -xzf faiss.tar.gz --strip-components=1 -C faiss && cd faiss && \
    mkdir build && cd build && cmake -DFAISS_ENABLE_GPU=OFF -DFAISS_ENABLE_PYTHON=OFF -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release -DFAISS_OPT_LEVEL=generic .. && \
    make -j10 && make install && \
    cd / && rm -rf /tmp/faiss*

RUN wget http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/json_v3.11.3.tar.gz -O /tmp/nlohmann.tar.gz && \
    cd /tmp && mkdir nlohmann && tar -xzf nlohmann.tar.gz --strip-components=1 -C nlohmann && cd nlohmann && \
    cp -rf single_include/nlohmann /usr/local/include/ && \
    cd / && rm -rf /tmp/nlohmann*

RUN wget https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/deps/tabulate-3a5830.tar.gz -O /tmp/tabulate.tar.gz && \
    cd /tmp && mkdir tabulate && tar -xzf tabulate.tar.gz --strip-components=1 -C tabulate && cd tabulate && \
    cp -rf include/tabulate /usr/local/include && \
    cd / && rm -rf /tmp/tabulate*

RUN wget http://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/5.x_deps/date_v3.0.2.tar.gz -O /tmp/date.tar.gz && \
    cd /tmp && mkdir date && tar -xzf date.tar.gz --strip-components=1 -C date && cd date && \
    mkdir build && cd build && \
    cmake -DUSE_SYSTEM_TZ_DB=ON -DBUILD_TZ_LIB=ON .. && make -j10 && make install && \
    cd / && rm -rf /tmp/date*

RUN export RUSTUP_DIST_SERVER=https://mirrors.ustc.edu.cn/rust-static && export RUSTUP_UPDATE_ROOT=https://mirrors.ustc.edu.cn/rust-static/rustup && curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
RUN pip3 install behave neo4j pyyaml
RUN ssh-keygen -A
RUN echo "/usr/local/lib64" >> /etc/ld.so.conf && ldconfig

RUN localedef -c -f UTF-8 -i en_US en_US.utf8
ENV LANG     en_US.utf8
ENV LC_ALL   en_US.utf8