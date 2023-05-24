# Ocelot LLVM-based compiler engine

Ocelot uses very old version of LLVM, currently 3.4. Eventually we will bring it up to the current version of LLVM, but it may take time. Therefore, for now we offload Ocelot components with LLVM dependency to a separate CMake project. This project is meant to be statically build in a Docker sandbox, where a needed old version of LLVM is available. Then we take it as a binary dependency up to the Ocelot's main CMake project.

Yes, this solution is not clean: we link libraries with C++ interfaces built by different versions of C++ compiler. This will not work, if ABI is not the same. But we are still doing this, due to no better way.
