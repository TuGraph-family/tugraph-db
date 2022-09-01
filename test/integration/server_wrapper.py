import os
import shlex
import subprocess
import time
import shutil
import string
import random

class ServerWrapper:
    def __init__(self, cmd, flag):
        self.cmd = shlex.split(cmd)
        self.successful_flag = flag
        self.log_file = "".join(random.sample(string.ascii_letters + string.digits, 8)) + ".log"


    def check_status(self):
        with open(self.log_file, 'r') as file:
            for line in file:
                if line.find(self.successful_flag) != -1:
                    return True
        return False

    def run(self, dir):
        with open(self.log_file, 'w+') as file:
            self.process = subprocess.Popen(self.cmd, stdout=file, stderr=file, close_fds=True)
            for i in range(60):
                time.sleep(1)
                if not self.check_status():
                    continue
                else:
                    return
            self.stop(dir)
            file.seek(0, 0)
            for line in file:
                print(line)
            raise Exception('Failed to start server')

    def stop(self, dir = []):
        self.process.kill()
        self.process.wait()
        if os.path.exists(self.log_file):
            os.remove(self.log_file)
        for d in dir:
            if os.path.exists(d):
                shutil.rmtree(d)