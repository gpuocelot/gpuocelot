/*!	\file LLVMModuleManager.h
	\date Thursday September 23, 2010
	\author Gregory Diamos <gregory.diamos@gatech.edu>
	\brief The header file for the LLVMModuleManager class
*/

#ifndef LLVM_MODULE_MANAGER_H_INCLUDED
#define LLVM_MODULE_MANAGER_H_INCLUDED

// Ocelot Includes
#include <ocelot/translator/Translator.h>
#include <ocelot/executive/ExecutableKernel.h>
#include <ocelot/executive/LLVMState.h>
#include <ocelot/ir/ControlFlowGraph.h>
#include <ocelot/ir/Module.h>

// Hydrazine Includes
#include <hydrazine/Thread.h>

// Standard Library Includes
#include <vector>

// Forward Declarations
namespace ir
{
	class Module;
	class PTXKernel;
}

namespace llvm
{
	class Module;
}

namespace executive
{

class LLVMContext;

/*! \brief A class that manages modules that are executed by LLVM kernels */
class LLVMModuleManager
{
public:
	typedef void (*Function)(LLVMContext*);
	typedef unsigned int FunctionId;
	typedef ExecutableKernel::TextureVector TextureVector;

public:
	/*! \brief Loads a module into the manager, kernels are now visible */
	void loadModule(const ir::Module* m,
		translator::Translator::OptimizationLevel l, Device* device);

	/*! \brief Is a module loaded? */
	bool isModuleLoaded(void* id);

	/*! \brief Gets the total number of functions in all modules */
	unsigned int totalFunctionCount();

	/*! \brief unLoad module from the database, this invalidates all ids */
	void unloadModule(void* id);

	/*! \brief Sets the current external function set for linking */
	void setExternalFunctionSet(const ir::ExternalFunctionSet& s);

	/*! \brief Clears the current external function set for linking */
	void clearExternalFunctionSet();

	/*! \brief Associate a hydrazine thread with this manager, 
		allowing it to communicate */
	void associate(hydrazine::Thread* thread);

	/*! \brief Get the id of the hydrazine thread */
	hydrazine::Thread::Id id();
	
public:
	class ModuleDatabase;

	class KernelAndTranslation
	{
	public:
		class MetaData
		{
		public:
			typedef std::unordered_map<ir::ControlFlowGraph::BasicBlock::Id, 
				ir::ControlFlowGraph::const_iterator> BlockIdMap;

		public:
			BlockIdMap           blocks;
			const ir::PTXKernel* kernel;
			Function             function;
			TextureVector        textures;
		
		public:
			unsigned int sharedSize;
			unsigned int localSize;
			unsigned int globalLocalSize;
			unsigned int parameterSize;
			unsigned int argumentSize;
			unsigned int constantSize;
			unsigned int warpSize;
			unsigned int subkernels;  
		};
	
	public:
		KernelAndTranslation(ir::PTXKernel* k = 0, 
			translator::Translator::OptimizationLevel level = 
			translator::Translator::NoOptimization,
			const ir::PTXKernel* parent = 0, FunctionId offset = 0,
			unsigned int subkernels = 0,
			Device* device = 0, const ModuleDatabase* database = 0);

	public:
		void               unload();
		MetaData*          metadata();
		const std::string& name() const;
	
	private:
		ir::PTXKernel*                            _kernel;
		llvm::Module*                             _module;
		translator::Translator::OptimizationLevel _optimizationLevel;
		MetaData*                                 _metadata;
		const ir::PTXKernel*                      _parent;
		FunctionId                                _offsetId;
		unsigned int                              _subkernels;
		Device*                                   _device;
		const ModuleDatabase*                     _database;
	};

	typedef KernelAndTranslation::MetaData MetaData;

	class DatabaseMessage
	{
	public:
		enum Type
		{
			GetId,
			GetFunction,
			KillThread,
			Exception,
			Invalid
		};
	public:
		Type type;
	};

	class GetFunctionMessage : public DatabaseMessage
	{
	public:
		void* moduleId;
		std::string kernelName;
		FunctionId  id;
		MetaData*   metadata;
		std::string errorMessage;
	};
	
public:
	typedef std::unordered_map<std::string, FunctionId> FunctionIdMap;
	typedef std::vector<KernelAndTranslation> KernelVector;
	
	class Module
	{
	public:
		Module(const KernelVector& kernels, FunctionId nextId,
			ir::Module* originalModule);
		
	public:
		void destroy();
	
	public:
		FunctionId getFunctionId(const std::string& kernelName) const;
		
		FunctionId lowId() const;
		FunctionId highId() const;
		
		bool empty() const;
		
		void shiftId(FunctionId nextId);
		
	private:
		FunctionIdMap  _ids;
		ir::Module*    _originalModule;
	};

	typedef std::unordered_map<void*, Module> ModuleMap;

	/*! \brief A thread safe-class for actually maintaining the modules */
	class ModuleDatabase : public hydrazine::Thread
	{
	public:
		/*! \brief Create and initialize the database */
		ModuleDatabase();
	
		/*! \brief Destroy the database */
		~ModuleDatabase();
	
		/*! \brief Load module into the database */
		void loadModule(const ir::Module* module,
			translator::Translator::OptimizationLevel l, Device* device);
	
		/*! \brief Is a module loaded? */
		bool isModuleLoaded(void* id);
	
		/*! \brief unLoad module from the database */
		void unloadModule(void* id);
		
		/*! \brief Gets the total number of functions in all modules */
		unsigned int totalFunctionCount() const;

		/*! \brief Sets the current external function set for linking */
		void setExternalFunctionSet(const ir::ExternalFunctionSet& s);

		/*! \brief Clears the current external function set for linking */
		void clearExternalFunctionSet();
		
	public:
		/*! \brief Get the id of a kernel by module and kernel name */
		FunctionId getFunctionId(void* id,
			const std::string& kernelName) const;

		/*! \brief Get the external function set */
		const ir::ExternalFunctionSet& getExternalFunctionSet() const;

	private:
		/*! \brief The entry point to the thread */
		void execute();
	
	private:
		ModuleMap                      _modules;
		KernelVector                   _kernels;
		ir::Module                     _barrierModule;
		const ir::ExternalFunctionSet* _externals;
		const ir::ExternalFunctionSet _externalsDefault;
	};
	
private:
	ModuleDatabase _database;
};

}

#endif

