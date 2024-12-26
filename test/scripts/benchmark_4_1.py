import time
import random
from faker import Faker
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
	"properties": [
	{
		"name": "id",
		"type": "INT64",
		"optional": false
	},
	{
		"name": "name",
		"type": "STRING",
		"optional": false
	},
	{
		"name": "male",
		"type": "BOOL",
		"optional": false
	},
	{
		"name": "address",
		"type": "STRING",
		"optional": false
	},
	{
		"name": "datetime",
		"type": "DATETIME",
		"optional": false
	},
	{
		"name": "desc",
		"type": "STRING",
		"optional": false
	}
	]
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
  "properties": [
	{
		"name": "id",
		"type": "INT64",
		"optional": false
	},
	{
		"name": "name",
		"type": "STRING",
		"optional": false
	},
	{
		"name": "male",
		"type": "BOOL",
		"optional": false
	},
	{
		"name": "address",
		"type": "STRING",
		"optional": false
	},
	{
		"name": "datetime",
		"type": "DATETIME",
		"optional": false
	},
	{
		"name": "desc",
		"type": "STRING",
		"optional": false
	}
  ]
}
    '''
    session.run("CALL db.createEdgeLabelByJson('%s')" %(e_schema))

def create_vertex(index):
    fake = Faker()
    batch = int(vertex_num / thread_num)
    start = int(index * batch)
    url = "bolt://{}:{}".format("127.0.0.1", "7687")
    auth_token = basic_auth("admin", "73@TuGraph")
    driver = GraphDatabase.driver(url, auth=auth_token, encrypted=False)
    session = driver.session(database="default")
    for i in range(batch):
        cypher = ("Create(n:Person {id:%d, name:$arg_name, male:$arg_male, address:$arg_address, datetime:datetime($arg_datetime), desc:$arg_desc})"
                  %(start + i))
        session.run(cypher,
                    arg_name=fake.name(),
                    arg_male=bool(fake.random_int(min=0, max=1)),
                    arg_address=fake.address(),
                    arg_datetime=fake.date_time_this_year().strftime("%Y-%m-%d %H:%M:%S"),
                    arg_desc=fake.sentence(nb_words=20)
                    )

def create_edge(index):
    fake = Faker()
    batch= int(vertex_num / thread_num)
    start = int(index * batch)
    end = int(start + batch - 1)
    url = "bolt://{}:{}".format("127.0.0.1", "7687")
    auth_token = basic_auth("admin", "73@TuGraph")
    driver = GraphDatabase.driver(url, auth=auth_token, encrypted=False)
    session = driver.session(database="default")
    for i in range(int(edge_num/thread_num)):
        cypher = "Match (n1:Person {id:$start_vid}), (n2:Person {id:$end_vid}) Create(n1)-[r:like {id:$arg_id, name:$arg_name, male:$arg_male, address:$arg_address, datetime:datetime($arg_datetime), desc:$arg_desc}]->(n2)"
        session.run(cypher,
                    start_vid=random.randint(start, end),
                    end_vid=random.randint(start, end),
                    arg_id=fake.random_int(),
                    arg_name=fake.name(),
                    arg_male=bool(fake.random_int(min=0, max=1)),
                    arg_address=fake.address(),
                    arg_datetime=fake.date_time_this_year().strftime("%Y-%m-%d %H:%M:%S"),
                    arg_desc=fake.sentence(nb_words=20))

def read(index, cypher):
    url = "bolt://{}:{}".format("127.0.0.1", "7687")
    auth_token = basic_auth("admin", "73@TuGraph")
    driver = GraphDatabase.driver(url, auth=auth_token, encrypted=False)
    session = driver.session(database="default")
    for i in range(10000):
        ret = session.run(cypher, start_id=random.randint(0, vertex_num), desc_str=desc)
        for item in ret.data():
            pass

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

def test_read(cypher):
    res_list=[]
    pool = Pool(thread_num)
    for i in range(thread_num):
        res = pool.apply_async(func=read, args=(i, cypher))
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
    test_read("Match (n:Person {id:$start_id})-[r]->(m) return m")
    test_read("Match (n:Person {id:$start_id})-[r]->(m) return m.id")
    test_read("Match (n:Person {id:$start_id})-[r*2]->(m) return count(m)")
    test_read("Match (n:Person {id:$start_id})-[r*3]->(m) return count(m)")


    t4 = time.time()
    test_read("Match (n:Person {id:$start_id})-[r]->(m) return m")
    t5 = time.time()
    elapsed_time = t5 - t4
    print(f"test_read1 cost: {elapsed_time:.6f} seconds", flush=True)

    t4 = time.time()
    test_read("Match (n:Person {id:$start_id})-[r]->(m) return m.id")
    t5 = time.time()
    elapsed_time = t5 - t4
    print(f"test_read2 cost: {elapsed_time:.6f} seconds", flush=True)

    t4 = time.time()
    test_read("Match (n:Person {id:$start_id})-[r*2]->(m) return count(m)")
    t5 = time.time()
    elapsed_time = t5 - t4
    print(f"test_read3 cost: {elapsed_time:.6f} seconds", flush=True)

    t4 = time.time()
    test_read("Match (n:Person {id:$start_id})-[r*3]->(m) return count(m)")
    t5 = time.time()
    elapsed_time = t5 - t4
    print(f"test_read4 cost: {elapsed_time:.6f} seconds", flush=True)