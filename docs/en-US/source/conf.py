# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

import os, subprocess, sys, shlex
project = 'TuGraph'
copyright = '2023, Ant Group'
author = 'Ant Group'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = ['myst_parser',
              'sphinx_panels',
              'sphinx.ext.autodoc',
              'sphinx.ext.napoleon',
              'sphinx.ext.viewcode']

templates_path = ['../../_templates']
exclude_patterns = []



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'

read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'
if read_the_docs_build:
    subprocess.run(shlex.split("wget https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/doc_deps/liblgraph_python_api.so"))
    subprocess.run(shlex.split("wget https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/doc_deps/liblgraph.so"))
    subprocess.run(shlex.split("wget https://tugraph-web.oss-cn-beijing.aliyuncs.com/tugraph/doc_deps/libjvm.so"))
    sys.path.insert(0, os.path.abspath('9.olap&procedure/1.procedure/'))
    # doxygen & breathe
    extensions.append("breathe")
    subprocess.run(shlex.split("doxygen"), cwd="9.olap&procedure/1.procedure/")
    breathe_projects = {"cpp_procedure": "9.olap&procedure/1.procedure/build/xml"}
    breathe_default_project = "cpp_procedure"
else:
    if os.path.exists("9.olap&procedure/1.procedure/3.C++-procedure") and \
            os.path.exists("9.olap&procedure/1.procedure/4.Python-procedure.rst") and \
            os.path.exists("9.olap&procedure/1.procedure/index.rst") and \
            os.path.exists("9.olap&procedure/1.procedure/index.rst.aci"):
        subprocess.run(shlex.split("rm -rf 3.C++-procedure 4.Python-procedure.rst index.rst"), cwd="9.olap&procedure/1.procedure/")
        subprocess.run(shlex.split("mv index.rst.aci index.rst"), cwd="9.olap&procedure/1.procedure/")
