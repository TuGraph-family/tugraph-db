from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

ext_modules = [
    Extension(
        "pagerank", ['pagerank_procedure.py'],
        libraries=['lgraph'],
        extra_compile_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"],
        include_dirs=["../../src", "../../include"],
        library_dirs=['../../build/output'],
        extra_link_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"]
    ),
    Extension(
        "sssp", ['sssp_procedure.py'],
        libraries=['lgraph'],
        extra_compile_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"],
        include_dirs=["../../src", "../../include"],
        library_dirs=['../../build/output'],
        extra_link_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"]
    ),
    Extension(
        "wcc", ['wcc_procedure.py'],
        libraries=['lgraph'],
        extra_compile_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"],
        include_dirs=["../../src", "../../include"],
        library_dirs=['../../build/output'],
        extra_link_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"]
    ),
    Extension(
        "lpa", ['lpa_procedure.py'],
        libraries=['lgraph'],
        extra_compile_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"],
        include_dirs=["../../src", "../../include"],
        library_dirs=['../../build/output'],
        extra_link_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"]
    ),
    Extension(
        "lcc", ['lcc_procedure.py'],
        libraries=['lgraph'],
        extra_compile_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"],
        include_dirs=["../../src", "../../include"],
        library_dirs=['../../build/output'],
        extra_link_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"]
    ),
    Extension(
        "bfs", ['bfs_procedure.py'],
        libraries=['lgraph'],
        extra_compile_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"],
        include_dirs=["../../src", "../../include"],
        library_dirs=['../../build/output'],
        extra_link_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"]
    )
]


setup(ext_modules=cythonize(
    ext_modules,
    compiler_directives={'language_level': "3str"},
    include_path=["../../src/cython/", "../../include/cython/"]
))
