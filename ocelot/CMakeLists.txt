cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project(gpuocelot CXX CUDA)

set(CMAKE_CUDA_ARCHITECTURES 35)
set(CMAKE_CUDA_SEPARABLE_COMPILATION ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(LLVM 3.4 REQUIRED)
find_package(FLEX 2.5 REQUIRED)
find_package(BISON 2.5 REQUIRED)
find_package(GLEW REQUIRED)

add_subdirectory(ThirdParty/hydrazine)

function(string_starts_with str search result)
	string(FIND "${str}" "${search}" out)
	if("${out}" EQUAL 0)
		set(${result} TRUE PARENT_SCOPE)
	else()
		set(${result} FALSE PARENT_SCOPE)
	endif()
endfunction()

string(REPLACE " " ";" LLVM_FLAGS_LIST "${LLVM_CXXFLAGS}")

foreach(ARG ${LLVM_FLAGS_LIST})
	set(STARTS FALSE)
	string_starts_with("${ARG}" "-D" STARTS)
	if (STARTS)
		# Must ignore -DNDEBUG, otherwise our assertions in Ocelot
		# will always be disabled.
		string_starts_with("${ARG}" "-DNDEBUG" STARTS)
		if (NOT STARTS)
			list(APPEND ${PROJECT_NAME}_DEFINITIONS ${ARG})
		endif()
	else()
		# TODO Currently, Ocelot uses typeid, while LLVM comes with -fno-rtti
		# list(APPEND ${PROJECT_NAME}_CXXFLAGS ${ARG})
	endif()
endforeach()

set(${PROJECT_NAME}_INCLUDE_DIRS
	${LLVM_INCLUDE_DIRS}
	${GLEW_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR})

set(${PROJECT_NAME}_LIBRARIES
	${LLVM_LIBRARIES}
	${GLEW_LIBRARIES}	
	hydrazine
	Boost::system
	tinfo
	dl)
	
set(${PROJECT_NAME}_LIBRARY_DIRS
	${LLVM_LIBRARY_DIRS})

include(CTest)

function(ocelot_add_tests name)
	file(GLOB TESTS "src/${name}/test/Test*.cpp" "src/${name}/test/Test*.cu")
	foreach(TEST ${TESTS})
		    get_filename_component(TEST_WE ${TEST} NAME_WE)
		    set(TEST_NAME ${PROJECT_NAME}_${name}_${TEST_WE})
		    add_executable(${TEST_NAME} ${TEST})
		    set_property(TARGET ${TEST_NAME} PROPERTY CXX_STANDARD 11)
		    target_compile_definitions(${TEST_NAME} PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
			target_compile_options(${TEST_NAME} PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
			target_include_directories(${TEST_NAME} PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
			target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/${name})
			target_link_libraries(${TEST_NAME} PRIVATE ${${PROJECT_NAME}_LIBRARIES})
		    target_link_libraries(${TEST_NAME} PRIVATE ${PROJECT_NAME})
			target_link_directories(${TEST_NAME} PRIVATE ${${PROJECT_NAME}_LIBRARY_DIRS})
		    add_test(NAME ${TEST_NAME} COMMAND $<TARGET_FILE:${TEST_NAME}> -v)
	endforeach()
endfunction()

add_library(${PROJECT_NAME}_ir STATIC
	src/ir/ControlFlowGraph.cpp
	src/ir/Dim3.cpp
	src/ir/ExternalFunctionSet.cpp
	src/ir/Global.cpp
	src/ir/ILInstruction.cpp
	src/ir/ILKernel.cpp
	src/ir/ILOperand.cpp
	src/ir/ILStatement.cpp
	src/ir/IRKernel.cpp
	src/ir/Instruction.cpp
	src/ir/Kernel.cpp
	src/ir/LLVMInstruction.cpp
	src/ir/LLVMKernel.cpp
	src/ir/LLVMStatement.cpp
	src/ir/Local.cpp
	src/ir/Module.cpp
	src/ir/PTXInstruction.cpp
	src/ir/PTXKernel.cpp
	src/ir/PTXOperand.cpp
	src/ir/PTXStatement.cpp
	src/ir/Parameter.cpp
	src/ir/Texture.cpp)
set_property(TARGET ${PROJECT_NAME}_ir PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROJECT_NAME}_ir PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME}_ir PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME}_ir PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
target_include_directories(${PROJECT_NAME}_ir PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_ir PUBLIC ${${PROJECT_NAME}_LIBRARIES})
target_link_directories(${PROJECT_NAME}_ir PUBLIC ${${PROJECT_NAME}_LIBRARY_DIRS})

add_library(${PROJECT_NAME}_analysis STATIC
	src/analysis/AffineAnalysis.cpp
	src/analysis/Analysis.cpp
	src/analysis/AnalysisFactory.cpp
	src/analysis/BranchInfo.cpp
	src/analysis/ControlDependenceAnalysis.cpp
	src/analysis/ControlTree.cpp
	src/analysis/ConvergentRegionAnalysis.cpp
	src/analysis/CycleAnalysis.cpp
	src/analysis/DataDependenceAnalysis.cpp
	src/analysis/DataflowGraph.cpp
	src/analysis/DependenceAnalysis.cpp
	src/analysis/DirectionalGraph.cpp
	src/analysis/DivergenceAnalysis.cpp
	src/analysis/DivergenceGraph.cpp
	src/analysis/DominatorTree.cpp
	src/analysis/HammockGraphAnalysis.cpp
	src/analysis/LoopAnalysis.cpp
	src/analysis/MemoryDependenceAnalysis.cpp
	src/analysis/PTXInstructionDependenceGraph.cpp
	src/analysis/PostdominatorTree.cpp
	src/analysis/PredicateAnalysis.cpp
	src/analysis/ProgramStructureGraph.cpp
	src/analysis/SSAGraph.cpp
	src/analysis/SafeRegionAnalysis.cpp
	src/analysis/SimpleAliasAnalysis.cpp
	src/analysis/SparseAnalysis.cpp
	src/analysis/StructuralAnalysis.cpp
	src/analysis/SuperblockAnalysis.cpp
	src/analysis/ThreadFrontierAnalysis.cpp)
set_property(TARGET ${PROJECT_NAME}_analysis PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROJECT_NAME}_analysis PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME}_analysis PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME}_analysis PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
target_include_directories(${PROJECT_NAME}_analysis PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_analysis PUBLIC ${${PROJECT_NAME}_LIBRARIES})
target_link_directories(${PROJECT_NAME}_analysis PUBLIC ${${PROJECT_NAME}_LIBRARY_DIRS})

