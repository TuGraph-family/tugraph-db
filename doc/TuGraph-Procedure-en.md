# TuGraph Stored Procedure Guide

Version: 3.3.0

2022/07/19

Ant Group

---

## Table of Contents

- [Introduction](#Introduction)
- [Guide](#Guide)
  - [Write Stored Procedures](#Write Stored Procedures)
    - [C++](#C++)
    - [Python](#Python)
  - [Install Stored Procedures](#Install Stored Procedures)
  - [List Stored Procedures](#List Stored Procedures)
  - [Call Stored Procedures](#Call Stored Procedures)
  - [Uninstall Stored Procedures](#Uninstall Stored Procedures)
  - [Upgrade Stored Procedures](#Upgrade Stored Procedures)

---

# Introduction
When users want to express complex logics of queries or updates (such as those currently not supported by Cypher, or those demanding high performance), using TuGraph stored procedures would be a better choice.

Similar to traditional database systems, TuGraph stored procedures run on the server side. Users wrap the processing logics inside stored procedures, install stored procedures into the database, and call them from clients via either RESTful/RPC. Stored procedures can be installed, uninstalled or upgraded dynamically without interrupting the server.

# Guide
TuGraph supports two types of stored procedures, one implemented using the C++ API and the other implemented using the Python API. These two types of stored procedures are managed separately, but their usages share many aspects in common. This guide presents a very simple tutorial of how to write, build, call, and manage (i.e. install/uninstall/upgrade) C++/Python stored procedures in TuGraph.

## Write Stored Procedures
### C++
The following shows the code of a simple TuGraph C++ stored procedure.
Calculate the number of outgoing and incoming edges and the sum of the number of incoming and outgoing edges for each node:

```C++
// standard_result.cpp
#include "lgraph/lgraph_result.h"
#include "iostream"

using namespace lgraph_api;

extern "C" LGAPI bool Process(GraphDB &db, const std::string &request, std::string &response) {
    // defination
    Result result({{"node", ResultElementType::NODE},
                   {"edge_num_sum", ResultElementType::INTEGER},
                   {"edge_num", ResultElementType::MAP}});

    auto txn = db.CreateReadTxn();
    for (auto vit = txn.GetVertexIterator(); vit.IsValid(); vit.Next()) {
        auto &record = result.NewRecord();
        std::map<std::string, FieldData> edge_num_map;
        record.insert("node", vit);

        int in_num_edges = 0;
        int out_num_edges = 0;

        for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) out_num_edges += 1;
        edge_num_map["out_num_edges"] = FieldData(out_num_edges);
        for (auto eit = vit.GetInEdgeIterator(); eit.IsValid(); eit.Next()) in_num_edges += 1;
        edge_num_map["in_num_edges"] = FieldData(in_num_edges);

        record.insert("edge_num_sum", FieldData(in_num_edges + out_num_edges));

        record.insert("edge_num", edge_num_map);
    }

    response = result.Dump();
    return true;
}
```
The entry of a TuGraph C++ stored procedure is the `Process` function, with three parameters:

* `db`: the TuGraph database instance
* `request`: the input data, which can be an arbitrary binary string
* `response`: the output data, which can also be an arbitrary binary string


The `Result` structure is used in the function. `Result` is the standard output format, which can effectively help you **visualize** your results.
You need to define your return type first, we provide:
`VOID`, 
`INTEGER`, 
`FLOAT`,
`DOUBLE`,
`BOOLEAN`,
`STRING`,
`NODE`,
`RELATIONSHIP`,
`PATH`,
`LIST`,
`MAP`,
`FIELD`,
`GRAPH_ELEMENT`,
`COLLECTION`,
`ANY`
fifteen types.

Users can use `Result` by three steps.
1. define your return type
```C++
Result result({{"node", ResultElementType::NODE},
                   {"edge_num_sum", ResultElementType::INTEGER},
                   {"edge_num", ResultElementType::MAP}});
```
We define three return types in demo, `NODE`, `INTEGER`, `MAP`, and corresponding title.
2. initialize `Record` and add element.
```C++
auto &record = result.NewRecord();
// insert multiple times
record.insert("node", vit);
```
3. `Dump` your result
```C++
response = result.Dump();
```
**Attention**
* The return value must be a **reference**.
* Assigning multiple `Record` at the same time may cause partial loss of data.


The return value of `Process` is a boolean value. When it is true, it means the operation succeeds; otherwise it means there exist errors during execution (users may return error information through `response`).

To build the above stored procedure, you can type the following in shell:

```bash
g++ -fno-gnu-unique -fPIC -g --std=c++14 -I/usr/local/include/lgraph -rdynamic -O3 -fopenmp -o age_10.so age_10.cpp /usr/local/lib64/liblgraph.so -shared
```
which should generate an `age_10.so` file.

### Python
The following snippet does the same thing as the above C++ stored procedure, but via TuGraph Python API:

```python
def Process(db, input):
    txn = db.CreateReadTxn()
    it = txn.GetVertexIterator()
    n = 0
    while it.IsValid():
        if it.GetLabel() == 'student' and it['age'] and it['age'] == 10:
            n = n + 1
        it.Next()
    return (True, str(nv))
```
The return value of TuGraph Python stored procedures is a tuple, consisting of a boolean value indicating whether the operation succeeds, and a string value containing the response data.

## Install Stored Procedures
Users may install plugins with the following Python snippet into the `school` graph (different graphs can have different stored procedures)：

```python
import requests
import json
import base64

data = {'name':'age_10'}
f = open('./age_10.so','rb')
content = f.read()
data['code_base64'] = base64.b64encode(content).decode()
data['description'] = 'Calculate number of students in the age of 10'
data['read_only'] = true
data['code_type'] = 'so'
js = json.dumps(data)
r = requests.post(url='http://127.0.0.1:7071/db/school/cpp_plugin', data=js,
            headers={'Content-Type':'application/json'})
print(r.status_code)    # 正常时返回200
```
One thing to note is that `data['code']` is a base64-encoded string since the binary content in `age_10.so` may not be transported via JSON directly. In addition, only administrators can manage stored procedures. Normal users can only call or list stored procedures.

Once installed, stored procedures are maintained in the database, so they will be loaded automatically on TuGraph server startup.

Installing Python stored procedures in TuGraph only differs in the URL (i.e. `http://127.0.0.1:7071/db/school/python_plugin`)

## List Stored Procedures
The following snippet shows how to list all C++ stored procedures installed in the `school` graph：
```python
>>> r = requests.get('http://127.0.0.1:7071/db/school/cpp_plugin')
>>> r.status_code
200
>>> r.text
'{"plugins":[{"description":"Calculate number of students in the age of 10", "name":"age_10", "read_only":true}]}'
```

## Retrieve Stored Procedures Detail

The following snippet shows how to one procedure detail, including code:

```python
>>> r = requests.get('http://127.0.0.1:7071/db/school/cpp_plugin/age_10')
>>> r.status_code
200
>>> r.text
'{"description":"Calculate number of students in the age of 10", "name":"age_10", "read_only":true, "code_base64":<CODE>, "code_type":"so"}'
```



## Call Stored Procedures

The following snippet shows how to call the `age_10` C++ stored procedure in the `school` graph:
```python
>>> r = requests.post(url='http://127.0.0.1:7071/db/school/cpp_plugin/age_10', data='',
                headers={'Content-Type':'application/json'})
>>> r.status_code
200
>>> r.text
9
```

## Uninstall Stored Procedures
You can uninstall the `age_10` C++ stored procedure with the following snippet:
```python
>>> r = requests.delete(url='http://127.0.0.1:7071/db/school/cpp_plugin/age_10')
>>> r.status_code
200
```

Note that only administrators can uninstall stored procedures.

## Upgrade Stored Procedures

You can upgrade a stored procedure with the following two steps:

1. Uninstall the existing one.
2. Install the new on.

TuGraph carefully manages the concurrency of stored procedure operations. Upgrading stored procedures will not affect concurrent runs of existing ones.