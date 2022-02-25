#
# makefile for crexx programs

.mxas.rxas:
	cpp -E $*.mxas | awk '/^[^#]/{ print }' > $*.rxas
	rxas $*
#	rxdas $*

.rexx.rxas:
	rxc $*

.rxas.rxbin:
	rxas $*
#	rxdas $*


RX_SRC		:= $(wildcard *.rexx)
AS_MXA		:= $(wildcard *.mxas)
AS_RXA		:= $(wildcard *.rxas)
AS_SRC		:= $(AS_RXA:.rxas=.rxbin)
MAS_SRC		:= $(AS_MXA:.mxas=.rxas)
RX_BIN		:= $(RX_SRC:.rexx=.rxbin)

.SUFFIXES: .rexx .rxas .mxas .rxbin

#
# target all compiles
#
all::	 $(MAS_SRC) $(AS_SRC) $(RX_BIN)

.PRECIOUS: %.rxas

#
# target clean removes compiled products
#
.PHONY: clean
clean: 
	rm -f *.rxbin	
	find . -name "*.rexx" | awk '{$$2 = $$1 ; sub ( /\.rexx/,".rxas",$$1 ) ; print $$1 }' | xargs rm -f
	find . -name "*.mxas" | awk '{$$2 = $$1 ; sub ( /\.mxas/,".rxas",$$1 ) ; print $$1 }' | xargs rm -f
