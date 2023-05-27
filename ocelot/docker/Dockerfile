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

FROM ubuntu:18.04 AS builder

ENV DEBIAN_FRONTEND noninteractive

COPY --from=downloader /cmake /cmake

RUN apt-get update && apt-get install --no-install-recommends -y \
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

FROM ubuntu:18.04

ENV DEBIAN_FRONTEND noninteractive

COPY --from=builder /cmake_install/ /usr/local/
COPY --from=builder /boost_install/ /usr/local/

# Avoid ERROR: invoke-rc.d: policy-rc.d denied execution of start.
RUN apt-get update && apt-get install --no-install-recommends -y \
        freeglut3-dev \
        software-properties-common llvm-5.0-dev \
        libglew-dev \
        libz-dev \
        make \
        patch \
	fish \
        less \
        vim && \
    add-apt-repository -y ppa:ubuntu-toolchain-r/test && \
    apt-get update && apt-get install --no-install-recommends -y \
        gcc-9 g++-9

RUN update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-9 40 && \
	update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-9 40

