# Git clonning no longer work for Ubuntu 14.04,
# so we clone with another stage on newer distro.
FROM ubuntu:20.04 AS downloader

ENV DEBIAN_FRONTEND noninteractive

RUN    apt-get update && apt-get install --no-install-recommends -y \
        git ca-certificates

WORKDIR /
RUN git clone https://gitlab.kitware.com/cmake/cmake.git && \
        cd cmake && \
        git checkout v3.21.4 && \
	rm -rf .git

FROM ubuntu:14.04 AS builder

ENV DEBIAN_FRONTEND noninteractive

COPY --from=downloader /cmake /cmake

RUN echo "#!/bin/sh\nexit 0" > /usr/sbin/policy-rc.d && \
    apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 0C69C6EC64E1C6DA && \
    printf "deb http://ppa.launchpad.net/hparch/gpuocelot/ubuntu trusty main" \
    > /etc/apt/sources.list.d/hparch-gpuocelot-trusty.list && \
    apt-get update && apt-get install --no-install-recommends -y \
        libssl-dev build-essential

WORKDIR /cmake
RUN ./bootstrap --prefix=/cmake_install --parallel=8 && \
        make -j8 && \
        make install

FROM ubuntu:14.04

ENV DEBIAN_FRONTEND noninteractive

COPY --from=builder /cmake_install/ /usr/local/

# Avoid ERROR: invoke-rc.d: policy-rc.d denied execution of start.
RUN echo "#!/bin/sh\nexit 0" > /usr/sbin/policy-rc.d && \
    apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 0C69C6EC64E1C6DA && \
    printf "deb http://ppa.launchpad.net/hparch/gpuocelot/ubuntu trusty main" \
    > /etc/apt/sources.list.d/hparch-gpuocelot-trusty.list && \
    apt-get update && apt-get install --no-install-recommends -y \
        bison=1:2.5.dfsg-2.1ubuntu1 \
        flex \
        freeglut3-dev \
        git \
        g++-4.8 llvm-3.4-dev \
        libbison-dev=1:2.5.dfsg-2.1ubuntu1 \
        libboost1.49-all-dev libboost1.49-dev \
        libglew-dev \
        libxi-dev \
        libXmu-dev \
        make \
        scons \
        vim \ 
        wget && \
    apt-mark hold libbison-dev bison

RUN mkdir /tmp/cuda_toolkit && \
    cd /tmp/cuda_toolkit && \
    wget http://developer.download.nvidia.com/compute/cuda/7.5/Prod/local_installers/cuda_7.5.18_linux.run && \
    chmod +x cuda_7.5.18_linux.run && \
    ./cuda_7.5.18_linux.run --tar mxvf

RUN cd /tmp/cuda_toolkit && \
    ./cuda-installer.pl -silent -toolkit && \
    cd / && rm -rf /tmp/cuda_toolkit && \
    printf "/usr/local/cuda/lib64\n/usr/local/cuda/lib" > /etc/ld.so.conf.d/cuda.conf && \
    sed -i "s|#error -- unsupported GNU version|#warning -- unsupported GNU version|g" /usr/local/cuda/include/host_config.h && \
    ldconfig

ENV PATH /usr/local/cuda/bin:$PATH

WORKDIR /usr/local/src
COPY . /usr/local/src/gpuocelot

#RUN cd gpuocelot/ocelot && mkdir build && cd build && cmake .. && make -j8

