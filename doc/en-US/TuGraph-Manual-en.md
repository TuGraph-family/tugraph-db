# TuGraph Users' Manual

Version: 3.3.0

2022/07/19

Ant Group

---

## Table Of Contents

- [Introduction](#introduction)
  - [Features](#features)
  - [Data Model](#data-model)
    - [Graph Model](#graph-model)
    - [Data Types](#data-types)
    - [Index](#index)
  - [Licensing](#licensing)
- [Install](#install)
  - [Prerequisites](#prerequisites)
  - [Installing With Docker Images](#installing-with-docker-images)
  - [Installing On Ubuntu](#installing-on-ubuntu)
  - [Installing on CentOS](#installing-on-centos)
- [Data Import](#data-import)
  - [Offline Import](#offline-import)
    - [Import Config File](#import-config-file)
      - [TargetDescription](#targetdescription)
      - [ColumnMapping](#columnmapping)
    - [Offline Import Example](#offline-import-example)
  - [Online Import](#online-import)
- [Configuring TuGraph Server](#configuring-tugraph-server)
  - [TuGraph Config Options](#tugraph-config-options)
  - [TuGraph Config File](#tugraph-config-file)
  - [Configuring With Command Line](#configuring-with-command-line)
- [Starting TuGraph Server](#starting-tugraph-server)
  - [TuGraph Running Mode](#tugraph-running-mode)
  - [Running TuGraph In Foreground](#running-tugraph-in-foreground)
  - [Starting TuGraph In Daemon Mode](#starting-tugraph-in-daemon-mode)
  - [Stopping TuGraph Daemon](#stopping-tugraph-daemon)
  - [Restarting TuGraph Daemon](#restarting-tugraph-daemon)
- [Accessing A TuGraph Server](#accessing-a-tugraph-server)
- [`lgraph_cypher` Query Client](#lgraph_cypher-query-client)
  - [Single-Command Mode](#single-command-mode)
    - [`lgraph_cypher` Options:](#lgraph_cypher-options)
    - [Command Examples](#command-examples)
  - [Interactive Mode](#interactive-mode)
    - [Entering Interactive Mode](#entering-interactive-mode)
    - [`lgraph_cypher` Commands](#lgraph_cypher-commands)
    - [Evaluating Cypher Queries:](#evaluating-cypher-queries)
    - [Auxiliary Functions](#auxiliary-functions)
- [TuGraph Visualizer](#tugraph-visualizer)
- [Enabling TuGraph High Availability](#enabling-tugraph-high-availability)
  - [Principles And Terminologies](#principles-and-terminologies)
  - [Enabling HA](#enabling-ha)
  - [Starting The First Server](#starting-the-first-server)
  - [Adding Servers To A Replication Group](#adding-servers-to-a-replication-group)
  - [Stopping Servers In Replication Group](#stopping-servers-in-replication-group)
  - [Restarting The Whole Replication Group](#restarting-the-whole-replication-group)
  - [Checking Replication Group Status](#checking-replication-group-status)
  - [Query Semantics In HA Mode](#query-semantics-in-ha-mode)
- [Managing TuGraph Servers](#managing-tugraph-servers)
  - [Logging](#logging)
    - [Server Log](#server-log)
    - [Audit Logging](#audit-logging)
  - [Exporting Data](#exporting-data)
  - [Database Backup](#database-backup)
  - [Database Warmup](#database-warmup)
  - [Task Management](#task-management)
- [FAQ](#faq)

---

# Introduction

Graph databases are a type of non-relational database that stores data in terms of vertexes and edges. It can be used to store complex data models such as social networks and transaction networks.

TuGraph is a graph database developed by Ant Group. This manual introduces the features of TuGraph and how to use it.

## Features

TuGraph is an efficient graph database that supports high data volume, low latency lookup and fast graph analysis. It is a disk-based database, supporting up to tens of terabytes of data. With its versatile APIs, TuGraph enables users to build applications easily while keeping the potential to optimize their application.

Functionalities:

- Labeled property graph model
- Multi-graph support
- Full ACID support with serializable transactions
- 25+ graph analysis algorithms embedded
- Graph visualization with web client
- REST API and RPC support
- OpenCypher query language
- Stored procedure with C++/Python/Java
- Efficient development of new graph algorithms with Traversal API

Performance and scalability:

- Supports up to tens of terabytes
- Visit millions of vertices per second
- High availability support
- Fast bulk import
- Online/offline backup

## Data Model

### Graph Model

TuGraph is a multi-graph, strong-schema property graph database. It supports directed graph with at most one trillion vertexes.

- Multi-graph: In TuGraph, each database server can host multiple graphs. Each graph can have its own access control configurations. Database administrators can create or delete the graphs.
- Property graph: Vertexes and edges in TuGraph can have properties associated with them. Each property can have different types.
- Strong-schema: Each vertex and edge must have a label. The number of properties and the type of each property can hardly be modified once the label has been created.
- Directed edge: Edges in TuGraph are directed. To simulate an undirected edge, the user can create two edges with opposite directions.

### Data Types

TuGraph supports multiple data types which can be used as properties. The data types are listed as follows:

<caption>Table 1. Supported data types in TuGraph</caption>

| Type     | MinValue            | MaxValue            | Description                                   |
| -------- | ------------------- | ------------------- | --------------------------------------------- |
| BOOL     | false               | true                | Boolean value                                 |
| INT8     | -128                | 127                 | 8-bit integer                                 |
| INT16    | -32768              | 32767               | 16-bit integer                                |
| INT32    | - 2^31              | 2^31 - 1            | 32-bit integer                                |
| INT64    | - 2^63              | 2^63 - 1            | 64-bit integer                                |
| DATE     | 0000-00-00          | 9999-12-31          | Date in the form of "YYYY-MM-DD"              |
| DATETIME | 0000-00-00 00:00:00 | 9999-12-31 23:59:59 | DateTime in the form of "YYYY-MM-DD hh:mm:ss" |
| FLOAT    |                     |                     | 32-bit floating point number                  |
| DOUBLE   |                     |                     | 64-bit floating point number                  |
| STRING   |                     |                     | Variable-length string                        |
| BLOB     |                     |                     | Binary data                                   |

_Binary data is input and output with BASE64 encoding in TuGraph._

### Index

TuGraph supports indexing for vertex fields.

An index can be a unique index or non-unique index. If a unique index is created for a vertex label, TuGraph will perform data integrity checking when modifications are made to vertexes of that label to guarantee the uniqueness of that index.

Each index is built on one field of one label. Multiple fields can be indexed with the same label.

Fields of `BLOB` type cannot be indexed.

## Licensing

TuGraph is proprietary software. The licensing fee is charged according to data scale and configurations. Please contact tugraph@service.alipay.com if you need a quota or more information.

# Install

## Prerequisites

Table 2 shows the system requirements for TuGraph. Though not mandatory, we recommend using NVMe SSDs in order to get maximum performance.

<caption>Table 2. System requirements for TuGraph</caption>

|      | Minimum   | Recommended              |
| ---- | --------- | ------------------------ |
| CPU  | X86_64    | Xeon E5 2670 v4          |
| DRAM | 4GB       | 256GB                    |
| Disk | 100GB     | 1TB NVMe SSD             |
| OS   | Linux 2.6 | Ubuntu 16.04, CentOS 7.3 |

## Installing With Docker Images

We recommend installing and running TuGraph with a docker image, which greatly simplifies the installation and system migration.

Before using a docker image, the user should make sure docker is properly installed. One can check the existence of docker with the following command line:

```bash
$ sudo docker --version
```

If docker has been properly installed, this command should print the version number of docker. Otherwise, docker is not installed or is not functioning properly and should be installed. A good guild for installing docker can be found at https://docs.docker.com/install/.

Currently, TuGraph is offered in Ubuntu 16.04 LTS and CentOS 7.3 docker images. The image files can be obtained by contacting tugraph@service.alipay.com.

Assuming you have downloaded the docker image named `tugraph_x.y.z.tar`, in which `x.y.z` is the version of TuGraph, you can then load the docker image into your docker repo with the following command:

```bash
$ sudo docker import tugraph_x.y.z.tar
```

If the loading is successful, you should have a docker image named tugraph_x.y.z on your machine, which you can run with:

```bash
$ sudo docker run -v /data_dir_on_host:/data_dir_in_docker -it tugraph_x.y.z /bin/bash
```

## Installing On Ubuntu

In addition to docker images, we also provide TuGraph in .deb packages, which you can use to install TuGraph on Ubuntu.

With an installation package named `tugraph_x.y.z.deb`, you can use the following command to install TuGraph:

```bash
$ sudo dpkg -i tugraph_x.y.z.deb
```

The above command will install TuGraph under `/user/local`. You can change the location by specifying the `--instdir=<directory>` option.

## Installing on CentOS

TuGraph is provided in .rpm packages as well, which can be used while installing on CentOS, with the following command:

```bash
$ rpm -ivh tugraph-x.y.z.rpm
```

The command will install TuGraph under `/usr/local` by default. If you want to change the location, use the `--prefix=<directory>` option.

# Data Import

After installation, you can import existing data into TuGraph with the `lgraph_import` bulk import tool.

`lgraph_import` supports importing data from CSV files and JSON data sources. It has two modes:

- _offline mode_ reads the data and imports it into a server data file, it should only be done when the server is offline.
- _online mode_ reads the data and sends it to an online server who will then write the data into its DB.

## Offline Import

The offline mode can only be used on an offline server. It will always create a new graph. So it is more suitable for the first data import on a newly installed TuGraph server.

To use `lgraph_import` in offline mode, you can specify the `lgraph_import --online false` option. To learn about the available command line options, use `lgraph_import --online false --help`:

```bash
$ ./lgraph_import --online false --help
Available command line options:
    --log               Log file to use, empty means stderr. Default="".
    -v, --verbose       Verbose level to use, higher means more verbose.
                        Default=1.
    ...
    -h, --help          Print this help message. Default=0.
```

Available options:

- **-c, --config_file** `config_file`: config file for this import, as explained in the next section.
- **--log** `log_dir`: log file directory. Default value is empty string, which indicates logging to console.
- **--verbose** `0/1/2`: log verbose level. Higher level gives more output. Default value is 1.
- **-i, --continue_on_error** `true/false`: when set to true, data consistency errors will be ignored so import can continue. Default value is `false`.
- **-d, --dir** `{directory}`: directory to store database file, defaults to `./lgraph_db`.
- **--delimiter** `{delimiter}`: delimiter used in the data files, only used in CSV format, defaults to `","`.
- **-u, --username** `{user}`: DB username. Need to be an admin user to perform offline import.
- **-p, --password** `{password}`: Password of the given DB user.
- **--overwrite** `true/false`: when set to true, overwrite the whole graph if it exists. Default value is `false`.
- **-g, --graph** `{graph_name}`: specify which graph to import into.
- **-h, --help**: print the help message.

The delimiter can be a single-character or multi-character string, and must not contain `\r` or `\n`.

To allow using special characters as delimiters, `lgraph_import` supports the following escapse sequences in the delimiter string:

| escape sequence | description                                                                 |
| --------------- | --------------------------------------------------------------------------- |
| \\              | backslash `\`                                                               |
| \a              | audible bell, byte 0x07 in ASCII encoding                                   |
| \f              | form-feed, byte 0x0c in ASCII encoding                                      |
| \t              | horizontal tab, byte 0x09 in ASCII encoding                                 |
| \v              | vertical tab, byte 0x0b in ASCII encoding                                   |
| \xnn            | two hex digits as a byte, such as \x9A                                      |
| \nnn            | three octal digits as a byte, such as \001, \443, value must not exceed 255 |

e.g.

```bash
$ ./lgraph_import -c import.config --delimiter "\x6B\002\\"
```

### Import Config File

`lgraph_import` is configured with a config file. A config file describes the paths of the input files, the vertex/edge they stand for, and the format of the vertex/edge.

### Config File Description

The config file consists of two major parts `schema` and `files`. The `schema` part defines the labels, and the `files` part describes the data files to be imported.

#### All Keywords

- schema
  - label
  - type（VERTEX or EDGE）
  - properties
    - name
    - type （BOOL，INT8，INT16，INT32，INT64，DATE，DATETIME，FLOAT，DOUBLE，STRING，BLOB）
    - optional
    - index (optional)
    - unique (optional, unique index)
  - primary (only vertex has, primary index)
  - constraints (only edge has)
- files
  - path
  - header (optional, default value is 0)
  - format (CSV or JSON)
  - label
  - columns
    - SRC_ID
    - DST_ID
    - SKIP
    - [property]
  - SRC_ID (only edge has, source label of the edge)
  - DST_ID (only edge has, destination label of the edge)

### Config File Example

```JSON
{
    "schema": [
        {
            "label" : "actor",
            "type" : "VERTEX",
            "properties" : [
                { "name" : "aid", "type":"STRING"},
                { "name" : "name", "type":"STRING"}
            ],
           "primary" : "aid"
        },
        {
            "label" : "movie",
            "type" : "VERTEX",
            "properties" : [
                {"name" : "mid", "type":"STRING"},
                {"name" : "name", "type":"STRING"},
                {"name" : "year", "type":"INT16"},
                {"name" : "rate", "type":"FLOAT", "optional":true}
            ],
           "primary" : "mid"
        },
        {
            "label" : "play_in",
            "type" : "EDGE",
            "properties" : [
                {"name" : "role", "type":"STRING"}
            ],
            "constraints" : [["actor", "movie"]]
        }
    ],
    "files" : [
        {
            "path" : "actors.csv",
            "header" : 2,
            "format" : "CSV",
            "label" : "actor",
            "columns" : ["aid","name"]
        },
        {
            "path" : "movies.csv",
            "header" : 2,
            "format" : "CSV",
            "label" : "movie",
            "columns" : ["mid","name","year","rate"]
        },
        {
            "path" : "roles.csv",
            "header" : 2,
            "format" : "CSV",
            "label" : "play_in",
            "SRC_ID" : "actor",
            "DST_ID" : "movie",
            "columns" : ["SRC_ID","role","DST_ID"]
        }
    ]
}
```

For the above config file, three labels are defined: two vertex types `actor` and `movie`, and one edge type `role`.
Each label describes these: the name of the label, the type(vertex or edge), what are the properties, and the type of each property field.
For vertex, it also defines `primary` property.
For edge, it also defines `constraints`, which is used to limit the combination of the starting and ending of the edge.

It also describes three data files, two vertex data files `actors.csv` and `movies.csv`, and one edge data file `roles.csv`.

The import tool will first create the three labels `actor`, `movie`, and `role` in TuGraph, and then do the data importing of the three files.

### Offline Import Example

In this example, we will import three files using offline import: `movies.csv`, `actors.csv`, and `roles.csv`.

`movies.csv` contains information on three movies, each uniquely identified with an id. The movies also have other properties such as name, title, production year and ratings.

```CSV
  [movies.csv]
  id, name, year, rating
  tt0188766,King of Comedy,1999,7.3
  tt0286112,Shaolin Soccer,2001,7.3
  tt4701660,The Mermaid,2016,6.3
```

`actors.csv` contains information of the actors, which also have unique id and names.

```CSV
  [actors.csv]
  id, name
  nm015950,Stephen Chow
  nm0628806,Man-Tat Ng
  nm0156444,Cecilia Cheung
  nm2514879,Yuqi Zhang
```

In `roles.csv`, each line specifies an edge from an actor to a movie indicating what role the actor has played in that movie.

```CSV
  [roles.csv]
  actor, role, movie
  nm015950,Tianchou Yin,tt0188766
  nm015950,Steel Leg,tt0286112
  nm0628806,test,tt0188766
  nm0628806,coach,tt0286112
  nm0156444,PiaoPiao Liu,tt0188766
  nm2514879,Ruolan Li,tt4701660
```

We use the config file in the above example

Note that we have two header lines in each file, so we need to specify the `HEADER=2` option. With the import config file, we can now import the data with the following command:

```bash
$ ./lgraph_import
        -c import.conf             # read config from import.conf
        --dir /data/lgraph_db      # save data in /data/lgraph_db
        --graph mygraph            # import into the graph named mygraph
```

**NOTE**：

- If the graph named `mygraph` already exists, the import tool will print an error message and quit. To forcibly overwrite the graph, you can use the `--overwrite true` option.
- The config file and data files must be stored with UTF-8 encoding (or plain ASCII, which is a subset of UTF-8). The import will fail with parser errors if any file is encoded with encodings other than UTF-8 (for example, UTF-8 with BOM or GBK).

## Online Import

The online import mode can be used to import a batch of files into an already-running TuGraph instance. This can come in handy for incremental batch update, which usually occurs at fixed time intervals.

The `lgraph_import --online true` option enables the import tool to work in online mode. Like `offline mode`, the online mode has its own set of command line options, which can be printed with `-h, --help` option:

```bash
$ lgraph_import --online true -h
Available command line options:
    --online            Whether to import online.
    -h, --help          Print this help message. Default=0.

Available command line options:
    --log               Log file to use, empty means stderr. Default="".
    -v, --verbose       Verbose level to use, higher means more verbose.
                        Default=1.
    -c, --config_file   Config file path.
    -r, --url           DB REST API address.
    -u, --username      DB username.
    -p, --password      DB password.
    -i, --continue_on_error
                        When we hit a duplicate uid or missing uid, should we
                        continue or abort. Default=0.
    -g, --graph         The name of the graph to import into. Default=default.
    --skip_packages     How many packages should we skip. Default=0.
    --delimiter         Delimiter used in the CSV files
    --breakpoint_continue
                        When the transmission process is interrupted,whether
                        to re-transmit from zero package next time. Default=false
    -h, --help          Print this help message. Default=0.
```

The configuration of files is specified in the config file, which has exactly the same format as in `offline mode`. However, instead of importing to a local database file, we are now sending the data to a running TuGraph instance, which is typically running on different from the client machine where the import tool runs. Hence we need to specify the URL of the remote machine's HTTP address, the DB user and password.

If the user and password are valid, and the graph exists, the import tool will send the data to the server, who then parses and writes the data into the specified graph. The data will be sent in packages of approximately 16MB in size, breaking at the nearest line break. Each package is imported atomically, meaning that if the package is successfully imported, all the data is successfully imported, otherwise, none of the data gets into the database. If `--continue_on_error true` is specified, data integrity errors are ignored and the violating lines are ignored. Otherwise, import will stop at the first erroneous package and print out the number of packages that have been imported. In that case, the user can modify the data to get rid of the error and then redo the import with `--skip_packages N` to skip the already-imported packages.

# Client SDK

## Python 语言

#### client Client Login

    client(url,                     string type，Login address
           user,                    string type，The username.
           password)                string type，The password.

#### callCypher Execute a cypher query，result is a tuple such as (bool, string) firs is the request success or failure flag，when success， second is the result from server， otherwise is reason for failure

    callCypher(cypher,              string type，inquire statement.
               graph,               string type，the graph to query.
               json_format,         bool type，Returns format， true is json，Otherwise, binary format
               timeout)             double type，Maximum execution time, overruns will be interrupted

#### loadPlugin Load a built-in plugin，result is a tuple such as (bool, string) firs is the request success or failure flag，when success， second is the result from server， otherwise is reason for failure

    loadPlugin(source_file,             string type，the source_file contain plugin code
                plugin_type,            string type，the plugin type, currently supported CPP and PY
                plugin_name,            string type，plugin name
                code_type,              string type，code type, currently supported PY, SO, CPP, ZIP
                plugin_description,     string type，plugin description
                read_only,              bool type，plugin is read only or not
                graph,                  string type，the graph to query.
                json_format,            bool type，Returns format， true is json，Otherwise, binary format
                timeout)                double type，Maximum execution time, overruns will be interrupted

#### callPlugin Execute a built-in plugin，result is a tuple such as (bool, string) firs is the request success or failure flag，when success， second is the result from server， otherwise is reason for failure

    callPlugin(plugin_type,         string type，the plugin type, currently supported CPP and PY
                plugin_name,        string type，plugin name
                param,              string type，the execution parameters
                plugin_time_out,    double type，maximum plugin execution time, overruns will be interrupted
                in_process,         support in future
                graph,              string type，the graph to query.
                json_format,        bool type，Returns format， true is json，Otherwise, binary format
                timeout)            double type，Maximum execution time, overruns will be interrupted

#### importSchemaFromFile import vertex or edge schema from file，result is a tuple such as (bool, string) firs is the request success or failure flag，when success， second is the result from server， otherwise is reason for failure

    importSchemaFromFile(schema_file,         string type，the schema_file contain schema, the format is the same as lgraph_import
                graph,              string type，the graph to query.
                json_format,        bool type，Returns format， true is json，Otherwise, binary format
                timeout)            double type，Maximum execution time, overruns will be interrupted

#### importDataFromFile import vertex or edge data from file，result is a tuple such as (bool, string) firs is the request success or failure flag，when success， second is the result from server， otherwise is reason for failure

    importDataFromFile(conf_file,         string type，the plugin type, data file contain format description and data,the format is the same as lgraph_import
                delimiter,          string type，data separator
                continue_on_error,  string type，whether to continue when importing data fails
                thread_nums,        int type，maximum number of threads
                skip_packages,      int type，skip packages number
                graph,              string type，the graph to query.
                json_format,        bool type，Returns format， true is json，Otherwise, binary format
                timeout)            double type，Maximum execution time, overruns will be interrupted

#### importSchemaFromContent import vertex or edge schema from content string，result is a tuple such as (bool, string) firs is the request success or failure flag，when success， second is the result from server， otherwise is reason for failure

    importSchemaFromContent(schema,         string type，the schema to imported， the format is the same as lgraph_import
                graph,              string type，the graph to query.
                json_format,        bool type，Returns format， true is json，Otherwise, binary format
                timeout)            double type，Maximum execution time, overruns will be interrupted

#### importDataFromContent import vertex or edge data from content string，此时应该已经导入了对应的 schema，result is a tuple such as (bool, string) firs is the request success or failure flag，when success， second is the result from server， otherwise is reason for failure

    importDataFromContent(desc,        string type，the plugin type, data file contain format description and data, the format is the same as lgraph_import
                data,               string type，the data to be imported
                delimiter,          string type，data separator
                continue_on_error,  string type，whether to continue when importing data fails
                thread_nums,        int type，maximum number of threads
                skip_packages,      int type，skip packages number
                graph,              string type，the graph to query.
                json_format,        bool type，Returns format， true is json，Otherwise, binary format
                timeout)            double type，Maximum execution time, overruns will be interrupted

# Configuring TuGraph Server

## TuGraph Config Options

TuGraph server loads configurations from both config file and command line options at startup time. If an option is given different values in the config file and command line, the value in the command line will be used.

The available options are as follows:

| option                        | <nobr>value type</nobr> | description                                                                                                                                                |
|:------------------------------| ----------------------- |:-----------------------------------------------------------------------------------------------------------------------------------------------------------|
| license                       | string                  | Path of the license file, by default `/var/lib/lgraph/fma.lic`                                                                                             |
| directory                     | string                  | Directory where the DB data is stored. If the directory does not exist, it will be created. Default value is `/var/lib/lgraph/data`                        |
| durable                       | boolean                 | Whether to turn on durable mode. Turning off durable mode improves write performance, but may cause data loss in case of system failure. By default `true` |
| host                          | string                  | HTTP address the server will listen on, typically set to the IP address of the server. Default value is `0.0.0.0`                                          |
| port                          | integer                 | HTTP server port, by default `7070`                                                                                                                        |
| enable_rpc                    | boolean                 | Whether to enable RPC server. Default=0.                                                                                                                   |
| rpc_port                      | integer                 | Port to be used for RPC and HA mode, by default `9090`                                                                                                     |
| enable_ha                     | boolean                 | Whether to turn on HA. Default value is `false`                                                                                                            |
| ha_log_dir                    | string                  | Directory where the HA log is stored, required in HA mode. Default="".                                                                                     |
| master                        | string                  | Initial list of machines in the form of host1:port1,host2:port2. Default="".                                                                               |
| verbose                       | integer                 | Verbosity of the log. Possible values are `0`, `1`, and `2`. Higher values produces more log messages. Default value is `1`                                |
| log_dir                       | string                  | Directory under which log files are written, by default `/var/log/lgraph/`                                                                                 |
| ssl_auth                      | bool                    | Whether to turn on SSL for HTTP service. Default value is `false`                                                                                          |
| server_cert                   | string                  | Path to the certificate file that the HTTPS server will use if `ssl_auth` is set to true. By default `/usr/local/etc/lgraph/server-cert.pem`               |
| server_key                    | string                  | Path to the public key that HTTPS server will use if `ssl_auth` is set to true. Default value is `/usr/local/etc/lgraph/server-key.pem`                    |
| enable_audit_log              | boolean                 | Whether to turn on audit logging, by default `false`                                                                                                       |
| audit_log_expire              | integer                 | Time-to-live for audit logs in units of hours. Default value is `0`, which means forever                                                                   |
| audit_log_dir                 | string                  | Directory to store audit logs. Default value is `$directory/_audit_log_`                                                                                   |
| load_plugins                  | boolean                 | Load all plugins when starting server. Default=1.                                                                                                          |
| optimistic_txn                | boolean                 | Enable optimistic multi-writer transaction for Cypher. Default=0.                                                                                          |
| disable_auth                  | boolean                 | Disable authentication for REST. Default=0.                                                                                                                |
| snapshot_interval             | integer                 | Snapshot interval in seconds. Default=86400.                                                                                                               |
| heartbeat_interval_ms         | integer                 | Interval for heartbeat in ms. Default=1000.                                                                                                                |
| heartbeat_failure_duration_ms | integer                 | Peer considered failed and marked OFFLINE after this duration. Default=60000.                                                                              |
| node_dead_duration_ms         | integer                 | Node considered completly dead and removed from list after this duration. Default=120000.                                                                  |
| enable_ip_check               | boolean                 | Enable IP whitelist check. Default=0.                                                                                                                      |
| idle_seconds                  | integer                 | Maximum number of seconds during which a subprocess can be idle. Default=600.                                                                              |
| enable_backup_log             | boolean                 | Whether to enable backup logging. Default=0.                                                                                                               |
| backup_log_dir                | string                  | Directory to store the backup files. Default="".                                                                                                           |
| snapshot_dir                  | string                  | Directory to store the snapshot files. Default="".                                                                                                         |
| thread_limit                  | integer                 | Maximum number of threads to use. Default=0, which uses the value specified in license file.                                                               |
| help                          | boolean                 | Print this help message. Default=0.                                                                                                                        |

## TuGraph Config File

The config file of TuGraph is stored in JSON format. It is recommended to store most of the configurations in the config file, and only use command line option to modify some configurations when needed temporarily.

A sample TuGraph config file can look like this:

```json
{
  "directory": "/var/lib/lgraph/data",
  "license": "/var/lib/lgraph/fma.lic",

  "port": 7090,
  "rpc_port": 9090,
  "enable_ha": false,

  "verbose": 1,
  "log_dir": "/var/log/lgraph",

  "ssl_auth": false,
  "server_key": "/usr/local/etc/lgraph/server-key.pem",
  "server_cert": "/usr/local/etc/lgraph/server-cert.pem"
}
```

## Configuring With Command Line

The `lgraph_server` command is used to start a TuGraph server instance.
Command line options can be used to override the configurations loaded from the config file when starting TuGraph servers.

In addition to all the config options described in [TuGraph Config Options](#TuGraph-Config-Options), the `lgraph-server` also determines the running mode of the started server, which can be `standard` or `daemon`, as will be described in [TuGraph Running Mode](#TuGraph-Running-Mode).

A typical `lgraph_server` command line may look like the following:

```bash
$ lgraph_server --config ./local_lgraph.json --port 7777 --mode start
```

# Starting TuGraph Server

## TuGraph Running Mode

A TuGraph server can be started as a foreground process as well as a daemon process running in the background.

When run as a foreground process, TuGraph can print the logs directly to the terminal, which can come in handy when debugging server configuration. However, since foreground processes are killed once the terminal exits, the user should make sure the terminal remains open while the TuGraph server is meant to be running.

In daemon mode, on the other hand, a TuGraph server can continue running even if the terminal that started it exits. Hence it is better to start TuGraph server in daemon mode for long-running servers.

## Running TuGraph In Foreground

With the `lgraph_server -d run` option, TuGraph server can be started as a foreground process. If `--log_dir ""` is also given, TuGraph will print logs on the terminal, making it easier to debug server configurations.

The output of a TuGraph server may look like this:

```
$ ./lgraph_server -c lgraph_standalone.json --log_dir ""
20200508120723.039: **********************************************************************
20200508120723.039: *                    TuGraph Graph Database v3.0.0                   *
20200508120723.040: *                                                                    *
20200508120723.041: *        Copyright(C) 2018 Ant Group. All rights reserved.           *
20200508120723.041: *                                                                    *
20200508120723.044: *             Licensed host: hostname      threads:0, ha:0           *
20200508120723.044: **********************************************************************
20200508120723.044: Server is configured with the following parameters:
20200508120723.045:   data directory:    ./lgraph_db
20200508120723.045:   license:           ./fma.lic
20200508120723.046:   enable ha:          0
20200508120723.046:   durable:            1
20200508120723.047:   host:               127.0.0.1
20200508120723.047:   REST port:          7071
20200508120723.048:   RPC port:           9091
20200508120723.048:   enable rpc:         0
20200508120723.051:   optimistic txn:     0
20200508120723.059:   verbose:            1
20200508120723.074:   log_dir:
20200508120723.074:   ssl_auth:           0
20200508120723.075:   resource dir:       ./resource

20200508120723.077: [Galaxy] Loading DB state from disk
20200508120723.110: [RestServer] Listening for REST on port 7071
20200508120723.110: [LGraphService] Server started.
```

In this mode, TuGraph can be terminated at any time with a CTRL-C.

## Starting TuGraph In Daemon Mode

A TuGraph server can be started with the `-d start` option:

```bash
$ ./lgraph_server -d start -c lgraph_daemon.json
Starting lgraph...
The service process is started at pid 12109.
```

This command starts a TuGraph server as a daemon process, which loads its configuration from the file `lgraph_daemon.json`.

Once the server is started, it starts to print logs in the log file, which can then be used to determine the status of the server.

## Stopping TuGraph Daemon

A TuGraph daemon process can be stopped with a `kill` command, as well as the `lgraph_server -d stop` command line.

Since there could be multiple TuGraph server processes running on the same machine, we distinguish different server processes with a `.pid` file, which is written to the working directory where the process was started. As a result, the `lgraph_server -d stop` needs to be run in the same working directory for it to stop the correct server process.

```bash
user@host:~/tugraph$ ./lgraph_server -d start -c lgraph_standalone.json
20200508122306.378: Starting lgraph...
20200508122306.379: The service process is started at pid 93.

user@host:~/tugraph$ cat ./lgraph.pid
93

user@host:~/tugraph$ ./lgraph_server -d stop -c lgraph_standalone.json
20200508122334.857: Stopping lgraph...
20200508122334.857: Process stopped.
```

## Restarting TuGraph Daemon

To restart a TuGraph daemon, use the `lgraph_server -d restart` command line:

```bash
$ ./lgraph_server -d restart
Stopping lgraph...
Process stopped.
Starting lgraph...
The service process is started at pid 20899.
```

# Accessing A TuGraph Server

There are also multiple ways to access data in a TuGraph database:

- **REST API:** TuGraph offers a set of REST APIs that covers the basic operations of the database, as well as calling Cypher queries and invoking plugins. Full documentation of the REST API can be found in [TuGraph REST API Manual](./TuGraph-Rest-API-en.md).
- **OpenCypher Query Language:** TuGraph supports OpenCypher, an open-source graph query language. In addition to the standard syntax of OpenCypher, TuGraph also has its own extensions. For more information on how to use the extended OpenCypher in TuGraph, please refer to [TuGraph OpenCypher Manual](./TuGraph-Cypher-en.md).
- **Plugin API:** In addition to OpenCypher queries, TuGraph also provides a plugin API that exposes low-level operations to users. A user can write a plugin with the plugin API and load it into the server. Since plugins are written with imperative languages (currently we support C++, Java, and Python), they can be used to implement arbitrarily complex logic. And if performance is a concern, the plugin can be written in native languages and optimized to its best. A reference of the plugin API and how to manage plugins can be found in [TuGraph Plugin Manual](./TuGraph-Procedure-en.md).

A user can access the TuGraph server in the following ways:

- **`lgraph_cypher` command line query client**: a command line tool that executes OpenCypher queries on the server and prints the results in the terminal.
- **[TuGraph Visualizer](#tugraph-visualizer):** a web interface that can be used to send Cypher queries and visualize the resulting sub-graph. It can also manage and call plugins.
- **HTTP requests:** TuGraph serves REST requests directly with HTTP, uses can use HTTP clients to send REST requests to a TuGraph server.
- **TuGraph Client SDK:** TuGraph provides client SDKs in multiple languages (currently C++, Java and Python), which can be called from a user application.

`lgraph_cypher` and TuGraph Visualizer offers interactive interface, thus are more suitable for human use, while REST API and the SDKs are designed to be used by programs.

# `lgraph_cypher` Query Client

TuGraph software distribution comes with a query client named `lgraph_cypher`, which can be used to submit OpenCypher requests to a TuGraph server. The `lgraph_cypher` client has two execution modes: a single-command mode and an interactive mode.

## Single-Command Mode

In single-command mode, `lgraph_cypher` can be used to submit one single Cypher query and print the result directly to the terminal, which can be easily redirected to a file. This is useful when we need to get a lot of results from the server and save them in a file.

In this mode, the `lgraph_cypher` tool has the following options:

### `lgraph_cypher` Options:

| option   | <nobr>value type</nobr> | description                                                                                        |
| -------- | ----------------------- | -------------------------------------------------------------------------------------------------- |
| --help   | \                       | Prints the help message                                                                            |
| -example | \                       | Prints example command lines                                                                       |
| -c       | string                  | Path to a TuGraph config file, used to read server ip and port                                     |
| -h       | string                  | IP of the server. Overrides the setting in the config file. By default `127.0.0.1`                 |
| -p       | string                  | HTTP port of the server. Overrides the setting in the config file. By default `7071`               |
| -u       | string                  | User name                                                                                          |
| -P       | string                  | Password                                                                                           |
| -f       | string                  | Path to a text file containing a single Cypher query                                               |
| -s       | string                  | A single Cypher query quoted with double quotes `"`                                                |
| -t       | int                     | Query timeout in seconds. Default value is `150`                                                   |
| -format  | string                  | Format in which the results are to be printed: either `plain` or `table`. Default value is `table` |

### Command Examples

**Cypher query in a text file**

```
$ ./lgraph_cypher.py -c /home/usr/lgraph_standalone.json -u user -P password -f /home/usr/cypher.json
```

**Cypher query as a string**

```
$ ./lgraph_cypher.py -c /home/usr/lgraph_standalone.json -u user -P password -s "MATCH (n) RETURN n"
```

## Interactive Mode

`lgraph_cypher` can also run in interactive mode, in which the client maintains a connection with the server and interacts with the user in a Read-Evaluate-Print-Loop.

### Entering Interactive Mode

If neither `-f` nor `-s` option is given, `lgraph_cypher` will enter interactive mode. For example:

```
$ ./lgraph_cypher.py -c /home/usr/lgraph_standalone.json -u admin -P 73@TuGraph
```

If the login succeeds, a welcome message will be printed:

```
**********************************************************************
*                   TuGraph Graph Database X.Y.Z                     *
*                                                                    *
*        Copyright(C) 2018 Ant Group. All rights reserved.           *
*                                                                    *
**********************************************************************
login success
----------------------------------
Host: 127.0.0.1
Port: 7071
Username: admin
----------------------------------
type ":help" to see all commands.
>
```

Now we have an interactive shell. The user can type in Cypher queries or use the `:help` command to check the available commands.

### `lgraph_cypher` Commands

Except for Cypher queries, the `lgraph_cypher` shell also accepts the following commands:

| command                  | <nobr>parameter</nobr>      | description                                                                                                                        |
| ------------------------ | --------------------------- | ---------------------------------------------------------------------------------------------------------------------------------- |
| :help                    | \                           | Print the help message                                                                                                             |
| :db_info                 | \                           | Get DB info                                                                                                                        |
| :clear                   | \                           | Clear current screen                                                                                                               |
| :use                     | `{graph_name}`              | Use this graph, by default `default` will be used                                                                                  |
| :source                  | `-t {N} -f {query_file}`    | Run the query stored in the `query_file` with a timeout of `N` seconds. `N` is by default `150`                                    |
| :exit                    | \                           | Exit the `lgraph_cypher` shell                                                                                                     |
| :format                  | `plain` or `table`          | Format in which to print the results, either `plain` or `table`                                                                    |
| :save all/command/result | `-f {file_path}` `{cypher}` | Store the Cypher query (`command`) or results (`result`) or both (`all`) to a file. By default, `file_path` is `/saved_cypher.txt` |

**NOTE:**

- Every command should start with a colon `:`.

**Example `:save` command:**

```
:save all -f /home/usr/saved.txt match (n) where return n, n.name limit 1000
```

### Evaluating Cypher Queries:

To Submit a cypher query, simply type in the cypher query and end it with a semicolon `;`.

```
>MATCH (n) RETURN n, n.name;
+---+---+-------------+
|   | n |n.name       |
+---+---+-------------+
| 0 | 0 |david        |
| 1 | 1 |Ann          |
| 2 | 2 |first movie  |
| 3 | 3 |Andres       |
+---+---+-------------+
time spent: 0.000520706176758
size of query: 4
>
```

Multi-line Cypher queries are allowed in `lgraph_cypher`. By pressing `ENTER` in the middle of a Cypher query, the shell will see it as a multi-line query and print a `=>` prompt at the beginning of the line, in which user can then continue to write the rest of the query.

For example:

```
login success
>MATCH (n)
=>WHERE n.uid='M11'
=>RETURN n, n.name;
```

### Auxiliary Functions

**Historical queries** User can use up and down arrows to lookup past queries in the shell.

**Auto-complete:** The shell will prompt for candidate commands while you type, you can use right arrows to accept the candidate.

# TuGraph Visualizer

TuGraph comes with a web interface that enables users to:

- Execute Cypher queries and visualize the resulting sub-graph
- Manage graphs in the database
- Manage and call plugins
- Check database status in real time
- Manage user accounts and access rights to individual graphs
- Manage tasks currently running in the database
- Analyze audit logs

For detailed descriptions of the TuGraph Visualizer, please refer to the [TuGraph Visualizer Manual](./TuGraph-Visualizer.md).

# Enabling TuGraph High Availability

## Principles And Terminologies

TuGraph provides high availability (`HA`) with data replication, meaning that each piece of data is stored in multiple TuGraph servers so that even if some server goes down, the others can continue to serve requests.

In HA mode, multiple TuGraph servers form a `replication group`. Each replication group consists of three or more TuGraph servers, one of them acting the role of `leader`, while others acting as `followers`. Write requests are served by the `leader`, who replicates each request to the followers and respond to the client only after the request gets replicated to the servers. This way, if any of the servers fails, the other servers will still have all the data that has been written so far. If the leader fails, the other servers will elect a new leader automatically.

## Enabling HA

To enable HA for TuGraph, we will need:

- Three or more TuGraph server instances
- The servers must have license with HA enabled
- HA mode must be turned on on each TuGraph server by setting `enable_ha` option to `true`
- `rpc_port` of each server must be properly configured

## Starting The First Server

The first server in a replication group will elect itself as the leader, and later servers must join the replication group.

The first server is started with either `--master ""` or `--master BOOTSTRAP` option, depending on whether the first server already has data in it.

- `--master "":` If the first server is empty, you can simply use the `--master ""` option.
- `--master BOOTSTRAP:` If the first server already has data in it (either imported with `lgraph_import` or transferred from a non-HA server), and it has not been used in HA mode before, you should use the `BOOTSTRAP` option to start the server in bootstrapping mode. In bootstrapping mode, the server will replicate its own data to newly-joined servers before adding them to the replication group, so that the data in every server is consistent.

An example command line to start the first server that is non-empty would look like this:

```bash
$ ./lgraph_server -c lgraph.json --rpc_port 9090 --enable_ha true --master BOOTSTRAP
```

## Adding Servers To A Replication Group

Once the first server is started, it forms a one-server replication group. This is a vulnerable state: if the server crashes, the services is interrupted. In TuGraph, a replication group can endure at most (N-1)/2 server crashes while still guaranteeing service availability. Thus at least three servers must be present in one replication group in order to guarantee service availability in the face of one server loss.

To add a server to a replication group, you should use the `--master {HOST:PORT}` option, in which `HOST` can be the IP address of any server already in that replication group, with `PORT` being its RPC port. For example:

```bash
./lgraph_server -c lgraph.json --rpc_port 9091 --enable_ha true --master 192.168.1.120:9090
```

This command will start a TuGraph server with HA enabled and try to add it to the replication group that contains the server `192.168.1.120:9090`. Note that joining a replication group requires the server to synchronize its data with the leader of the replication group and may take quite a while, depending on the size of the data.

## Stopping Servers In Replication Group

When a server is taken offline by pressing `CTRL-C`, it will inform the current leader, which will then remove the server from the replication group. If a leader is taken offline, it will transfer leadership to another server before it stops.

If a server is killed or lost connection with other servers in the replication group, it will be treated as a failed node and the leader will remove it from the replication group after a certain timeout.

If any server leaves the replication group and wants to rejoin, it must start with the `--master {HOST:PORT}` option, where `HOST` is the IP address of a server that is currently in the replication group.

## Restarting The Whole Replication Group

Restarting the whole replication group is not recommended, since it interrupts the service. In case when this is needed, one can take all the servers offline. But when restarting, the last server that was shutdown must be started first.

## Checking Replication Group Status

Current status of the replication group can be obtained in the TuGraph Visualizer, with REST API, as well as with Cypher query.

In TuGraph Visualizer, the list of servers in replication group and their roles can be found in the DBInfo section.

With REST API, the information can be fetched with `GET /info/peers`.

In Cypher, the `CALL dbms.listServers()` statement is used to query the status info of current replication group.

## Query Semantics In HA Mode

In HA mode, different servers in the same replication group may not always be in the same state. For performance reasons, the leader will consider a request `committed` if it has be replicated to more than half of the servers. Although the remaining servers will eventually get the new request, there is a period in time when the servers have inconsistent state. It is also possible that a client sent a request to a server that has just restarted, thus has older state, and is waiting to join a replication group.

To make sure clients see a consistent data view, especially to get rid of `reverse time travel`, in which a client reads a state that is older than it has previously seen, each TuGraph server maintains a monotonically increasing version number. The mapping from a version number to a database state is globally consistent in the replication group, meaning that if two servers have the same version number, they must have exactly the same data. When responding to a request, the server packs its version number in the response. So clients get to know which version it has seen. The client can choose to send this version number together with the request. Upon receiving a request with a version number, the server will compare the version number with its current version and will reject the request if its own version is lower than the requested one. This mechanism makes sure that clients never read older state than it previously did.

# Managing TuGraph Servers

## Logging

TuGraph keeps two types of logs: the server log and the audit log. The server log records human-readable messages of server status, while the audit log maintains an encrypted log of every operation performed on the server.

### Server Log

The server log keeps track of server status messages such as server start and stop, as well as the requests the server has served and their corresponding responses. The verbosity of the server log is configurable with the `verbose` option. And the location of the log is specified in the `log_dir` option.

The default `verbose` level is `1`, at which level the server will only print logs for major events such as server start/stop. Requests and responses are not recorded in this level.

### Audit Logging

The audit log records every request and response, along with the user who sent the request and the time when the request is received. Audit log can only be turned on or off. The result can be queried with the TuGraph visualizer and the REST API.

## Exporting Data

In case you need to export a TuGraph database, a toolkit called `lgraph_export` is provided in TuGraph. The toolkit exports a graph into a bunch of CSV files and generates an import config file with the format described in [Import Config File](#import-config-file).

The toolkit command line looks like this:

```bash
$ lgraph_export -d {database_dir} -e {export_destination_dir} -g {graph_to_use} -u {username} -p {password}
```

in which:

- `-d {database_dir}` specifies the data directory of TuGraph, by default `./testdb`
- `-e {export_destination_dir}` is the destination directory under with the files are written, by default `./exportdir`
- `-g {graph_to_use}` is the name of the graph to export, by default `default`
- `-u {username}` specifies the user name to be used to access the database
- `-p {password}` is the password
- `-s {field separator}` specifies the string used to separate fields in the output files, defaults to comma
- `-h` prints the help message.

## Database Backup

The `lgraph_backup` toolkit is used to backup the whole database to a different directory. The command line options are as follows:

```bash
$ lgraph_backup -s {source_dir} -d {destination_dir} -c {true/false}
```

in which:

- `-s {source_dir}` is the source data directory
- `-d {destination_dir}` is the destination directory
- `-c {true/false}` specifies whether to perform data compaction during backup. Compaction makes smaller data files, but also slows down the process. By default, it is set to `false`.

## Database Warmup

TuGraph is a disk-based database. Data is loaded into memory only when it is accessed. With a freshly started TuGraph, queries will results in frequent disk IO and thus performance is suboptimal. Warming up the database before intensive querying can help improve performance.

The `lgraph_warmup` toolkit is used to perform warmup. It is used as follows:

```bash
$ lgraph_warmup -d {directory} -g {graph_list}
```

in which:

- `-d {db_dir}` option specifies the data directory of the TuGraph server
- `-g {graph_list}` specifiers the names of graphs to warmup, separated with commas.

Depending on the data size and performance of the disk, the warmup time may vary. Warming up a large database on a hard drive may take a long time.

## Task Management

TuGraph keeps track of long-running tasks. The list of currently running tasks can be queried with the TuGraph Visualizer, with REST API or Cypher. Long-running tasks can also be killed by database administrators.

## Memory Limit

TuGraph can set memory limit for each user. When a user executes a statement that exceeds the memory limit, the execution will be terminated and an error message will be returned. Set by CALL dbms.setUserMemoryLimit, and it is unlimited without setting. The user can also obtain the real-time memory usage of the user.

# resource monitoring

lgraph_monitor can monitoring resource usage on the machine where the tugraph resides， It periodically sends RPC requests to Tugraph to check the current state and imports the results into Prometheus, we can use grafana to show the result

## start tugraph
```bash
./lgraph_server -c lgraph_standalone.json
```

## start prometheus
1. install prometheus，see also https://prometheus.io/
2. configure prometheus.yml to acquire monitor data from lgraph_monitor
3. start prometheus

## start monitor

```bash
$ ./lgraph_monitor -u admin -p 73@TuGraph --monitor_host 127.0.0.1:9999 --sampling_interval_ms 150
Available command line options:
    --server_host           Host on which the tugraph rpc server runs. Default="127.0.0.1:9091"
    -u, --user              DB username
    -p, --password          DB password
    --monitor_host          Host on which the monitor restful server runs. Default="127.0.0.1:9999".
    --sampling_interval_ms  sampling interval in millisecond. Default=150
```

## install the grafana template

1. Install and start grafana ，see also https://grafana.com/
2. Configure the grafana data source, select the prometheus data source, and fill in the IP address and port
3. Click "+", select "import", select "Upload JSON file"
   ", select the grafana template file (/deps/TuGraph-web/grafana-template/TuGraph-grafana-template.json)

# FAQ

**1. "Error opening config file xxx, exiting..."**

This error message is seen when TuGraph fails to read the config file. Make sure you specified the right path and the current user has access to that file.

**2. "Failed to parse command line option: Option xxx cannot be recognized."**

Typical error when command line options are not set correctly. One can use `-h, --help` to print the available options.

**3. "Error opening license file xxx"**

This means the license path is not set correctly, or the user does not have access to that file.
