## 搭建环境

1. 对于 windows 系统，需要 wsl。
2. mit 给的指南：https://pdos.csail.mit.edu/6.828/2021/tools.html，参照那个指南或者这个指南都行
3. 官方 repo：https://github.com/mit-pdos/xv6-riscv，clone 那个或者从网络学堂下载源码都行
4. 需要 riscv-toolchain-gnu：https://github.com/riscv-collab/riscv-gnu-toolchain
   ```
   sudo apt-get update && sudo apt-get upgrade
   sudo apt-get install git build-essential gdb-multiarch qemu-system-misc gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu
   ```

或者 clone 下来之后对着源码自己编译，如果你愿意的话。自己编译记得安装 qemu。
5. 编译并运行 xv6。在 clone 下的 xv6-riscv 中 `make qemu` 即可。kernel 在 `kernel\kernel`，挂载的文件系统为同目录下 fs.img。如果需要重新生成，使用 `make clean`。
6. 添加声卡。打开 Makefile，在 161 行后另起一行，添加
   `QEMUOPTS += -device intel-hda`

该声卡的相关信息发在群里了。hda 标准的 device 只有 intel-hda 和 ich9-intel-hda，前者是 ich6，后者标准更现代，具体选择哪个或者使用 hda 之外的规范（最好别考虑）可以再议。
7. 我还没有找到怎么把 fs.img 挂载到其他系统的方法。目前我能想到的唯一往里边加东西的途径是用 mkfs 把新的旧的一块写进去，可以参照 Makefile 137 行的内容。

## ref

往届播放：https://github.com/THSS13/XV6/blob/master/docs/%E9%9F%B3%E9%A2%91%E6%92%AD%E6%94%BE%E6%96%87%E6%A1%A3.pdf
