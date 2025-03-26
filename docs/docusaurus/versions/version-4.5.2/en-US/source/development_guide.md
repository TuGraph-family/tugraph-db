# Business Development Guide

## Connect to Tugraph DB

### Driver Connection

Tugraph DB is compatible with Neo4j's communication protocol, so you can use Neo4j drivers to connect to Tugraph DB's server.

[bolt driver Introduction](7.client-tools/5.bolt-client.md)

[bolt driver Example](https://github.com/TuGraph-family/tugraph-db/tree/master/demo/Bolt)

### Terminal connection

The driver is used within the business code, while for terminal access on the server, you can use the CLI client.

[console client Introduction](7.client-tools/6.bolt-console-client.md)

## graph Operations

### Create Graph Project

```
CALL dbms.graph.createGraph('graph1')
```

### Delete Subgraph Project

```
CALL dbms.graph.deleteGraph('graph1')
```

### Clear Graph Project

#### Delete all node and edge data, and graph schema

```
CALL db.dropDB()
```

#### Delete all node and edge data, but keep the graph schema.

```
CALL db.dropAllVertex()
```

### View Graph Schema

```
CALL dbms.graph.getGraphSchema()
```

### List all graph projects

```
CALL dbms.graph.listGraphs()
```

### Refresh subgraph file system cache data

```
CALL db.flushDB()
```

## Vertex type operations

### Create vertex type

The following JSON defines a vertex type named node1.

```json
{
	"label": "node1",
	"primary": "id",
	"type": "VERTEX",
	"detach_property": true,
	"properties": [{
		"name": "id",
		"type": "INT32",
		"optional": false
	}, {
		"name": "name",
		"type": "STRING",
		"optional": false,
		"index": true
	}, {
		"name": "num",
		"type": "INT32",
		"optional": false,
		"index": true,
		"unique": true
	}, {
		"name": "desc",
		"type": "STRING",
		"optional": true
	}]
}

```

Serialize the JSON above into a string and pass it as a parameter. It is recommended to use the driver's parameterization feature to avoid manually concatenating the statement.

```
CALL db.createVertexLabelByJson($json_data)
```

### View vertex type schema

```
CALL db.getVertexSchema('node1')
```

### Delete vertex type

>This operation will also delete all vertices of this type, which can be time-consuming if there is a large amount of data.

The following example deletes the vertex type node1 along with all vertices of this type.

```
CALL db.deleteLabel('vertex', 'node1')
```

### Add field to vertex type

>This operation will also change the attribute data of all vertices of this type, which can be time-consuming if there is a large amount of data.

In the following example, two fields are added to the vertex type `node1`: `field1`, which is of string type, optional, with a default value of `null`; `field2`, which is of `int64` type, required, with a default value of 0.

```
CALL db.alterLabelAddFields('vertex', 'node1', ['field1', string, null ,true], ['field2', int64, 0, false])
```

### Delete field from vertex type

>This operation will also change the attribute data of all vertices of this type, which can be time-consuming if there is a large amount of data.

In the following example, two fields are deleted from the vertex type node1: `field1` and `field2`.

```
CALL db.alterLabelDelFields('vertex', 'node1', ['field1', 'field2'])
```

### Add index to vertex type

>This operation will also build index data synchronously, which can be time-consuming if there is a large amount of data.

In the following example, a non-unique index is added to the `field1` field for the vertex type `node1`.

```
CALL db.addIndex('node1', 'field1', false)
```

In the following example, a unique index is added to the `field2` field for the vertex type  `node1`.

```
CALL db.addIndex('node1', 'field2', true)
```

### Delete index from vertex type

In the following example, the index on the `field1` field of the vertex type `node1` is deleted.

```
CALL db.deleteIndex('node1', 'field1')
```


## Edge type operations

### Create edge type

The following JSON defines the schema for an edge named `edge1`.

```json
{
  "label": "edge1",
  "type": "EDGE",
  "detach_property": true,
  "constraints": [
    ["node1", "node2"]
  ],
  "properties": [{
    "name": "id",
    "type": "INT32",
    "optional": false
  }, {
    "name": "name",
    "type": "STRING",
    "optional": false,
    "index": true
  }, {
    "name": "num",
    "type": "INT32",
    "optional": false,
    "index": true,
    "unique": true
  }, {
    "name": "desc",
    "type": "STRING",
    "optional": true
  }]
}
```

Serialize the JSON into a string and pass it as a parameter. It is recommended to use the driver's parameterization feature to avoid concatenating the statement manually.

```
CALL db.createEdgeLabelByJson($json_data)
```

### View edge type schema

```
CALL db.getEdgeSchema('edge1')
```

### Delete edge type

>This operation will synchronously delete all edges of that type, which may be time-consuming when dealing with a large amount of data.

In the following example, delete the edge type `edge1` along with all edge data of that type.

```
CALL db.deleteLabel('edge', 'edge1')
```

### Add field to edge type

>This operation will synchronously modify properties of all edges of that type, which may be time-consuming when dealing with a large amount of data.

In the following example, for the edge type `edge1`, two fields are added at once: `field1`, which is of string type, optional, with a default value of `null`; and `field2`, which is of `int64` type, required, with a default value of `0`.

```
CALL db.alterLabelAddFields('edge', 'edge1', ['field1', string, null ,true], ['field2', int64, 0, false])
```

### Delete field from edge type

>This operation will synchronously update the attribute data of all edges of that type, which may be time-consuming when dealing with a large amount of data.

In the following operation, for the edge type edge1, two fields are deleted at once: field1 and field2.

```
CALL db.alterLabelDelFields('edge', 'edge1', ['field1', 'field2'])
```

### Add index to edge type

>This operation will synchronously build index data, which may be time-consuming when dealing with a large amount of data.

In the following example, for the edge type `edge1`, a non-unique index is added to the field `field1`.

```
CALL db.addEdgeIndex('edge1', 'field1', false, false)
```

In the following example, for the edge type `edge1`, a unique index is added to the field `field2`.

```
CALL db.addEdgeIndex('edge1', 'field2', true, false)
```

### Delete index from edge type

In the following example, for the edge type edge1, the index on the field field1 is removed.

```
CALL db.deleteEdgeIndex('edge1', 'field1')
```

## View the current vertex and edge data quantity in real-time.

The following example returns all vertex and edge types along with the current data quantity for each type.

It reads statistical data and is a lightweight operation.

```
CALL dbms.meta.countDetail()
```

## Import data

### Batch upsert vertex data

If the vertex does not exist, insert it; if it exists, update the vertex attributes based on the primary key field value to determine existence.

The second parameter is of type list, where each element in the list is of type map. Each map contains the vertex fields and their corresponding values.

It is recommended to use the parameterization feature in the driver. Directly pass a list structure as the second parameter to avoid constructing the statement manually.

```
CALL db.upsertVertex('node1', [{id:1, name:'name1'},{id:2, name:'name2'}])
```

### Batch upsert edge data

If the edge of a certain type does not exist between two vertices, insert it; if it exists, update the attributes of the edge. In other words, there can be only one edge of the same type between the two vertices.

The fourth parameter is of type list, where each element is a map. Each map contains the starting vertex type's primary key field and corresponding value, the ending vertex type's primary key field and corresponding value, and the edge type's own attribute fields and values. Each map contains at least two elements.

The second and third parameters serve the fourth parameter. They specify the types of the starting and ending vertices, respectively, and indicate which fields in the fourth parameter represent the starting vertex primary key field value and the ending vertex primary key field value.

Note: The primary key fields configured in the second and third parameters for the starting and ending vertices are not the actual primary key field names in the vertex schemas. They serve merely as placeholders and identifiers to facilitate recognizing which fields in the fourth parameter represent the starting and ending vertex primary key fields.

It is recommended to use the parameterization feature in the driver to avoid constructing the statements manually.

```
CALL db.upsertEdge('edge1',{type:'node1',key:'node1_id'}, {type:'node2',key:'node2_id'}, [{node1_id:1,node2_id:2,score:10},{node1_id:3,node2_id:4,score:20}])
```

### Batch upsert edge data - determine uniqueness based on edge attributes

The previously described upsert logic allows only one edge of the same type between two vertices. If multiple edges of the same type are allowed between two vertices, and uniqueness needs to be determined based on a specific attribute on the edge, an additional field needs to be added as follows:

```
CALL db.upsertEdge('edge1',{type:'node1',key:'node1_id'}, {type:'node2',key:'node2_id'}, [{node1_id:1,node2_id:2,score:10},{node1_id:3,node2_id:4,score:20}], 'score')
```

An additional field score is included at the end. The logic changes to: if an edge1 type edge with a score value equal to a specific value does not exist between two vertices, then insert; otherwise, update the attributes of that edge.

The score field on the edge needs to have a special pair unique index added in advance, as follows:

```
CALL db.addEdgeIndex('edge1', 'score', false, true)
```

### DataX

https://github.com/ljcui/DataX/tree/bolt 。

This DataX implementation's tugraph writer internally calls the db.upsertVertex and db.upsertEdge methods described above. The tugraph reader internally calls TuGraph's bolt client, supporting streaming reads.

For detailed usage, see [TuGraph-DataX Introduction](6.utility-tools/7.tugraph-datax.md)

### Offline Data Import

If you have the schema of a subgraph and all the vertex and edge data within the subgraph (in CSV or JSON format), you can use the lgraph_import tool to offline import these data into graph data.

This method is suitable for the initial stage to load a batch of full data. Note that the server must be stopped during the import. Once the import is completed and the server is restarted, you will be able to see the generated subgraph data.

Refer to the `Offline Full Data Import` section in [lgraph_import Usage Introduction](6.utility-tools/1.data-import.md).


## Export Data

### Online Remote Streaming Data Export

[lgraph_cli Introduction](7.client-tools/6.bolt-console-client.md)

CSV

```
echo "match(n:person) return n.id, n.name;" | lgraph_cli --ip 127.0.0.1 --port 7687 --graph default --user admin --password 73@TuGraph --format csv > output.txt
```

JSON

```
echo "match(n:person) return n.id, n.name;" | lgraph_cli --ip 127.0.0.1 --port 7687 --graph default --user admin --password 73@TuGraph --format json > output.txt
```

### Locally Export All Data from the Entire Graph

[lgraph_export Introduction](6.utility-tools/2.data-export.md)

CSV

```
lgraph_export -d your_db_path -e export_data -g default -f json -u admin -p 73@TuGraph
```

JSON

```
lgraph_export -d your_db_path -e export_data -g default -f json -u admin -p 73@TuGraph
```

### Bolt Cluster Mode

[Cluster Configuration](5.installation&running/9.bolt_cluster.md)

[Operational Practice](5.installation&running/10.bolt_cluster_practice.md)

### Cross-Version Data Migration

[lgraph_export 使用介绍](6.utility-tools/2.data-export.md)

For migrating data between incompatible versions, you can use lgraph_export to export all the data from the old version server into text files (which include the graph schema as well as all vertex and edge data). Then, choose an appropriate method to import these text files into the new version.
