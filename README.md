# GPUOcelot: A dynamic compilation framework for PTX

[Ocelot](http://gpuocelot.gatech.edu/) is a just-in-time compiler, which retargets PTX assembler (used internally by CUDA) for non-NVIDIA hardware.

We make Ocelot great again: yes, you can compile it in Ubuntu 22.04 with LLVM 15, recent version of CUDA, and most of the tests are passing (see below).

## Features

* PTX 2.2 and Fermi device architecture: Floating point results should be within the ULP limits in the PTX ISA manual. Over 500 unit tests verify that the behaviour matches NVIDIA devices.

* Four backends: A functional PTX emulator. A PTX to LLVM to x86/ARM JIT. A PTX to CAL JIT for AMD devices (beta). A PTX to PTX JIT for NVIDIA devices.

* A full-featured PTX 2.2 IR: An analysis/optimization pass interface over PTX (Control flow graph, dataflow graph, dominator/postdominator trees, structured control tree). Optimizations can be plugged in as modules.

* Correctness checking tools: A memory checker (detects unaligned and out of bounds accesses). A race detector. An interactive debugger (allows stepping through PTX instructions).

* An instruction trace analyzer interface: Allows user-defined modules to receive callbacks when PTX instructions are executed. Can be used to compute metrics over applications or perform correctness checks.

* A CUDA API frontend: Existing CUDA programs can be directly linked against Ocelot. Device pointers can be shared across host threads. Multiple devices can be controlled from the same host thread (cudaSetDevice can be called multiple times).

## Building

- installing dependencies (ubuntu 22.04)  

```
sudo apt-get install -y g++ cmake ninja-build llvm-15-dev libz-dev libglew-dev flex bison libfl-dev libboost-thread-dev libboost-filesystem-dev nvidia-cuda-toolkit-gcc libzstd-dev
```

```
git submodule init
git submodule update
cd ocelot
mkdir build
cd build
cmake ..
make -j12
```

Building on other distros (currently only fedora 37 was tested) requires rebuilding llvm with `-DBUILD_LLVM=ON` and turning off cuda tests with `-DBUILD_TESTS_CUDA=OFF`, as newer version of nvcc (12.0+) shipped with most distros don't support the old `sm_35` architecture.

## Testing

```
cd build
cmake .. -DBUILD_TESTS=ON
ctest
```

The new version of Ocelot passes 100% of the original test suite:

```
Test project /home/marcusmae/gpuocelot/gpuocelot/ocelot/build
      Start  1: gpuocelot_analysis_TestDataflowGraph
 1/32 Test  #1: gpuocelot_analysis_TestDataflowGraph ............   Passed    0.01 sec
      Start  2: gpuocelot_api_TestDeviceSwitching
 2/32 Test  #2: gpuocelot_api_TestDeviceSwitching ...............   Passed    0.02 sec
      Start  3: gpuocelot_api_TestExternalFunctions
 3/32 Test  #3: gpuocelot_api_TestExternalFunctions .............   Passed    0.03 sec
      Start  4: gpuocelot_cuda_TestCudaGenericMemory
 4/32 Test  #4: gpuocelot_cuda_TestCudaGenericMemory ............   Passed    0.01 sec
      Start  5: gpuocelot_cuda_TestCudaGlobals
 5/32 Test  #5: gpuocelot_cuda_TestCudaGlobals ..................   Passed    0.16 sec
      Start  6: gpuocelot_cuda_TestCudaMalloc
 6/32 Test  #6: gpuocelot_cuda_TestCudaMalloc ...................   Passed    0.42 sec
      Start  7: gpuocelot_cuda_TestCudaNestedParallelism
 7/32 Test  #7: gpuocelot_cuda_TestCudaNestedParallelism ........   Passed    0.14 sec
      Start  8: gpuocelot_cuda_TestCudaSequence
 8/32 Test  #8: gpuocelot_cuda_TestCudaSequence .................   Passed    0.12 sec
      Start  9: gpuocelot_cuda_TestCudaTexture2D
 9/32 Test  #9: gpuocelot_cuda_TestCudaTexture2D ................   Passed    0.12 sec
      Start 10: gpuocelot_cuda_TestCudaTexture3D
10/32 Test #10: gpuocelot_cuda_TestCudaTexture3D ................   Passed    0.13 sec
      Start 11: gpuocelot_cuda_TestCudaTextureArray
11/32 Test #11: gpuocelot_cuda_TestCudaTextureArray .............   Passed    0.12 sec
      Start 12: gpuocelot_cuda_TestCudaTextureCube
12/32 Test #12: gpuocelot_cuda_TestCudaTextureCube ..............   Passed    0.13 sec
      Start 13: gpuocelot_cuda_TestFunctionCall
13/32 Test #13: gpuocelot_cuda_TestFunctionCall .................   Passed    0.13 sec
      Start 14: gpuocelot_cuda_TestIndirectFunctionCall
14/32 Test #14: gpuocelot_cuda_TestIndirectFunctionCall .........   Passed    0.12 sec
      Start 15: gpuocelot_cuda_TestIndirectFunctionCallDriver
15/32 Test #15: gpuocelot_cuda_TestIndirectFunctionCallDriver ...   Passed    0.01 sec
      Start 16: gpuocelot_cuda_TestIndirectFunctionCallOcelot
16/32 Test #16: gpuocelot_cuda_TestIndirectFunctionCallOcelot ...   Passed    0.01 sec
      Start 17: gpuocelot_cuda_TestPTXAssembly
17/32 Test #17: gpuocelot_cuda_TestPTXAssembly ..................   Passed   10.13 sec
      Start 18: gpuocelot_executive_TestEmulator
18/32 Test #18: gpuocelot_executive_TestEmulator ................   Passed    0.01 sec
      Start 19: gpuocelot_executive_TestInstructions
19/32 Test #19: gpuocelot_executive_TestInstructions ............   Passed    0.01 sec
      Start 20: gpuocelot_executive_TestKernels
20/32 Test #20: gpuocelot_executive_TestKernels .................   Passed    0.02 sec
      Start 21: gpuocelot_executive_TestLLVMKernels
21/32 Test #21: gpuocelot_executive_TestLLVMKernels .............   Passed    0.06 sec
      Start 22: gpuocelot_ir_TestLLVMInstructions
22/32 Test #22: gpuocelot_ir_TestLLVMInstructions ...............   Passed    0.01 sec
      Start 23: gpuocelot_parser_TestLexer
23/32 Test #23: gpuocelot_parser_TestLexer ......................   Passed    0.01 sec
      Start 24: gpuocelot_parser_TestParser
24/32 Test #24: gpuocelot_parser_TestParser .....................   Passed    0.01 sec
      Start 25: gpuocelot_transforms_TestOptimizations
25/32 Test #25: gpuocelot_transforms_TestOptimizations ..........   Passed    0.01 sec
      Start 26: gpuocelot_translator_TestPTXToLLVMTranslator
26/32 Test #26: gpuocelot_translator_TestPTXToLLVMTranslator ....   Passed    0.01 sec
      Start 27: hydrazine_TestArgumentParser
27/32 Test #27: hydrazine_TestArgumentParser ....................   Passed    0.00 sec
      Start 28: hydrazine_TestMath
28/32 Test #28: hydrazine_TestMath ..............................   Passed    0.04 sec
      Start 29: hydrazine_TestThread
29/32 Test #29: hydrazine_TestThread ............................   Passed    0.03 sec
      Start 30: hydrazine_TestTimer
30/32 Test #30: hydrazine_TestTimer .............................   Passed    0.00 sec
      Start 31: hydrazine_TestXmlArgumentParser
31/32 Test #31: hydrazine_TestXmlArgumentParser .................   Passed    0.01 sec
      Start 32: hydrazine_TestXmlParser
32/32 Test #32: hydrazine_TestXmlParser .........................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 32

Total Test time (real) =  12.08 sec
```

## Debugging

Debugging alongside with LLVM is often desired for fixing Ocelot issues. In order to simplify the process, we offer an option to build LLVM from source. It takes time, but the produced Ocelot shared library will have all debug info:

```
cmake -DBUILD_LLVM=ON -DCMAKE_BUILD_TYPE=Debug ..
```

## Resources

* Source code documentation
[http://gpuocelot.gatech.edu/doxygen](http://gpuocelot.gatech.edu/doxygen)
* Mailing list
[http://groups.google.com/group/gpuocelot](http://groups.google.com/group/gpuocelot)

## Former developers

* Gregory Daimos
* Andrew Kerr
* Naila Farooqui
* Jin Wang
* Si Li
* Haicheng Wu
* Sudnya Diamos
* Prasun Gera 
* Nathan Bell
* Sylvain Collange
* David Luebke
* Diogo Sampaio
* Ryuta Suzuki
* Steve Worley
* Ignacio Llamas
* James Bigler
* Greg Humphreys

