# 忘记'admin'密码
> TuGraph 提供了重置密码的功能，当用户忘记管理者账号`admin`密码时，可以通过重置密码的方式来修改密码。

## 1.重置密码
首先，需要停止TuGraph服务端。如果是容器内部署，需要进入容器中执行如下命令：

```bash
lgraph_server -c /usr/local/etc/lgraph.json -d stop
```

再次启动TuGraph服务端时，需要添加如下参数：

```bash
--reset_admin_password 1
```

如下所示：
```bash
lgraph_server -c /usr/local/etc/lgraph.json --reset_admin_password 1 --log_dir ""
```

这一操作可以使得TuGraph服务端在启动时，重置管理者`admin`的密码为默认密码：`73@TuGraph`。
密码重置成功会给出相关信息“Reset admin password successfully”并关闭当前服务端进程。

## 2.重启服务

用户需要以正常模式重新启动服务端，然后使用默认账号密码进行登录，登录后重新设置密码即可正常使用。
重新启动TuGraph服务的命令如下：

```bash
lgraph_server -c /usr/local/etc/lgraph.json -d start
```
