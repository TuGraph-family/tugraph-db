# --- add python plugin ---
import sys
sys.path.append('../build/output')
import bfs_standalone as python_plugin

if __name__ == "__main__":
    python_plugin.Standalone(input_dir=
                             "../test/integration/data/algo/fb_unweighted",
                             root=0)
