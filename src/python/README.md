# 增加Cypher查询

1. 更新compile下面的镜像，编译cpprest增加-fPIC参数
2. deps/build_deps.sh 增加编译参数支持-fPIC
3. lgraph_galaxy.h 增加token，并且将变量变成protected
3. python_api.cpp继承Galaxy，同时增加Cypher语句查询
4. python_api.cpp直接绑定继承后的类

