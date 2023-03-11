import sys
import importlib
sys.path.append("./algo/")

if __name__ == "__main__":
    algo = importlib.import_module(sys.argv[1])
    algo.Standalone(sys.argv[2])