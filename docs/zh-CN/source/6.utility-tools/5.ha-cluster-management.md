# 集群管理

> 此文档主要介绍 TuGraph HA 集群的管理工具，主要包括删除节点、leader转移和生成snapshot功能

## 1. 简介

HA集群启动之后，可以使用`lgraph_peer`工具进行集群管理，可以执行删除节点，转移leader和生成snapshot等功能。

## 2. 删除节点

对于TuGraph HA集群中长期离线或者产生网络分区的节点，可以使用`lgraph_peer`的`remove_peer`命令删除节点。命令示例如下所示：
```shell
$ lgraph_peer --command remove_peer --peer {peer_id} --conf {group_conf}
```

其中：

- `--command remove_peer` 指定要执行的操作为remove_peer，即删除节点。
- `--peer {peer_id}` 指定要删除节点的rpc网络地址，如 `127.0.0.1:9092`。
- `--conf {group_conf}` 指定HA集群的成员配置（可连通主节点即可），如 `127.0.0.1:9092,127.0.0.1:9093,127.0.0.1:9094` 。

## 3. leader 转移

当需要对主节点执行停机或重启操作时，为减少集群的不可服务时间，可以使用`lgraph_peer`的`transfer_leader`命令转移主节点。命令示例如下所示：

```shell
$ lgraph_peer --command transfer_leader --peer {peer_id} --conf {group_conf}
```

其中：

- `--command transfer_leader` 指定要执行的操作为transfer_leader，即转移主节点。
- `--peer {peer_id}` 指定要成为主节点的rpc网络地址，如 `127.0.0.1:9092`。
- `--conf {group_conf}` 指定HA集群的成员配置（可连通主节点即可），如 `127.0.0.1:9092,127.0.0.1:9093,127.0.0.1:9094` 。

## 4. 生成snapshot

出于节点启动时设置ha_snapshot_interval_s为-1以默认不打snapshot或其他原因，
当需要让某个节点手动生成snapshot时，可以使用`lgraph_peer`的`snapshot`命令。命令示例如下所示：

```shell
$ lgraph_peer --command snapshot --peer {peer_id}
```

其中：

- `--command snapshot` 指定要执行的操作为snapshot，即生成快照。
- `--peer {peer_id}` 指定要生成快照的节点的rpc网络地址，如 `127.0.0.1:9092`。