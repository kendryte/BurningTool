# 说明
* 本项目有子模块
* 默认所有人都使用vscode开发
* 使用vcpkg处理依赖、cmake编译
* 调试模式链接了libasan，用于查找内存问题，发布模式没有

# 依赖
* UBuntu:
	* libusb: libudev-dev
	* qt6: libdouble-conversion-dev // TODO: 不知道qt本身的名字是什么
* RHEL:
	* libusb: systemd-devel
	* qt6: qt6-qtbase-devel qt6-qttools-devel

* Windows:
  * 请在默认位置安装Qt6，建议保持使用最新版
  * 必须安装：
    * Qt/Qt 6.x.x/MinGW x.x.x 64-bit
    * Qt/Qt 6.x.x/Additional Libraries/Qt Image Formats （用于显示svg格式的图标）
  * 推荐安装：
    * Qt/Qt 6.x.x/Qt Debug Information Files

# 结构说明
* library项目为动态链接库，用于实际执行烧录过程
* test-binary是用来开发library的工具项目
* cli是基于Qt的命令行版本的烧录工具，目前没有开发
* gui是基于Qt的图形界面程序

# 构建（非GUI）
1. clone本项目（注意：有子模块）
	```bash
	git clone https://github.com/kendryte/k510_build_image.git
	git submodule update --init --recursive --depth 5
	```

1. 安装vcpkg
	```bash
	# windows
	./vcpkg/bootstrap-vcpkg.bat
	# 其他
	./vcpkg/bootstrap-vcpkg.sh
	```

1. 执行cmake configure
	```bash
	mkdir build-xxxx
	cd build-xxxx
	cmake ..
	```
	注意观察输出，执行出错不会有很明显的标志，此时进入下一步可能编出老版本。首次运行需要下载并编译依赖包，可能产生很多错误，尤其是网络和缺少系统依赖问题，请自行解决。


1. 执行cmake build
	```bash
	cmake --build .

# 构建（GUI）
用`Qt Creator`打开`gui/CMakeLists.txt`

# 常见问题
### 终端进程启动失败: Starting directory (cwd) "/data/DevelopmentRoot/github.com/kendryte/BurningTool/build-vscode" does not exist。
编译（build）前必须运行配置（configure）

### ==125182==ASan runtime does not come first in initial library list; you should either link runtime to your application or manually preload it with LD_PRELOAD.

`LD_PRELOAD=/usr/lib64/libasan.so.6 ./build-vscode/test-binary/test`
或
bash ./run.sh

### /usr/bin/ld: ../library/libcanaan_burn.so: undefined reference to `__asan_report_store4'
