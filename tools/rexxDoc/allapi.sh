find . -name '*.crexx' \
| sort \
| xargs -I{} sh -c 'crexx rexxDoc --args "$1"; echo' _ {}
> all-apis.tex
