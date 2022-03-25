# 勘智系列芯片烧录工具

# 基本介绍
本项目分三部分：
* library：C语言实现，实现串口和USB通信
* cli：命令行实现
* gui：基于Qt6的可视化实现

# 下载
TODO

# 开发
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
1. 执行cmake build
	```bash
	cmake --build .

# 开源代码协议
## MIT
* [sanitizers-cmake](https://github.com/arsenm/sanitizers-cmake)
* [vcpkg](https://github.com/microsoft/vcpkg)
* [slip](https://github.com/marcinbor85/slip) 有改动
* [libsercomm](https://github.com/ingeniamc/sercomm)

## LGPL
* [libusb](https://github.com/libusb/libusb)

## GPL
* [Qt](https://www.qt.io/)

## 专用协议
* [argtable3](https://github.com/argtable/argtable3)
