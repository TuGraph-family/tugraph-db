import os
import shlex
import subprocess
import shutil
import string
import random

import logging

log = logging.getLogger(__name__)

class ToolkitsWrapper:
    def __init__(self, cmd, flags):
        self.cmd = cmd
        self.successful_flags = flags
        self.log_file = "".join(random.sample(string.ascii_letters + string.digits, 8)) + ".log"


    def check_status(self):
        with open(self.log_file, 'r') as file:
            for line in file:
                for flag in self.successful_flags:
                    if line.find(flag) != -1:
                        self.successful_flags.remove(flag)
                        log.info("from  [%s]  find   [%s]", line, flag)
        return len(self.successful_flags) == 0

    def run(self, dir = []):
        with open(self.log_file, 'w+') as file:
            self.process = subprocess.Popen(self.cmd, shell=True, stdout=file, stderr=file, close_fds=True)
            self.process.wait()
            if self.check_status():
                return
            file.seek(0, 0)
            for line in file:
                log.info(line)
            raise Exception('Failed to exec command')

    def stop(self, dir = []):
        self.process.kill()
        if os.path.exists(self.log_file):
            os.remove(self.log_file)
        for d in dir:
            if os.path.exists(d):
                shutil.rmtree(d)
