#!/bin/bash
#===============================================================================
#
#          FILE:  run.sh
# 
#         USAGE:  ./run.sh 
# 
#   DESCRIPTION:  
# 
#          BUGS:  ---
#         NOTES:  ---
#        AUTHOR:  zj (), xiaojian@whu.edu.cn
#       COMPANY:  whu.sklse
#       VERSION:  1.0
#       CREATED:  2015年09月19日 17时32分00秒 CST
#      REVISION:  ---
#===============================================================================

mpicxx -g -O0 -o dcmd MessagePasser.h MessagePasser.cpp dcmd.cpp -L /usr/local/lib -lreadline -ledit
