import time
import random
from neo4j import GraphDatabase, basic_auth
from  multiprocessing import Pool

thread_num=20
vertex_num=100000
edge_num = vertex_num*10
desc = \
    '''
    图是更自然的关系表达方式，不需要进行分表操作。图本身是万物互联的关系，使得数据所见即所存。实际上看到的模型和存储到数据库中是一致的。图具有更强的兼容性，在数据治理方面，可以很好的处理传统关系型数据库中庞大复杂的表之间关系。图的点、边不需要太多限制，就可以轻松把各种异构数据放到同个图中。举个例子，图能够将蛋白质的关联关系、分子结构以及已有研究放到同一个图中，图模型可以容纳更多的数据源，并轻松的处理它们之间的关系，当数据量足够时便可以轻松地把这些数据都使用起来，推导出更好的结果。此外，图能做很多更灵活的算法设计，比如寻找路径、发现群体等
    '''

def create_index():
    url = "bolt://{}:{}".format("127.0.0.1", "7687")
    auth_token = basic_auth("admin", "73@TuGraph")
    driver = GraphDatabase.driver(url, auth=auth_token, encrypted=False)
    session = driver.session(database="default")
    v_schema = '''
{
	"label": "Person",
	"primary": "id",
	"type": "VERTEX",
	"detach_property": true,
	"properties": [{
		"name": "id",
		"type": "INT64",
		"optional": false
	}, {
		"name": "desc",
		"type": "STRING",
		"optional": false
	}]
}
    '''
    session.run("CALL db.createVertexLabelByJson('%s')" %(v_schema))
    e_schema = '''
{
  "label": "like",
  "type": "EDGE",
  "detach_property": true,
  "constraints": [
    ["Person", "Person"]
  ],
  "properties": [{
    "name": "desc",
    "type": "STRING",
    "optional": false
  }]
}
    '''
    session.run("CALL db.createEdgeLabelByJson('%s')" %(e_schema))

def create_vertex(index):
    batch = int(vertex_num / thread_num)
    start = int(index * batch)
    url = "bolt://{}:{}".format("127.0.0.1", "7687")
    auth_token = basic_auth("admin", "73@TuGraph")
    driver = GraphDatabase.driver(url, auth=auth_token, encrypted=False)
    session = driver.session(database="default")
    for i in range(batch):
        cypher = "Create(n:Person {id:%d, desc: $desc_str})" %(start + i)
        session.run(cypher, desc_str=desc)

def create_edge(index):
    batch= int(vertex_num / thread_num)
    start = int(index * batch)
    end = int(start + batch - 1)
    url = "bolt://{}:{}".format("127.0.0.1", "7687")
    auth_token = basic_auth("admin", "73@TuGraph")
    driver = GraphDatabase.driver(url, auth=auth_token, encrypted=False)
    session = driver.session(database="default")
    for i in range(int(edge_num/thread_num)):
        cypher = "Match (n1:Person {id:$start_vid}), (n2:Person {id:$end_vid}) Create(n1)-[r:like {desc:$desc_str}]->(n2)"
        session.run(cypher, start_vid=random.randint(start, end), end_vid=random.randint(start, end), desc_str=desc)

def create_vertexes():
    res_list=[]
    pool = Pool(thread_num)
    for i in range(thread_num):
        res = pool.apply_async(func=create_vertex, args=(i,))
        res_list.append(res)
    for res in res_list:
        print(res.get())
    pool.close()
    pool.join()

def create_edges():
    res_list=[]
    pool = Pool(thread_num)
    for i in range(thread_num):
        res = pool.apply_async(func=create_edge, args=(i,))
        res_list.append(res)
    for res in res_list:
        print(res.get())
    pool.close()
    pool.join()

if __name__ == '__main__':
    t1 = time.time()
    create_index()
    create_vertexes()
    t2 = time.time()
    elapsed_time = t2 - t1
    print(f"create_vertexes cost: {elapsed_time:.6f} seconds", flush=True)
    create_edges()
    t3 = time.time()
    elapsed_time = t3 - t2
    print(f"create_edges cost: {elapsed_time:.6f} seconds", flush=True)