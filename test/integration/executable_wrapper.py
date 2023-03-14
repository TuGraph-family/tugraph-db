import os
import shlex
import subprocess
import string
import random
import logging

log = logging.getLogger(__name__)

class ExecutableWrapper:
    def __init__(self, cmd):
        log.info("cmd : %s", cmd)
        self.cmd = shlex.split(cmd)
        self.log_file = "".join(random.sample(string.ascii_letters + string.digits, 8)) + ".log"

    def run(self):
        with open(self.log_file, 'w+') as file:
            self.process = subprocess.Popen(self.cmd, stdout=file, stderr=file, close_fds=True)
            self.process.wait()


    def get_ret_code(self):
        return self.process.returncode

    def get_log(self):
        log.info("log : %s", self.log_file)
        return self.log_file

    def stop(self):
        self.process.kill()
        if os.path.exists(self.log_file):
            os.remove(self.log_file)