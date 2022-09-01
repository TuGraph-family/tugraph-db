import requests
import sys
import json
from requests.exceptions import Timeout


class request_exception(Exception):
    def __init__(self, message, r):
        self.response = r
        super(request_exception, self).__init__(message)


class db():
    def __init__(self, host, port, username, password, graph):
        self.http_headers = {"Content-Type": "application/json", "Accept": "application/json"}
        self.graph = graph
        self.login(host, port, username, password, graph)

    def login(self, host, port, username, password, graph):
        login_url = "http://{IP}:{PORT}/login".format(IP=host, PORT=port)
        j_data = {}
        j_data["user"] = username
        j_data["password"] = password
        r = requests.post(url=login_url, json=j_data)
        if r.status_code != requests.codes.ok:
            raise request_exception("Error occured during Post request", r)
        jwt = r.json()["jwt"]
        self.jwt = jwt
        self.http_headers["Authorization"] = "Bearer " + jwt
        self.cypher_url = "http://{IP}:{PORT}/cypher".format(IP=host, PORT=port)
        self.root_url = "http://{IP}:{PORT}".format(IP=host, PORT=port)
        self.graph = graph

    def post_cypher_request(self, data, timeout=None):
        dest_url = self.cypher_url
        data["graph"] = self.graph
        r = requests.post(url=dest_url, headers=self.http_headers, json=data, timeout=timeout)
        if r.status_code != requests.codes.ok:
            raise request_exception("Error occured during Post request", r)
        return r

    def post_root_request(self, data, dest, timeout=None):
        dest_url = self.root_url + dest
        r = requests.post(url=dest_url, headers=self.http_headers, json=data, timeout=timeout)
        if r.status_code != requests.codes.ok:
            raise request_exception("Error occured during Post request", r)
        return r

    def get_root_request(self, dest, timeout=None):
        dest_url = self.root_url + dest
        r = requests.get(url=dest_url, headers=self.http_headers, timeout=timeout)
        if r.status_code != requests.codes.ok:
            raise request_exception("Error occured during Get request", r)
        return r
