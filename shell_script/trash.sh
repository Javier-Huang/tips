#!/bin/bash
#回收站
alias rm='trash'
alias rl='trashlist'

LocalTrash="$HOME/.local/share/Trash/files"
trash()
{
    local files=""
    [ ! -d $LocalTrash ] && mkdir -p $LocalTrash
    while [ -n "$1" ];do
        # 移除-r,-f等参数
        if [[ ! $1 =~ ^- ]];then
            if [ -z "$files" ];then
                files=$1
            else 
                files=$files" $1"
            fi
        fi
        shift
    done
    mv --backup="numbered" $files $LocalTrash
}

trashlist()
{
    echo -e "================= Garbage Lists in Trash ================="
    echo -e "================= $LocalTrash ================="
    echo -e "\a"
    echo -e "\aUse 'cleartrash' to clear all garbages in Trash!!!"
    ls --color=auto -lh -S $LocalTrash | more
}
#清空回收站
cleartrash()
{
    echo -ne "\aClear all garbages in Trash, Sure?"
    read confirm
    if [ "$confirm" == 'y' -o "$confirm" == 'Y' ] ;then
        /bin/rm -rf $LocalTrash/* &> /dev/null
        /bin/rm -rf $LocalTrash/.* &> /dev/null
    fi
}
