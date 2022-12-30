
import logging
import pytest
import os
import asyncio
import warnings

log = logging.getLogger(__name__)

SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb --port 7073 --rpc_port 9093",
               "cleanup_dir":["./testdb"]}

RESTTOPT = {"port":"7073", "user":"admin", "password":"73@TuGraph"}

echo_plugin_content = '''
def Process(db, input):
    return (True, input)
'''


def async_test(coro):
    def wrapper(*args, **kwargs):
        warnings.simplefilter("ignore", ResourceWarning)
        loop = asyncio.new_event_loop()
        return loop.run_until_complete(coro(*args, **kwargs))
    return wrapper


class TestRest():

    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("rest", [RESTTOPT], indirect=True)
    @async_test
    def test_get_info(self, server, rest):
        res = await rest.get_server_info()
        log.info("res : %s", res)
        assert('cpu' in res)

    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("rest", [RESTTOPT], indirect=True)
    @async_test
    def test_plugin_load_unload(self, server, rest):
        path = './tmp_plugin.py'
        with open(path, 'w') as f:
            f.write(echo_plugin_content)
        # load success
        await rest.load_plugin(name='echo', desc='an echo plugin',
                            file_type='py', file_path=path,
                            read_only=True,
                            raw_output=False)
        # loading an existing plugin, expect 400
        await r = rest.load_plugin(name='echo', desc='an echo plugin',
                                file_type='py', file_path=path,
                                read_only=True,
                                raw_output=True)
        assert(r[0] == 400)
        # now try calling the plugin
        await r = rest.call_plugin(plugin_type='py', plugin_name='echo', input='hello world')
        assert(r['result'] == 'hello world')
        # list plugins
        r = await rest.list_plugins(plugin_type='py')
        assert(len(r) == 1)
        assert(r[0]['name'] == 'echo')
        # try delete the plugin
        r = await rest.del_plugin(plugin_type='py', plugin_name='echo')
        # now list again, and should get empty list
        r = await rest.list_plugins(plugin_type='py')
        assert(len(r) == 0)
        os.remove(path)
