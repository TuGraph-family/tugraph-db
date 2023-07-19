# Introduction
GPC是一个基于编译技术的算子加速器，主要针对图学习网络的训练和推理加速，目前主要应用在GPU上。
在图学习网络中，稀疏矩阵的计算占用了较大比重的计算时间。因此，通过加速稀疏矩阵的计算，可以有效加速图学习网络的训练和推理。
GPC通过扩展Halide编译器，将算法和调度分离，并引入了现代GPU的稀疏计算优化技术，如Shared Memory Preloading，Register Tiling和Row Balancing等技术。因此，GPC在稀疏计算上，对比cuSparse和其他sofa，最高能够加速4.75倍。
在DGL框架上测试，针对GAT/GCN/GraphSAGE等模型的训练和推理（Full Batch），对比原来DGL的性能，有1.3~2.2倍的加速。
另外，GPC对DGL等框架进行了无侵入式的加速，用户只需要在python端添加一行代码，就可以对图学习网络的训练和推理进行自动加速。

# Install
在tugraph docker环境下，运行
```bash
pip install  http://jinyue-open-read.oss-cn-hangzhou-zmf.aliyuncs.com/gpc%2Fgpc-1.0-cp310-cp310-linux_x86_64.whl
```
用户如果需要再GPU上运行，需要用户自行安装相应的GPU驱动和环境。


# Usage
用户只需要在编译安装GPC后，在训练和推理的代码中，加入一行代码即可:
```
gpc.gcompile(g)
```
其中g代表DGL中的图实例，可以参照下面这个加速gcn模型训练的例子：
```
    import torch
    import dgl
    import gpc
    ...

    # create GCN model
    model = GCN(g,
                in_feats,
                args.n_hidden,
                n_classes,
                args.n_layers,
                F.relu,
                args.dropout)


    gpc.gcompile(g)

    if cuda:
        model.cuda()
    loss_fcn = torch.nn.CrossEntropyLoss()

    # use optimizer
    optimizer = torch.optim.Adam(model.parameters(),
                                 lr=args.lr,
                                 weight_decay=args.weight_decay)

    # initialize graph
    dur = []

    model.train()

    # train gcn model
```
更多例子可以参考learn/benchmarks目录下的gcn/gat/graphsage。


# Benchmarks
在learn/benchmarks目录下运行：
```bash
python kernels.py
```


# Build
dependency:
bazel 2.1.0
cmake 3.22.1
gcc 8.4.0

## LLVM 12.0
build llvm with flag: -D_GLIBCXX_USE_CXX11_ABI=0, because torch does.
```bash
wget -c https://github.com/llvm/llvm-project/releases/download/llvmorg-12.0.0/llvm-project-12.0.0.src.tar.xz -O llvm-project.tar.xz
tar xf llvm-project.tar.xz

cmake -DCMAKE_BUILD_TYPE=Release \
        -DLLVM_ENABLE_PROJECTS="clang;lld;clang-tools-extra" \
        -DLLVM_TARGETS_TO_BUILD="X86;ARM;NVPTX;AArch64;Mips;Hexagon;WebAssembly" \
        -DLLVM_ENABLE_TERMINFO=OFF -DLLVM_ENABLE_ASSERTIONS=ON \
        -DLLVM_ENABLE_EH=ON -DLLVM_ENABLE_RTTI=ON -DLLVM_BUILD_32_BITS=OFF \
        -DCMAKE_CXX_FLAGS_RELEASE="-O3 -DNDEBUG -D_GLIBCXX_USE_CXX11_ABI=0" \
        -S llvm-project/llvm -B llvm-build


cmake --build llvm-build -j32
cmake --install llvm-build --prefix llvm-install

export LLVM_ROOT=$PWD/llvm-install
export LLVM_CONFIG=$LLVM_ROOT/bin/llvm-config

```

## Halide 12.0
build Halide with flag: -D_GLIBCXX_USE_CXX11_ABI=0, because torch does.
```bash
mkdir third_party -p && cd third_party
git clone https://github.com/halide/Halide.git
cd Halide
git checkout c3ff4d2d1973843bb498f63eed02b36f96de2e4e
git checkout -- .
git apply ../tugraph-db/learn/compiler/patch/patchhalide.diff
sed -i 's/TUTORIAL_CXX_FLAGS ?=/TUTORIAL_CXX_FLAGS ?= -D_GLIBCXX_USE_CXX11_ABI=0/g' Makefile
sed -i 's/CXX_FLAGS =/CXX_FLAGS = -D_GLIBCXX_USE_CXX11_ABI=0/g' Makefile
make -j32 WITH_EXCEPTIONS=1 CXX="g++ -D_GLIBCXX_USE_CXX11_ABI=0"

```


## Bazel
```bash
wget https://github.com/bazelbuild/bazel/releases/download/6.2.0/bazel-6.2.0-installer-linux-x86_64.sh
chmod +x bazel-6.2.0-installer-linux-x86_64.sh
 ./bazel-6.2.0-installer-linux-x86_64.sh
```

## DGL
pip install  dgl==1.0.0+cu117 -f https://data.dgl.ai/wheels/cu117/repo.html

## GPC
```bash
bazel build --cxxopt=-D_GLIBCXX_USE_CXX11_ABI=0  compiler/gpc/ops:libplugin.so
python setup.py install
```


# Benchmark
Run:
```bash
cd benchmarks
python dgl-new.py
```



# Docker
See the dockerfile under docker directory.
