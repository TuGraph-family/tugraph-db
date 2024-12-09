.. TuGraph documentation master file, created by
   sphinx-quickstart on Mon Jul 16 10:29:17 2018.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

TuGraph Procedure API - Python
====================================
Version: 4.3.2

Copyright (C) 2018-2022 Ant Group.


.. toctree::
   :maxdepth: 2

Introduction
------------------
Similar to SQLite and Noe4j, TuGraph can work in embedded mode. In embedded mode, it works
like a library. You can write your own application and call the library functions to create, query
and modify the graph. In this case, all the data exchange between the application and the graph
database goes in the same process. It is very simple and efficient.

This is the python API document for TuGraph embedded mode. With the embedded API, the user
can open or create a database, and then query or modify the database.

API
------------------
.. automodule:: liblgraph_python_api
   :members:


Indices and tables
-------------------

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
