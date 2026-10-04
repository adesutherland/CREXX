options levelc

start='1.00'
do x=start to 2 by 1
 say 'scale' x
end x
say 'after-scale' x
do x='001' to 2
 say 'leading' x
end x
do x='+1' to 2
 say 'plus' x
end x
do x='1E1' to 11
 say 'exponent' x
end x
do x='1.20E1' to 13
 say 'adjusted' x
end x
do x='.500' to 1.500 by .500
 say 'fraction' x
end x
do x='-1.0' to 1
 say 'negative' x
end x
