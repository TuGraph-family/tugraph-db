#!/bin/python

import argparse
import python_client

ip = ''
port = ''
graph = ''
user = ''
password = ''
cypher = ''

def parse_args():
    parser = argparse.ArgumentParser(description="TuGraph Rpc Client for python")
    parser.add_argument('-i', '--ip', help='ip for graph server')
    parser.add_argument('-p', '--port', help='port for graph server')
    parser.add_argument('-g', '--graph', help='graph name')
    parser.add_argument('-u', '--user', help='user name')
    parser.add_argument('-c', '--cypher', help='cypher to query')
    parser.add_argument('--password', help='user password')
    args = parser.parse_args()
    if args.ip:
        global ip
        ip = args.ip
    if args.port:
        global port
        port = args.port
    if args.graph:
        global graph
        graph = args.graph
    if args.user:
        global user
        user = args.user
    if args.password:
        global password
        password = args.password
    if args.cypher:
        global cypher
        cypher = args.cypher


def call_cypher():
    url = ip + ":" + port
    c = python_client.client(url, user, password)
    ret = c.callCypher(cypher, graph)
    if ret[0]:
        print("query succeed, result :")
        print(ret[1])
    else:
        print("query failed, because :")
        print(ret[1])

if  __name__ == '__main__':
    parse_args()
    call_cypher()