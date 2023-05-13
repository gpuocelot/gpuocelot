FROM ubuntu:trusty

ENV DEBIAN_FRONTEND noninteractive

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
    g++-4.7 clang++-3.4 libc++-dev \
    libbison-dev=1:2.5.dfsg-2.1ubuntu1 \
    libboost1.49-all-dev libboost1.49-dev libboost-mpi-python1.49-dev libboost-mpi-python1.49.0 \
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
    wget http://developer.download.nvidia.com/compute/cuda/5_0/rel-update-1/installers/cuda_5.0.35_linux_64_ubuntu11.10-1.run && \
    chmod +x cuda_5.0.35_linux_64_ubuntu11.10-1.run && \
    ./cuda_5.0.35_linux_64_ubuntu11.10-1.run --tar mxvf

RUN cd /tmp/cuda_toolkit && \
    ./cuda-installer.pl -silent -toolkit && \
    cd / && rm -rf /tmp/cuda_toolkit && \
    printf "/usr/local/cuda/lib64\n/usr/local/cuda/lib" > /etc/ld.so.conf.d/cuda.conf && \
    sed -i "s|#error -- unsupported GNU version|#warning -- unsupported GNU version|g" /usr/local/cuda/include/host_config.h && \
    ldconfig

#ENV PATH /usr/local/cuda/bin:$PATH

#RUN update-alternatives --install /usr/bin/cc cc /usr/bin/clang 40 && \
#    update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 40

#WORKDIR /usr/local/src
#COPY . /usr/local/src/gpuocelot

#RUN cd gpuocelot && ./build.py --thread 4 --install && \
#    ldconfig && \ 
#    cd trace-generators && scons install && \
#    ldconfig
