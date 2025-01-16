## 介绍

etcd raft原生的golang版本 https://github.com/etcd-io/raft

本项目是etcd raft的C++版本,基于原仓库的某个commit点，commit id在 `commit` 文件中

## 特点
- 对golang版本的etcd raft代码几乎逐行翻译，逻辑上100%与golang版本一致。
- 所有的test case代码也翻译并跑通，保证逻辑正确。
- 变量名、函数名、注释、全部与golang版本保持一致，只做在必要条件下的微调。
- Header-only，所有逻辑全部放在头文件，不需要单独编译成lib。
- 只翻译到RawNode层，因为这层之下属于纯状态逻辑，不涉及golang的协程特性。
- 定期merge原仓库的更新。
- 只依赖protobuf库，其他不依赖，最低C++11可编译通过。

## 完善
- 关键路径上多余的的内存拷贝。
- 一些不该暴漏的私有变量隐藏。