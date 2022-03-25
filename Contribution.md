# 说明
* 本项目有子模块
* 默认所有人都使用vscode开发
* 使用vcpkg处理依赖、cmake编译
* 调试模式链接了libasan，用于查找内存问题，发布模式没有

# 构建
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

# 常见问题
### 终端进程启动失败: Starting directory (cwd) "/data/DevelopmentRoot/github.com/kendryte/BurningTool/build-vscode" does not exist。
编译（build）前必须运行配置（configure）

### 

`LD_PRELOAD=/usr/lib64/libasan.so.6 ./build-vscode/test-binary/test`

### /usr/bin/ld: ../library/libcanaan_burn.so: undefined reference to `__asan_report_store4'
