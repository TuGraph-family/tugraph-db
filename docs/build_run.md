# 编译和运行

## 编译

tugraph-db有制作好的编译镜像，专门用来源码编译，拉取编译镜像
```
docker pull tugraph/5.x-compile-alinux3
```
进到容器里面，拉取源码编译
```
git clone https://github.com/TuGraph-family/tugraph-db.git
cd tugraph-db && git checkout v5.x
mkdir build && cd build && cmake .. && make -j10

在build目录下会生成 lgraph_server 和 lgraph_cli 两个可执行文件 
```

## 运行
```
默认参数启动
./lgraph_server --mode start

启动的时候指定pid文件路径、监听端口、数据路径、日志路径等参数
./lgraph_server --mode=start --pid_file=./lgraph.pid --bolt_port=7687 --data_path=./data  --log_path=./log 
```

## 停止
```
./lgraph_server --mode stop

./lgraph_server --pid_file=./lgraph.pid --mode=stop
```
