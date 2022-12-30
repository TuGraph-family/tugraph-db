import os
import sys
import asyncio
import warnings
import unittest

from TuGraphClient import AsyncTuGraphClient, TuGraphClient
# from TestTools import *


def async_test(coro):
    def wrapper(*args, **kwargs):
        warnings.simplefilter("ignore", ResourceWarning)
        loop = asyncio.get_event_loop()
        return loop.run_until_complete(coro(*args, **kwargs))
    return wrapper


class ServerProcessTest(unittest.TestCase):
    def test_start(self):
        executable = '.\server.exe' if os.name == 'nt' else './lgraph_server'
        conf = GlobalConfig()
        conf.db_dir = './testdb'
        p = ServerProcess(executable, './lgraph_standalone.json', conf)
        self.assertTrue(p.expect('Server started.') is not None)
        p.kill()


class AsyncTuGraphClientTest(unittest.TestCase):

    def setUp(self):
        self.url = 'localhost:7071'
        self.client = TuGraphClient(self.url, 'admin', '73@TuGraph')
        self.aclient = AsyncTuGraphClient(self.url, 'admin', '73@TuGraph')

    @async_test
    async def test_async_list_graph(self):
        graphs = await self.aclient.list_graphs()
        self.assertTrue('default' in graphs)

    def test_async_list_graph(self):
        graphs = self.client.list_graphs()
        self.assertTrue('default' in graphs)

    @async_test
    async def test_list_plugins(self):
        cpp_plugins = await self.aclient.list_plugins('cpp')
        py_plugins = await self.aclient.list_plugins('py')
        self.assertEqual([], cpp_plugins)
        self.assertEqual([], py_plugins)

    def test_list_plugins(self):
        cpp_plugins = self.client.list_plugins('cpp')
        py_plugins = self.client.list_plugins('py')
        self.assertEqual([], cpp_plugins)
        self.assertEqual([], py_plugins)

    @async_test
    async def test_async_get_server_info(self):
        server_info = await self.aclient.get_server_info()
        self.assertTrue('cpu' in server_info)
        self.assertTrue('db_config' in server_info)
        self.assertTrue('db_space' in server_info)
        self.assertTrue('lgraph_version' in server_info)
        self.assertTrue('memory' in server_info)
        self.assertTrue('up_time' in server_info)

    def test_get_server_info(self):
        server_info = self.client.get_server_info()
        self.assertTrue('cpu' in server_info)
        self.assertTrue('db_config' in server_info)
        self.assertTrue('db_space' in server_info)
        self.assertTrue('lgraph_version' in server_info)
        self.assertTrue('memory' in server_info)
        self.assertTrue('up_time' in server_info)

    def test_get_server_list(self):
        server_list = self.client.get_server_list()
        self.assertTrue(len(server_list) > 0)
        self.assertTrue(self.url in server_list)

    @async_test
    async def test_async_cypher(self):
        result = await self.aclient.call_cypher('match (a) return a limit 1')
        self.assertTrue('elapsed' in result)
        self.assertTrue('header' in result)
        self.assertTrue('result' in result)
        self.assertTrue('size' in result)
        self.assertEqual([{'name': 'a', 'type': 1}], result['header'])

    def test_cypher(self):
        result = self.client.call_cypher('match (a) return a limit 1')
        self.assertTrue('elapsed' in result)
        self.assertTrue('header' in result)
        self.assertTrue('result' in result)
        self.assertTrue('size' in result)
        self.assertEqual([{'name': 'a', 'type': 1}], result['header'])


if __name__ == '__main__':
    unittest.main()
