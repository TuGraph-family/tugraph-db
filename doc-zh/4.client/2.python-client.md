
# TuGraph Python SDK

> 此文档主要街TuGraph Python SDK的使用说明


### **使用示例**

#### 调用Cypher

```python
TuGraphClient import TuGraphClient

client = TuGraphClient("127.0.0.1:7071" , "admin", "73@TuGraph")
cypher = "match (n) return properties(n) limit 1"
res = client.call_cypher(cypher)
print(res)
```

#### 调用存储过程

```python
TuGraphClient import TuGraphClient

client = TuGraphClient("127.0.0.1:7071" , "admin", "73@TuGraph")
plugin_type = "cpp"
plugin_name = "khop"
plugin_input = "{\"root\": 10, \"hop\": 3}"
res = client.call_plugin(plugin_type, plguin_name, plugin_input)
print(res)
```
