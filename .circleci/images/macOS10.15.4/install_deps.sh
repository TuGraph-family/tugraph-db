# install libs
brew install libomp cmake automake protobuf@3.6 leveldb zlib gflags openssl@1.1 python@3.8 snappy node@12
export OPENSSL_ROOT_DIR="/usr/local/Cellar/openssl@1.1/1.1.1g"
export PROTOBUF_ROOT_DIR="/usr/local/Cellar/protobuf@3.6/3.6.1.3_2"
export PATH="$PATH:/usr/local/opt/node@12/bin:$PROTOBUF_ROOT_DIR/bin"
ln -s $OPENSSL_ROOT_DIR/include/openssl /usr/local/include
ln -s $PROTOBUF_ROOT_DIR/include/google/protobuf /usr/local/include/google
ln -s $OPENSSL_ROOT_DIR/lib/lib* /usr/local/lib
ln -s $PROTOBUF_ROOT_DIR/lib/lib* /usr/local/lib

# install boost@1.68
wget https://dl.bintray.com/boostorg/release/1.68.0/source/boost_1_68_0.tar.gz \
    && tar xzf boost_1_68_0.tar.gz && cd boost_1_68_0 \
    && ./bootstrap.sh --with-libraries=system,random,thread,filesystem,chrono,atomic,date_time,regex,stacktrace --with-toolset=clang \
    && ./b2 -j16 cxxflags="-std=c++14 -fPIC" install && rm -rf /boost_*

