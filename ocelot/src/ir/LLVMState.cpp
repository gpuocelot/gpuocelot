/*! \file LLVMState.cpp
	\date Friday September 24, 2010
	\author Gregory Diamos <gregory.diamos@gatech.edu>
	\brief The header file for the LLVMState class.
*/

// Ocelot Includes
#include <ocelot/executive/LLVMState.h>
#include <ocelot/executive/LLVMModuleManager.h>

// Hydrazine Includes
#include <hydrazine/debug.h>

#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

// Preprocessor Defines
#ifdef REPORT_BASE
#undef REPORT_BASE
#endif

#define REPORT_BASE 0

namespace llvm {

// https://reviews.llvm.org/D19094
LLVMContext& getGlobalContext()
{
	return *executive::LLVMState::context();
}

} // namespace llvm

namespace executive
{

LLVMState& LLVMState::get()
{
	static LLVMState s;
	return s;
}

llvm::ExecutionEngine* LLVMState::jit()
{
	return LLVMState::get()._jit;
}

llvm::LLVMContext* LLVMState::context()
{
	return LLVMState::get()._context;
}

LLVMModuleManager* LLVMState::moduleManager()
{
	return LLVMState::get()._manager;
}

LLVMState::LLVMState() : _jit(0), _context(0), _module(0)
{
	_context = new llvm::LLVMContext();

	report("Bringing the LLVM JIT-Compiler online.");

	// https://stackoverflow.com/a/38801376/4063520	
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmParser();
	llvm::InitializeNativeTargetAsmPrinter();

	auto m = std::make_unique<llvm::Module>("Ocelot-LLVM-JIT-Blank Module", *_context);
	_module = m.get();
	assertM(_module != 0, "Creating global module failed.");

	llvm::TargetOptions Opts;
	llvm::RTDyldMemoryManager* MemMgr = new llvm::SectionMemoryManager();
	llvm::EngineBuilder factory(std::move(m));
	factory.setEngineKind(llvm::EngineKind::JIT);
	factory.setTargetOptions(Opts);
	factory.setMCJITMemoryManager(std::unique_ptr<llvm::RTDyldMemoryManager>(MemMgr));
	_jit = factory.create();
	_jit->DisableLazyCompilation(true);
	
	assertM(_jit != 0, "Creating the JIT failed.");
	report(" The JIT is alive.");

	_manager = new LLVMModuleManager();
}

LLVMState::~LLVMState()
{
	delete _manager;
	delete _context;
}

} // namespace executive

