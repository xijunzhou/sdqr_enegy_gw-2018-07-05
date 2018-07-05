----20180207
电表数据采集：部分电表不支持负荷数据采集命令，所以，对于负荷数据只能一个个采集上来，
完成一个电表所有数据采集后才存入数据库!一个表所有的数据采集上来需要的时间为：N * Tc，N-> 负荷数据项数  Tc-> 采集周期。






----20180326
sqlite数据库表情况
1、网关由2个数据库文件组成，分别是config.db、data.db
2、config.db有两个tables，分别是档案表(archive)和采集策略表(rule)
	   |_archive    数据字段：type | dev_id | net_id       即：类型 | 设备ID | 无线通讯网络ID
	   |_rule       数据字段：type | dev_id | net_id | collect_time | collect_cycle | no_response  类型 | 设备ID | 无线通讯网络ID | 下一次采集时间 | 采集周期 | 设备未响应次数
3、data.db存储历史记录数据，每一个设备创建一个单独的表，表的命名规则如下：设备名称_类型 _设备ID
     如：
        单相电表：elect_meter_40_000000060004	、 elect_meter_40_0000000324576、elect_meter_40_0000000554575
        三相电表：elect3_meter_40_000000060004、……
        水    表：water_meter_40_000000060004	、……
        气    表：gas_meter_40_000000060004	、……
        
     历史数据存储格式为：Timestamp,dev_id,value
     如：
        1522045238|000000040006|{\"type\":\"40\",\"id\":\"000000040006\",\"Timestamp\":\"20180326142038\",\"EnergyForward\":\"10.15 kWh\",\"EnergyBackward\":\"0.00 kvarh\",\"Voltage\":\"222.6 v\",\"Current\":\"0.039 A\",\"Frequency\":\"-110.00 hz\",\"Factor\":\"0.004\",\"ActPower\":\"-0.0065 kw\",\"ReactPower\":\"1.8322 kvar\"}
        
4、config.db数据库中的档案数据(archive表)通过远程或本地USB口方式下发和更新，网关程序在档案数据更新后重启运行
   a、档案数据在PC机上可通过记事本录入archive.txt文件中，录入格式如下(type,dev_id,net_id)：
        64,000000039803,1
				64,000000039804,2
				64,000000039805,3
				64,000000039806,4
				64,000000039807,5
				64,000000039808,6
				64,000000039809,7
				64,000000039810,8
				64,000000039811,9    
	b、archive.txt录入完成后，通过远程或本地USB口，将文件下发到网关程序同一级目录
	c、通过telnet登录网关设备，检查archive.txt内容和格式是否正确。确认后在网关程序下执行如下步骤
	      root@ubuntu:~/win7/gateway-collecter# sqlite3 config.db
				SQLite version 3.21.0 2017-10-24 18:55:49
				Enter ".help" for usage hints.
				sqlite> .separator ','
				sqlite> .import archive.txt archive
				sqlite> select * from archive;
				64,000000039803,1
				64,000000039804,2
				64,000000039805,3
				64,000000039806,4
				64,000000039807,5
				64,000000039808,6
				64,000000039809,7
				64,000000039810,8
				64,000000039811,9
	d、	档案更新完成
5、重启网关程序，按照新的档案运行



----20180329
添加智能采集周期更新函数，采用一元线性回归方程 y = bx + a
1、从历史记录中，取最新的N(10)条记录，将时间 和 值 的变化值([time0 - time1 time1 - time2,……] [value0 - value1 value1 - value2,……] )作为回归方程输入，得到一元线性回归方程的 b 和 a 的值，
2、以最少变化值(DATA_RULE_BASE) 作为输出变量 x ，得到 y 即采集周期。 




==================================================================V0.3=========20180510
1、添加开合闸功能
2、将主程序改写为线程模式，当前包含另个线程：主线程 和 费控线程
3、添加cjson.c cjson.h两个文件
4、将串口初始化放在主程序入口初始化

5、启动程序目录：/home/root/
   ----主程序:meter_vX.Y,
   ----监护程序:daemen,
   ----主程序版本：version,
   ----表档案更新文件：meter_archive.txt
   
6、数据、配置文件目录：/var/meter/meter_config
   ----数据库文件：meter.db
   ----表档案配置文件:meter_config
   ----主程序日志：meter.log
   ----监护程序日志：daemonlog
   
7、启动脚本/etc/init.d/gateway.sh  内容：【注意：脚本命令在linux下编辑，不要在window下编辑！！！！】
		#! /bin/sh
		### BEGIN INIT INFO
		# Provides:          gateway
		# Required-Start:    $local_fs $network
		# Required-Stop:     $local_fs
		# Default-Start:     2 3 4 5
		# Default-Stop:      0 1 6
		# Short-Description: gateway
		### END INIT INFO
		
		mypath="/home/root"
		Day=$(date +%d)
		ver=$(cat $mypath/version)
		cat /var/meter/meter.log > /var/meter/gateway_log_$Day.log
		cat /var/meter/daemon.log > /var/meter/gateway_log_$Day.log
		chmod a+x $mypath/meter_$ver
		$mypath/meter_$ver > /var/meter/meter.log 2>&1 &
		$mypath/daemon > /var/meter/daemon.log 2>&1 &
		
		exit 0

   
8、程序部署方法
   I、网关设备部署包target_gateway 包括：
      ----主程序版本文件：  				version
      ----主程序执行文件：  				meter_vx.y
      ----监护程序执行文件：				daemon
      ----表档案更新文件：  				meter_archive.txt
      ----表档案配置文件：  				meter_config
      ----开机启动脚本：    				gateway.sh
      ----部署文件脚本：            install_gateway
   II、云平台部署包target_cloud 包括：  
      ----云平台主程序执行文件：    epeis
      ----云平台远程更新执行文件：  gateway_update
      ----云平台开机启动脚本：      gateway_dsc
      ----部署文件脚本：            install_cloud
   III、网关设备安装方法：
   		----下载target_gateway.tar.bz到设备
   		----解压target_gateway文件 tar -xvf target_gateway.tar.bz
   		----root权限执行脚本 sudo ./install_gateway 
   IV、云平台安装方法：
   		----在云平台解压target_cloud文件 tar -xvf target_cloud.tar.bz
   		----root权限执行脚本 sudo ./install_cloud 		
   
   V、远程更新方法：
      ----将target包中的   meter_vx.y 、meter_archive.txt   复制到WEB服务器静态文件目录：118.24.14.213/opt/static-files  
      
			      [gateway@VM_136_107_centos static-files]$ ls -al
						total 442548
						drwxr-xr-x  2 root root     4096 May  8 16:36 .
						drwxr-xr-x. 6 root root     4096 Feb 28 18:22 ..
						-rw-r--r--  1 root root      403 May  7 15:46 meter_archive.txt
						-rw-r--r--  1 root root   570354 May  5 17:49 meter_v1.0
						……
						……
						……
			-----执行云平台远程更新执行文件，命令格式如下：(指定网关编号)			     
						Usage: 程序名 文件名/时间 网关编号
						./gateway_dsc meter_v1.0 9059af4abbb3 
						./gateway_dsc meter_archive.txt.txt 9059af4abbb3
						./gateway_dsc time 9059af4abbb3
						./gateway_dsc test
	 					
    VI、程序处理
             主程序：meter_vx.y 与 配置升级程序 daemon 启动等待60秒
             
    VII、数据推送内容：
            类型 表号 时间 正向有功总电能 开合闸状态            
            elect_meter {"type":"40","id":"000000072376","Timestamp":"20180511164148","EnergyForward":"33.02 kWh","Trip":"0"}









------------------------------------TO DO-----------------------------------

1、添加全数据buff解码，当前protocol_decode解码程序只截取buff前面一条正确的DTL645帧\CJ188帧
   需要增加对剩余buff数据帧解码。