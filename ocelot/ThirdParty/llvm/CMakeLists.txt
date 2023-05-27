cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project(gpuocelot_llvm CXX ASM)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system REQUIRED)
find_package(LLVM REQUIRED CONFIG)
find_package(ZLIB REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

include(AddLLVM)

if (NOT TARGET hydrazine)
add_subdirectory(ThirdParty/hydrazine EXCLUDE_FROM_ALL)
endif()

if (NOT TARGET res_embed)
add_subdirectory(ThirdParty/res_embed EXCLUDE_FROM_ALL)
endif()

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${PROJECT_NAME}_DEFINITIONS)

set(${PROJECT_NAME}_INCLUDE_DIRS
	${LLVM_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/../../include)

file(GLOB LLVM_LIBRARIES ${LLVM_LIBRARY_DIR}/*.a)

set(${PROJECT_NAME}_LINK_LIBRARIES
	-Wl,--gc-sections -Wl,--start-group ${LLVM_LIBRARIES} -Wl,--end-group
	hydrazine
	Boost::system
	ZLIB::ZLIB
	tinfo
	${CMAKE_DL_LIBS})
	
set(${PROJECT_NAME}_LIBRARY_DIRS
	${LLVM_LIBRARY_DIRS})

add_library(${PROJECT_NAME} SHARED
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
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME} PRIVATE PRIVATE -fexceptions)
target_include_directories(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_LINK_LIBRARIES} res::embed::static
	-static-libgcc -static-libstdc++)
#-Wl,--allow-shlib-undefined -nodefaultlibs -nostartfiles -ldl_pic -lrt_pic -lc_pic -lpthread) # -Wl,--version-script -Wl,${CMAKE_CURRENT_SOURCE_DIR}/VERSION.txt)
