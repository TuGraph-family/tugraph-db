import time
from neo4j import GraphDatabase, basic_auth
from  multiprocessing import Pool

def create_index():
    url = "bolt://{}:{}".format("127.0.0.1", "7687")
    auth_token = basic_auth("admin", "73@TuGraph")
    driver = GraphDatabase.driver(url, auth=auth_token, encrypted=False)
    session = driver.session(database="default")
    indexes = [
        "CALL db.index.fulltext.createNodeIndex('Message_content', ['Message'], ['content'])",
        
        "CALL db.createUniquePropertyConstraint('City_id', 'City', 'id')",
        "CALL db.createUniquePropertyConstraint('Comment_id', 'Comment', 'id')",
        "CALL db.createUniquePropertyConstraint('Country_id', 'Country', 'id')",
        "CALL db.createUniquePropertyConstraint('Forum_id', 'Forum', 'id')",
        "CALL db.createUniquePropertyConstraint('Message_id', 'Message', 'id')",
        "CALL db.createUniquePropertyConstraint('Organisation_id', 'Organisation', 'id')",
        "CALL db.createUniquePropertyConstraint('Person_id', 'Person', 'id')",
        "CALL db.createUniquePropertyConstraint('Post_id', 'Post', 'id')",
        "CALL db.createUniquePropertyConstraint('Tag_id', 'Tag', 'id')",
        "CALL db.createUniquePropertyConstraint('TagClass_id', 'TagClass', 'id')",
        "CALL db.createUniquePropertyConstraint('Place_id', 'Place', 'id')",
    ]
    for index in indexes:
        session.run(index)

def create_vertex(i):
    # split --numeric-suffixes --suffix-length=2 -l 318173 sf1-1.txt sf1-vertex-
    url = "bolt://{}:{}".format("127.0.0.1", "7687")
    auth_token = basic_auth("admin", "73@TuGraph")
    driver = GraphDatabase.driver(url, auth=auth_token, encrypted=False)
    session = driver.session(database="default")
    filename = '/root/tugraph-db-pro/ldbc/sf1-vertex-' + "{:02}".format(i)
    with open(filename, 'r', encoding='utf-8') as file:
        for line in file:
            line = line.strip()
            session.run(line)

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

def create_edges():
    res_list=[]
    pool = Pool(20)
    for i in range(20):
        res = pool.apply_async(func=create_edge, args=(i,))
        res_list.append(res)
    for res in res_list:
        print(res.get())
    pool.close()
    pool.join()

def create_edge(i):
    # split --numeric-suffixes --suffix-length=2 -l 862802 sf1-2.txt sf1-edge-
    url = "bolt://{}:{}".format("127.0.0.1", "7687")
    auth_token = basic_auth("admin", "73@TuGraph")
    driver = GraphDatabase.driver(url, auth=auth_token, encrypted=False)
    session = driver.session(database="default")
    filename = '/root/tugraph-db-pro/ldbc/sf1-edge-' + "{:02}".format(i)
    with open(filename, 'r', encoding='utf-8') as file:
        for line in file:
            line = line.strip()
            session.run(line)

if __name__ == '__main__':
    start_time = time.time()
    create_index()
    create_vertexes()
    create_edges()
    end_time = time.time()
    elapsed_time = end_time - start_time
    print(f"cost: {elapsed_time:.6f} seconds")


