#!/bin/sh
# Make CREXX on CMS

# Exit if there is an error
set -e

# IPL
herccontrol -v
herccontrol "ipl 141" -w "USER DSC LOGOFF AS AUTOLOG1"
herccontrol "/cp start c" -w "RDR"
herccontrol "/cp start d class a" -w "PUN"

# LOGON CMSUSER
herccontrol "/cp disc" -w "^VM/370 Online"
herccontrol "/logon cmsuser cmsuser" -w "^CMS VERSION"
herccontrol "/" -w "^Ready;"

# Get PackCC
#yata -c -d packcc
#herccontrol -m >tmp; read mark <tmp; rm tmp
#echo "USERID  CMSUSER\n:READ  YATA     TXT     " > tmp
#cat yata.txt >> tmp
#netcat -q 0 localhost 3505 < tmp
#rm tmp
#herccontrol -w "HHCRD012I" -f $mark
#herccontrol "/" -w "RDR FILE"
#herccontrol "/yata -x -f READER -d f" -w "^Ready;"

# Get Assembler
yata -c -d assembler
herccontrol -m >tmp; read mark <tmp; rm tmp
echo "USERID  CMSUSER\n:READ  YATA     TXT     " > tmp
cat yata.txt >> tmp
netcat -q 0 localhost 3505 < tmp
rm tmp
herccontrol -w "HHCRD012I" -f $mark
herccontrol "/yata -x -f READER -d f" -w "^Ready;"

# Make source tape and vmarc
herccontrol "/cp disc" -w "^VM/370 Online"
herccontrol "/logon operator operator" -w "RECONNECTED AT"
hetinit -n -d crexxsrc.aws
herccontrol "devinit 480 io/crexxsrc.aws" -w "^HHCPN098I"
herccontrol "/attach 480 to cmsuser as 181" -w "TAPE 480 ATTACH TO CMSUSER"
herccontrol "devinit 00d io/crexxsrc.vmarc" -w "^HHCPN098I"
herccontrol "/cp disc" -w "^VM/370 Online"
herccontrol "/logon cmsuser cmsuser" -w "RECONNECTED AT"
herccontrol "/begin"
herccontrol "/tape dump * * f" -w "^Ready;"
herccontrol "/detach 181" -w "^Ready;"
herccontrol "/vmarc pack * * f (pun" -w "^Ready;"

# Build
herccontrol "/ACC 193 E (ERASE" -w "^Ready;"
herccontrol "/cmsmkas" -w "^Ready;" -t 250
herccontrol "/rename * * e = = e2" -w "^Ready;"

# Test suite
# herccontrol "/cmsteas" -w "^Ready;"

# Make binary tape and vmarc
herccontrol "/cp disc" -w "^VM/370 Online"
herccontrol "/logon operator operator" -w "RECONNECTED AT"
hetinit -n -d crexxbin.aws
herccontrol "devinit 480 io/crexxbin.aws" -w "^HHCPN098I"
herccontrol "/attach 480 to cmsuser as 181" -w "TAPE 480 ATTACH TO CMSUSER"
herccontrol "devinit 00d io/crexxbin.vmarc" -w "^HHCPN098I"
herccontrol "/cp disc" -w "^VM/370 Online"
herccontrol "/logon cmsuser cmsuser" -w "RECONNECTED AT"
herccontrol "/begin"
herccontrol "/tape dump * * e" -w "^Ready;"
herccontrol "/detach 181" -w "^Ready;"
herccontrol "/vmarc pack * * e (pun" -w "^Ready;"

# LOGOFF
herccontrol "/logoff" -w "^VM/370 Online"

# SHUTDOWN
herccontrol "/logon operator operator" -w "RECONNECTED AT"
herccontrol "/shutdown" -w "^HHCCP011I"
