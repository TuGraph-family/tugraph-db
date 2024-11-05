import time
from neo4j import GraphDatabase, basic_auth
from  multiprocessing import Pool
import uuid

def random_string(length):
    result = str(uuid.uuid4())[:length]
    return result

schema = '''
{
	"label": "node",
	"primary": "id",
	"type": "VERTEX",
	"detach_property": true,
	"properties": [{
		"name": "id",
		"type": "INT64"
	}, {
		"name": "field0",
		"type": "STRING",
		"optional": false
	}, {
		"name": "field1",
		"type": "STRING",
		"optional": false
	}, {
		"name": "field2",
		"type": "STRING",
		"optional": false
	}, {
		"name": "field3",
		"type": "STRING",
		"optional": false
	}, {
		"name": "field4",
		"type": "STRING",
		"optional": false
	}, {
		"name": "field5",
		"type": "STRING",
		"optional": false
	}, {
		"name": "field6",
		"type": "STRING",
		"optional": false
	}, {
		"name": "field7",
		"type": "STRING",
		"optional": false
	}, {
		"name": "field8",
		"type": "STRING",
		"optional": false
	}, {
		"name": "field9",
		"type": "STRING",
		"optional": false
	}]
}
'''

def create_vector_index():
    url = "bolt://{}:{}".format("127.0.0.1", "7687")
    auth_token = basic_auth("admin", "73@TuGraph")
    driver = GraphDatabase.driver(url, auth=auth_token, encrypted=False)
    session = driver.session(database="default")
    ret = session.run("CALL db.createVertexLabelByJson($json_data)", json_data=schema)
    print(ret.data())

def create_vertex(i):
    url = "bolt://{}:{}".format("127.0.0.1", "7687")
    auth_token = basic_auth("admin", "73@TuGraph")
    driver = GraphDatabase.driver(url, auth=auth_token, encrypted=False)
    session = driver.session(database="default")
    filename = '/root/tugraph-db-pro/ldbc/chunk-' + "{:02}".format(i)
    for i in range(i*1000000, (i+1)*1000000):
        param = {}
        param['id'] = i
        for i in range(0, 10):
            param['field'+str(i)] = random_string(100)
        session.run("CALL db.upsertVertex('node', $param)", param=[param])

def create_vertexes():
    res_list=[]
    pool = Pool(10)
    for i in range(10):
        res = pool.apply_async(func=create_vertex, args=(i,))
        res_list.append(res)
    for res in res_list:
        print(res.get())
    pool.close()
    pool.join()

if __name__ == '__main__':
    start_time = time.time()
    create_vector_index()
    create_vertexes()
    end_time = time.time()
    elapsed_time = end_time - start_time
    print(f"cost: {elapsed_time:.6f} seconds")


