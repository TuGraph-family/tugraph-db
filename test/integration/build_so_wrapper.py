import os
import shlex
import subprocess
import random
import string

class BuildSoWrapper:
    def __init__(self, cmd):
        self.cmd = shlex.split(cmd)
        self.log_file = "".join(random.sample(string.ascii_letters + string.digits, 8)) + ".log"

    def check_so(self, so_name):
        lscmd = "ls {}".format(so_name)
        file = os.popen(lscmd)
        res = file.read().strip()
        if res == so_name:
                return True
        return False

    def build_so(self, so_name):
        with open(self.log_file, 'w+') as file:
            self.process = subprocess.Popen(self.cmd, stdout=file, stderr=file, close_fds=True)
            self.process.wait()
            if self.check_so(so_name):
                return
            file.seek(0, 0)
            for line in file:
                log.info(line)
            raise Exception('Failed to build {} '.format(so_name))

    def remove_so(self, so_name):
        if os.path.exists(so_name):
            os.remove(so_name)
        if os.path.exists(self.log_file):
            os.remove(self.log_file)