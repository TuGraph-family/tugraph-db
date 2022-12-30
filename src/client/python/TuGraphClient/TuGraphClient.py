#!/bin/python3

import json
import asyncio
import base64
import logging
import warnings
from functools import partial
import httpx


requests = httpx.AsyncClient()
warnings.simplefilter("ignore", UserWarning)


# TODO: implement load balancing
class AsyncTuGraphClient:
    # 默认的客户端使用异步方式访问

    def __init__(self, start_host_port, username, password, graph='default', use_https=False, load_balance=False,
                 retry=3, retry_interval_s=1):
        self.http_headers = {"Content-Type": "application/json", "Accept": "application/json"}
        self.curr_server = start_host_port
        self.view_version = -1
        self.username = username
        self.password = password
        self.graph = graph
        self.load_balance = load_balance
        self.use_https = use_https
        self.retry = retry
        self.retry_interval_s = retry_interval_s
        # login
        self.servers = []
        self.master = ''
        self.worker_threads = []
        self._sync(self.__login_and_get_host_list__)

    def _sync(self, func):
        warnings.simplefilter("ignore", DeprecationWarning)
        return asyncio.get_event_loop().run_until_complete(func())

    async def __get_result_with_retry__(self, get_func, retries=None, retry_interval_s=None, raise_on_error=True):
        '''
        Retry getting results with get_func
        If connection error, retry.
        If cannot get response after retrying, return (-1, error_message)
        If 307: redirect
        If 400 or 500: raise if raise_on_error=True, otherwise return (status_code, error_message)
        If failed after retry, raise if raise_on_error=True, otherwise return (status_code, error_message)
        If 200: return (200, json)

        Return json or (status_code, error_message)
        '''
        if retries is None:
            retries = self.retry
        if retry_interval_s is None:
            retry_interval_s = self.retry_interval_s

        for i in range(0, retries):
            try:
                r = await get_func()
                js = {}
                try:
                    js = json.loads(r.text or "{}")
                except Exception as e:
                    logging.error(e)
                if r.status_code == 307:
                    location = js['location']
                    self.curr_server = location
                    logging.info('Redirected to server: {}'.format(location))
                    continue
                if 'server_version' in r.headers:
                    self.view_version = max(int(r.headers['server_version']), self.view_version)
                    self.http_headers['server_version'] = str(self.view_version)
                if r.status_code == 400 or r.status_code == 500:
                    # if raise, just raise exception
                    # otherwise, return 
                    if raise_on_error:
                        raise Exception('Error returned from server, code=[{}], msg=[{}]'.format(
                            r.status_code,
                            js['error_message']
                        ))
                    else:
                        return (r.status_code, js['error_message'])
                if r.status_code == 200:
                    return (200, js)
                else:
                    return (r.status_code, 'Unexpected return code from server: {}'.format(r.status_code))
            except Exception as e:
                logging.exception(e)
                msg = 'Failed to get response from server after retrying: {}'.format(e)
                if (i == retries - 1):
                    if raise_on_error:
                        raise IOError(msg)
                    else:
                        return (-1, msg)
                else:
                    await asyncio.sleep(retry_interval_s)

    async def __get_with_retry__(self, relative_url, return_json_only=True):
        '''
        Returns json response if return_json_only=True, otherwise return (status_code, json/err_msg)
        '''
        get_func = partial(requests.get, url=self.__get_url_base__() + relative_url, headers=self.http_headers)
        r = await self.__get_result_with_retry__(get_func, raise_on_error=return_json_only)
        if return_json_only:
            return r[1]
        else:
            return r

    async def __post_with_retry__(self, relative_url, data_dict, return_json_only=True):
        '''
        Returns json response if return_json_only=True, otherwise return (status_code, json/err_msg)
        '''
        get_func = partial(
            requests.post,
            url=self.__get_url_base__() + relative_url,
            headers=self.http_headers,
            json=data_dict
        )
        r = await self.__get_result_with_retry__(get_func, raise_on_error=return_json_only)
        if return_json_only:
            return r[1]
        else:
            return r

    async def __del_with_retry__(self, relative_url):
        '''
        Raises if there is error.
        '''
        get_func = partial(requests.delete, url=self.__get_url_base__() + relative_url, headers=self.http_headers)
        return await self.__get_result_with_retry__(get_func, True)

    async def __try_get__(self, relative_url):
        '''
        Returns (status_code, json/err_msg)
        '''
        get_func = partial(requests.get, url=self.__get_url_base__() + relative_url, headers=self.http_headers)
        return await self.__get_result_with_retry__(get_func, retries=1, raise_on_error=False)

    async def __try_post__(self, relative_url, data_dict):
        '''
        Returns (status_code, json/err_msg)
        '''
        get_func = partial(
            requests.post,
            url=self.__get_url_base__() + relative_url,
            headers=self.http_headers,
            json=data_dict,
        )
        return await self.__get_result_with_retry__(get_func, retries=1, raise_on_error=False)

    async def __try_del__(self, relative_url):
        '''
        Returns (status_code, json/err_msg)
        '''
        get_func = partial(requests.delete, url=self.__get_url_base__() + relative_url, headers=self.http_headers)
        return await self.__get_result_with_retry__(get_func, retries=1, raise_on_error=False)

    def __get_url_base__(self):
        return ('https://{}/' if self.use_https else 'http://{}/').format(self.curr_server)

    async def __login_and_get_host_list__(self):
        # try:
        j_data = {}
        j_data["user"] = self.username
        j_data["password"] = self.password
        r = await self.__post_with_retry__('login', j_data)
        jwt = r['jwt']
        self.http_headers["Authorization"] = "Bearer " + jwt
        # except Exception as e:
        #    raise IOError('Failed to login to server {}: {}'.format(self.curr_server, e))
        try:
            r = await self.__get_with_retry__('info/ha_state')
            if r == 'NO_HA':
                self.servers = [self.curr_server]
                self.master = self.curr_server
                return
            # in ha mode
            r = await self.__get_with_retry__('info/peers')
            self.servers = [p['rest_address'] for p in r]
            r = await self.__get_with_retry__('info/leader')
            self.master = r['rest_address']
        except Exception as e:
            raise IOError('Failed to get peer and master info: {}'.format(e))

    def get_curr_server(self):
        return self.curr_server

    def get_server_list(self):
        return self.servers

    def get_curr_master(self):
        return self.master

    async def list_graphs(self):
        r = await self.__get_with_retry__('db')
        return list(r.keys())

    def set_curr_graph(self, graph):
        self.graph = graph

    async def call_cypher(self, cypher, raw_output=False, timeout=0):
        data = {"script": cypher, "graph": self.graph, "timeout": timeout}
        if raw_output:
            r = await self.__try_post__('cypher', data)
        else:
            r = await self.__post_with_retry__('cypher', data)
        return r

    async def load_plugin(self, name, desc, file_type, file_path, read_only, raw_output=False):
        '''
        Load a plugin from local file.
        mode: can be 'zip', 'cpp', 'so', 'py'

        Returns: (True, '') if success, (False, err_msg) if error
        '''
        with open(file_path, 'rb') as f:
            content = f.read()
        data = {}
        data['name'] = name
        data['code_base64'] = base64.b64encode(content).decode()
        data['description'] = desc
        data['read_only'] = read_only
        data['code_type'] = file_type
        if file_type == 'cpp' or file_type == 'zip' or file_type == 'so':
            url = 'cpp_plugin'
        elif file_type == 'py':
            url = 'python_plugin'
        else:
            err = 'Invalid file type: [{}]'.format(file_type)
            if raw_output:
                return (False, err)
            else:
                raise Exception(err)
        plugin_url = "db/{}/{}".format(self.graph, url)
        if raw_output:
            r = await self.__try_post__(plugin_url, data)
        else:
            r = await self.__post_with_retry__(plugin_url, data)
        return r

    def __get_plugin_url__(self, plugin_type_str, raw_output):
        if plugin_type_str == 'cpp':
            url = 'cpp_plugin'
            return (url, None)
        elif plugin_type_str == 'py':
            url = 'python_plugin'
            return (url, None)
        else:
            err = 'Invalid plugin type: [{}]'.format(plugin_type_str)
            if raw_output:
                return (None, err)
            else:
                raise Exception(err)

    async def call_plugin(self, plugin_type, plugin_name, input, raw_output=False, timeout=0):
        '''
        Calls a plugin

        plugin_type: type of the plugin, could be 'cpp' or 'py'
        plugin_name: name of the plugin
        input: a str

        Returns result of the call as a str
        '''
        url = self.__get_plugin_url__(plugin_type, raw_output)
        if url[0] is None:
            return (False, url[1])
        url = url[0]
        data = {"data": input, "timeout": timeout}
        plugin_url = "db/{}/{}/{}".format(self.graph, url, plugin_name)
        if raw_output:
            r = await self.__try_post__(plugin_url, data)
        else:
            r = await self.__post_with_retry__(plugin_url, data)
        return r

    async def del_plugin(self, plugin_type, plugin_name, raw_output=False):
        url = self.__get_plugin_url__(plugin_type, raw_output)
        if url[0] is None:
            return (False, url[1])
        url = url[0]
        plugin_url = "db/{}/{}/{}".format(self.graph, url, plugin_name)
        if raw_output:
            r = await self.__try_del__(plugin_url)
        else:
            r = await self.__del_with_retry__(plugin_url)
        return r

    async def list_plugins(self, plugin_type, raw_output=False):
        url = self.__get_plugin_url__(plugin_type, raw_output)
        if url[0] is None:
            return (False, url[1])
        url = url[0]
        plugin_url = "db/{}/{}".format(self.graph, url)
        if raw_output:
            r = await self.__try_get__(plugin_url)
        else:
            r = await self.__get_with_retry__(plugin_url)
        return r

    async def get_plugin_info(self, plugin_type, plugin_name, raw_output=False):
        url = self.__get_plugin_url__(plugin_type, raw_output)
        if url[0] is None:
            return (False, url[1])
        url = url[0]
        plugin_url = "db/{}/{}/{}".format(self.graph, url, plugin_name)
        if raw_output:
            r = await self.__try_get__(plugin_url)
        else:
            r = await self.__get_with_retry__(plugin_url)
        return r

    async def get_server_info(self):
        return await self.__get_with_retry__('info')


