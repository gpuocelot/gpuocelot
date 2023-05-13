# GPUOCelot: A dynamic compilation framework for PTX

Ocelot is a modular dynamic compilation framework for heterogeneous system, providing various backend targets for CUDA programs and analysis modules for the PTX virtual instruction set. **Ocelot currently allows CUDA programs to be executed on NVIDIA GPUs, AMD GPUs, and x86-CPUs at full speed without recompilation.** For more information, check [http://gpuocelot.gatech.edu/](http://gpuocelot.gatech.edu/).

## Installation

Please check the installation guide [here](https://github.com/gtcasl/gpuocelot/wiki/Installation).

```
apt install libboost-all-dev  flex bison g++ scons clang libgl1-mesa-dev
```

```
git submodule init
git submodule update
```

```
python3 build.py --install
```

## Resources

* Source code documentation
[http://gpuocelot.gatech.edu/doxygen](http://gpuocelot.gatech.edu/doxygen)
* Mailing list
[http://groups.google.com/group/gpuocelot](http://groups.google.com/group/gpuocelot)

## Former developers

We would like to thank the following people, who have contributed novel ideas, software, and tests to the project:

* Nathan Bell
* Sylvain Collange
* David Luebke
* Diogo Sampaio
* Ryuta Suzuki
* Steve Worley
* Ignacio Llamas
* James Bigler
* Greg Humphreys

