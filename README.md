# GPUOcelot: A dynamic compilation framework for PTX

[Ocelot](http://gpuocelot.gatech.edu/) is a just-in-time compiler, which retargets PTX assembler (used internally by CUDA) for non-NVIDIA hardware.

We make Ocelot great again: yes, you can compile it in Ubuntu 22.04 with recent version of CUDA, and most of the tests are passing (see below).

## Building

```
git submodule init
git submodule update
mkdir build
cd build
cmake ..
make -j12
```

## Testing

```
cd build
ctest
```

Currently, the following status is reported:

```
Test project /home/marcusmae/gpuocelot/gpuocelot/ocelot/build
      Start  1: gpuocelot_analysis_TestDataflowGraph
 1/31 Test  #1: gpuocelot_analysis_TestDataflowGraph ............   Passed    0.01 sec
      Start  2: gpuocelot_api_TestDeviceSwitching
 2/31 Test  #2: gpuocelot_api_TestDeviceSwitching ...............   Passed    0.02 sec
      Start  3: gpuocelot_api_TestExternalFunctions
 3/31 Test  #3: gpuocelot_api_TestExternalFunctions .............   Passed    0.03 sec
      Start  4: gpuocelot_cuda_TestCudaGenericMemory
 4/31 Test  #4: gpuocelot_cuda_TestCudaGenericMemory ............   Passed    0.02 sec
      Start  5: gpuocelot_cuda_TestCudaGlobals
 5/31 Test  #5: gpuocelot_cuda_TestCudaGlobals ..................***Failed    0.17 sec
      Start  6: gpuocelot_cuda_TestCudaMalloc
 6/31 Test  #6: gpuocelot_cuda_TestCudaMalloc ...................   Passed    0.43 sec
      Start  7: gpuocelot_cuda_TestCudaNestedParallelism
 7/31 Test  #7: gpuocelot_cuda_TestCudaNestedParallelism ........   Passed    0.13 sec
      Start  8: gpuocelot_cuda_TestCudaSequence
 8/31 Test  #8: gpuocelot_cuda_TestCudaSequence .................   Passed    0.14 sec
      Start  9: gpuocelot_cuda_TestCudaTexture2D
 9/31 Test  #9: gpuocelot_cuda_TestCudaTexture2D ................   Passed    0.11 sec
      Start 10: gpuocelot_cuda_TestCudaTexture3D
10/31 Test #10: gpuocelot_cuda_TestCudaTexture3D ................   Passed    0.12 sec
      Start 11: gpuocelot_cuda_TestCudaTextureArray
11/31 Test #11: gpuocelot_cuda_TestCudaTextureArray .............   Passed    0.12 sec
      Start 12: gpuocelot_cuda_TestCudaTextureCube
12/31 Test #12: gpuocelot_cuda_TestCudaTextureCube ..............   Passed    0.13 sec
      Start 13: gpuocelot_cuda_TestFunctionCall
13/31 Test #13: gpuocelot_cuda_TestFunctionCall .................   Passed    0.13 sec
      Start 14: gpuocelot_cuda_TestIndirectFunctionCall
14/31 Test #14: gpuocelot_cuda_TestIndirectFunctionCall .........   Passed    0.12 sec
      Start 15: gpuocelot_cuda_TestIndirectFunctionCallDriver
15/31 Test #15: gpuocelot_cuda_TestIndirectFunctionCallDriver ...***Failed    0.01 sec
      Start 16: gpuocelot_cuda_TestIndirectFunctionCallOcelot
16/31 Test #16: gpuocelot_cuda_TestIndirectFunctionCallOcelot ...Subprocess aborted***Exception:   0.12 sec
      Start 17: gpuocelot_cuda_TestPTXAssembly
17/31 Test #17: gpuocelot_cuda_TestPTXAssembly ..................   Passed   10.40 sec
      Start 18: gpuocelot_executive_TestEmulator
18/31 Test #18: gpuocelot_executive_TestEmulator ................***Exception: SegFault  0.11 sec
      Start 19: gpuocelot_executive_TestInstructions
19/31 Test #19: gpuocelot_executive_TestInstructions ............   Passed    0.02 sec
      Start 20: gpuocelot_executive_TestKernels
20/31 Test #20: gpuocelot_executive_TestKernels .................   Passed    0.04 sec
      Start 21: gpuocelot_executive_TestLLVMKernels
21/31 Test #21: gpuocelot_executive_TestLLVMKernels .............   Passed    0.09 sec
      Start 22: gpuocelot_parser_TestLexer
22/31 Test #22: gpuocelot_parser_TestLexer ......................   Passed    0.01 sec
      Start 23: gpuocelot_parser_TestParser
23/31 Test #23: gpuocelot_parser_TestParser .....................   Passed    0.01 sec
      Start 24: gpuocelot_transforms_TestOptimizations
24/31 Test #24: gpuocelot_transforms_TestOptimizations ..........   Passed    0.01 sec
      Start 25: gpuocelot_translator_TestPTXToLLVMTranslator
25/31 Test #25: gpuocelot_translator_TestPTXToLLVMTranslator ....   Passed    0.01 sec
      Start 26: hydrazine_TestArgumentParser
26/31 Test #26: hydrazine_TestArgumentParser ....................   Passed    0.00 sec
      Start 27: hydrazine_TestMath
27/31 Test #27: hydrazine_TestMath ..............................   Passed    0.22 sec
      Start 28: hydrazine_TestThread
28/31 Test #28: hydrazine_TestThread ............................   Passed    0.03 sec
      Start 29: hydrazine_TestTimer
29/31 Test #29: hydrazine_TestTimer .............................   Passed    0.00 sec
      Start 30: hydrazine_TestXmlArgumentParser
30/31 Test #30: hydrazine_TestXmlArgumentParser .................   Passed    0.01 sec
      Start 31: hydrazine_TestXmlParser
31/31 Test #31: hydrazine_TestXmlParser .........................   Passed    0.01 sec

87% tests passed, 4 tests failed out of 31

Total Test time (real) =  12.82 sec

The following tests FAILED:
	  5 - gpuocelot_cuda_TestCudaGlobals (Failed)
	 15 - gpuocelot_cuda_TestIndirectFunctionCallDriver (Failed)
	 16 - gpuocelot_cuda_TestIndirectFunctionCallOcelot (Subprocess aborted)
	 18 - gpuocelot_executive_TestEmulator (SEGFAULT)
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