add_library(${PROJECT_NAME}_api STATIC
	src/api/OcelotConfiguration.cpp
	src/api/OcelotRuntime.cpp
	src/api/ocelot.cpp)
set_property(TARGET ${PROJECT_NAME}_api PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROJECT_NAME}_api PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME}_api PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME}_api PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
target_include_directories(${PROJECT_NAME}_api PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_api PUBLIC ${${PROJECT_NAME}_LIBRARIES})
target_link_directories(${PROJECT_NAME}_api PUBLIC ${${PROJECT_NAME}_LIBRARY_DIRS})

add_library(${PROJECT_NAME}_cal STATIC
	src/cal/CalDriver.cpp)
set_property(TARGET ${PROJECT_NAME}_cal PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROJECT_NAME}_cal PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME}_cal PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME}_cal PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
target_include_directories(${PROJECT_NAME}_cal PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_cal PUBLIC ${${PROJECT_NAME}_LIBRARIES})
target_link_directories(${PROJECT_NAME}_cal PUBLIC ${${PROJECT_NAME}_LIBRARY_DIRS})
	
add_library(${PROJECT_NAME}_cuda STATIC
	src/cuda/CudaDriver.cpp
	src/cuda/CudaDriverApi.cpp
	src/cuda/CudaDriverFrontend.cpp
	src/cuda/CudaDriverInterface.cpp
	src/cuda/CudaRuntime.cpp
	src/cuda/CudaRuntimeInterface.cpp
	src/cuda/CudaWorkerThread.cpp
	src/cuda/FatBinaryContext.cpp
	src/cuda/cuda_runtime.cpp)
set_property(TARGET ${PROJECT_NAME}_cuda PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROJECT_NAME}_cuda PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME}_cuda PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME}_cuda PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
target_include_directories(${PROJECT_NAME}_cuda PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_cuda PUBLIC ${${PROJECT_NAME}_LIBRARIES})
target_link_directories(${PROJECT_NAME}_cuda PUBLIC ${${PROJECT_NAME}_LIBRARY_DIRS})
	