class TuGraphClient(AsyncTuGraphClient):
    # 还是暴露之前的TuGraphClient
    # 将之前暴露的8个接口重新封装成可以直接同步方式调用

    def list_graphs(self):
        return self._sync(partial(AsyncTuGraphClient.list_graphs, self))

    def call_cypher(self, cypher, raw_output=False, timeout=0):
        return self._sync(partial(AsyncTuGraphClient.call_cypher, self, cypher, raw_output=raw_output, timeout=timeout))

    def load_plugin(self, name, desc, file_type, file_path, read_only, raw_output=False):
        return self._sync(partial(AsyncTuGraphClient.load_plugin, self, name, desc, file_type, file_path, read_only, raw_output=raw_output))

    def call_plugin(self, plugin_type, plugin_name, input, raw_output=False, timeout=0):
        return self._sync(partial(AsyncTuGraphClient.call_plugin, self, plugin_type, plugin_name, input, raw_output=raw_output, timeout=timeout))

    def del_plugin(self, plugin_type, plugin_name, raw_output=False):
        return self._sync(partial(AsyncTuGraphClient.del_plugin, self, plugin_type, plugin_name, raw_output=raw_output))

    def list_plugins(self, plugin_type, raw_output=False):
        return self._sync(partial(AsyncTuGraphClient.list_plugins, self, plugin_type, raw_output=raw_output))

    def get_plugin_info(self, plugin_type, plugin_name, raw_output=False):
        return self._sync(partial(AsyncTuGraphClient.get_plugin_info, self, plugin_type, plugin_name, raw_output=raw_output))

    def get_server_info(self):
        return self._sync(partial(AsyncTuGraphClient.get_server_info, self))

