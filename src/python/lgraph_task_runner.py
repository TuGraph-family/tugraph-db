'''
This is the entry point to python plugins.
MasterThread starts the worker threads and waits until program terminates.
ModuleManager manages modules in Python space.
WorkerThread trys to pop task from a queue shared between C++ and Python, and then
execute it. If the task is read_only and in_process=False, the WorkerThread will
execute the task with a subprocess, otherwise it executes directly in the process.
'''

import os, sys
import time

sys.path.append(os.path.dirname(__file__))

python2 = (sys.hexversion < 0x3000000)

if python2:
    from importlib import import_module
    from imp import reload
else:
    import queue
    from importlib import reload, import_module
import traceback
import logging
import logging.handlers as handlers

from liblgraph_python_api import *
import lgraph_db_python


def setup_logger(level=logging.INFO):
    '''
    Setup root logger so we can easily use it
    Params:
        level:  string  logging level
    '''
    logging.root.setLevel(level)
    # formatter = logging.Formatter('[%(asctime)s] %(name)s-%(levelname)s: %(message)s')
    # handler = logging.StreamHandler(sys.stdout)
    # handler.setFormatter(formatter)
    # logging.root.addHandler(handler)


setup_logger(logging.WARN)


class PluginManager:
    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        pass

    def __init__(self, db_dir):
        self.functions = {}
        self.db_dir = db_dir
        self.plugin_dir = None

    def LoadModule(self, module_name):
        logging.info('trying to load module %s' % module_name)
        error_code = PluginErrorCode.SUCCESS
        output = ""
        try:
            path = self.plugin_dir + "/" + module_name + ".so"
            if python2:
                module = imp.load_source(module_name, path)
            else:
                import importlib.util
                spec = importlib.util.spec_from_file_location(module_name, path)
                module = importlib.util.module_from_spec(spec)
                spec.loader.exec_module(module)
            self.functions[module_name] = getattr(module, 'Process')
        except (KeyboardInterrupt, SystemExit):
            logging.warn('process being killed')
            raise
        except Exception as e:
            error_code = PluginErrorCode.INPUT_ERR
            output = str(e)
        logging.info('load module returned {}:{}'.format(error_code, output))
        return (error_code, output)

    '''
    Unload a plugin from Python process.
    '''

    def DelModule(self, module_name):
        logging.info('trying to del module {}'.format(module_name))
        error_code = PluginErrorCode.SUCCESS
        output = ""
        if not module_name in self.functions:
            error_code = PluginErrorCode.INTERNAL_ERR
            output = "module {} not found".format(module_name)
            logging.info('current modules: {}'.format(self.functions))
        else:
            del self.functions[module_name]
        logging.info('del module returned {}:{}'.format(error_code, output))
        return (error_code, output)

    '''
    Invoke a function
    '''

    def InvokeFunction(self, db, function, input, read_only):
        logging.info('trying to call function {}, read_only:{}, input: {}'.format(function, read_only, input))
        error_code = PluginErrorCode.SUCCESS
        output = ""
        if not function in self.functions:
            error_code = PluginErrorCode.INTERNAL_ERR
            output = "module {} not found".format(function)
        else:
            try:
                success = False
                try:
                    (success, output) = self.functions[function](db, input)
                except Exception as e:
                    success = False
                    output = "Exception occured in plugin: {}".format(e)
                if ((not isinstance(success, bool)) or
                        (not (isinstance(output, str) or ((not python2) and isinstance(output, bytes))))):
                    output = "Illegal output from plugin. Pluging must output a (bool, str) tuple."
                    error_code = PluginErrorCode.INTERNAL_ERR
                if (not success):
                    error_code = PluginErrorCode.INPUT_ERR
                # db.Close()
            except (KeyboardInterrupt, SystemExit):
                logging.warn('process being killed')
                raise
            except Exception as e:
                traceback.print_exc()
                error_code = PluginErrorCode.INTERNAL_ERR
                output = str(e)
        logging.info('call function returned {}:{}'.format(error_code, output))
        return (error_code, output)

    '''
    Main entry, run the task specified by task.
    '''

    def RunTask(self, task):
        user = task.user
        graph = task.graph
        plugin_dir = task.plugin_dir
        function = task.function
        input = task.get_input()
        read_only = task.read_only
        logging.info('RunTask(graph={}, func={}, read_only={}, input={})'.format(graph, function, read_only, input))
        error_code = PluginErrorCode.SUCCESS
        output = ""
        try:
            self.plugin_dir = plugin_dir
            if function == '__lgraph_load_module__':
                (error_code, output) = self.LoadModule(input.decode())
            elif function == '__lgraph_del_module__':
                (error_code, output) = self.DelModule(input.decode())
            else:
                # TODO: do not reload module if it already exists and was not modified
                (error_code, output) = self.LoadModule(function)
                if not function in self.functions:
                    error_code = PluginErrorCode.INTERNAL_ERR
                    output = "module {} not found".format(function)
                if (error_code != PluginErrorCode.SUCCESS):
                    return (error_code, output)
                import inspect
                sig = inspect.signature(self.functions[function])
                params = sig.parameters
                using_cython_api = str(params[list(params)[0]]).count("lgraph_db_python.PyGraphDB") > 0
                if using_cython_api:
                    galaxy = lgraph_db_python.PyGalaxy(self.db_dir)
                    galaxy.SetUser(user)
                    db = galaxy.OpenGraph(graph, read_only)
                    (error_code, output) = self.InvokeFunction(db, function, input, read_only)
                    del db
                    del galaxy
                else:
                    with Galaxy(self.db_dir, False, False) as galaxy:
                        galaxy.SetUser(user)
                        with galaxy.OpenGraph(graph, read_only) as db:
                            (error_code, output) = self.InvokeFunction(db, function, input, read_only)
        except (KeyboardInterrupt, SystemExit):
            raise
        except Exception as e:
            (error_code, output) = (PluginErrorCode.INTERNAL_ERR, traceback.format_exc())
        logging.info('RunTask returned {}:{}'.format(error_code, output))
        sys.stdout.flush()
        sys.stderr.flush()
        return (error_code, output)


def GetPM(db_dir):
    return PluginManager(db_dir)


'''
Main function
'''
if __name__ == '__main__':
    if (len(sys.argv) < 4):
        print('usage: lgraph_task_runner.py {in_pipe_name} {out_pipe_name} {db_dir} [{pgid}]')
        sys.exit(1)
    in_pipe_name = sys.argv[1]
    out_pipe_name = sys.argv[2]
    db_dir = sys.argv[3]

    if len(sys.argv) >= 5:
        os.setpgid(0, int(sys.argv[4]))

    while True:
        task = TaskInput.ReadTaskInput(in_pipe_name)
        stime = time.time()
        with GetPM(db_dir) as plugin_manager:
            (error_code, output) = plugin_manager.RunTask(task)
            TaskOutput.WriteTaskOutput(out_pipe_name, error_code, output)
        etime = time.time()
        logging.info("outer use " + str(int((etime - stime) * 1000)) + " ms")
