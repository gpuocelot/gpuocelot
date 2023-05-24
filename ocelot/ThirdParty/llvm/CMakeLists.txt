cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project(gpuocelot_llvm CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

#set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system REQUIRED)
find_package(LLVM 3.4 REQUIRED)

if (NOT TARGET hydrazine)
add_subdirectory(ThirdParty/hydrazine EXCLUDE_FROM_ALL)
endif()

if (NOT TARGET res_embed)
add_subdirectory(ThirdParty/res_embed EXCLUDE_FROM_ALL)
endif()

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
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/../../include)

message(STATUS ${${PROJECT_NAME}_INCLUDE_DIRS})

set(${PROJECT_NAME}_LINK_LIBRARIES
	${LLVM_LIBRARIES}
	hydrazine
	Boost::system
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
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_compile_options(${PROJECT_NAME} PRIVATE PRIVATE ${${PROJECT_NAME}_CXXFLAGS})
target_include_directories(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_LINK_LIBRARIES} res::embed::static)
	#-Wl,--allow-shlib-undefined
        #-Wl,-zundefs
	#-static-libgcc -static-libstdc++ -Wl,-Bstatic)
