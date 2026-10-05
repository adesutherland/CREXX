options levelc
source = '0001FF'x || 'Ā🙂'
say 'mapped=' || translate(source, 'AB', , '.')
say 'empty=' || translate(source, '')
say 'outside=' || translate('Ā🙂', 'XY')
exit
