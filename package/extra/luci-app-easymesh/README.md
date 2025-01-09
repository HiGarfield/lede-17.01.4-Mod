# luci-app-easymesh
OpenWRT上的“简单Mesh”插件

Forck 自 https://github.com/ntlf9t/luci-app-easymesh 的有线+无线回程的mesh luci设置插件。

在原版的基础上做了如下修改：

* “FT协议” 改为 “FT over the Air”
* “重关联截止时间” 改为 20000
* 默认的“移动域” 改为 5555
* 默认的漫游RSSI改为 -65 / -80
* 修复了无法正确关闭802.11V协议的问题
* 新增了针对2.4G Wifi是否开启802.11R协议的开关
* 增加了对luci-proto-batman-adv的依赖

目前此插件已在 https://github.com/coolsnowwolf/lede lean版openwrt 和 https://github.com/immortalwrt/immortalwrt immortalwrt 的23.05分支上测试通过。

如需单独编译本项目，可以在OpenWRT源码根目录执行：

    git clone https://github.com/theosoft-git/luci-app-easymesh.git package/luci-app-easymesh
    make menuconfig # choose LUCI -> Applications -> luci-app-easymesh
    make package/luci-app-easymesh/compile V=s
编译结果输出在bin/packages/xxx/base/luci-app-easymesh_xxx.ipk
