/*! \file LLVMExecutionManager.cpp
	\author Gregory Diamos <gregory.diamos@gatech.edu>
	\date Friday September 24, 2010
	\brief The source file for the LLVMExecutionManager
*/

#ifndef LLVM_EXECUTION_MANAGER_CPP_INCLUDED
#define LLVM_EXECUTION_MANAGER_CPP_INCLUDED

// Ocelot Includes
#include <ocelot/executive/LLVMExecutionManager.h>
#include <ocelot/executive/LLVMWorkerThread.h>
#include <ocelot/executive/LLVMModuleManager.h>
#include <ocelot/executive/LLVMExecutableKernel.h>
#include <ocelot/ir/Module.h>
#include <ocelot/api/OcelotConfiguration.h>

// Hydrazine Includes
#include <hydrazine/debug.h>

// Standard Library Includes
#include <algorithm>

// Preprocessor Macros
#ifdef REPORT_BASE
#undef REPORT_BASE
#endif

#define REPORT_BASE 0

namespace executive
{

void LLVMExecutionManager::launch(const LLVMExecutableKernel& kernel)
{
	_manager.launch(kernel);
}

void LLVMExecutionManager::setWorkerThreadCount(unsigned int threads)
{
	_manager.setWorkerThreadCount(threads);
}

unsigned int LLVMExecutionManager::threads()
{
	return _manager.threads();
}

void LLVMExecutionManager::flushTranslatedKernels()
{
	return _manager.flushTranslatedKernels();
}

LLVMExecutionManager::Manager::~Manager()
{
	report("Tearing down LLVM worker threads.");
	for(WorkerVector::iterator worker = _workers.begin();
		worker != _workers.end(); ++worker)
	{
		delete *worker;
	}
}

void LLVMExecutionManager::Manager::launch(const LLVMExecutableKernel& kernel)
{
	typedef api::OcelotConfiguration Config;
	
	report("Launching LLVM kernel '" << kernel.name << "'.");
	if(threads() == 0)
	{
		setWorkerThreadCount(std::min(hydrazine::getHardwareThreadCount(),
			(unsigned)Config::get().executive.workerThreadLimit));
	}

	if(!LLVMState::moduleManager()->isModuleLoaded(kernel.module->id()))
	{
		LLVMState::moduleManager()->loadModule(kernel.module, kernel.optimization(),
			kernel.device);	
	}
	
	for(WorkerVector::iterator worker = _workers.begin();
		worker != _workers.end(); ++worker)
	{
		(*worker)->setupCta(kernel);
	}
	
	unsigned int ctasFinished = 0;
	unsigned int ctasLaunched = 0;
	unsigned int totalCtas = kernel.gridDim().z * kernel.gridDim().x *
		kernel.gridDim().y;
	
	for(WorkerVector::iterator worker = _workers.begin();
		worker != _workers.end(); ++worker)
	{
		if(ctasLaunched == totalCtas) break;
		
		report(" Launched CTA " << ctasLaunched 
			<< " on worker " << (*worker)->id());
		(*worker)->launchCta(ctasLaunched++);
	}
	
	while(ctasFinished < totalCtas)
	{
		hydrazine::Thread::Id id = (*_workers.begin())->testGroup(true).first;

		LLVMWorkerThread* worker = static_cast<LLVMWorkerThread*>(
			(*_workers.begin())->find(id));

		worker->finishCta();
		++ctasFinished;

		report(" CTA finished on worker " << worker->id());

		if(ctasLaunched < totalCtas)
		{
			report(" Launched CTA " << ctasLaunched 
				<< " on worker " << worker->id());
			worker->launchCta(ctasLaunched++);
		}
	}
}

void LLVMExecutionManager::Manager::setWorkerThreadCount(unsigned int threads)
{
	if(threads < _workers.size())
	{
		report("Decreasing llvm worker thread count from " << _workers.size() 
			<< " to " << threads << ".");
		for(unsigned int thread = threads; thread != _workers.size(); ++thread)
		{
			delete _workers[thread];
		}
		_workers.resize(threads);
	}
	else if(threads > _workers.size())
	{
		report("Increasing llvm worker thread count from " << _workers.size() 
			<< " to " << threads << ".");
		unsigned int currentSize = _workers.size();
		_workers.resize(threads);
		
		for(unsigned int thread = currentSize;
			thread != _workers.size(); ++thread)
		{
			_workers[thread] = new LLVMWorkerThread;
		}
		
		for(unsigned int thread = currentSize; 
			thread != _workers.size(); ++thread)
		{
			LLVMState::moduleManager()->associate(_workers[thread]);
		}
	}
}

unsigned int LLVMExecutionManager::Manager::threads() const
{
	return _workers.size();
}

void LLVMExecutionManager::Manager::flushTranslatedKernels()
{
	for(WorkerVector::iterator worker = _workers.begin();
		worker != _workers.end(); ++worker)
	{
		(*worker)->flushTranslatedKernels();
	}
}


LLVMExecutionManager::Manager LLVMExecutionManager::_manager;

}

#endif

