# UMP
Unity MemPerf
UnityMemperf是一款专为Unity引擎安卓平台IL2CPP运行时打造的高性能内存分析、泄露检查、快照对比工具

## 技术选择
通过JDWP（ Java Debug Wire Protocol）技术进行对Unity安卓程序的动态库注入

在动态库的 JNI_OnLoad 中对 Profile 程序进行初始化（开启 TCP Server，开启检测线程等）

在JNI_OnLoad函数中，通过dlopen与dlsym可以获取到 il2cpp_capture_memory_snapshot 函数接口，通过此接口即可实现远程操控的内存快照截取操作
## 当前存在问题：
有概率拉不起server

有部分数据解析不出
## 编译使用方法

```bash
Android内为注入手机的os，使用NDK进行编译
# 需要android-ndk-r16b或更高
android-ndk-r16b/ndk-build
操作步骤为：
cd Android
cd jni
ndk-build.cmd
生成出来的so目录为：
Android
  --libs
生成文件有arm64-v8a与armeabi-v7a请按照当前使用手机的版本进行使用
```

```bash
查询手机版本：
(使用前请自行安装adb与本地Java环境)
adb shell getprop ro.product.cpu.abi
注入时步骤为:
cd Android
cd jni
.\artinjector.bat -i ..\libs\arm64-v8a(or)armeabi-v7a(请根据当前手机cpu类型选择版本)\libumemperf.so -p <package_name>(注入的应用包名)
```

```bash
memory_report_decode为数据解析软件
编译软件步骤为：
cd memory_report_decode
mkdir build         创建临时的构建文件夹（随便什么名字）
cd build
cmake ..               (cmake + CMakeLists.txt所在目录，生成make file，windows下是生成vs工程）
使用vs打开build下的 ALL_BUILD.vcxproj 进行编译
```
```bash
memory_report_decode为数据解析软件
编译软件步骤为：
cd memory_report_decode
mkdir build         创建临时的构建文件夹（随便什么名字）
cd build
cmake ..               (cmake + CMakeLists.txt所在目录，生成make file，windows下是生成vs工程）
使用vs打开build下的 ALL_BUILD.vcxproj 进行编译
使用方法：
第一个参数为存储.rawsnapshot的地址，第二哥参数为输出地址如1.json
如.\memory_report_decoder.exe C:\Users\Administrator\1.rawsnapshot C:\Users\Administrator\Desktop\123\1.json
```
```bash
全套已编译的包在release目录下，可用简易socket_client进行测试
生成出的rawsnapshot再通过memory_report_decoder进行解析
```
