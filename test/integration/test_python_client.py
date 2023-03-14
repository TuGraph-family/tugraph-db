import logging
import asyncio
import warnings
import subprocess
import time
from TuGraphClient import AsyncTuGraphClient, TuGraphClient
import os

log = logging.getLogger(__name__)


def async_test(coro):
    def wrapper(*args, **kwargs):
        warnings.simplefilter("ignore", ResourceWarning)
        loop = asyncio.get_event_loop()
        return loop.run_until_complete(coro(*args, **kwargs))
    return wrapper

class TestAsyncTuGraphClient:
    def setup_class(self):
        self.url = 'localhost:7071'
        try:
            # start server in ci command
            self.log = open('./test_async_tugraph_client_out.log', 'w+')
            self.process = subprocess.Popen([
                './lgraph_server',
                '-c', './lgraph_standalone.json'
            ], stdout=self.log, stderr=self.log, close_fds=True)
            time.sleep(3)
        except Exception as e:
            # in dev environment, start server before run tests
            logging.debug(e)

        # init client and async client
        self.client = TuGraphClient(self.url, 'admin', '73@TuGraph')
        self.aclient = AsyncTuGraphClient(self.url, 'admin', '73@TuGraph')

    def teardown_class(self):
        try:
            self.process.kill()
            self.process.wait()
            self.log.close()
            os.remove("./test_async_tugraph_client_out.log")
        except Exception as e:
            logging.debug(e)

    @async_test
    async def test_async_list_graph(self):
        graphs = await self.aclient.list_graphs()
        assert 'default' in graphs

    def test_list_graph(self):
        graphs = self.client.list_graphs()
        assert 'default' in graphs

    @async_test
    async def test_async_list_plugins(self):
        cpp_plugins = await self.aclient.list_plugins('cpp')
        py_plugins = await self.aclient.list_plugins('py')
        assert [] == cpp_plugins
        assert [] == py_plugins

    def test_list_plugins(self):
        cpp_plugins = self.client.list_plugins('cpp')
        py_plugins = self.client.list_plugins('py')
        assert [] == cpp_plugins
        assert [] == py_plugins

    @async_test
    async def test_async_get_server_info(self):
        server_info = await self.aclient.get_server_info()
        assert 'cpu' in server_info
        assert 'db_config' in server_info
        assert 'db_space' in server_info
        assert 'lgraph_version' in server_info
        assert 'memory' in server_info
        assert 'up_time' in server_info

    def test_get_server_info(self):
        server_info = self.client.get_server_info()
        assert 'cpu' in server_info
        assert 'db_config' in server_info
        assert 'db_space' in server_info
        assert 'lgraph_version' in server_info
        assert 'memory' in server_info
        assert 'up_time' in server_info

    def test_get_server_list(self):
        server_list = self.client.get_server_list()
        assert len(server_list) > 0
        assert self.url in server_list

    @async_test
    async def test_async_cypher(self):
        result = await self.aclient.call_cypher('match (a) return a limit 1')
        assert 'elapsed' in result
        assert 'header' in result
        assert 'result' in result
        assert 'size' in result
        assert [{'name': 'a', 'type': 1}] == result['header']

    def test_cypher(self):
        result = self.client.call_cypher('match (a) return a limit 1')
        assert 'elapsed' in result
        assert 'header' in result
        assert 'result' in result
        assert 'size' in result
        assert [{'name': 'a', 'type': 1}] == result['header']