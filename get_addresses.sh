#! /bin/bash

SYS_STATFS=`grep " sys_statfs$" System.map* | sed s/' T sys_statfs'/''/`
SYS_READ=`grep " sys_read$" System.map* | sed s/' T sys_read'/''/` 
SYS_CALL_TABLE=`grep " sys_call_table$" System.map* | sed s/' R sys_call_table'/''/`

echo '/* sys_addresses.h auto-generated */'
echo ''
echo '#ifndef _SYS_ADDRESSES_H_'
echo '#define _SYS_ADDRESSES_H_'
echo ''
echo '#define SYS_STATFS 0x'$SYS_STATFS
echo '#define SYS_READ 0x'$SYS_READ
echo '#define SYS_CALL_TABLE 0x'$SYS_CALL_TABLE
echo ''
echo '#define DEFAULT_X "'$PWD'/xxxxx"'
echo '#define DEFAULT_Y "'$PWD'/yyy"'
echo ''
echo '#endif'