add_library(${PROJECT_NAME}_executive STATIC
	src/executive/ATIExecutableKernel.cpp
	src/executive/ATIGPUDevice.cpp
	src/executive/CTAContext.cpp
	src/executive/CooperativeThreadArray.cpp
	src/executive/Device.cpp
	src/executive/EmulatedKernel.cpp
	src/executive/EmulatedKernelScheduler.cpp
	src/executive/EmulatorCallStack.cpp
	src/executive/EmulatorDevice.cpp
	src/executive/ExecutableKernel.cpp
	src/executive/FrameInfo.cpp
	src/executive/LLVMContext.cpp
	src/executive/LLVMCooperativeThreadArray.cpp
	src/executive/LLVMExecutableKernel.cpp
	src/executive/LLVMExecutionManager.cpp
	src/executive/LLVMFunctionCallStack.cpp
	src/executive/LLVMModuleManager.cpp
	src/executive/LLVMState.cpp
	src/executive/LLVMWorkerThread.cpp
	src/executive/MulticoreCPUDevice.cpp
	src/executive/NVIDIAExecutableKernel.cpp
	src/executive/NVIDIAGPUDevice.cpp
	src/executive/PassThroughDevice.cpp
	src/executive/ReconvergenceMechanism.cpp
	src/executive/RemoteDevice.cpp
	src/executive/RuntimeException.cpp
	src/executive/TextureOperations.cpp)
set_property(TARGET ${PROJECT_NAME}_executive PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROJECT_NAME}_executive PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME}_executive PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME}_executive PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
target_include_directories(${PROJECT_NAME}_executive PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_executive PUBLIC ${${PROJECT_NAME}_LIBRARIES})
target_link_directories(${PROJECT_NAME}_executive PUBLIC ${${PROJECT_NAME}_LIBRARY_DIRS})

set(LEXER_OUT "${CMAKE_CURRENT_BINARY_DIR}/ptx.cpp")
set(PARSER_OUT "${CMAKE_CURRENT_BINARY_DIR}/ptxgrammar.cpp")

flex_target(LEXER "src/parser/ptx.ll" "${LEXER_OUT}" DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/ptx.hpp")
bison_target(PARSER "src/parser/ptxgrammar.yy" "${PARSER_OUT}" DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/ptxgrammar.hpp")
add_flex_bison_dependency(LEXER PARSER)

add_library(${PROJECT_NAME}_parser STATIC
	src/parser/PTXLexer.cpp
	src/parser/PTXParser.cpp
	"${LEXER_OUT}"
	"${PARSER_OUT}")
set_property(TARGET ${PROJECT_NAME}_parser PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROJECT_NAME}_parser PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME}_parser PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME}_parser PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
target_include_directories(${PROJECT_NAME}_parser PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME}_parser PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME}_parser PUBLIC ${${PROJECT_NAME}_LIBRARIES})
target_link_directories(${PROJECT_NAME}_parser PUBLIC ${${PROJECT_NAME}_LIBRARY_DIRS})

add_library(${PROJECT_NAME}_trace STATIC
	src/trace/InteractiveDebugger.cpp
	src/trace/KernelTimer.cpp
	src/trace/MemoryChecker.cpp
	src/trace/MemoryRaceDetector.cpp
	src/trace/TraceEvent.cpp
	src/trace/TraceGenerator.cpp)
set_property(TARGET ${PROJECT_NAME}_trace PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROJECT_NAME}_trace PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME}_trace PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME}_trace PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
target_include_directories(${PROJECT_NAME}_trace PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_trace PUBLIC ${${PROJECT_NAME}_LIBRARIES})
target_link_directories(${PROJECT_NAME}_trace PUBLIC ${${PROJECT_NAME}_LIBRARY_DIRS})

add_library(${PROJECT_NAME}_transforms STATIC
	src/transforms/AddLocationMetadata.cpp
	src/transforms/AffineLinearScan.cpp
	src/transforms/AffineRegister.cpp
	src/transforms/AggressivePredication.cpp
	src/transforms/AssignFallThroughEdge.cpp
	src/transforms/CoalescedRegister.cpp
	src/transforms/ConstantPropagationPass.cpp
	src/transforms/ConvertPredicationToSelectPass.cpp
	src/transforms/DeadCodeEliminationPass.cpp
	src/transforms/DefaultLayoutPass.cpp
	src/transforms/DivergenceLinearScan.cpp
	src/transforms/DivergenceRegister.cpp
	src/transforms/EnforceLockStepExecutionPass.cpp
	src/transforms/FunctionInliningPass.cpp
	src/transforms/GlobalValueNumberingPass.cpp
	src/transforms/HoistParameterLoadsPass.cpp
	src/transforms/HoistSpecialValueDefinitionsPass.cpp
	src/transforms/IPDOMReconvergencePass.cpp
	src/transforms/LinearScanRegisterAllocationPass.cpp
	src/transforms/LoopUnrollingPass.cpp
	src/transforms/MIMDThreadSchedulingPass.cpp
	src/transforms/MemoryArray.cpp
	src/transforms/ModuleLinkerPass.cpp
	src/transforms/MoveEliminationPass.cpp
	src/transforms/Pass.cpp
	src/transforms/PassFactory.cpp
	src/transforms/PassManager.cpp
	src/transforms/PriorityLayoutPass.cpp
	src/transforms/ReadableLayoutPass.cpp
	src/transforms/RemoveBarrierPass.cpp
	src/transforms/SharedPtrAttribute.cpp
	src/transforms/SimplifyControlFlowGraphPass.cpp
	src/transforms/SimplifyExternalCallsPass.cpp
	src/transforms/SpillPolicy.cpp
	src/transforms/SplitBasicBlockPass.cpp
	src/transforms/StructuralTransform.cpp
	src/transforms/SubkernelFormationPass.cpp
	src/transforms/SyncEliminationPass.cpp
	src/transforms/ThreadFrontierReconvergencePass.cpp)
