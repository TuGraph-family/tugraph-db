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
default_addr = '0.0.0.0:7076'
url_base = 'http://{}{}'


def Login(addr=default_addr):
    r = requests.post(url=url_base.format(addr, '/login'),
                      data=json.dumps({'user': 'admin', 'password': '73@TuGraph'}),
                      headers=headers)
    if (r.status_code != 200):
        logging.error('error logging in: code={}, text={}'.format(r.status_code, r.text))
        sys.exit(1)
    else:
        headers['Authorization'] = 'Bearer ' + json.loads(r.text)['jwt']


def GetAuditLog(addr, begin, end, user, num, order):
    # /info/log/?begin_time=time1&end_time=time2&user=user1&num_log=100
    query = '/info/log/?begin_time={}&end_time={}&user={}&num_log={}&descending_order={}'.format(begin, end, user, num,
                                                                                                 order)
    url = url_base.format(addr, query)
    r = requests.get(url=url, headers=headers)
    # logging.info('r : {}'.format(r.text))
    res_array = json.loads(r.text)
    print('index | begin_time | end_time | user | graph | type | read_write | success | content')
    for res in res_array:
        print('{} | {} | {} | {} | {} | {} | {} | {} | {}'.format(res['index'], res['begin_time'], res['end_time'],
                                                                  res['user'], res['graph'], res['type'],
                                                                  res['read_write'], res['success'], res['content']))


if __name__ == '__main__':
    '''
    Examples:
    $ python lgraph_auditlog.py -b '2020-07-01 00:00:00' -e '2020-07-30 00:00:00' -u 'admin' -n 100 -a '192.168.1.24:7076' -o 1
    '''
    # parse arguments
    ap = argparse.ArgumentParser(description='TuGraph AuditLog Manager',
                                 formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    # begin end user num order
    ap.add_argument('-b', dest='begin', action='store', default='2020-07-01 00:00:00',
                    help='begin time, format:YYYY-mm-dd HH:MM:SS')
    ap.add_argument('-e', dest='end', action='store', default='', help='end time, format:YYYY-mm-dd HH:MM:SS')
    ap.add_argument('-u', dest='user', action='store', default='', help='the operator of the queried log')
    ap.add_argument('-n', dest='num', action='store', type=int, default=10, help='amount of audit log')
    ap.add_argument('-o', dest='order', action='store', type=int, default=1, choices=[0, 1],
                    help='descending order, 1 true, 0 false')
    ap.add_argument('-a', dest='address', action='store', default=default_addr, help='server address')

    args = ap.parse_args()
    begin = args.begin
    end = args.end
    user = args.user
    num = args.num
    address = args.address
    if args.order == 0:
        order = False
    else:
        order = True
    # logging.info('meta info:\nbegin={}\nend={}\nuser={}\nnum={}\naddress={}\norder={}\n'.format(begin, end, user, num, address, order))
    # login
    Login(address)
    GetAuditLog(address, begin, end, user, num, order)
