#! /bin/bash

:<<!
启动一个空的TuGraph
web界面 帮助=>快速上手=>点击 '一键创建图模型' 和 '一键创建数据', 创建一些测试数据。
!

#注意: 请修改job_person_to_stream.json中ip port等地址配置参数

# 导出点person的数据
python3 datax/bin/datax.py job_person_to_stream.json

# 导出边acted_in的数据
python3 datax/bin/datax.py job_acted_in_to_stream.json