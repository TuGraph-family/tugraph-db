 # TuGraph DataX工具

Version: 3.3.0

2022/07/19

蚂蚁集团

---

## 1 简介

TuGraph在阿里开源的DataX基础上添加了TuGraph的写插件以及TuGraph jsonline数据格式的支持，其他数据源可以通过DataX往TuGraph里面写数据。

DataX介绍参考 https://github.com/alibaba/DataX

支持的功能包括：

* 从 MySQL、SQL Server、Oracle、PostgreSQL、HDFS、Hive、HBase、OTS、ODPS、Kafka 等各种异构数据源导入 TuGraph
* 将 TuGraph 导入相应的目标源 （待开发）

## 2 编译安装

```bash
git clone git@code.alipay.com:fma/DataX.git
mvn -U clean package assembly:assembly -Dmaven.test.skip=true
```
编译出来的DataX文件在 target 目录下

## 3 文本数据通过DataX导入TuGraph

我们以TuGraph手册中导入工具lgraph_import章节举的数据为例子，有三个csv数据文件，如下：

`actors.csv`

```CSV
nm015950,Stephen Chow
nm0628806,Man-Tat Ng
nm0156444,Cecilia Cheung
nm2514879,Yuqi Zhang
```

`movies.csv`

```CSV
tt0188766,King of Comedy,1999,7.3
tt0286112,Shaolin Soccer,2001,7.3
tt4701660,The Mermaid,2016,6.3
```

`roles.csv`

```CSV
nm015950,Tianchou Yin,tt0188766
nm015950,Steel Leg,tt0286112
nm0628806,,tt0188766
nm0628806,coach,tt0286112
nm0156444,PiaoPiao Liu,tt0188766
nm2514879,Ruolan Li,tt4701660
```

然后建三个DataX的job配置文件：

`job_actors.json`

```json
{
    "job": {
        "setting": {
            "speed": {
                "channel":1
            }
        },
        "content": [
            {
                "reader": {
                    "name": "txtfilereader",
                    "parameter": {
                        "path": ["actors.csv"],
                        "encoding": "UTF-8",
                        "column" : [
                            {
                                "index": 0,
                                "type": "string"
                            },
                            {
                                "index": 1,
                                "type": "string"
                            }
                        ],
                        "fieldDelimiter": ","
                    }
                },
                "writer": {
                    "name": "tugraphwriter",
                    "parameter": {
                        "host": "127.0.0.1",
                        "port": 7071,
                        "username": "admin",
                        "password": "73@TuGraph",
                        "graphName": "default",
                        "schema" : [
                            {
                                "label" : "actor",
                                "type" : "VERTEX",
                                "properties" : [
                                    { "name" : "aid", "type":"STRING"},
                                    { "name" : "name", "type":"STRING"}
                                ],
                                "primary" : "aid"
                            }
                      ],
                      "files" : [
                          {
                              "label" : "actor",
                              "format" : "JSON",
                              "columns" : ["aid","name"]
                          }
                      ]
                    }
                }
            }
        ]
    }
}
```

`job_movies.json`

```json
{
    "job": {
        "setting": {
            "speed": {
                "channel":1
            }
        },
        "content": [
            {
                "reader": {
                    "name": "txtfilereader",
                    "parameter": {
                        "path": ["movies.csv"],
                        "encoding": "UTF-8",
                        "column" : [
                            {
                              "index": 0,
                              "type": "string"
                            },
                            {
                              "index": 1,
                              "type": "string"
                            },
                            {
                              "index": 2,
                              "type": "string"
                            },
                            {
                              "index": 3,
                              "type": "string"
                            }
                        ],
                        "fieldDelimiter": ","
                    }
                },
                "writer": {
					"name": "tugraphwriter",
					"parameter": {
						"host":"127.0.0.1",
						"port": 7071,
						"username": "admin",
						"password": "73@TuGraph",
						"graphName": "default",
                        "schema" : [
                            {
                                "label" : "movie",
                                "type" : "VERTEX",
                                "properties" : [
                                    {"name" : "mid", "type":"STRING"},
                                    {"name" : "name", "type":"STRING"},
                                    {"name" : "year", "type":"STRING"},
                                    {"name":"rate", "type":"FLOAT", "optional":true}
                                ],
                                "primary" : "mid"
                            }
                        ],
                        "files" : [
                            {
                                "label" : "movie",
                                "format" : "JSON",
                                "columns" : ["mid","name","year","rate"]
                            }
                        ]
					}
                }
            }
        ]
    }
}

```

`job_roles.json`

```json
{
    "job": {
        "setting": {
            "speed": {
                "channel":1
            }
        },
        "content": [
            {
                "reader": {
                    "name": "txtfilereader",
                    "parameter": {
                        "path": ["roles.csv"],
                        "encoding": "UTF-8",
                        "column" : [
                            {
                                "index": 0,
                                "type": "string"
                            },
                            {
                                "index": 1,
                                "type": "string"
                            },
                            {
                                "index": 2,
                                "type": "string"
                            }
                        ],
                        "fieldDelimiter": ","
                    }
                },
                "writer": {
					"name": "tugraphwriter",
					"parameter": {
                        "host":"127.0.0.1",
                        "port": 7071,
                        "username": "admin",
                        "password": "73@TuGraph",
                        "graphName": "default",
                        "schema" : [
                            {
                                "label" : "play_in",
                                "type" : "EDGE",
                                "properties" : [
                                    {"name" : "role", "type":"STRING"}
                                ]
                            }
                        ],
                        "files" : [
                            {
                                "label" : "play_in",
                                "format" : "JSON",
                                "SRC_ID" : "actor",
                                "DST_ID" : "movie",
                                "columns" : ["SRC_ID","role","DST_ID"]
                            }
                        ]
					}
                }
            }
        ]
    }
}
```

