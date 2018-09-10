# README

---

## NyanCat OS

### 运行需求

OS: Ubuntu 16.04 32bit

虚拟机: Bochs 2.6.9

编译器： gcc 5.4.0

### 运行方法

在根目录下打开Terminal:

```
make && make install
cd ..
make image && bochs

```

待bochs就绪后，输入c:

```
...
<bochs:1>c

```

### 指令列表

| 指令  | 参数                                | 说明                       |
| ----- | ----------------------------------- | -------------------------- |
| cat   | file_name0 file_name1 ... file_name | 打开并连接文件             |
| cd    | dir_name / ..                       | 打开指定的文件夹或回到上级 |
| clear |                                     | 清空屏幕输出               |
| cp    | file_src file_dst                   | 复制文件                   |
| date  |                                     | 查看日期和时间             |
| echo  | string                              | 输出指定字符串             |
| edit  | file_name                           | 编辑文件                   |
| ls    | [-h -a -l]                          | 列出当前目录文件           |
| mkdir | dir_name                            | 新建文件夹                 |
| mv    | file_src file_dst                   | 移动文件                   |
| ps    |                                     | 查看当前进程               |
| pwd   |                                     | 查看当前目录               |
| rm    | file_name                           | 删除文件                   |
| touch | file_name                           | 新建文件                   |

