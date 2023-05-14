# GPUOCelot: A dynamic compilation framework for PTX

Ocelot is a modular dynamic compilation framework for heterogeneous system, providing various backend targets for CUDA programs and analysis modules for the PTX virtual instruction set. **Ocelot currently allows CUDA programs to be executed on NVIDIA GPUs, AMD GPUs, and x86-CPUs at full speed without recompilation.** For more information, check [http://gpuocelot.gatech.edu/](http://gpuocelot.gatech.edu/).

## Building

```
git submodule init
git submodule update
docker build -t gpuocelot .
```

## Testing

```
docker run --rm -it gpuocelot bash
cd gpuocelot
./build.py  -t full
```

Currently, the following status is reported:

```
13 out of 20 tests passed.

Passing tests:
 (23.450s) : /usr/local/src/gpuocelot/.release_build/ocelot/TestParser : Passed
 (33.398s) : /usr/local/src/gpuocelot/.release_build/ocelot/TestDataflowGraph : Passed
 (36.570s) : /usr/local/src/gpuocelot/.release_build/ocelot/TestPTXToLLVMTranslator : Passed
 (0.015s) : /usr/local/src/gpuocelot/.release_build/ocelot/TestInstructions : Passed
 (0.012s) : /usr/local/src/gpuocelot/.release_build/ocelot/TestLLVMKernels : Passed
 (9.724s) : /usr/local/src/gpuocelot/.release_build/ocelot/TestLexer : Passed
 (0.006s) : /usr/local/src/gpuocelot/.release_build/ocelot/TestExternalFunctions : Passed
 (0.055s) : /usr/local/src/gpuocelot/.release_build/ocelot/TestCudaMalloc : Passed
 (0.008s) : /usr/local/src/gpuocelot/.release_build/ocelot/TestLLVMInstructions : Passed
 (0.013s) : /usr/local/src/gpuocelot/.release_build/ocelot/TestDeviceSwitching : Passed
 (0.023s) : /usr/local/src/gpuocelot/.release_build/ocelot/TestKernels : Passed
 (62.731s) : /usr/local/src/gpuocelot/.release_build/ocelot/TestOptimizations : Passed
 (0.020s) : /usr/local/src/gpuocelot/.release_build/ocelot/TestEmulator : Passed

Failing tests:
 (0.320s) : /usr/local/src/gpuocelot/.release_build/ocelot/TestCudaTexture2D : Did not complete.
 (0.181s) : /usr/local/src/gpuocelot/.release_build/ocelot/TestFunctionCall : Did not complete.
 (0.300s) : /usr/local/src/gpuocelot/.release_build/ocelot/TestIndirectFunctionCall : Did not complete.
 (2.777s) : /usr/local/src/gpuocelot/.release_build/ocelot/TestPTXAssembly : Did not complete.
 (0.259s) : /usr/local/src/gpuocelot/.release_build/ocelot/TestCudaTextureArray : Did not complete.
 (0.180s) : /usr/local/src/gpuocelot/.release_build/ocelot/TestCudaGlobals : Did not complete.
 (0.341s) : /usr/local/src/gpuocelot/.release_build/ocelot/TestCudaSequence : Did not complete.
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

