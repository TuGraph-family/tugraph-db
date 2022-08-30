#! /bin/bash

:<<!
需要提前向mysql灌入一些测试数据
假设mysql的登陆账户是root，密码是root
登陆mysql客户端: mysql -u root -p
执行 mysql_data.sql 中 的sql语句，创建测试用的库、表、以及一些测试数据。
!

#运行datax，读取mysql表数据，导入tugraph
#注意: 请修改job_mysql_to_tugraph.json中mysqlreader的ip port等地址配置参数
python3 datax/bin/datax.py job_mysql_to_tugraph.json