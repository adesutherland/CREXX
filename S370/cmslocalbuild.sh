#!/bin/bash
# Copy and build source files to VM370
# This tests the cmsbuild.sh and cmsinstall.sh scripts
# that are used by the automated build process
#
# **** Assumes we are run from the S370 directory ****
#

# STOP MSYS2 rewriting directory paths in the docker container
export MSYS2_ARG_CONV_EXCL="vm370:;/opt"

# Build Lexers / Parsers
../cmake-build-release/re2c/re2c -o ./rxasscan.c ../assembler/rxasscan.re
../cmake-build-release/lemon/lemon -c -d. ../assembler/rxasgrmr.y
../cmake-build-release/re2c/re2c -o ./rxcposcn.c ../compiler/rxcposcn.re
../cmake-build-release/lemon/lemon -c -d. ../compiler/rxcpopgr.y
../cmake-build-release/re2c/re2c -o ./rexbscan.c ../compiler/rxcpbscn.re
../cmake-build-release/lemon/lemon -c -d. ../compiler/rxcpbgmr.y

# Start Docker
docker kill vm370
docker pull adriansutherland/vm370:builder
docker run --rm -d -p 3270:3270 -p 8038:8038 -p 3505:3505 --name vm370 adriansutherland/vm370:builder

# Copy files to Docker
docker cp "../S370" "vm370:/opt/hercules/vm370/io"
docker cp "../avl_tree" "vm370:/opt/hercules/vm370/io"
docker cp "../platform" "vm370:/opt/hercules/vm370/io"
docker cp "../assembler" "vm370:/opt/hercules/vm370/io"
docker cp "../compiler" "vm370:/opt/hercules/vm370/io"
docker cp "../disassembler" "vm370:/opt/hercules/vm370/io"
docker cp "../interpreter" "vm370:/opt/hercules/vm370/io"
docker cp "../machine" "vm370:/opt/hercules/vm370/io"

# Build
docker exec vm370 bash -c "cd /opt/hercules/vm370/io/S370 && dos2unix cmsbuild.sh"
docker exec vm370 bash -c "cd /opt/hercules/vm370/io/S370 && chmod +x cmsbuild.sh"
docker exec vm370 bash -c "cd /opt/hercules/vm370/io/S370 && ./cmsbuild.sh"

# Install
#docker exec vm370 bash -c "cd /opt/hercules/vm370/io/S370 && dos2unix cmsinstall.sh"
#docker exec vm370 bash -c "cd /opt/hercules/vm370/io/S370 && chmod +x cmsinstall.sh"
#docker exec vm370 bash -c "cd /opt/hercules/vm370/io/S370 && ./cmsinstall.sh"
