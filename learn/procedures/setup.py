from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize


ext_modules = [
    Extension(
        'getdb', ['getdb.py'],
        libraries=['lgraph'],
        extra_compile_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"],
        include_dirs=["../../src", "../../include"],
        library_dirs=['../../build/output'],
        extra_link_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"]
    ),
    Extension(
        'edge_sampling', ['edge_sampling.py'],
        libraries=['lgraph'],
        extra_compile_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"],
        include_dirs=["../../src", "../../include"],
        library_dirs=['../../build/output'],
        extra_link_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"]
    ),
    Extension(
        'negative_sampling', ['negative_sampling.py'],
        libraries=['lgraph'],
        extra_compile_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"],
        include_dirs=["../../src", "../../include"],
        library_dirs=['../../build/output'],
        extra_link_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"]
    ),
    Extension(
        'neighbors_sampling', ['neighbors_sampling.py'],
        libraries=['lgraph'],
        extra_compile_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"],
        include_dirs=["../../src", "../../include"],
        library_dirs=['../../build/output'],
        extra_link_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"]
    ),
    Extension(
        'random_walk', ['random_walk.py'],
        libraries=['lgraph'],
        extra_compile_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"],
        include_dirs=["../../src", "../../include"],
        library_dirs=['../../build/output'],
        extra_link_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"]
    )
    Extension(
        'node2vec_sampling', ['node2vec_sampling.py'],
        libraries=['lgraph'],
        extra_compile_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"],
        include_dirs=["../../src", "../../include"],
        library_dirs=['../../build/output'],
        extra_link_args=['-Wall', '-g', "-fno-gnu-unique", "-fPIC", "--std=c++17", "-rdynamic", "-O3", "-fopenmp"]
    )
]

setup(
    name='procedures',
    version='1.0',
    description='A Cython extension module',
    ext_modules=cythonize(
    ext_modules,
    compiler_directives={'language_level': "3str"},
    include_path=["../../src/cython/", "../../include/cython/"])
)
