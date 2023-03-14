#!/usr/bin/python3
import click
import sys
import time
import os
import json
import requests
import getpass
import six

from prettytable import PrettyTable
from color_format import *
from db import *
from shell_object import *
from requests.exceptions import Timeout
from prompt_toolkit import PromptSession, prompt
from prompt_toolkit.auto_suggest import AutoSuggestFromHistory


@click.command()
@click.option("-example", "--example_info", default=False, is_flag=True, help="show command example")
@click.option("-c", "--config", default="", help="server cofig file path, support .json file type")
@click.option("-g", "--graph", default="default", help="graph name")
@click.option("-h", "--host", default="127.0.0.1", help="host IP, default value is 127.0.0.1")
@click.option("-p", "--port", default="7071", help="server REST port number, default value is 7071")
@click.option("-u", "--username", default="admin", help="username, default value is \"admin\"")
@click.option("-P", "--password", default="", help="password, default value is empty")
@click.option("-f", "--cypher", default="",
              help="cyper command file, support .json http request input or file that contain cypher commands")
@click.option("-s", "--cypher_query", default="", help='do one line cypher query, example -s "MATCH (n) RETURN n"')
@click.option("-t", "--timeout", default=150, help="timeout value for cypher queries, default value is 150 seconds")
@click.option("-format", "--return_format", default="table",
              help="query result format, support table, plain, json style format")
def start(example_info, username, host, port, timeout, password, config, cypher, cypher_query, return_format, graph):
    if example_info:
        click.secho("------------------")
        click.secho("usage:")
        click.secho(
            "[python lgraph_cypher.py -c {server config file path} -u {user name} -P {password} -h {host ip} -p {port number} -f {cypher command file path} -t {timeout value for cypher query} -s {cypher query command}")
        click.secho("------------------")
        click.secho("cofig file login example:")
        click.secho(
            "python lgraph_cypher.py -c /home/usr/code/TuGraph/build/lgraph_standalone.json -u admin -P 73@TuGraph -t 300")
        click.secho("cypher file query example:")
        click.secho(
            "python lgraph_cypher.py -c /home/usr/code/TuGraph/build/lgraph_standalone.json -u admin -P 73@TuGraph -f /home/usr/FMA_shell/cypher.json -t 300")
        click.secho("cypher one line example:")
        click.secho(
            "python lgraph_cypher.py -c /home/usr/code/TuGraph/build/lgraph_standalone.json -u admin -P 73@TuGraph -t 300 -s \"MATCH (n) RETURN n\" ")
        click.secho("------------------")
        sys.exit()

    if config:
        with open(config, "r") as con:
            c = json.load(con)
            if "host" in c.keys():
                host = c["host"]
            if "port" in c.keys():
                port = c["port"]
            if "graph" in c.keys():
                graph = c["graph"]
        con.close()

    if not username or not password:
        username = six.moves.input("User: ")
        password = getpass.getpass("Password: ")

    if return_format not in set(["plain", "table", "json"]):
        print(term_color.ERRORC + "unsupported value for format. Expecting \"table\" or \"plain\" or \"json\" " + term_color.ENDC)
        sys.exit(1)

    try:
        f = FMA_shell(host, port, username, password, timeout, return_format, graph)
        if cypher:
            cypher_r = f.query_from_file(cypher, timeout)
            print_format_response(cypher_r, return_format)
            sys.exit()
        elif cypher_query:
            cypher_r = f.query_one_line(cypher_query)
            print_format_response(cypher_r, return_format)
            sys.exit()
        f.print_welcome()
    except request_exception as e:
        response = e.response
        print(term_color.WARNINGC + str(e) + term_color.ENDC)
        print(term_color.ERRORC + "Error occured:" + term_color.ENDC)
        print(term_color.ERRORC + "++++" + term_color.ENDC + response.json()[
            "error_message"] + term_color.ERRORC + "++++" + term_color.ENDC)
    except Timeout:
        print(term_color.ERRORC + "Request time out, timeout" + term_color.ENDC)
    except Exception as e:
        print(term_color.ERRORC + str(e) + term_color.ENDC)
    except OSError as e:
        print(term_color.ERRORC + str(e) + term_color.ENDC)
    else:
        f.test_run()


if __name__ == "__main__":
    start()
