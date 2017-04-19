# VolView插件说明

虽然在未来不打算再在VolView进行开发了，但是作为一个功能比较基础和全面的工具，是可以适当了解一下的。
这里的文件夹`C`应该放在`VolViewSrc/VolViewLibraries/KWVolVIew/Plugins`下，覆盖原有的同名文件夹。在上述目录中，还有`ITK`和`VTK`两个目录，应该分别是存放引入了ITK和VTK库的插件。我们目前不深入研究了。`C`文件夹下有一个`vvSample.cxx`，简明扼要地介绍了开发自定义插件的流程。

# 如何使用
注意，如果把文件夹`C`拷贝到上述目录，然后就编译运行VolView，并不能成功添加插件。请打开`Plugins`目录下的`CMakeList.txt`，仿照其中的语句添加类型如下语句：
```cmake
ADD_LIBRARY(vvYourPluginName MODULE C/vvYourPluginName.cxx)  #新增。井号后是注释
....
SET (PLUGINS
...
C/YouPluginName  #新增
```
然后编译运行。
