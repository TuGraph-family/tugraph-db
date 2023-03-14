from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

ext_modules = [
    Extension(
        "pagerank", ['pagerank.py'],
        libraries=['lgraph'],
        extra_compile_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"],
        include_dirs=['../../deps/fma-common', "../../src", "../../include"],
        library_dirs=['../../build/output'],
        extra_link_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"]
    ),
    Extension(
        "sssp", ['sssp.py'],
        libraries=['lgraph'],
        extra_compile_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"],
        include_dirs=['../../deps/fma-common', "../../src", "../../include"],
        library_dirs=['../../build/output'],
        extra_link_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"]
    ),
    Extension(
        "wcc", ['wcc.py'],
        libraries=['lgraph'],
        extra_compile_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"],
        include_dirs=['../../deps/fma-common', "../../src", "../../include"],
        library_dirs=['../../build/output'],
        extra_link_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"]
    ),
    Extension(
        "lpa", ['lpa.py'],
        libraries=['lgraph'],
        extra_compile_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"],
        include_dirs=['../../deps/fma-common', "../../src", "../../include"],
        library_dirs=['../../build/output'],
        extra_link_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"]
    ),
    Extension(
        "lcc", ['lcc.py'],
        libraries=['lgraph'],
        extra_compile_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"],
        include_dirs=['../../deps/fma-common', "../../src", "../../include"],
        library_dirs=['../../build/output'],
        extra_link_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"]
    ),
    Extension(
        "bfs", ['bfs.py'],
        libraries=['lgraph'],
        extra_compile_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"],
        include_dirs=['../../deps/fma-common', "../../src", "../../include"],
        library_dirs=['../../build/output'],
        extra_link_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"]
    )
]


setup(ext_modules=cythonize(
    ext_modules,
    compiler_directives={'language_level': "3str"},
    include_path=["../../src/cython/"]
))
