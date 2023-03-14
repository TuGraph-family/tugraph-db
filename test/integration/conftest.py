import os
import time
import pytest
import logging
import liblgraph_client_python
from build_so_wrapper import BuildSoWrapper
from toolkits_wrapper import ToolkitsWrapper
from executable_wrapper import ExecutableWrapper
from server_wrapper import  ServerWrapper
from common import list_directory
from TuGraphClient import TuGraphClient

log = logging.getLogger(__name__)

@pytest.fixture(scope="function")
def server(request):
    log.info("server start")
    cmd = request.param.get("cmd")
    dirs = request.param.get("cleanup_dir")
    server = ServerWrapper(cmd, 'Listening for REST on port')
    server.run(dirs)
    log.info("Server started")
    yield
    server.stop(dirs)
    log.info("Server stoped")

server_1 = server
server_2 = server
server_3 = server

@pytest.fixture(scope="function")
def client(request):
    log.info("client start")
    for i in range(60):
        try:
            c = liblgraph_client_python.client(request.param.get("host"), request.param.get("user"), request.param.get("password"))
            log.info("client succeed")
            break
        except Exception as e:
            time.sleep(1)
            log.info("retry connect %s times", i + 1)
            continue
            raise Exception("client create failed")
    yield c
    c.close()
    log.info("Client stoped")

client_1 = client
client_2 = client
client_3 = client

@pytest.fixture(scope="function")
def importor(request):
    log.info("importor start")
    cmd = request.param.get("cmd")
    dirs = request.param.get("cleanup_dir")
    importor = ToolkitsWrapper(cmd, ['Import finished in'])
    importor.run(dirs)
    log.info("importor succeed")
    yield
    importor.stop(dirs)

importor_1 = importor
importor_2 = importor

@pytest.fixture(scope="function")
def exportor(request):
    log.info("exportor start")
    cmd = request.param.get("cmd")
    dirs = request.param.get("cleanup_dir")
    exportor = ToolkitsWrapper(cmd, ["Export successful in"])
    exportor.run(dirs)
    log.info("exportor succeed")
    yield
    exportor.stop(dirs)

@pytest.fixture(scope="function")
def backup_binlog(request):
    log.info("backup_binlog start")
    cmd = request.param.get("cmd")
    dirs = request.param.get("cleanup_dir")
    binlog = ToolkitsWrapper(cmd, ['Processed'])
    binlog.run(dirs)
    log.info("backup_binlog succeed")
    yield
    binlog.stop(dirs)

@pytest.fixture(scope="function")
def backup_copy_dir(request):
    log.info("backup_binlog start")
    cmd = request.param.get("cmd")
    dirs = request.param.get("cleanup_dir")
    bcd = ToolkitsWrapper(cmd, ['Backing up data from'])
    bcd.run(dirs)
    log.info("backup_binlog succeed")
    yield
    bcd.stop(dirs)

@pytest.fixture(scope="function")
def build_so(request):
    cmd = request.param.get("cmd")
    so_name = request.param.get("so_name")
    cmd_size = len(cmd)
    so_size = len(so_name)
    idx = 0
    if cmd_size != so_size:
        raise Exception('Failed to build so, because cmd count not equal to the so_name cound')
    builders = []
    while idx < cmd_size:
        builder = BuildSoWrapper(cmd[idx])
        builder.build_so(so_name[idx])
        builders.append(builder)
        idx += 1
    yield
    while idx >= 0:
        idx -= 1
        builders[idx].remove_so(so_name[idx])

build_so_1 = build_so
build_so_2 = build_so
build_so_3 = build_so


@pytest.fixture(scope="function")
def copy_snapshot(request):
    src = request.param.get("src") + "/snapshot"
    dst = request.param.get("dst")
    dirs = list_directory(src)
    if len(dirs) != 1:
        raise Exception('copy {} failed, snapshot should be only one'.format(name))
    src = dirs[0]
    log.info("src : %s", src)
    cmd = "cp -r {}/. {}".format(src, dst)
    os.popen(cmd)
    for i in range(1200):
        src_res = os.popen("ls -all {}/".format(src))
        src_res = src_res.read().split("\n")
        log.info("src_res : %s", src_res)
        dst_res = os.popen("ls -all {}/".format(dst))
        dst_res = dst_res.read().split("\n")
        log.info("dst_res : %s", dst_res)
        for name in src_res:
            if name not in dst_res:
                time.sleep(1)
                continue
        return
    raise Exception('copy {} failed'.format(name))

@pytest.fixture(scope="function")
def copydir(request):
    src = request.param.get("src")
    dst = request.param.get("dst")
    cmd = "cp -r {}/. {}".format(src, dst)
    os.popen(cmd)
    for i in range(1200):
        src_res = os.popen("ls -all {}/".format(src))
        src_res = src_res.read().split("\n")
        dst_res = os.popen("ls -all {}/".format(dst))
        dst_res = dst_res.read().split("\n")
        for name in src_res:
            if name not in dst_res:
                time.sleep(1)
                continue
        return
    raise Exception('copy {} failed'.format(name))

@pytest.fixture(scope="function")
def exec(request):
    log.info("exec start")
    cmd = request.param.get("cmd")
    exec = ExecutableWrapper(cmd)
    exec.run()
    if exec.get_ret_code() != 0:
        with open(exec.get_log(), 'r') as file:
            log.info("open %s", exec.get_log())
            for line in file:
                log.info(line)
    exec.stop()
    assert exec.get_ret_code() == 0
    yield
    exec.stop()

@pytest.fixture(scope="function")
def algo(request):
    log.info("algo start")
    cmd = request.param.get("cmd")
    result = request.param.get("result")
    exportor = ToolkitsWrapper(cmd, result)
    exportor.run()
    log.info("exportor succeed")
    yield
    exportor.stop()


@pytest.fixture(scope="function")
def bash(request):
    cmd = request.param.get("cmd")
    bash = ExecutableWrapper(cmd)
    bash.run()
    if bash.get_ret_code() != 0:
        with open(bash.get_log(), 'r') as file:
            for line in file:
                log.info(line)
    bash.stop()
    assert bash.get_ret_code() == 0

bash_1 = bash

@pytest.fixture(scope="function")
def rest(request):
    log.info("client start")
    rest = TuGraphClient('localhost:{}'.format(request.param.get("port")), request.param.get("user"), request.param.get("password"))
    log.info("client succeed")
    yield rest
