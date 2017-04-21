# VolView的编译安装

## 〇.系统要求
如果要成功从源码编译VolView，推荐的配置如下：

|                   |Windows   | Linux  |
|:-----------: | :------------: | :------------: |
| 软件需求  |  Visual Studio 2008/2005   | gcc4.x + Sublime3 |
| 系统需求  |  Windows7 64位及以上 | Ubuntu 14.04 64位 |

为什么感觉软件都比较老呢（比如明明有新的VS2015，Ubuntu16.04可用）？因为可能是VolView项目开发较早，源代码在新版本的编译器下就编不过去了。如果不喜欢来回切换系统，没有安装Ubuntu的经验，但是又想尝试Ubuntu的话，可以用Vmware安装虚拟机。建议大家多尝试使用Linux系统，没有坏处。

## 一. 安装cmake

### Windows

- Windows下直接下载安装[Cmake.exe](https://cmake.org/)。

### Ubuntu
打开终端：
```shell
sudo apt-get install ccmake-curses-gui
```
> `cmake`和`ccmake-curses-gui`有什么联系和区别呢？
区别大概是后者带有GUI，终端输入`ccmake`命令，可以打开GUI模式（样子和Windows下一样）。

> Cmake是啥？make又是啥？
参考[CMake vs. make](https://prateekvjoshi.com/2014/02/01/cmake-vs-make/)

## 二. 编译VolView
### Windows:
打开Cmake。
先Config，再Generate。
这时到构建目录去，可以看到，已经生成了VS工程文件。
打开VS，构建运行。
中途可能会出小小的错，请发挥你们的聪明才智自行解决。

### Ubuntu
解压源码压缩包，进入；新建文件夹release（名字随意），进入。
```shell
tar zxvf VolViewSrc.tar.gz
cd VolViewSrc
mkdir release
cd release
sudo cmake ../VolViewPlatformComplete
```
注意，cmake过程中可能会报错
```
OPENGL_gl_LIBRARY (ADVANCED)
    linked by target "vtkftgl" in directory /home/lzf/文件/VolViewSrc/VolViewExternalLibraries/VTK/Utilities/ftgl
-- Configuring incomplete, errors occurred!
```
我们需要安装openGL
```shell
sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev
```
可以参考：www.linuxidc.com/Linux/2012-05/60771.htm

最后`sudo make`。可能需要等待较长时间。过程中可能出错，请发挥你们的聪明才智自行解决。改好接着make。

# VolView使用注意事项
- 打开GPU加速；
- 打开的医学图像路径中不要带有中文，否则无法打开。
