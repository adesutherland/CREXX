#!/bin/sh
# Install CREXX on CMS

# Exit if there is an error
set -e

# IPL
herccontrol -v
herccontrol "ipl 141" -w "USER DSC LOGOFF AS AUTOLOG1"

# LOGON MAINT AND READ TAPE
herccontrol "/cp disc" -w "^VM/370 Online"
herccontrol "/logon maint cpcms" -w "^CMS VERSION"
herccontrol "/" -w "^Ready;"
herccontrol "devinit 480 io/crexxbin.aws" -w "^HHCPN098I"
herccontrol "/attach 480 to maint as 181" -w "TAPE 480 ATTACH TO MAINT"
herccontrol "/access 19e c" -w "^Ready;"
herccontrol "/tape load * * c2" -w "^Ready;"
# herccontrol "/rename rxas module c1 = = c2"  -w "^Ready;"
herccontrol "/detach 181" -w "^Ready;"
herccontrol "/release c"  -w "^Ready;"
herccontrol "/ipl 190" -w "^CMS VERSION"
herccontrol "/savesys cms" -w "^CMS VERSION"
herccontrol "/" -w "^Ready;"
herccontrol "/logoff" -w "^VM/370 Online"

# LOGON maintc
herccontrol "/logon maintc maintc" -w "^CMS VERSION"
herccontrol "/" -w "^Ready;"

# Sanity test
herccontrol "/rxas -v" -w "^Ready;"

# LOGOFF
herccontrol "/logoff" -w "^VM/370 Online"

# SHUTDOWN
herccontrol "/logon operator operator" -w "RECONNECTED AT"
herccontrol "/shutdown" -w "^HHCCP011I"