set_property(TARGET ${PROJECT_NAME}_transforms PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROJECT_NAME}_transforms PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME}_transforms PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME}_transforms PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
target_include_directories(${PROJECT_NAME}_transforms PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_transforms PUBLIC ${${PROJECT_NAME}_LIBRARIES})
target_link_directories(${PROJECT_NAME}_transforms PUBLIC ${${PROJECT_NAME}_LIBRARY_DIRS})

add_library(${PROJECT_NAME}_translator STATIC
	src/translator/PTXToILTranslator.cpp
	src/translator/PTXToLLVMTranslator.cpp
	src/translator/Translator.cpp)
set_property(TARGET ${PROJECT_NAME}_translator PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROJECT_NAME}_translator PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME}_translator PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME}_translator PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
target_include_directories(${PROJECT_NAME}_translator PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_translator PUBLIC ${${PROJECT_NAME}_LIBRARIES})
target_link_directories(${PROJECT_NAME}_translator PUBLIC ${${PROJECT_NAME}_LIBRARY_DIRS})

add_library(${PROJECT_NAME}_util STATIC
	src/util/ExtractedDeviceState.cpp
	src/util/OcelotServerConnection.cpp
	src/util/RemoteDeviceMessage.cpp)
set_property(TARGET ${PROJECT_NAME}_util PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROJECT_NAME}_util PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME}_util PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME}_util PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
target_include_directories(${PROJECT_NAME}_util PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_util PUBLIC ${${PROJECT_NAME}_LIBRARIES})
target_link_directories(${PROJECT_NAME}_util PUBLIC ${${PROJECT_NAME}_LIBRARY_DIRS})

add_library(${PROJECT_NAME} SHARED src/ocelot.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE
	"-Wl,--whole-archive"
	${PROJECT_NAME}_analysis
	${PROJECT_NAME}_api
	${PROJECT_NAME}_cal
	${PROJECT_NAME}_cuda
	${PROJECT_NAME}_executive
	${PROJECT_NAME}_ir
	${PROJECT_NAME}_parser
	${PROJECT_NAME}_trace
	${PROJECT_NAME}_transforms
	${PROJECT_NAME}_translator
	${PROJECT_NAME}_util
	"-Wl,--no-whole-archive")
target_link_libraries(${PROJECT_NAME} PUBLIC
	Boost::system Boost::filesystem)

add_executable(LoadPtx src/tools/LoadPtx.cpp)
set_property(TARGET LoadPtx PROPERTY CXX_STANDARD 11)
target_link_libraries(LoadPtx ${PROJECT_NAME} hydrazine)

#add_executable(OcelotConfig src/tools/OcelotConfig.cpp)
#set_property(TARGET OcelotConfig PROPERTY CXX_STANDARD 11)
#target_link_libraries(OcelotConfig ${PROJECT_NAME} hydrazine)

add_executable(OcelotKernelTestHarness src/tools/KernelTestHarness.cpp)
set_property(TARGET OcelotKernelTestHarness PROPERTY CXX_STANDARD 11)
target_link_libraries(OcelotKernelTestHarness ${PROJECT_NAME} hydrazine)

add_executable(OcelotLinker src/tools/OcelotLinker.cpp)
set_property(TARGET OcelotLinker PROPERTY CXX_STANDARD 11)
target_link_libraries(OcelotLinker ${PROJECT_NAME} hydrazine)

add_executable(OcelotServer src/tools/OcelotServer.cpp)
set_property(TARGET OcelotServer PROPERTY CXX_STANDARD 11)
target_link_libraries(OcelotServer ${PROJECT_NAME} hydrazine)

add_executable(PTXOptimizer src/tools/PTXOptimizer.cpp)
set_property(TARGET PTXOptimizer PROPERTY CXX_STANDARD 11)
target_link_libraries(PTXOptimizer ${PROJECT_NAME} hydrazine)

