/*! \file EmulatorCallStack.h
	\author Gregory Diamos <gregory.diamos@gatech.edu>
	\date Friday July 9, 2010
	\brief The header file for the EmulatorCallStack class.
*/

#ifndef EMULATOR_CALL_STACK_H_INCLUDED
#define EMULATOR_CALL_STACK_H_INCLUDED

// Standard Library Includes
#include <vector>

namespace executive
{

/*! \brief A class implementing a multi-threaded variably sized call stack
	for the emulator device.
*/
class EmulatorCallStack
{
	private:
		typedef std::vector<char> DataVector;
		typedef std::vector<unsigned int> SizeStack;
		typedef long long unsigned int RegisterType;

	private:
		/*! \brief The current stack pointer */
		unsigned int _stackPointer;
		/*! \brief The number of threads used by this stack */
		unsigned int _threadCount;
		/*! \brief The offset for local memory */
		unsigned int _localMemoryBase;
		/*! \brief The offset for the register file */
		unsigned int _registerFileBase;
		/*! \brief A history of the sizes of stack frames */
		SizeStack _stackFrameSizes;
		/*! \brief A history of the sizes of local memory */
		SizeStack _localMemorySizes;
		/*! \brief A history of the sizes of register files */
		SizeStack _registerFileSizes;
		/*! \brief A history of the sizes of the shared memory block */
		SizeStack _sharedMemorySizes;
		/*! \brief The actual data for the stack */
		DataVector _stack;
		/*! \brief A resizable array for shared memory */
		DataVector _sharedMemory;

	public:
		/*! \brief Create a new call stack for a set number of threads */
		EmulatorCallStack(unsigned int threads = 0, 
			unsigned int initialFrameSize = 0, unsigned int registers = 0, 
			unsigned int localSize = 0, unsigned int sharedSize = 0);
	
		/*! \brief Get a pointer to the base of the current stack frame */
		void* stackFramePointer(unsigned int thread);
		/*! \brief Get a pointer to the previous stack frame */
		void* previousStackFramePointer(unsigned int thread);

		/*! \brief Get a pointer to the register file for a given thread */
		RegisterType* registerFilePointer(unsigned int thread);
		/*! \brief Get a pointer to local memory for a given thread */
		void* localMemoryPointer(unsigned int thread);
		/*! \brief Get a pointer to shared memory */
		void* sharedMemoryPointer();
	
		/*! \brief Get the current register file size */
		unsigned int registerCount() const;
		/*! \brief Get the current size of local memory */
		unsigned int localMemorySize() const;
		/*! \brief Get the shared memory size */
		unsigned int sharedMemorySize() const;
		
		/*! \brief Get the size of the current frame */
		unsigned int stackFrameSize() const;
		/*! \brief Get the size of the previous frame */
		unsigned int previousFrameSize() const;
		/*! \brief Get the total size of the stack in bytes */
		unsigned int stackSize() const;

		/*! \brief Push a new frame onto the stack */
		void pushFrame(unsigned int stackSize, unsigned int registers, 
			unsigned int localSize, unsigned int sharedSize);
		/*! \brief Pop the current frame */
		void popFrame();
};

}

#endif
