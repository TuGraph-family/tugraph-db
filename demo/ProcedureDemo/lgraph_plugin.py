import base64
import json
import requests
import sys
import argparse
import logging

def setup_logger(level=logging.INFO):
    '''
    Setup root logger so we can easily use it
    Params:
        level:  string  logging level
    '''
    logging.root.setLevel(level)

setup_logger(logging.INFO)

headers = {'Content-Type': 'application/json'}
default_addr='0.0.0.0:7070'
url_base='http://{}{}'

def Login(addr=default_addr):
    r = requests.post(url=url_base.format(addr, '/login'),
                      data=json.dumps({'user':'admin', 'password':'73@TuGraph'}),
                      headers=headers)
    if (r.status_code != 200):
        logging.error('error logging in: code={}, text={}'.format(r.status_code, r.text))
        sys.exit(1)
    else:
        headers['Authorization'] = 'Bearer ' + json.loads(r.text)['jwt']

def SyncSlave(addr):
    r = requests.post(url=url_base.format(addr, '/misc/sync_meta'), data=json.dumps({}), headers=headers)
    if (r.status_code != 200):
        logging.error('error syncing meta of slave server {}'.format(addr))

def LoadPlugin(name, path, read_only, desc, is_cpp=True, addr=default_addr):
    with open(path, 'rb') as f:
        content = f.read()
    data = {}
    data['name'] = name
    data['code_base64'] = base64.b64encode(content).decode()
    data['description'] = desc
    data['read_only'] = read_only
    js = json.dumps(data)
    if is_cpp:
        url = url_base.format(addr, '/db/default/cpp_plugin')
    else:
        url = url_base.format(addr, '/db/default/python_plugin')
    r = requests.post(url=url, data=js, headers=headers)
    logging.info('LoadPlugin: code={}, text={}'.format(r.status_code, r.text))

def DelPlugin(name, is_cpp=True, addr=default_addr):
    if is_cpp:
        url = url_base.format(addr, '/db/default/cpp_plugin/' + name)
    else:
        url = url_base.format(addr, '/db/default/python_plugin/' + name)
    r = requests.delete(url=url, headers=headers)
    logging.info('DelPlugin: code={}, text={}'.format(r.status_code, r.text))

def ListPlugin(is_cpp=True, addr=default_addr):
    if is_cpp:
        url = url_base.format(addr, '/db/default/cpp_plugin/')
    else:
        url = url_base.format(addr, '/db/default/python_plugin/')
    r = requests.get(url=url, headers=headers)
    logging.info('ListPlugin: code={}, text={}'.format(r.status_code, r.text))

def CallPlugin(name, data='', is_cpp=True, addr=default_addr):
    if is_cpp:
        url = url_base.format(addr, '/db/default/cpp_plugin/' + name)
    else:
        url = url_base.format(addr, '/db/default/python_plugin/' + name)
    dic={}
    dic['data']=data
    r = requests.post(url=url, headers=headers, data=json.dumps(dic))
    logging.info('CallPlugin: code={}, text={}'.format(r.status_code, r.text))

def CreateLabel(addr=default_addr):
    url = url_base.format(addr, '/db/default/label')
    r = requests.post(url=url, headers=headers, data='{"fields":[{"name":"fid","optional":false,"type":"int64"}],"is_vertex":false,"name":"friends"}')
    logging.info('AddLabel: code={}, text={}'.format(r.status_code, r.text))

if __name__ == '__main__':
    '''
    Examples:
    $ python lgraph_plugin.py -c Load -n "scanx" -p "./scan_graph.so" -r 1 -d "get the number of vertice and edges" -a "0.0.0.0:7076"
    INFO:root:LoadPlugin: code=200, text=

    $ python lgraph_plugin.py -c List -a "0.0.0.0:7076"
    INFO:root:ListPlugin: code=200, text={"plugins":[{"description":"get the number of vertice and edges","name":"scanx","read_only":true}]}

    $ python lgraph_plugin.py -c Remove -n "scanx" -a "0.0.0.0:7076"
    INFO:root:DelPlugin: code=200, text=

    # call plugin
    $ curl -XPOST http://{ip}:{port}/db/cpp_plugin/scanx -H 'Accept:application/json' -H 'Content-Type:application/json' \
            -d '{"data":"{\"scan_edges\":true,\"times\":1}"}'
    '''
    # parse arguments
    ap = argparse.ArgumentParser(description='TuGraph CPP Plugin Manager',
                                 formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    ap.add_argument('-c', dest='command', action='store', default='Load',
                    choices=['Load', 'Remove', 'List'], help='command type')
    ap.add_argument('-n', dest='name', action='store', help='plugin name')
    ap.add_argument('-p', dest='path', action='store', default='', help='the url of plugin file')
    ap.add_argument('-r', dest='read_only', action='store', type=int, default=1,
                    choices=[0, 1], help='1 if plugin is read only, 0 otherwise')
    ap.add_argument('-d', dest='description', action='store', default='', help='plugin description')
    ap.add_argument('-a', dest='address', action='store', default=default_addr, help='server address')
    args = ap.parse_args()
    command = args.command
    name = args.name
    path = args.path
    description = args.description
    address = args.address
    if args.read_only == 0:
        read_only = False
    else:
        read_only = True
    # login
    Login(address)
    if command == 'Load':
        LoadPlugin(name, path, read_only, description, True, address)
    elif command == 'Remove':
        DelPlugin(name, True, address)
    elif command == 'List':
        ListPlugin(True, address)
