# 备份恢复

> 此文档主要介绍 TuGraph 的数据备份和恢复功能。

## 1.数据备份

TuGraph 可以通过 `lgraph_backup` 工具来进行数据备份。
`lgraph_backup` 工具可以将一个 TuGraph 数据库中的数据备份到另一个目录下，它的用法如下：

```bash
$ lgraph_backup -s {source_dir} -d {destination_dir} -c {true/false}
```

其中：

- `-s {source_dir}` 指定需要备份的数据库（源数据库）所在目录。
- `-d {destination_dir}` 指定备份文件（目标数据库）所在目录。
  如果目标数据库不为空，`lgraph_backup` 会提示是否覆盖该数据库。
- `-c {true/false}` 指明是否在备份过程中进行 compaction。
  compaction 能使产生的备份文件更紧凑，但备份时间也会变长。该选项默认为 `true`。

## 2.数据恢复

使用`lgraph_backup` 工具得到的目标数据库`{destination_dir}`备份了源数据库
`{source_dir}`的所有子图，但不包含HA集群的raft信息，从而保证服务和集群能
以备份数据库成功重启并与源数据库的数据一致。使用如下命令可以用备份数据库重启服务，
在服务启动时会恢复所有子图的存储过程，保证备份服务和原服务完全一致。

```bash
$ lgraph_server -c lgraph.json --directory {destination_dir} -d start
```

其中：

- `-d {destination_dir}` 指定备份文件（目标数据库）所在目录。