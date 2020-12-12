#!/bin/bash
# Copy and build source files to VM370
# This tests the cmsbuild.sh and cmsinstall.sh scripts
# that are used by the automated build process

# STOP MSYS2 rewriting directory paths in the docker container
export MSYS2_ARG_CONV_EXCL="vm370:;/opt"

docker kill vm370
docker pull adriansutherland/vm370:builder
docker run --rm -d -p 3270:3270 -p 8038:8038 -p 3505:3505 --name vm370 adriansutherland/vm370:builder


yata -c -f yata.txt

docker cp yata.txt "vm370:/opt/hercules/vm370/io"
rm yata.txt
docker exec vm370 bash -c "cd /opt/hercules/vm370/io && yata -x"
docker exec vm370 bash -c "rm /opt/hercules/vm370/io/yata.txt"
docker cp cmsbuild.sh "vm370:/opt/hercules/vm370/io"
docker exec vm370 bash -c "cd /opt/hercules/vm370/io && chmod +x cmsbuild.sh"
docker cp cmsinstall.sh "vm370:/opt/hercules/vm370/io"
docker exec vm370 bash -c "cd /opt/hercules/vm370/io && chmod +x cmsinstall.sh"

# Lemon
yata -c -d lemon -f yata.txt
docker cp yata.txt "vm370:/opt/hercules/vm370/io"
rm yata.txt
docker exec vm370 bash -c "mkdir /opt/hercules/vm370/io/lemon"
docker exec vm370 bash -c "cd /opt/hercules/vm370/io && yata -x -d lemon"
docker exec vm370 bash -c "rm /opt/hercules/vm370/io/yata.txt"

# Assembler
yata -c -d assembler -f yata.txt
docker cp yata.txt "vm370:/opt/hercules/vm370/io"
rm yata.txt
docker exec vm370 bash -c "mkdir /opt/hercules/vm370/io/assembler"
docker exec vm370 bash -c "cd /opt/hercules/vm370/io && yata -x -d assembler"
docker exec vm370 bash -c "rm /opt/hercules/vm370/io/yata.txt"

# Build
docker exec vm370 bash -c "cd /opt/hercules/vm370/io && dos2unix cmsbuild.sh"
docker exec vm370 bash -c "cd /opt/hercules/vm370/io && ./cmsbuild.sh"

# Install
#docker exec vm370 bash -c "cd /opt/hercules/vm370/io && chmod +x cmsinstall.sh && ./cmsinstall.sh"