ocelot_add_tests(analysis)
ocelot_add_tests(api)
ocelot_add_tests(cal)
ocelot_add_tests(cuda)
ocelot_add_tests(executive)
ocelot_add_tests(ir)
ocelot_add_tests(parser)
ocelot_add_tests(trace)
ocelot_add_tests(transforms)
ocelot_add_tests(translator)
ocelot_add_tests(util)

#++ -o .release_build/ocelot/TestCudaGenericMemory
#++ -o .release_build/ocelot/TestCudaGlobals
#++ -o .release_build/ocelot/TestCudaMalloc
#++ -o .release_build/ocelot/TestCudaSequence
#++ -o .release_build/ocelot/TestCudaTexture2D
#++ -o .release_build/ocelot/TestCudaTexture3D
#++ -o .release_build/ocelot/TestCudaTextureArray
#++ -o .release_build/ocelot/TestCudaTextureCube
#++ -o .release_build/ocelot/TestDataflowGraph
#++ -o .release_build/ocelot/TestDeviceSwitching
#++ -o .release_build/ocelot/TestDriverAPISequence
#++ -o .release_build/ocelot/TestEmulator
#++ -o .release_build/ocelot/TestExternalFunctions
#++ -o .release_build/ocelot/TestFunctionCall
#++ -o .release_build/ocelot/TestIndirectFunctionCall
#++ -o .release_build/ocelot/TestIndirectFunctionCallOcelot
#++ -o .release_build/ocelot/TestInstructions
#++ -o .release_build/ocelot/TestKernels
#++ -o .release_build/ocelot/TestLLVMInstructions
#++ -o .release_build/ocelot/TestLLVMKernels
#++ -o .release_build/ocelot/TestLexer
#++ -o .release_build/ocelot/TestOptimizations
#++ -o .release_build/ocelot/TestPTXAssembly
#++ -o .release_build/ocelot/TestPTXToLLVMTranslator
#++ -o .release_build/ocelot/TestParser
#++ -o .release_build/ocelot/libocelot.so
#++ -o .release_build/tests/cuda2.2/AlignedTypes
#++ -o .release_build/tests/cuda2.2/AsyncAPI
#++ -o .release_build/tests/cuda2.2/BandwidthTest
#++ -o .release_build/tests/cuda2.2/BicubicTexture
#++ -o .release_build/tests/cuda2.2/BinomialOptions
#++ -o .release_build/tests/cuda2.2/Bitonic
#++ -o .release_build/tests/cuda2.2/BlackScholes
#++ -o .release_build/tests/cuda2.2/BoxFilter
#++ -o .release_build/tests/cuda2.2/Clock
#++ -o .release_build/tests/cuda2.2/ConvolutionFFT2D
#++ -o .release_build/tests/cuda2.2/ConvolutionSeparable
#++ -o .release_build/tests/cuda2.2/ConvolutionTexture
#++ -o .release_build/tests/cuda2.2/CppIntegration
#++ -o .release_build/tests/cuda2.2/Dct8x8
#++ -o .release_build/tests/cuda2.2/DeviceQuery
#++ -o .release_build/tests/cuda2.2/DwtHaar1D
#++ -o .release_build/tests/cuda2.2/Dxtc
#++ -o .release_build/tests/cuda2.2/Eigenvalues
#++ -o .release_build/tests/cuda2.2/FastWalshTransform
#++ -o .release_build/tests/cuda2.2/FluidsGL
#++ -o .release_build/tests/cuda2.2/Histogram256
#++ -o .release_build/tests/cuda2.2/Histogram64
#++ -o .release_build/tests/cuda2.2/ImageDenoising
#++ -o .release_build/tests/cuda2.2/Mandelbrot
#++ -o .release_build/tests/cuda2.2/MatrixMul
#++ -o .release_build/tests/cuda2.2/MersenneTwister
#++ -o .release_build/tests/cuda2.2/MonteCarlo
#++ -o .release_build/tests/cuda2.2/Nbody
#++ -o .release_build/tests/cuda2.2/OceanFFT
#++ -o .release_build/tests/cuda2.2/Particles
#++ -o .release_build/tests/cuda2.2/PostProcessGL
#++ -o .release_build/tests/cuda2.2/QuasirandomGenerator
#++ -o .release_build/tests/cuda2.2/RecursiveGaussian
#++ -o .release_build/tests/cuda2.2/Reduction
#++ -o .release_build/tests/cuda2.2/ScalarProd
#++ -o .release_build/tests/cuda2.2/Scan
#++ -o .release_build/tests/cuda2.2/ScanLargeArray
#++ -o .release_build/tests/cuda2.2/SimpleAtomicIntrinsics
#++ -o .release_build/tests/cuda2.2/SimpleCUFFT
#++ -o .release_build/tests/cuda2.2/SimpleGL
#++ -o .release_build/tests/cuda2.2/SimpleMultiGPU
#++ -o .release_build/tests/cuda2.2/SimpleStreams
#++ -o .release_build/tests/cuda2.2/SimpleTemplates
#++ -o .release_build/tests/cuda2.2/SimpleTexture
#++ -o .release_build/tests/cuda2.2/SimpleTexture3D
#++ -o .release_build/tests/cuda2.2/SimpleVoteIntrinsics
#++ -o .release_build/tests/cuda2.2/SimpleZeroCopy
#++ -o .release_build/tests/cuda2.2/SobolQRNG
#++ -o .release_build/tests/cuda2.2/Template
#++ -o .release_build/tests/cuda2.2/ThreadFenceReduction
#++ -o .release_build/tests/cuda2.2/Transpose
#++ -o .release_build/tests/cuda2.2/TransposeNew
#++ -o .release_build/tests/cuda2.2/UnstructuredMandelbrot
#++ -o .release_build/tests/cuda2.2/VolumeRender
#++ -o .release_build/tests/cuda2.3/3DFD
#++ -o .release_build/tests/cuda2.3/AtomicPerformance
#++ -o .release_build/tests/cuda2.3/SortingNetwork
#++ -o .release_build/tests/cuda2.3/TestBarriers
#++ -o .release_build/tests/cuda2.3/TestCallLatency
#++ -o .release_build/tests/cuda2.3/TestInstructionThroughput
#++ -o .release_build/tests/cuda2.3/TestMemoryAccesses
#++ -o .release_build/tests/cuda2.3/TestSpecialFunctions
#++ -o .release_build/tests/cuda2.3/TestThrustAOSSorting
#++ -o .release_build/tests/cuda2.3/TestThrustArbitraryTransform
#++ -o .release_build/tests/cuda2.3/TestThrustBasicVector
#++ -o .release_build/tests/cuda2.3/TestThrustBoundingBox
#++ -o .release_build/tests/cuda2.3/TestThrustBucketSort2D
#++ -o .release_build/tests/cuda2.3/TestThrustConstantIterator
#++ -o .release_build/tests/cuda2.3/TestThrustCountingIterator
#++ -o .release_build/tests/cuda2.3/TestThrustFillCopy
#++ -o .release_build/tests/cuda2.3/TestThrustHistogram
#++ -o .release_build/tests/cuda2.3/TestThrustMaxAbsDiff
#++ -o .release_build/tests/cuda2.3/TestThrustMinMax
#++ -o .release_build/tests/cuda2.3/TestThrustMode
#++ -o .release_build/tests/cuda2.3/TestThrustNorm
#++ -o .release_build/tests/cuda2.3/TestThrustPaddedGridReduction
#++ -o .release_build/tests/cuda2.3/TestThrustSaxpy
#++ -o .release_build/tests/cuda2.3/TestThrustSort
#++ -o .release_build/tests/cuda2.3/TestThrustStreamCompact
#++ -o .release_build/tests/cuda2.3/TestThrustSum
#++ -o .release_build/tests/cuda2.3/TestThrustTransformIterator
#++ -o .release_build/tests/cuda2.3/TestThrustUnwrapPointer
#++ -o .release_build/tests/cuda2.3/TestThrustWrapPointer
#++ -o .release_build/tests/cuda2.3/TestThrustZipDot
#++ -o .release_build/tests/cuda2.3/vectorAdd
#++ -o .release_build/tests/cuda2.3/worleyTestExternalSharedMemory
#++ -o .release_build/tests/cuda3.2/AlignedTypes
#++ -o .release_build/tests/cuda3.2/AsyncAPI
#++ -o .release_build/tests/cuda3.2/BandwidthTest
#++ -o .release_build/tests/cuda3.2/BicubicTexture
#++ -o .release_build/tests/cuda3.2/BinomialOptions
#++ -o .release_build/tests/cuda3.2/BlackScholes
#++ -o .release_build/tests/cuda3.2/BoxFilter
#++ -o .release_build/tests/cuda3.2/Clock
#++ -o .release_build/tests/cuda3.2/ConvolutionFFT2D
#++ -o .release_build/tests/cuda3.2/ConvolutionSeparable
#++ -o .release_build/tests/cuda3.2/ConvolutionTexture
#++ -o .release_build/tests/cuda3.2/CppIntegration
#++ -o .release_build/tests/cuda3.2/Dct8x8
#++ -o .release_build/tests/cuda3.2/DeviceQuery
#++ -o .release_build/tests/cuda3.2/DwtHaar1D
#++ -o .release_build/tests/cuda3.2/Dxtc
#++ -o .release_build/tests/cuda3.2/Eigenvalues
#++ -o .release_build/tests/cuda3.2/FastWalshTransform
#++ -o .release_build/tests/cuda3.2/FluidsGL
#++ -o .release_build/tests/cuda3.2/Histogram256
#++ -o .release_build/tests/cuda3.2/ImageDenoising
#++ -o .release_build/tests/cuda3.2/Mandelbrot
#++ -o .release_build/tests/cuda3.2/MatrixMul
#++ -o .release_build/tests/cuda3.2/MersenneTwister
#++ -o .release_build/tests/cuda3.2/MonteCarlo
#++ -o .release_build/tests/cuda3.2/Nbody
#++ -o .release_build/tests/cuda3.2/OceanFFT
#++ -o .release_build/tests/cuda3.2/PostProcessGL
#++ -o .release_build/tests/cuda3.2/QuasirandomGenerator
#++ -o .release_build/tests/cuda3.2/RecursiveGaussian
#++ -o .release_build/tests/cuda3.2/Reduction
#++ -o .release_build/tests/cuda3.2/ScalarProd
#++ -o .release_build/tests/cuda3.2/Scan
#++ -o .release_build/tests/cuda3.2/SimpleAtomicIntrinsics
#++ -o .release_build/tests/cuda3.2/SimpleCUFFT
#++ -o .release_build/tests/cuda3.2/SimpleGL
#++ -o .release_build/tests/cuda3.2/SimpleMultiGPU
#++ -o .release_build/tests/cuda3.2/SimpleStreams
#++ -o .release_build/tests/cuda3.2/SimpleTemplates
#++ -o .release_build/tests/cuda3.2/SimpleTexture
#++ -o .release_build/tests/cuda3.2/SimpleTexture3D
#++ -o .release_build/tests/cuda3.2/SimpleVoteIntrinsics
#++ -o .release_build/tests/cuda3.2/SimpleZeroCopy
#++ -o .release_build/tests/cuda3.2/SobelFilter
#++ -o .release_build/tests/cuda3.2/SobolQRNG
#++ -o .release_build/tests/cuda3.2/Template
#++ -o .release_build/tests/cuda3.2/ThreadFenceReduction
#++ -o .release_build/tests/cuda3.2/Transpose
#++ -o .release_build/tests/cuda3.2/VolumeRender
#++ -o .release_build/tests/cuda4.1sdk/AlignedTypes
#++ -o .release_build/tests/cuda4.1sdk/AsyncAPI
#++ -o .release_build/tests/cuda4.1sdk/BandwidthTest
#++ -o .release_build/tests/cuda4.1sdk/BilateralFilter
#++ -o .release_build/tests/cuda4.1sdk/BinomialOptions
#++ -o .release_build/tests/cuda4.1sdk/BlackScholes
#++ -o .release_build/tests/cuda4.1sdk/BoxFilter
#++ -o .release_build/tests/cuda4.1sdk/Clock
#++ -o .release_build/tests/cuda4.1sdk/ConvolutionSeparable
#++ -o .release_build/tests/cuda4.1sdk/ConvolutionTexture
#++ -o .release_build/tests/cuda4.1sdk/CppIntegration
#++ -o .release_build/tests/cuda4.1sdk/DwtHaar1D
#++ -o .release_build/tests/cuda4.1sdk/Eigenvalues
#++ -o .release_build/tests/cuda4.1sdk/FDTD3d
#++ -o .release_build/tests/cuda4.1sdk/FastWalshTransform
#++ -o .release_build/tests/cuda4.1sdk/Histogram
#++ -o .release_build/tests/cuda4.1sdk/ImageDenoising
#++ -o .release_build/tests/cuda4.1sdk/InlinePTX
#++ -o .release_build/tests/cuda4.1sdk/MergeSort
#++ -o .release_build/tests/cuda4.1sdk/MersenneTwister
#++ -o .release_build/tests/cuda4.1sdk/MonteCarlo
#++ -o .release_build/tests/cuda4.1sdk/Nbody
#++ -o .release_build/tests/cuda4.1sdk/QuasirandomGenerator
#++ -o .release_build/tests/cuda4.1sdk/ScalarProd
#++ -o .release_build/tests/cuda4.1sdk/SimpleAtomicIntrinsics
#++ -o .release_build/tests/cuda4.1sdk/SimpleGL
#++ -o .release_build/tests/cuda4.1sdk/SimpleLayeredTexture
#++ -o .release_build/tests/cuda4.1sdk/SimpleMultiCopy
#++ -o .release_build/tests/cuda4.1sdk/SimpleMultiGPU
#++ -o .release_build/tests/cuda4.1sdk/SimplePitchLinearTexture
#++ -o .release_build/tests/cuda4.1sdk/SimplePrintf
#++ -o .release_build/tests/cuda4.1sdk/SimpleStreams
#++ -o .release_build/tests/cuda4.1sdk/SimpleTemplates
#++ -o .release_build/tests/cuda4.1sdk/SimpleTexture
#++ -o .release_build/tests/cuda4.1sdk/SimpleTexture3D
#++ -o .release_build/tests/cuda4.1sdk/SimpleZeroCopy
#++ -o .release_build/tests/cuda4.1sdk/SobelFilter
#++ -o .release_build/tests/cuda4.1sdk/SobolQRNG
#++ -o .release_build/tests/cuda4.1sdk/SortingNetworks
#++ -o .release_build/tests/cuda4.1sdk/Template
#++ -o .release_build/tests/cuda4.1sdk/Transpose
#++ -o .release_build/tests/cuda4.1sdk/VectorAdd
#++ -o .release_build/tests/cuda4.1sdk/VolumeRender
#++ -o .release_build/tests/parboil/bfs
#++ -o .release_build/tests/parboil/cutcp
#++ -o .release_build/tests/parboil/fft
#++ -o .release_build/tests/parboil/histo
#++ -o .release_build/tests/parboil/lbm
#++ -o .release_build/tests/parboil/mm
#++ -o .release_build/tests/parboil/mri-q
#++ -o .release_build/tests/parboil/sad
#++ -o .release_build/tests/parboil/spmv
#++ -o .release_build/tests/parboil/stencil
#++ -o .release_build/tests/parboil/tpacf
#++ -o libocelotTrace.so


