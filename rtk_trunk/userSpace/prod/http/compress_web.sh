#!/bin/bash

# 压缩页面代码脚本，leon
# 2016年10月14日14:20:55

path=$1 #输入的页面代码路径
tools_path=$2   #压缩工具的路径

if [ "$1" = "" ] || [ "$2" = "" ] ; then
    echo "args err"
    echo "help: ./compress_web [web_path] [compress_tools_path]"
    echo "example: ./compress_web ./compress_tools"
    exit -1
fi

path=${path/%\//} #去掉末尾的'/'
web_floder=${path##*/}  
save_floder=${web_floder}_mini
save_path=${path/$web_floder/$save_floder}
mkdir -p $save_path

file_list=`find $path -type f ! -path "*.svn*"`

echo "#####################################"
echo "####### compress web...##############"

for file in $file_list
do
    save_path=${file/$web_floder/$save_floder}
    mkdir -p ${save_path%/*}
    echo "compress $file..."
    case $file in
        *.js)
            java -jar $tools_path/yuicompressor.jar --type js --charset utf-8 --nomunge $file -o $save_path
            # A9手机界面common.js压缩会失败，换jsmin压缩
            if [ $? != 0 ] ; then
                echo "yuicompressor压缩失败，已换用jsmin压缩"
                cat $file | $tools_path/jsmin > $save_path
                [ $? != 0 ] && exit -1
            fi
            ;;
        *.css)
            java -jar $tools_path/yuicompressor.jar --type css --charset utf-8 --nomunge $file -o $save_path
            [ $? != 0 ] && exit -1
            ;;
        *.html)
            ${tools_path}/compress_html $file > $save_path
            [ $? != 0 ] && exit -1
            ;;
        *.json)
            ${tools_path}/compress_json $file > $save_path
            [ $? != 0 ] && exit -1
            ;;
        *)
            cp $file $save_path
            [ $? != 0 ] && exit -1
            ;;
    esac
done
echo "####### compress done ###############"
echo "#####################################"

