#!/bin/sh
# Make cREXX on VM/370

# Exit if there is an error
set -e

# Collate Source
mkdir src
cp ../avl_tree/*.c src
cp ../avl_tree/*.h src
cp ../machine/*.c src
cp ../machine/*.h src
cp ../interpreter/*.c src
cp ../interpreter/*.h src
cp ../disassembler/*.c src
cp ../disassembler/*.h src
cp ../assembler/*.c src
cp ../assembler/*.h src
cp ../compiler/*.c src
cp ../compiler/*.h src
cp ../platform/platform.h src
cp ./cmsmake.exec src
cp ./*.h src
cp ./*.c src
cp ./*.parm src
# Tests
cp ../tests/ascommon.rxas src
cp ../tests/asebcdic.rxas src

# IPL
herccontrol -v
herccontrol "ipl 141" -w "USER DSC LOGOFF AS AUTOLOG1"
herccontrol "/cp start c" -w "RDR"
herccontrol "/cp start d class a" -w "PUN"

# LOGON CMSUSER
herccontrol "/cp disc" -w "^VM/370 Online"
herccontrol "/logon CMSUSER CMSUSER" -w "^CMS"
herccontrol "/" -w "^Ready;"
herccontrol "/purge rdr" -w "^Ready;"

# Get Source
yata -c -d src
herccontrol -m >tmp; read mark <tmp; rm tmp
echo "USERID  CMSUSER\n:READ  ARCHIVE  YATA    " > tmp
cat archive.yata >> tmp
netcat -q 0 localhost 3505 < tmp
rm tmp
herccontrol -w "HHCRD012I" -f $mark
herccontrol "/yata -x -f READER -d d" -w "^Ready;"

# Make source tape and vmarc
herccontrol "/cp disc" -w "^VM/370 Online"
herccontrol "/logon operator operator" -w "RECONNECTED AT"
hetinit -n -d ../crexxsrc.aws
herccontrol "devinit 480 io/crexxsrc.aws" -w "^HHCPN098I"
herccontrol "/attach 480 to CMSUSER as 181" -w "TAPE 480 ATTACH"
herccontrol "devinit 00d io/crexxsrc.vmarc" -w "^HHCPN098I"
herccontrol "/cp disc" -w "^VM/370 Online"
herccontrol "/logon CMSUSER CMSUSER" -w "RECONNECTED AT"
herccontrol "/begin"
herccontrol "/tape dump * * d" -w "^Ready;"
herccontrol "/detach 181" -w "^Ready;"
herccontrol "/vmarc pack * * d (pun" -w "^Ready;"

# Build
herccontrol "/cmsmake" -w "^Ready;" -t 250

# Make binary tape and vmarc
herccontrol "/cp disc" -w "^VM/370 Online"
herccontrol "/logon operator operator" -w "RECONNECTED AT"
hetinit -n -d ../crexxbin.aws
herccontrol "devinit 480 io/crexxbin.aws" -w "^HHCPN098I"
herccontrol "/attach 480 to CMSUSER as 181" -w "TAPE 480 ATTACH"
herccontrol "devinit 00d io/crexxbin.vmarc" -w "^HHCPN098I"
herccontrol "/cp disc" -w "^VM/370 Online"
herccontrol "/logon CMSUSER CMSUSER" -w "RECONNECTED AT"
herccontrol "/begin"
herccontrol "/tape dump * * e" -w "^Ready;"
herccontrol "/detach 181" -w "^Ready;"
herccontrol "/vmarc pack * * e (pun" -w "^Ready;"

# LOGOFF
herccontrol "/logoff" -w "^VM/370 Online"

# SHUTDOWN
herccontrol "/logon operator operator" -w "RECONNECTED AT"
herccontrol "/shutdown" -w "^HHCCP011I"
