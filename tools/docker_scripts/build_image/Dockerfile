FROM ubuntu
LABEL maintainer="rvjansen@xs4all.nl"
# Set the working directory
WORKDIR /

# Copy the current directory contents into the container at /app
#ADD clone.sh /

#
RUN apt-get update && \
apt-get install apt-utils -y && \
apt-get install readline-common -y && \
apt-get install git -y && \
apt-get install openjdk-8-jdk -y && \
apt-get install zip -y && \
apt-get install nano -y && \
apt-get install zsh -y && \
apt-get install build-essential -y && \
apt-get install cmake -y && \
apt-get install regina-rexx -y && \
apt-get install ninja-build


RUN git clone https://github.com/adesutherland/CREXX.git crexx-code

WORKDIR /crexx-code
RUN git checkout feature/f0049

WORKDIR /
RUN mkdir crexx_release

WORKDIR /crexx_release
RUN cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ../crexx-code && ninja

# WORKDIR /netrexx-code/bin
# RUN chmod +x nrc
# RUN chmod +x pipe
# RUN chmod +x pipc
# RUN chmod +x nrws
# RUN chmod +x NetRexxC.sh

# WORKDIR /
# RUN mddir crexx-code

# Make port 80 available to the world outside this container
#EXPOSE 80

# Define environment variable
ENV CLASSPATH .:/netrexx-code/build/lib/NetRexxF.jar
ENV PATH $PATH://crexx_release/bin:/crexx_release/compiler:/crexx_release/assembler:/crexx_release/disassembler:/crexx_release/interpreter:/crexx_release/cpacker:/crexx_release/debugger

WORKDIR /crexx-code
# Run when the container launches
CMD 'git pull'
#CMD 'make'

WORKDIR /crexx_release
# Run when the container launches
CMD cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ../crexx-code && ninja
