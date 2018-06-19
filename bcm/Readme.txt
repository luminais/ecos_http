include projects: 
N3,W316R
W303R C0
W308R C0


0,如果没有交叉编译环境，要到版本服务器上的Input目录下载bcm_ecos_gnutools.tgz
然后，解压到本地/opt目录：
tar -xzvf bcm_ecos_gnutools.tgz -C /opt

1,把src\ecos\router\config\xxxx.config拷到src\ecos\router目录下的.config

2,在src\ecos\router\config\cfg文件中建立你的OEM配置文件

3,在src\ecos\router目录下
make menuconfig，第一个menuconfig退出时，保存，
到第二个menuconfig时，根据下面情况选择：
如果是N3，在Device Drivers Options下选择Wireless feature variant为ap
如果是W303R，在Device Drivers Options下选择Wireless feature variant为apsta

4,make oldconfig;make clean;make all
===============================================================================
2011/08/16 Modify:
include projects: 
N3,W316R->8MB_AP
W303R C0->16MB
W308R C0->16MB


0,如果没有交叉编译环境，要到版本服务器上的Input目录下载bcm_ecos_gnutools.tgz
然后，解压到本地/opt目录：
tar -xzvf bcm_ecos_gnutools.tgz -C /opt

1,在src\ecos\router\config\cfg文件中建立你的OEM配置文件

2,把src\ecos\router\config\xxxx.config拷到src\ecos\router目录下的.config

3,must do this if you have just finished step 2.
在src\ecos\router目录下
选择 "8MB_AP" or "8MB_APSTA" or "16MB"
make PROFILE=8MB_AP oldconfig

4,make clean;make all

===============================================================================
2011/11/10 Modify:
增加A5 V1->8MB_AP
切换到SDK:ecos-router-5.110.27.21.tgz

2013/5/28 Modify:
解决断电丢配置问题，增添flash双备份方案，主要修改代码如下：
增加两个文件 sys_backupcfg.c sys_backupcfg.h 在nvram目录下，在操作flash时增加锁，并在rc.c里边增加定时器来检测是否需要备份

2013/06/03 特别版本:
问题：特殊环境中，PPPOE服务器所回应维链包（echo reply）的magic number字段异常（和请求包中一致），导致ppp链路被我公司路由器主动断开连接。针对此问题，仅发布特别版本
两种解决方案：方法1，配置中将echo设置为被动模式（不主动发维链包），参见配置文件w316r_cn_PASSIVE_ECHO_MODE.config
	           方法2，在lcp.c文件中lcp_received_echo_reply函数 有判断“请求包和回应包magic number是否相同”，如果是，则直接return；------------可将此return语句注销。