cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

if (APPLE)
	project(gpuocelot C CXX ASM)
else()
	project(gpuocelot C CXX CUDA ASM)
endif()

option(BUILD_LLVM "Use LLVM compiled from sources: default OFF" OFF)
option(BUILD_TESTS "Build tests: default OFF" OFF)
option(BUILD_TESTS_CUDA "Build CUDA tests: default ON" ON)
option(BUILD_TOOLS "Build tool executables: default ON" ON)

if ("x${CMAKE_BUILD_TYPE}" STREQUAL "x")
set(CMAKE_BUILD_TYPE "Release")
endif()

list(APPEND CMAKE_CXX_FLAGS "-fexceptions -Wno-return-type -Wno-terminate")

# in order to retain the PTX code in the text executables,
# the target GPU architecture has to be virtual.
set(CMAKE_CUDA_ARCHITECTURES 35-virtual)
list(APPEND CUDA_NVCC_FLAGS "-Wno-deprecated-gpu-targets -Wno-deprecated-declarations")

set(CMAKE_CUDA_SEPARABLE_COMPILATION ON)

if (APPLE)
	set(CMAKE_PREFIX_PATH
		"/opt/homebrew"
		# keg-only:
		"/opt/homebrew/opt/llvm@15"
		"/opt/homebrew/opt/bison")
endif()

find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(FLEX 2.5 REQUIRED)
find_package(BISON 2.5 REQUIRED)
find_package(GLEW REQUIRED)
find_package(ZLIB REQUIRED)
find_library(ZSTD_LIB NAMES zstd libzstd)
find_package(Curses REQUIRED)

if ("x${BUILD_LLVM}" STREQUAL "xOFF")
find_package(LLVM REQUIRED CONFIG)

