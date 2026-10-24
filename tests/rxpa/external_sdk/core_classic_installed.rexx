options levelc
parse value 'left:right' with first ':' second
if first <> 'left' | second <> 'right' then exit 1
say 'CORE_CLASSIC_PACKAGE_PASS'
exit 0
