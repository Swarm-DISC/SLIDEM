FROM ubuntu
RUN apt -y update
RUN apt -y upgrade
RUN apt -y install wget
RUN apt -y install gcc
RUN apt -y install g++
RUN apt -y install cmake
RUN apt -y install make
RUN apt -y install bash
RUN apt -y install zlib1g
RUN apt -y install zlib1g-dev
RUN apt -y install libxml2
RUN apt -y install libxml2-dev
RUN apt -y install libgsl-dev
RUN apt -y install git
RUN apt -y install gdb
RUN apt -y install cgdb
RUN apt -y install vim
RUN apt -y install pkg-config
RUN apt -y install zip
RUN apt -y install unzip
RUN addgroup science
RUN useradd -G science dataflow
USER dataflow
WORKDIR /home/dataflow/src
RUN cd /home/dataflow && \
wget https://spdf.gsfc.nasa.gov/pub/software/cdf/dist/cdf38_1/linux/cdf38_1-dist-cdf.tar.gz && \
tar xf cdf38_1-dist-cdf.tar.gz && \
cd cdf38_1-dist && \
make OS=linux ENV=gnu CURSES=no all
USER root
RUN cd /home/dataflow/cdf38_1-dist && \
make INSTALLDIR=/usr/local install && \
ldconfig
USER dataflow
RUN cd /home/dataflow && \
rm -rf cdf38_1-dist*
VOLUME ["/home/dataflow/src"]
ENTRYPOINT ["/bin/bash"]



