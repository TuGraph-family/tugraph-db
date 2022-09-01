#! /bin/bash

#运行datax，读取odps表数据，导入tugraph
#注意: job_odps_to_tugraph_vertex.json、job_odps_to_tugraph_edge.json中odpsreader的accessId, accessKey, endpoint等odps配置参数
python3 datax/bin/datax.py job_odps_to_tugraph_vertex.json
python3 datax/bin/datax.py job_odps_to_tugraph_edge.json