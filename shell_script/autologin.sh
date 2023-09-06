#!/bin/bash
#autologin sshName
#listlogin
#cpt sshName localFile/localDir
#cpf sshName remoteFile/remoteDir
sshpass=`which sshpass`
ssh=`which ssh`
know_host_file="$HOME/.ssh/known_hosts"
conf_file="$HOME/.ssh/addssh"
local_download="$HOME/.tftpboot"
local_tmp_download="$HOME/ssh_download"

#有ssh记录返回0，否则返回1
_isExistSsh()
{
    local sshName=$1
    [ ! -e $conf_file ] && return 1
    if cat $conf_file | grep -Eq "$sshName:";then
        return 0
    fi
    return 1
}

_getSshInfo()
{
    local sshName=$1
    local info=`cat $conf_file | sed -n "/$sshName:/p"`
    echo $info
}

_getSshInfoName()
{
    local name=`echo $*|cut -d : -f 1`
    echo $name
}

_getSshInfoIp()
{
    local ip=`echo $*|cut -d : -f 2`
    echo $ip
}

_getSshInfoUserName()
{
    local username=`echo $*|cut -d : -f 3`
    echo $username
}

_getSshInfoDir()
{
    local dir=`echo $*|cut -d : -f 4`
    echo $dir
}

_getSshInfoPasswd()
{
    local passwd=`echo $*|cut -d : -f 5`
	passwd=`echo $passwd | base64 -d`
    echo $passwd
}

_sshLogin()
{
    local sshName=$1
    local info=`_getSshInfo $sshName`
    if [ ! -z "$info" ];then
        local ip=$(_getSshInfoIp "$info")
        local username=$(_getSshInfoUserName "$info")
        local passwd=$(_getSshInfoPasswd "$info")
        $sshpass -p "$passwd" $ssh $username@$ip
    fi
}


listlogin(){

echo -e "name\tip\tusername\tdir\tpasswd"
if [ -e $conf_file ];then
    for line in `cat $conf_file`
    do
        local name=$(_getSshInfoName "$line")
        local ip=$(_getSshInfoIp "$line")
        local username=$(_getSshInfoUserName "$line")
        local dir=$(_getSshInfoDir "$line")
        local passwd=$(_getSshInfoPasswd "$line")
        echo -e "$name  $ip  $username  $dir  $passwd"
    done
fi

}

autologin(){

if [ -z $1 ];then
    echo "autologin sshname"
    return 1
fi
local sshname=$1

_isExistSsh $sshname
if [ $? != 0 ];then
    echo "没有相关ssh记录"
    read -n1 -p "是否创建新的ssh记录[y/n]" answer 
    if [ "$answer" == "n" ] || [ "$answer" == "N" ];then
        exit 0
    else 
        echo ""
        /usr/local/bin/addssh
    fi
    return 1
fi

_sshLogin $sshname
}

cpf()
{
if [ $# -gt 3 ] || [ $# -le 1 ];then
    echo "cpf sshname remoteFileName [localDir]"
    return 1
fi
local sshname=$1
local remoteFileName=$2
local localDir=$3

_isExistSsh $sshname
[ $? != 0 ] && {
    echo "没有相关ssh记录"
    return 1
}

local info=$(_getSshInfo $sshname)
local ip=$(_getSshInfoIp "$info")
local username=$(_getSshInfoUserName "$info")
local passwd=$(_getSshInfoPasswd "$info")

[ -z $localDir ] && localDir=$local_download

[ ! -d $localDir ] && mkdir -p $localDir

$sshpass -p $passwd scp -r $username@$ip:$remoteFileName $localDir
[ $? == 0 ] && echo "传送成功"

} 

cpt(){
if [ $# -gt 3 ] || [ $# -le 1 ];then
    echo "cpt sshname localFileName [remoteDir]"
    return 1
fi
local sshname=$1
local localFileName=$2
local remoteDir=$3

_isExistSsh $sshname
[ $? != 0 ] && {
    echo "没有相关ssh记录"
    return 1
}

local info=$(_getSshInfo $sshname)
local ip=$(_getSshInfoIp "$info")
local username=$(_getSshInfoUserName "$info")
local dir=$(_getSshInfoDir "$info")
local passwd=$(_getSshInfoPasswd "$info")

[ -z $remoteDir ] && remoteDir=$dir

$sshpass -p $passwd scp -r $localFileName $username@$ip:$remoteDir
[ $? == 0 ] && echo "传送成功"

} 


cptf()
{
    if [ $# -gt 4 ] || [ $# -le 1 ];then
        echo "cptf srcSsh srcWhere targetSsh [targetWhere]"
        return 1
    fi

    local srcSsh=$1
    local srcWhere=$2
    local targetSsh=$3
    local targetWhere=$4
    _isExistSsh $srcSsh
    [ $? != 0 ] && {
        echo "$srcSsh记录不存在"
        return 1
    }
    _isExistSsh $targetSsh 
    [ $? != 0 ] && {
        echo "$targetSsh记录不存在"
        return 1
    }
    [ -d $local_tmp_download ] && rm $local_tmp_download/*

    cpf $srcSsh $srcWhere $local_tmp_download
    cpt $targetSsh $local_tmp_download/* $targetWhere 
}

sshmount()
{
    if [ $# -lt 2 ];then
        echo "sshmount sshName remoteDir localDir"
        return 1
    fi
    local sshname=$1
    local remoteDir=$2
    local localDir=$3
    _isExistSsh $sshname
    [ $? != 0 ] && {
        echo "$sshname记录不存在"
        return 1
    }
    local info=$(_getSshInfo $sshname)
    local ip=$(_getSshInfoIp "$info")
    local username=$(_getSshInfoUserName "$info")
    local passwd=$(_getSshInfoPasswd "$info")

    [ -z $localDir ] && localDir=$HOME/sshfs
    [ ! -d $localDir ] && mkdir -p $localDir
    # $sshpass -p "$passwd"  sshfs $username@$ip:$remoteDir $HOME/sshfs
    sshfs -o transform_symlinks $username@$ip:$remoteDir $localDir
}
