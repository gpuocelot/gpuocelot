FROM nvidia/cuda:11.8.0-devel-ubuntu22.04 AS builder

RUN apt update &&\
	apt install --no-install-recommends -y g++ cmake ninja-build llvm-15-dev libz-dev libglew-dev flex bison libfl-dev libboost-thread-dev libboost-filesystem-dev nvidia-cuda-toolkit-gcc libzstd-dev ninja-build

COPY . /ocelot

RUN cd /ocelot && \
	mkdir build && \
	cd build && \
	cmake -DBUILD_TESTS=ON -G Ninja .. && \
	ninja
