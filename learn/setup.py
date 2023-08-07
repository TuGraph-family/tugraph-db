import os
from setuptools import setup, find_packages
from torch.utils.cpp_extension import BuildExtension, CppExtension, CUDAExtension, CUDA_HOME

# ref https://docs.python.org/3/extending/building.html
# ref2

# copy libplugin.so and libHalide.so to gpc dir.
os.system('cp bazel-bin/compiler/gpc/ops/libplugin.so compiler/gpc/')
os.system("patchelf --set-rpath '$ORIGIN' compiler/gpc/libplugin.so")
os.system('cp third_party/Halide/bin/libHalide.so compiler/gpc/')

# compile torch op modules.
ext_modules = []
ext_modules.append(
	CppExtension(
		'gpc_sparse',
		['csrc/sparse.cc'],
		extra_compile_args=['-g', '-Wl,-z,origin', '-lz'],
		include_dirs=['./compiler/gpc/', 'csrc/'],
		library_dirs=['.', 'compiler/gpc/'],
		libraries=['plugin'],
		runtime_library_dirs=['$ORIGIN', '$ORIGIN/gpc'],
		depends=['./compiler/gpc/ops/plugin.h']
	)
)


setup(
	name='gpc',
	version='1.0',
	description='sparse gpc package',
	packages=['gpc'],
	package_dir={'gpc':'compiler/gpc'},
	package_data={
		'gpc':['*.so'],
		'':['libplugin.so', 'libHalide.so']
	},
	ext_modules=ext_modules,
	cmdclass={'build_ext': BuildExtension}
	)
