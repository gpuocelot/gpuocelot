/*! \file LLVMState.h
	\date Friday September 24, 2010
	\author Gregory Diamos <gregory.diamos@gatech.edu>
	\brief The header file for the LLVMState class.
*/

#ifndef LLVM_STATE_H_INCLUDED
#define LLVM_STATE_H_INCLUDED

namespace llvm
{
	class ExecutionEngine;
	class LLVMContext;
	class Module;
}

namespace executive
{

class LLVMModuleManager;

/*! \brief A class for managing global llvm state */
class LLVMState
{
public:
	/*! \brief Get a reference to the LLVM context */
	static llvm::LLVMContext* context();

	/*! \brief Get a reference to the jit */
	static llvm::ExecutionEngine* jit();

	/*! \brief Get a reference to the module manager */
	static LLVMModuleManager* moduleManager();

	LLVMState(LLVMState const&) = delete;
	LLVMState& operator=(LLVMState const&) = delete;

	~LLVMState();

private:

	static LLVMState& get();

	/*! \brief Build the jit */
	LLVMState();

	/*! \brief LLVM context */
	llvm::LLVMContext* _context;

	/*! \brief LLVM JIT Engine */
	llvm::ExecutionEngine* _jit;
	
	/*! \brief LLVM fake mofule */
	llvm::Module* _module;

	LLVMModuleManager* _manager;
};

}

#endif

