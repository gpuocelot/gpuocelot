# Git clonning no longer work for Ubuntu 14.04,
# so we clone with another stage on newer distro.
FROM ubuntu:20.04 AS downloader

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install --no-install-recommends -y \
        git ca-certificates wget

WORKDIR /
RUN git clone https://gitlab.kitware.com/cmake/cmake.git && \
        cd cmake && \
        git checkout v3.21.4 && \
	rm -rf .git

# Download Boost
ENV BOOST_VERSION=1_63_0
RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.63.0/source/boost_${BOOST_VERSION}.tar.gz

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

# Build static boost with -fPIC, so that we could link it statically to libgpuocelot_llvm
WORKDIR /
ENV BOOST_VERSION=1_63_0
COPY --from=downloader /boost_${BOOST_VERSION}.tar.gz /
RUN tar xfz boost_${BOOST_VERSION}.tar.gz \
        && rm boost_${BOOST_VERSION}.tar.gz \
        && cd boost_${BOOST_VERSION} \
        && ./bootstrap.sh --prefix=/boost_install \
        && ./b2 cxxflags=-fPIC cflags=-fPIC install \
        && cd .. \
        && rm -rf boost_${BOOST_VERSION}

FROM ubuntu:14.04

ENV DEBIAN_FRONTEND noninteractive

COPY --from=builder /cmake_install/ /usr/local/
COPY --from=builder /boost_install/ /usr/local/

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
        gcc g++ llvm-3.4-dev \
        libbison-dev=1:2.5.dfsg-2.1ubuntu1 \
        libglew-dev \
        libxi-dev \
        libXmu-dev \
        make \
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

