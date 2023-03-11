import click
import sys
import time
import os
import json
import requests
import datetime

from prettytable import PrettyTable
from color_format import *
from db import *
from requests.exceptions import Timeout
from prompt_toolkit import PromptSession, prompt
from prompt_toolkit.auto_suggest import AutoSuggestFromHistory


class user_exit_exception(Exception):
    pass


def print_format_response(r, return_format):
    j_data = r.json()
    time = j_data["elapsed"]
    size = j_data["size"]
    if return_format == "table":
        p_table = PrettyTable(["id"] + [query_labels["name"] for query_labels in j_data["header"]])
        for index, i in enumerate(j_data["result"]):
            p_table.add_row([index] + i)
        p_table.align = "l"
        print(p_table)
        print("time spent: {}".format(time))
        print("size of query: {}".format(size))
    elif return_format == "plain":
        for index, i in enumerate(j_data["header"]):
            print(i["name"] + ":")
            result = [item[index] for item in j_data["result"]]
            print("\n".join(map(str, result)))
    elif return_format == "json":
        print(json.dumps(j_data, indent=4))


class FMA_shell():
    def __init__(self, host, port, username, password, timeout, return_format, graph):
        self.host = host
        self.port = port
        self.username = username
        self.password = password
        self.timeout = timeout
        self.return_format = return_format
        self.db = db(host, port, username, password, graph)
        self.graph = graph
        r = self.db.get_root_request("/info", timeout=self.timeout)
        self.db_version = r.json()["lgraph_version"]

    def print_help(self):
        click.secho("-----------help message-----------")
        click.secho("Host: {0:<10}".format(self.host))
        click.secho("Port: {0:<10}".format(self.port))
        click.secho("Version: {0:<10}".format(self.db_version))
        click.secho("Username: {0:<10}".format(self.username))
        click.secho("----------------------------------")
        click.secho("Supported commands:")
        click.secho(":help")
        click.secho(":db_info")
        click.secho(":clear")
        click.secho(":use {graph_name}")
        click.secho(
            ":source -t {timeout value, default value is the same as the timeout value set during login} -f {cypher command file}")
        click.secho(":exit")
        click.secho(":format {plain/table}")
        click.secho(":save {all/command/result} -f {save file path} {cypher query command}")
        click.secho("----------------------------------")
        click.secho("Cypher query language:")
        click.secho(
            "Enter cypher command to do database query. Each command ends with \";\". Example: MATCH (n) RETURN n;")

    def print_welcome(self):
        click.secho("**********************************************************************")
        click.secho("*               LightningGraph Graph Database {}                  *".format(self.db_version))
        click.secho("*                                                                    *")
        click.secho("*      Copyright(C) 2018-2021 Ant Group. All rights reserved.        *")
        click.secho("*                                                                    *")
        click.secho("**********************************************************************")
        click.secho("login success")
        click.secho("----------------------------------")
        click.secho("Host: {0:<10}".format(self.host))
        click.secho("Port: {0:<10}".format(self.port))
        click.secho("Username: {0:<10}".format(self.username))
        click.secho("----------------------------------")
        click.secho("type \":help\" to see all commands.")

    def handle_save(self, flag, all_input, file_path=""):
        def parse_save_result(r):
            saved_text = []
            for index, i in enumerate(r.json()["header"]):
                saved_text.append(i["name"] + ":" + "\n")
                if r.json()["result"]: saved_text.extend([str(i[index]) + "\n" for i in r.json()["result"]])
            return saved_text

        if not file_path:
            file_path = "saved_cypher.txt"
            command = " ".join(all_input[2:])
        elif os.path.isdir(file_path):
            file_path = os.path.join(file_path, "saved_cypher.txt")
            command = " ".join(all_input[4:])
        else:
            command = " ".join(all_input[4:])
        if not command:
            print(term_color.ERRORC + "empty cypher command" + term_color.ENDC)
            return
        print(term_color.WARNINGC + "save output to file: %s" % file_path + term_color.ENDC)
        print(term_color.WARNINGC + "current cypher command: %s" % command[:-1] + term_color.ENDC)
        if flag == "command":
            write_text = ["# command: \n" + command[:-1] + "\n"]
        elif flag == "all":
            result = self.query_one_line(command)
            saved_text = parse_save_result(result)
            write_text = ["# command: \n" + command[:-1] + "\n"] + ["#result: \n"] + saved_text
        elif flag == "result":
            result = self.query_one_line(command[:-1])
            saved_text = parse_save_result(result)
            write_text = ["# result: \n"] + saved_text
        with open(file_path, "a") as write_file:
            write_file.write("\n# %s \n" % datetime.datetime.now())
            write_file.writelines(write_text)
        write_file.close()
        print("finished saving to file {}".format(file_path))

    def handle_login(self):
        pass

    def query_from_file(self, f, timeout=None):
        if not timeout:
            timeout = self.timeout
        with open(f, "r") as cypher_file:
            j_data = cypher_file.readlines()
        cypher_file.close()
        if f.endswith(".json") or f.endswith(".JSON"):
            j_data = json.loads(j_data[0].strip().encode('utf8'))
        else:
            j_data = " ".join([x.strip() for x in j_data])
            if not j_data.startswith('{"script"'):
                j_data = {"script": j_data, "graph": self.graph}
        r = self.db.post_cypher_request(j_data, timeout=timeout)
        return r

    def query_one_line(self, line):
        j_data = {"script": line, "graph": self.graph}
        r = self.db.post_cypher_request(j_data, timeout=self.timeout)
        return r

    def handle_text(self, prompt_session, text):
        text = text.strip()
        if not text:
            return
        if text == ":exit":
            raise user_exit_exception
        elif text == ":help":
            self.print_help()
        elif text == ":clear":
            os.system("clear")
        elif text.startswith(":use"):
            all_input = [x.strip() for x in text.split()]
            if (len(all_input) != 2):
                print(
                    term_color.ERRORC + "Syntax error." + term_color.ENDC + " Please follow syntax: \":use {graph_name}\"")
            self.graph = all_input[1]
            self.db = db(self.host, self.port, self.username, self.password, self.graph)
        elif text.startswith(":db_info"):
            all_input = [x.strip() for x in text.split()]
            timeout = self.timeout
            r = self.db.get_root_request("/info", timeout=timeout)
            print(r.text)
        elif text.startswith(":format"):
            all_input = [x.strip() for x in text.split()]
            old_format = self.return_format
            if len(all_input) <= 1 or len(all_input) > 2 or all_input[1] not in set(["plain", "table"]):
                print(
                    term_color.ERRORC + "Syntax error." + term_color.ENDC + " Please follow syntax: \":format {plain or table}\"")
            else:
                self.return_format = all_input[1]
                print("query result format changed from {} to {}".format(old_format, self.return_format))

        elif text.lower().split()[0].startswith(("match", "create", "call", "profile", "explain",
                                                 "MATCH", "CREATE", "CALL", "PROFILE", "EXPLAIN")):
            while not text.endswith(";"):
                text_continue = prompt_session.prompt(u"=>", auto_suggest=AutoSuggestFromHistory())
                text_continue = " " + text_continue
                text += text_continue
            prompt_session.history.append_string(text)
            r = self.query_one_line(text)
            print_format_response(r, self.return_format)

        elif text.startswith(":source"):
            all_input = [x.strip() for x in text.split()]
            timeout = 150
            if "-t" in all_input:
                timeout = int(all_input[all_input.index("-t") + 1])
            if "-f" not in all_input:
                print(
                    term_color.ERRORC + "Syntax error." + term_color.ENDC + " Please follow syntax: \":source -t {timeout value} -f {cypher command file}\"")
            else:
                file_index = all_input.index("-f") + 1
                if file_index >= len(all_input):
                    print(
                        term_color.ERRORC + "Empty file path." + term_color.ENDC + " Please follow syntax: \":source -t {timeout value} -f {cypher command file}\"")
                else:
                    cypher_file = all_input[file_index]
                    r = self.query_from_file(cypher_file, timeout)
                    print_format_response(r, self.return_format)

        elif text.startswith(":save"):
            all_input = [x.strip() for x in text.split()]
            file_path = all_input[all_input.index("-f") + 1] if "-f" in all_input and all_input[2] == "-f" else ""
            flag = all_input[1]
            if flag not in set(["all", "command", "result"]):
                print(
                    term_color.WARNINGC + "save command must start with :save all, :save command or :save result, but got {}".format(
                        ":save " + flag) + term_color.ENDC)
                return
            self.handle_save(flag, all_input, file_path)

        else:
            print("{}Unknow command: \"{}\" {}".format(term_color.ERRORC, text, term_color.ENDC))
            print('type \":help\" to see all commands')

    def send_import_data(self, data):
        return self.db.post_root_request(data, "/import/text", timeout=self.timeout)

    def test_run(self):
        session = PromptSession()
        while True:
            try:
                text = session.prompt(u"{}> ".format(self.graph), auto_suggest=AutoSuggestFromHistory())
                self.handle_text(session, text)
            except KeyboardInterrupt:
                if text:
                    text = ""
                else:
                    print("\nprogram suspended")
                    sys.exit(1)
            except request_exception as e:
                r = e.response
                print(term_color.WARNINGC + str(e) + term_color.ENDC)
                print(term_color.ERRORC + "error occured:" + term_color.ENDC)
                print(term_color.ERRORC + "++++" + term_color.ENDC + r.json()[
                    "error_message"] + term_color.ERRORC + "++++" + term_color.ENDC)
            except Timeout:
                print(term_color.ERRORC + "Request time out, timeout" + term_color.ENDC)
            except user_exit_exception:
                print("exiting")
                sys.exit(1)
            except Exception as e:
                print(term_color.WARNINGC + str(e) + term_color.ENDC)
            except OSError as e:
                print(term_color.WARNINGC + str(e) + term_color.ENDC)
