#!/bin/bash
ROOT=/reasy #reasy工具链目录
SVN_SERVER_IP=192.168.98.20
SVN_SERVER_ROOT=svn://$SVN_SERVER_IP/web-tool/reasy-linux32/reasy

PATH=$PATH:$ROOT/bin/
export PATH

function check_network()
{
	ping_res=$(ping $1  -c 3|grep "3 received, 0% packet loss")
	if [ "$ping_res" == "" ]
	then
		echo "check network ..........fail!"
		return 0;
	else
		echo "check network ..........ok!"
		return 1;
	fi
}

function check_ok() {
	echo 权限OK
	reasy -v
}


function svn_co() {
	svn co $SVN_SERVER_ROOT $ROOT
	chmod 777 -R $ROOT
	echo reasy工具链下载成功
	return;
}

function chmod_root() {
	echo 正在添加可执行权限
	chmod 777 -R $ROOT
	#test -x $ROOT/bin/fis && check_ok || echo 无法添加可执行权限，请手动执行chmod
	return
}

function check() {
	if [ ! -d $ROOT/bin ] 
	then
		echo 不存在reasy编译工具链或工具链已损坏，将从服务器上下载
		check_network $SVN_SERVER_IP
		if [ $? -eq 0 ]
		then 
			echo 网络故障，无法连接192.168.98.20
			exit 0
		fi
		svn_co	
	fi
	test -x $ROOT/bin/reasy && check_ok || chmod_root
}


function check_update() {
	args=$*
	#echo ${args##*---updated}#检测是否存在---updated---
	# if [ "${args##*---updated}" == "---" ]; then
	# 	return 0;
	# fi

	#检查web目录下是否有update.txt，如果有，则执行svn up
	get_var $args  #读取-r得到web目录
	if [ $? ]; then
		echo "webroot is: $webroot";

		#检测是否需要更新工具链
		if [ -e $webroot/update.txt ]; then
			echo "检测到${webroot}/update.txt,编译工具链更新中..."
			check_network $SVN_SERVER_IP
			if [ $? -eq 0 ]; then 
					echo 网络故障，无法连接192.168.98.20,请确认网络是否畅通
			else
				echo 正在更新reasy工具链
				svn up $ROOT
				echo reasy工具链更新OK
				rm -rf $webroot/update.txt
			fi

			#检测是否需要更新脚本
			if [ -e $webroot/update-script.txt ]; then
				self=$0
				script=${self##*[/\\]}
				echo "检测到${webroot}/update-script.txt,${script}将进行更新..."
				

				if [ -e $ROOT/reasy-make.sh ]; then
					echo "存在${ROOT}/reasy-make.sh"
				 	cp $self $self.bak -f
				 	cp $ROOT/reasy-make.sh $self -f
				 	chmod 777 $self
				 	rm -rf $webroot/update-script.txt
				 	return 9
				fi
			fi

		fi
		
	else 
		return 1;
	fi	
}

function get_var ()
{
	local opt=0;
    for p in $*;
    do
    	if [ $opt -eq 1 ];then
    		webroot=$p;
    		return 1;
    	fi
    	if [ "$p" == "-r" ];then
    		opt=1;
    	fi
    	if [ "$p" == "-u" ];then
    		echo 正在更新reasy工具链
    		svn cleanup $ROOT
    		svn up $ROOT
    		echo reasy工具链更新OK
    		exit;
    	fi
    done;
}


check
check_update $*

if [ $? -eq 9 ];then
	echo "检测到脚本已经更新，重新执行中..."
	exec "$0" "$@"
else 
	echo "正在执行reasy"
	exec "reasy" "$@"
fi
