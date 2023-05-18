/*! \file OcelotRuntime.h
	\author Gregory Diamos
	\date Tuesday August 11, 2009
	\brief The header file for the OcelotRuntime class.
*/

#ifndef OCELOT_RUNTIME_H_INCLUDED
#define OCELOT_RUNTIME_H_INCLUDED

#include <ocelot/api/OcelotConfiguration.h>

#include <ocelot/trace/MemoryChecker.h>
#include <ocelot/trace/MemoryRaceDetector.h>
#include <ocelot/trace/InteractiveDebugger.h>
#include <ocelot/trace/KernelTimer.h>

#include <ocelot/transforms/StructuralTransform.h>
#include <ocelot/transforms/ConvertPredicationToSelectPass.h>
#include <ocelot/transforms/LinearScanRegisterAllocationPass.h>
#include <ocelot/transforms/MIMDThreadSchedulingPass.h>
#include <ocelot/transforms/SyncEliminationPass.h>
#include <ocelot/transforms/HoistSpecialValueDefinitionsPass.h>
#include <ocelot/transforms/SimplifyControlFlowGraphPass.h>
#include <ocelot/transforms/EnforceLockStepExecutionPass.h>
#include <ocelot/transforms/FunctionInliningPass.h>

namespace ocelot
{
	/*! \brief This is an interface for managing state associated with Ocelot */
	class OcelotRuntime	{
	private:
		trace::MemoryChecker _memoryChecker;
		trace::MemoryRaceDetector _raceDetector;
		trace::InteractiveDebugger _debugger;
		trace::KernelTimer _kernelTimer;

		transforms::StructuralTransform _structuralTransform;
		transforms::ConvertPredicationToSelectPass _predicationToSelect;
		transforms::LinearScanRegisterAllocationPass _linearScanAllocation;
		transforms::MIMDThreadSchedulingPass _mimdThreadScheduling;
		transforms::SyncEliminationPass _syncElimination;
		transforms::HoistSpecialValueDefinitionsPass _hoistSpecialValues;
		transforms::SimplifyControlFlowGraphPass _simplifyCFG;
		transforms::EnforceLockStepExecutionPass _enforceLockStepExecution;
		transforms::FunctionInliningPass _inliner;
		
		bool _initialized;
		
	public:
		//! \brief initializes Ocelot runtime state
		OcelotRuntime();
	
		//! \brief initializes the Ocelot runtime object with the 
		//         Ocelot configuration object
		void configure( const api::OcelotConfiguration &c );
					
	};
}

#endif

