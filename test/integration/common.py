import os

def list_directory(path):
    dirs = os.listdir(path)
    res = []
    for p in dirs:
        p = os.path.join(path, p)
        if os.path.isdir(p):
            res.append(p)
    return res


def list_file(path):
    dirs = os.listdir(path)
    res = []
    for p in dirs:
        p = os.path.join(path, p)
        if os.path.isfile(p):
            res.append(p)
    return res