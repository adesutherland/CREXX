options levelc
f = 'levelc-address-three-bytes.txt'
address system with input stream f output stem out.
'wc -c'
say 'bytes=' || strip(out.1) || '/' || rc || '/' || .rc || '/' || .rs
lines.0 = 2
lines.1 = 'first'
lines.2 = 'second'
address system with output stem count. input stem lines.
'wc -l'
say 'lines=' || strip(count.1) || '/' || rc || '/' || .rc || '/' || .rs
empty_file = 'levelc-address-empty-output.txt'
address system '' with output stream empty_file
say 'blank=' || rc || '/' || .rc || '/' || .rs
nul_file = 'levelc-address-nul-output.bin'
address system "printf '\000'" with output stream nul_file
say 'nul=' || rc || '/' || .rc || '/' || .rs
address system
'exit 7'
say 'nonzero=' || rc || '/' || .rc || '/' || .rs
