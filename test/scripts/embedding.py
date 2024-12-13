import time
from neo4j import GraphDatabase, basic_auth
from  multiprocessing import Pool
import numpy as np

'''
import numpy as np
filename = 'chunk.txt'
with open(filename, 'w', encoding='utf-8') as f:
    for id in range(1, 1000000):
        line = 'CREATE (:Chunk {id:%d, embedding:%s})\n' %(id)
        f.write(line)
'''

def create_vector_index():
    url = "bolt://{}:{}".format("127.0.0.1", "7687")
    auth_token = basic_auth("admin", "73@TuGraph")
    driver = GraphDatabase.driver(url, auth=auth_token, encrypted=False)
    session = driver.session(database="default")
    indexes = [
        "CALL db.index.vector.createNodeIndex('chunk_embeding','Chunk', 'embedding', {dimension:1000})",
    ]
    for index in indexes:
        session.run(index)

def create_vertex(i):
    # split --numeric-suffixes --suffix-length=2 -l 100000 chunk.txt chunk-
    url = "bolt://{}:{}".format("127.0.0.1", "7687")
    auth_token = basic_auth("admin", "73@TuGraph")
    driver = GraphDatabase.driver(url, auth=auth_token, encrypted=False)
    session = driver.session(database="default")
    filename = './chunk_' + str(i) + ".txt"
    with open(filename, 'r', encoding='utf-8') as file:
        for line in file:
            line = line.strip()
            session.run(line, embedding=np.random.rand(1000).tolist())

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