`./lgraph_server -c lgraph_standalone.json  -d 'run'` 启动TuGraph后依次执行如下三个命令：

```
python3 datax/bin/datax.py  job_actors.json
```
```
python3 datax/bin/datax.py  job_movies.json
```
```
python3 datax/bin/datax.py  job_roles.json
```

## 4 MySQL数据通过DataX导入TuGraph

我们在 `test` database下建立如下电影 `movies` 表

```sql
CREATE TABLE `movies` (
  `mid`  varchar(200) NOT NULL,
  `name` varchar(100) NOT NULL,
  `year` int(11) NOT NULL,
  `rate` float(5,2) unsigned NOT NULL,
  PRIMARY KEY (`mid`)
);
```

往表中插入几条数据

```sql
insert into
test.movies (mid, name, year, rate)
values 
('tt0188766', 'King of Comedy', 1999, 7.3),
('tt0286112', 'Shaolin Soccer', 2001, 7.3),
('tt4701660', 'The Mermaid',   2016,  6.3);
```

建立一个DataX的job配置文件

`job_mysql_to_tugraph.json`

**配置字段方式**
```json
{
    "job": {
        "setting": {
            "speed": {
                "channel":1
            }
        },
        "content": [
            {
                "reader": {
                    "name": "mysqlreader",
                    "parameter": {
                        "username": "root",
                        "password": "root",
                        "column": [
                            "mid",
                            "name",
                            "year",
                            "rate"
                        ],
                        "splitPk": "mid",
                        "connection": [
                          {
                            "table": [
                              "movies"
                            ],
                            "jdbcUrl": [
                              "jdbc:mysql://127.0.0.1:3306/test?useSSL=false"
                            ]
                          }
                        ]
                    }
                },
                "writer": {
                    "name": "tugraphwriter",
                    "parameter": {
                        "host":"127.0.0.1",
                        "port": 7071,
                        "username": "admin",
                        "password": "73@TuGraph",
                        "graphName": "default",
                        "schema" : [
                            {
                                "label" : "movie",
                                "type" : "VERTEX",
                                "properties" : [
                                    {"name" : "mid", "type":"STRING"},
                                    {"name" : "name", "type":"STRING"},
                                    {"name" : "year", "type":"STRING"},
                                    {"name":"rate", "type":"FLOAT", "optional":true}
                                ],
                                "primary" : "mid"
                            }
                        ],
                        "files" : [
                            {
                                "label" : "movie",
                                "format" : "JSON",
                                "columns" : [
                                  "mid",
                                  "name",
                                  "year",
                                  "rate"
                                ]
                            }
                        ]
                    }
                }
            }
        ]
    }
}
```
**写简单sql方式**

```json
{
    "job": {
        "setting": {
            "speed": {
                "channel":1
            }
        },
        "content": [
            {
                "reader": {
                    "name": "mysqlreader",
                    "parameter": {
                        "username": "root",
                        "password": "root",
                        "connection": [
                          {
                            "querySql": [
                              "select mid, name, year, rate from test.movies where year > 2000;"
                            ],
                            "jdbcUrl": [
                              "jdbc:mysql://127.0.0.1:3306/test?useSSL=false"
                            ]
                          }
                        ]
                    }
                },
                "writer": {
                    "name": "tugraphwriter",
                    "parameter": {
                        "host":"127.0.0.1",
                        "port": 7071,
                        "username": "admin",
                        "password": "73@TuGraph",
                        "graphName": "default",
                        "schema" : [
                            {
                                "label" : "movie",
                                "type" : "VERTEX",
                                "properties" : [
                                    {"name" : "mid", "type":"STRING"},
                                    {"name" : "name", "type":"STRING"},
                                    {"name" : "year", "type":"STRING"},
                                    {"name" : "rate", "type":"FLOAT", "optional":true}
                                ],
                                "primary" : "mid"
                            }
                        ],
                        "files" : [
                            {
                                "label" : "movie",
                                "format" : "JSON",
                                "columns" : [
                                    "mid",
                                    "name",
                                    "year",
                                    "rate"
                                ]
                            }
                        ]
                    }
                }
            }
        ]
    }
}

```

`./lgraph_server -c lgraph_standalone.json  -d 'run'` 启动TuGraph后执行如下命令：

```
python3 datax/bin/datax.py  job_mysql_to_tugraph.json
```

## 5 Odps数据通过DataX导入TuGraph

Odps，又名MaxCompute，为阿里云计算产品之一，其使用上类似Mysql。在将Odps数据导入TuGraph时，可以参考上述Mysql导入的过程。

在导入前，需获取链接Odps的相关配置，包括: AK, endpoint等相关配置，其配置信息可以参考Odpscmd的配置文件。
