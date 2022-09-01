#! /bin/bash

:<<!
需要提前向hive灌入一些测试数据
登陆hive客户端
执行 hive_data.sql 中 的sql语句，创建测试用的hive表、以及一些测试数据。
!

#运行datax，读取hive表数据，导入tugraph
#注意: 请修改job_hive_to_tugraph.json中hdfsreader的ip port等地址配置参数
python3 datax/bin/datax.py job_hive_to_tugraph.json