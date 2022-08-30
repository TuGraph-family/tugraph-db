 # TuGraph Cypher工具

Version: 3.3.0

2022/07/19

蚂蚁集团

---

### 环境准备

```bash
python3 -m pip install \
		click==7.0 \
		prompt_toolkit==2.0.9 \
		prettytable==0.7.2 \
		requests==2.22.0
```
### **交互式查询**
#### 启动
```bash
python3 lgraph_cypher.py -u admin -P 73@TuGraph --host 127.0.0.1 -p 7071
```
#### 查询
```bash
default> match (n) return properties(n) limit 1;
```
#### 帮助
```bash
default> :help
```
#### 退出
```bash
default> :exit
```