#c++  -c -O2 -Wall -std=c++11 ocelot/tools/LoadPtx.cpp

#flex -t src/parser/ptx.ll > .release_build/ocelot/ptx.cpp
#c++  -c -O2 -Wall -std=c++11 -pthread -DOCELOT_CXXFLAGS="\"-O2 -Wall -std=c++11 -pthread\"" -DPACKAGE="\"ocelot\"" -DVERSION="\"2.1.0\"" -DOCELOT_PREFIX_PATH="\"/usr/local\"" -DOCELOT_LDFLAGS="\"-locelot\"" -L"\"/usr/local/lib\"" -DOCELOT_INCLUDE_PATH="\"/usr/local/include\"" -DOCELOT_LIB_PATH="\"/usr/local/lib\"" -DOCELOT_BIN_PATH="\"/usr/local/bin\"" -Iocelot/hydrazine/include -Iocelot -I/usr/include -I/usr/include -I.release_build/ocelot -Iocelot tools/OcelotConfig.cpp
#c++  -c -O2 -Wall -std=c++11 ocelot/tools/KernelTestHarness.cpp
#c++  -c -O2 -Wall -std=c++11 ocelot/tools/OcelotLinker.cpp
#c++  -c -O2 -Wall -std=c++11 ocelot/tools/OcelotServer.cpp
#c++  -c -O2 -Wall -std=c++11 ocelot/tools/PTXOptimizer.cpp
#c++  -c -O2 -Wall -std=c++11 ocelot/cuda/test/memory/generic.cpp
#c++  -c -O2 -Wall -std=c++11 ocelot/analysis/test/TestDataflowGraph.cpp
#c++  -c -O2 -Wall -std=c++11 ocelot/api/test/TestDeviceSwitching.cpp
#c++  -c -O2 -Wall -std=c++11 ocelot/cuda/test/driver/sequence.cpp
#c++  -c -O2 -Wall -std=c++11 ocelot/executive/test/TestEmulator.cpp
#c++  -c -O2 -Wall -std=c++11 ocelot/api/test/TestExternalFunctions.cpp
#c++  -c -O2 -Wall -std=c++11 ocelot/cuda/test/functions/indirectCallOcelot.cpp
#c++  -c -O2 -Wall -std=c++11 ocelot/executive/test/TestInstructions.cpp
#c++  -c -O2 -Wall -std=c++11 ocelot/executive/test/TestKernels.cpp
#c++  -c -O2 -Wall -std=c++11 ocelot/ir/test/TestLLVMInstructions.cpp
#c++  -c -O2 -Wall -std=c++11 ocelot/executive/test/TestLLVMKernels.cpp
#c++  -c -O2 -Wall -std=c++11 ocelot/parser/test/TestLexer.cpp
#c++  -c -O2 -Wall -std=c++11 ocelot/transforms/test/TestOptimizations.cpp
#c++  -c -O2 -Wall -std=c++11 ocelot/ir/test/TestPTXAssembly.cpp
#c++  -c -O2 -Wall -std=c++11 ocelot/translator/test/TestPTXToLLVMTranslator.cpp
#c++  -c -O2 -Wall -std=c++11 ocelot/parser/test/TestParser.cpp

