import os
import sys
import unittest

from TuGraphClient import TuGraphClient
from TestTools import *


class ServerProcessTest(unittest.TestCase):
    def test_start(self):
        executable = '.\server.exe' if os.name == 'nt' else './lgraph_server'
        conf = GlobalConfig()
        conf.db_dir = './testdb'
        p = ServerProcess(executable, './lgraph_standalone.json', conf)
        self.assertTrue(p.expect('Server started.') is not None)
        p.kill()


class TuGraphClientTest(unittest.TestCase):
    def test_login(self):
        c = TuGraphClient('localhost:7071', 'admin', '73@TuGraph')


if __name__ == '__main__':
    unittest.main()