file(GLOB LLVM_LIBRARIES ${LLVM_LIBRARY_DIR}/*.a)
else()
include(ExternalProject)

find_package(Python3 3.4 REQUIRED)
set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})

set(LLVM_SOURCE_DIR "")
set(LLVM_URL
    "https://github.com/gpuocelot/gpuocelot/releases/download/llvm15/llvm-15.0.7.src.tar.xz"
    CACHE STRING "URL of LLVM archive to use."
)
set(LLVM_ARCHIVE_SHA256
    "3734b39bcdc6cff0c77cb2276ee5714b50ce6b95565b289bcfaa85f1cd1ec54a"
    CACHE STRING ""
)
set(LLVM_DOWNLOAD_NAME "llvm-15.0.7.src.tar.xz")
set(LLVM_INSTALL_DIR ${CMAKE_BINARY_DIR}/llvm)

if ("x${CMAKE_BUILD_TYPE}" STREQUAL "xDebug")
set(IS_DEBUG_BUILD ON)
else()
set(IS_DEBUG_BUILD OFF)
endif()

# https://github.com/avast/retdec/blob/master/CMakeLists.txt
ExternalProject_Add(llvm-project
	SOURCE_DIR ${LLVM_SOURCE_DIR}
	URL ${LLVM_URL}
	URL_HASH SHA256=${LLVM_ARCHIVE_SHA256}
	DOWNLOAD_NAME ${LLVM_DOWNLOAD_NAME}
	CMAKE_ARGS
		-DCMAKE_INSTALL_PREFIX=${LLVM_INSTALL_DIR}
		-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
		-DCMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}
		-DCMAKE_CXX_FLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}

		# Force Python3.
		-DPYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}

		# Use provided libtool
		-DCMAKE_LIBTOOL=${CMAKE_LIBTOOL}

		# Targets to be build.
		-DLLVM_TARGETS_TO_BUILD=host

		# Our tools depending on LLVM require RTTI, so build LLVM with it.
		-DLLVM_REQUIRES_RTTI=YES

		# When building in the debug mode (=> assertions are enabled), we
		# have to build LLVM with assertions. This prevents link errors
		# when building the middle-end and back-end (see
		# https://github.com/oclint/oclint/issues/129).
		-DLLVM_ENABLE_ASSERTIONS=${IS_DEBUG_BUILD}

		# Disable the emission of warnings, which are useless since we do
		# not modify the LLVM sources (of course, except for a few
		# exceptions).
		-DLLVM_ENABLE_WARNINGS=NO

		# Enable exceptions. We want to get std::bad_alloc().
		# Without this, LLVM aborts instead of throwing.
		-DLLVM_ENABLE_RTTI=ON
		-DLLVM_ENABLE_EH=ON

		# Disable the generation of targets for tests (we don't need them).
		-DLLVM_INCLUDE_TOOLS=OFF
		-DLLVM_INCLUDE_UTILS=OFF
		-DLLVM_INCLUDE_RUNTIMES=OFF
		-DLLVM_INCLUDE_EXAMPLES=OFF
		-DLLVM_INCLUDE_TESTS=OFF
		-DLLVM_INCLUDE_GO_TESTS=OFF
		-DLLVM_INCLUDE_BENCHMARKS=OFF
		-DLLVM_INCLUDE_DOCS=OFF

		# Disable build of unnecessary LLVM parts.
		-DLLVM_BUILD_TOOLS=OFF
		-DLLVM_BUILD_UTILS=OFF
		-DLLVM_BUILD_RUNTIMES=OFF
		-DLLVM_BUILD_RUNTIME=OFF
		-DLLVM_BUILD_EXAMPLES=OFF
		-DLLVM_BUILD_TESTS=OFF
		-DLLVM_BUILD_BENCHMARKS=OFF
		-DLLVM_BUILD_DOCS=OFF
		-DLLVM_ENABLE_BINDINGS=OFF
		# We don't want this so that we don't have to link with tinfo.
		# It looks like terminal colors are working even without this.
		-DLLVM_ENABLE_TERMINFO=OFF

		# Force the use of the same compiler as used to build the top-level
		# project. Otherwise, the external project may pick up a different
		# compiler, which may result in link errors.
		"${CMAKE_C_COMPILER_OPTION}"
		"${CMAKE_CXX_COMPILER_OPTION}"
		-DCMAKE_POSITION_INDEPENDENT_CODE=${CMAKE_POSITION_INDEPENDENT_CODE})

set(LLVM_INCLUDE_DIRS ${LLVM_INSTALL_DIR}/include)
set(LLVM_LIBRARY_DIR ${LLVM_INSTALL_DIR}/lib)
set(LLVM_DEFINITIONS_LIST "")

set(LLVM_LIBRARIES
	LLVMMIRParser
	LLVMInterfaceStub
	LLVMFrontendOpenACC
	LLVMDWARFLinker
	LLVMLinker
	LLVMDebugInfoGSYM
	LLVMAsmPrinter
	LLVMProfileData
	LLVMFuzzerCLI
	LLVMOption
	LLVMX86Disassembler
	LLVMRemarks
	LLVMCodeGen
	LLVMCore
	LLVMX86CodeGen
	LLVMAggressiveInstCombine
	LLVMRuntimeDyld
	LLVMInterpreter
	LLVMMCA
	LLVMDebuginfod
	LLVMBitReader
	LLVMSupport
	LLVMCFGuard
	LLVMTableGenGlobalISel
	LLVMObjCARCOpts
	LLVMX86Info
	LLVMLineEditor
	LLVMCoverage
	LLVMFuzzMutate
	LLVMX86Desc
	LLVMDlltoolDriver
	LLVMBinaryFormat
	LLVMOrcJIT
	LLVMExtensions
	LLVMBitWriter
	LLVMGlobalISel
	LLVMFileCheck
	LLVMAnalysis
	LLVMSelectionDAG
	LLVMWindowsManifest
	LLVMJITLink
	LLVMTarget
	LLVMPasses
	LLVMipo
	LLVMMCDisassembler
	LLVMIRReader
	LLVMSymbolize
	LLVMMC
	LLVMX86TargetMCA
	LLVMObjCopy
	LLVMMCJIT
	LLVMLTO
	LLVMInstrumentation
	LLVMCoroutines
	LLVMDebugInfoMSF
	LLVMDebugInfoPDB
	LLVMDWP
	LLVMMCParser
	LLVMXRay
	LLVMInstCombine
	LLVMScalarOpts
	LLVMLibDriver
	LLVMVectorize
	LLVMAsmParser
	LLVMTableGen
	LLVMOrcTargetProcess
	LLVMObjectYAML
	LLVMDebugInfoDWARF
	LLVMExecutionEngine
	LLVMDebugInfoCodeView
	LLVMFrontendOpenMP
	LLVMOrcShared
	LLVMTextAPI
	LLVMX86AsmParser
	LLVMDemangle
	LLVMTransformUtils
	LLVMBitstreamReader
	LLVMWindowsDriver
	LLVMObject)

add_library(llvm INTERFACE)
add_dependencies(llvm llvm-project)
endif()

set(BUILD_SHARED_LIBS OFF)  # ThirdParty/res_embed option
add_subdirectory(ThirdParty/hydrazine)
add_subdirectory(ThirdParty/res_embed EXCLUDE_FROM_ALL)

include(ResEmbed)

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${PROJECT_NAME}_DEFINITIONS)

set(${PROJECT_NAME}_INCLUDE_DIRS
	${LLVM_INCLUDE_DIRS}
	${GLEW_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/cuda-fatbin-decompression
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR})

set(${PROJECT_NAME}_LINK_LIBRARIES
	GLEW::GLEW
	hydrazine
	Boost::system
	Boost::filesystem
	ZLIB::ZLIB
	${CURSES_LIBRARIES}
	${ZSTD_LIB}
	${CMAKE_DL_LIBS})

if ("x${BUILD_LLVM}" STREQUAL "xON")
list(APPEND ${PROJECT_NAME}_LINK_LIBRARIES llvm)
endif()

set(${PROJECT_NAME}_LIBRARY_DIRS ${LLVM_LIBRARY_DIR})

include(CTest)

function(ocelot_add_tests name)
	file(GLOB TESTS "src/${name}/test/Test*.cpp" "src/${name}/test/Test*.cu")
	foreach(TEST ${TESTS})
		    get_filename_component(TEST_WE ${TEST} NAME_WE)
		    set(TEST_NAME ${PROJECT_NAME}_${name}_${TEST_WE})
		    add_executable(${TEST_NAME} ${TEST})
		    set_property(TARGET ${TEST_NAME} PROPERTY CXX_STANDARD 14)
		    target_compile_definitions(${TEST_NAME} PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
			target_compile_options(${TEST_NAME} PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
			target_include_directories(${TEST_NAME} PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
			target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/${name})
			target_link_libraries(${TEST_NAME} PRIVATE ${${PROJECT_NAME}_LINK_LIBRARIES})
		    target_link_libraries(${TEST_NAME} PRIVATE ${PROJECT_NAME})
			target_link_directories(${TEST_NAME} PRIVATE ${${PROJECT_NAME}_LIBRARY_DIRS})
		    add_test(NAME ${TEST_NAME} COMMAND $<TARGET_FILE:${TEST_NAME}> -v)
		    set_property(TEST ${TEST_NAME} PROPERTY ENVIRONMENT LD_PRELOAD=$<TARGET_FILE:${PROJECT_NAME}>)
	endforeach()
endfunction()

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
set_property(TARGET ${PROJECT_NAME}_analysis PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME}_analysis PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME}_analysis PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME}_analysis PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
target_include_directories(${PROJECT_NAME}_analysis PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_analysis PUBLIC ${${PROJECT_NAME}_LINK_LIBRARIES})
target_link_directories(${PROJECT_NAME}_analysis PUBLIC ${${PROJECT_NAME}_LIBRARY_DIRS})

add_library(${PROJECT_NAME}_api STATIC
	src/api/OcelotConfiguration.cpp
	src/api/OcelotRuntime.cpp
	src/api/ocelot.cpp
	src/api/interfaces/emulated.cpp)
set_property(TARGET ${PROJECT_NAME}_api PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME}_api PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME}_api PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME}_api PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
target_include_directories(${PROJECT_NAME}_api PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_api PUBLIC ${${PROJECT_NAME}_LINK_LIBRARIES})
target_link_directories(${PROJECT_NAME}_api PUBLIC ${${PROJECT_NAME}_LIBRARY_DIRS})

res_embed(TARGET ${PROJECT_NAME}_api NAME "configure_ocelot" PATH ${CMAKE_CURRENT_SOURCE_DIR}/configure.ocelot KEYWORD)

add_library(${PROJECT_NAME}_cal STATIC
	src/cal/CalDriver.cpp)
set_property(TARGET ${PROJECT_NAME}_cal PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME}_cal PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME}_cal PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME}_cal PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
target_include_directories(${PROJECT_NAME}_cal PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_cal PUBLIC ${${PROJECT_NAME}_LINK_LIBRARIES})
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
	src/cuda/FatBinaryPTXLoader.cpp
	src/cuda/cuda_runtime.cpp
	ThirdParty/cuda-fatbin-decompression/fatbin-decompress.c
	ThirdParty/cuda-fatbin-decompression/utils.c)
set_property(TARGET ${PROJECT_NAME}_cuda PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME}_cuda PROPERTY C_STANDARD 99)
set_property(TARGET ${PROJECT_NAME}_cuda PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME}_cuda PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME}_cuda PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
target_include_directories(${PROJECT_NAME}_cuda PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_cuda PUBLIC ${${PROJECT_NAME}_LINK_LIBRARIES})
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
	src/executive/LLVMWorkerThread.cpp
	src/executive/MulticoreCPUDevice.cpp
	src/executive/NVIDIAExecutableKernel.cpp
	src/executive/NVIDIAGPUDevice.cpp
	src/executive/PassThroughDevice.cpp
	src/executive/ReconvergenceMechanism.cpp
	src/executive/RemoteDevice.cpp
	src/executive/RuntimeException.cpp
	src/executive/TextureOperations.cpp)
set_property(TARGET ${PROJECT_NAME}_executive PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME}_executive PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME}_executive PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME}_executive PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
target_include_directories(${PROJECT_NAME}_executive PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_executive PUBLIC ${${PROJECT_NAME}_LINK_LIBRARIES})
target_link_directories(${PROJECT_NAME}_executive PUBLIC ${${PROJECT_NAME}_LIBRARY_DIRS})

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
        src/ir/LLVMModuleManager.cpp
        src/ir/LLVMState.cpp
        src/ir/LLVMStatement.cpp
        src/ir/Local.cpp
        src/ir/Module.cpp
        src/ir/PTXInstruction.cpp
        src/ir/PTXKernel.cpp
        src/ir/PTXOperand.cpp
        src/ir/PTXStatement.cpp
        src/ir/Parameter.cpp
        src/ir/Texture.cpp)
set_property(TARGET ${PROJECT_NAME}_ir PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME}_ir PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME}_ir PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_include_directories(${PROJECT_NAME}_ir PUBLIC ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME}_ir PUBLIC ${${PROJECT_NAME}_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME}_ir PUBLIC ${${PROJECT_NAME}_LINK_LIBRARIES} res::embed::static)

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
set_property(TARGET ${PROJECT_NAME}_parser PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME}_parser PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME}_parser PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME}_parser PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
target_include_directories(${PROJECT_NAME}_parser PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME}_parser PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME}_parser PUBLIC ${${PROJECT_NAME}_LINK_LIBRARIES})
target_link_directories(${PROJECT_NAME}_parser PUBLIC ${${PROJECT_NAME}_LIBRARY_DIRS})

add_library(${PROJECT_NAME}_trace STATIC
	src/trace/InteractiveDebugger.cpp
	src/trace/KernelTimer.cpp
	src/trace/MemoryChecker.cpp
	src/trace/MemoryRaceDetector.cpp
	src/trace/TraceEvent.cpp
	src/trace/TraceGenerator.cpp)
set_property(TARGET ${PROJECT_NAME}_trace PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME}_trace PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME}_trace PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME}_trace PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
target_include_directories(${PROJECT_NAME}_trace PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_trace PUBLIC ${${PROJECT_NAME}_LINK_LIBRARIES})
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
set_property(TARGET ${PROJECT_NAME}_transforms PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME}_transforms PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME}_transforms PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME}_transforms PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
target_include_directories(${PROJECT_NAME}_transforms PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_transforms PUBLIC ${${PROJECT_NAME}_LINK_LIBRARIES})
target_link_directories(${PROJECT_NAME}_transforms PUBLIC ${${PROJECT_NAME}_LIBRARY_DIRS})

add_library(${PROJECT_NAME}_translator STATIC
	src/translator/PTXToILTranslator.cpp
	src/translator/PTXToLLVMTranslator.cpp
	src/translator/Translator.cpp)
set_property(TARGET ${PROJECT_NAME}_translator PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME}_translator PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME}_translator PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME}_translator PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
target_include_directories(${PROJECT_NAME}_translator PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_translator PUBLIC ${${PROJECT_NAME}_LINK_LIBRARIES})
target_link_directories(${PROJECT_NAME}_translator PUBLIC ${${PROJECT_NAME}_LIBRARY_DIRS})

add_library(${PROJECT_NAME}_util STATIC
	src/util/ExtractedDeviceState.cpp
	src/util/OcelotServerConnection.cpp
	src/util/RemoteDeviceMessage.cpp)
set_property(TARGET ${PROJECT_NAME}_util PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME}_util PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME}_util PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME}_util PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
target_include_directories(${PROJECT_NAME}_util PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_util PUBLIC ${${PROJECT_NAME}_LINK_LIBRARIES})
target_link_directories(${PROJECT_NAME}_util PUBLIC ${${PROJECT_NAME}_LIBRARY_DIRS})

add_library(${PROJECT_NAME} SHARED src/ocelot.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_INCLUDE_DIRS})
set(${PROJECT_NAME}_static_libs
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
)

# ensure all symbols are included
if (APPLE)
	foreach(lib ${${PROJECT_NAME}_static_libs})
		target_link_libraries(${PROJECT_NAME} PRIVATE
			"-Wl,-force_load" ${lib}
		)
	endforeach(lib ${${PROJECT_NAME}_static_libs})
else()
	target_link_libraries(${PROJECT_NAME} PRIVATE
		"-Wl,--whole-archive"
		${${PROJECT_NAME}_static_libs}
		"-Wl,--no-whole-archive"
	)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_LINK_LIBRARIES})

# link llvm and remove unused sections
if(APPLE)
	target_link_libraries(${PROJECT_NAME} PRIVATE -Wl,-dead_strip ${LLVM_LIBRARIES})
else()
	target_link_libraries(${PROJECT_NAME} PRIVATE -Wl,--gc-sections -Wl,--start-group ${LLVM_LIBRARIES} -Wl,--end-group)
endif()

target_link_directories(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${ZLIB_LIBRARIES})
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION $<IF:$<PLATFORM_ID:Darwin>,/usr/local/lib,/usr/lib>)
install(CODE "execute_process(COMMAND ldconfig)")

if(BUILD_TOOLS)
	add_executable(LoadPtx src/tools/LoadPtx.cpp)
	set_property(TARGET LoadPtx PROPERTY CXX_STANDARD 14)
	target_link_libraries(LoadPtx ${PROJECT_NAME} hydrazine)

	add_executable(OcelotKernelTestHarness src/tools/KernelTestHarness.cpp)
	set_property(TARGET OcelotKernelTestHarness PROPERTY CXX_STANDARD 14)
	target_link_libraries(OcelotKernelTestHarness ${PROJECT_NAME} hydrazine)

	add_executable(OcelotLinker src/tools/OcelotLinker.cpp)
	set_property(TARGET OcelotLinker PROPERTY CXX_STANDARD 14)
	target_link_libraries(OcelotLinker ${PROJECT_NAME} hydrazine)

	add_executable(OcelotServer src/tools/OcelotServer.cpp)
	set_property(TARGET OcelotServer PROPERTY CXX_STANDARD 14)
	target_link_libraries(OcelotServer ${PROJECT_NAME} hydrazine)

	add_executable(PTXOptimizer src/tools/PTXOptimizer.cpp)
	set_property(TARGET PTXOptimizer PROPERTY CXX_STANDARD 14)
	target_link_libraries(PTXOptimizer ${PROJECT_NAME} hydrazine)
endif()

if(BUILD_TESTS)
	ocelot_add_tests(analysis)
	ocelot_add_tests(api)
	ocelot_add_tests(cal)
	ocelot_add_tests(executive)
	ocelot_add_tests(ir)
	ocelot_add_tests(parser)
	ocelot_add_tests(trace)
	ocelot_add_tests(transforms)
	ocelot_add_tests(translator)
	ocelot_add_tests(util)

	res_embed(TARGET ${PROJECT_NAME}_executive_TestEmulator NAME "TestEmulator_ptx"
		PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/executive/test/TestEmulator.ptx KEYWORD)

	res_embed(TARGET ${PROJECT_NAME}_executive_TestInstructions NAME "TestInstructions_ptx"
		PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/executive/test/TestInstructions.ptx KEYWORD)

	res_embed(TARGET ${PROJECT_NAME}_executive_TestKernels NAME "TestKernels_ptx"
		PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/executive/test/TestKernels.ptx KEYWORD)

	res_embed(TARGET ${PROJECT_NAME}_executive_TestLLVMKernels NAME "TestLLVMKernels_ptx"
		PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/executive/test/TestLLVMKernels.ptx KEYWORD)

	if(BUILD_TESTS_CUDA)
		ocelot_add_tests(cuda)
		res_embed(TARGET ${PROJECT_NAME}_cuda_TestCudaGenericMemory NAME "generic_ptx"
			PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/cuda/test/TestCudaGenericMemory.ptx KEYWORD)

		res_embed(TARGET ${PROJECT_NAME}_cuda_TestIndirectFunctionCallDriver NAME "IndirectCallDriver_ptx"
			PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/cuda/test/TestIndirectFunctionCallDriver.ptx KEYWORD)

		res_embed(TARGET ${PROJECT_NAME}_cuda_TestIndirectFunctionCallOcelot NAME "IndirectCallDriver_ptx"
			PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/cuda/test/TestIndirectFunctionCallDriver.ptx KEYWORD)
	endif()
endif()

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

