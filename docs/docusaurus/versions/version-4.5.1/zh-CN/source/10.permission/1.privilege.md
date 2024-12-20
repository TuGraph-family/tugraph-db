# 用户权限

## 1.介绍

> TuGraph 的权限是基于角色的访问控制进行管理，定义访问控制的权限分配给角色，角色再分配给用户。

## 2.权限层级

- Global 层：即全局权限，对管理、图操作均有权限；
- Graph 层：即图级别，对每个图的权限；
- （仅商业化版本支持）Property 层：即属性级别，对某个属性的权限控制

## 3.权限关键字

目前权限的控制较为简洁

- Global 层目前为 admin 权限，并且预置了 admin 用户；

* Graph 层的操作权限分为四种：none，read，write，full
  - none：无权限，对于图没有任何操作权限
  - read：只读权限，对于图只具备读取权限
  - write：读写权限，对于图不仅具备读取权限，还具备了写入的权限
  - full：所有权限，对于图不仅具备读写权限，同时也具备删除图、修改图、修改 Schema 等权限
* （仅商业化版本支持）Property 层的权限分别为：none，read，write
  - none：无权限，对于该属性没有任何操作权限
  - read：只读权限，对于该属性只具备读取权限
  - write：读写权限，对于该属性不仅具备读取权限，还具备了写入的权限

## 4.常用权限操作

### 4.1.用户操作

- 创建用户

```cypher
CALL dbms.security.createUser(user_name::STRING,password::STRING)
```

- 删除用户

```cypher
CALL dbms.security.deleteUser(user_name::STRING)
```

- 修改当前用户密码

```cypher
CALL dbms.security.changePassword(current_password::STRING,new_password::STRING)
```

- 修改指定用户密码

```cypher

CALL dbms.security.changeUserPassword(user_name::STRING,new_password::STRING)
```

- 禁用/启用用户

```cypher
CALL dbms.security.disableUser(user::STRING,disable::BOOLEAN)
```

- 列出所有用户

```cypher
CALL dbms.security.listUsers()
```

- 列出当前用户信息

```cypher
CALL dbms.security.showCurrentUser()
```

- 获取用户详情

```cypher
CALL dbms.security.getUserInfo(user::STRING)
```

### 4.2.角色操作

- 创建角色

```cypher
CALL dbms.security.createRole(role_name::STRING,desc::STRING)
```

- 删除角色

```cypher
CALL dbms.security.deleteRole(role_name::STRING
```

- 列出所有角色

```cypher
CALL dbms.security.listRoles()
```

- 禁用/启用角色

```cypher
CALL dbms.security.disableRole(role::STRING,disable::BOOLEAN)
```

### 4.3.赋予用户角色

- 新增用户与角色的联系

```cypher
CALL dbms.security.addUserRoles(user::STRING,roles::LIST)
```

- 删除用户与角色的联系

CALL dbms.security.deleteUserRoles(user::STRING,roles::LIST)

- 清空用户角色的关系并重建

```cypher
CALL dbms.security.rebuildUserRoles(user::STRING,roles::LIST)
```

### 4.4.角色赋权

- 修改角色对指定图的访问权限

```cypher
CALL dbms.security.modRoleAccessLevel(role::STRING,access_level::MAP)
```

示例

```cypher
CALL dbms.security.modRoleAccessLevel("test_role", {test_graph1:"FULL", test_graph2:"NONE"})
```