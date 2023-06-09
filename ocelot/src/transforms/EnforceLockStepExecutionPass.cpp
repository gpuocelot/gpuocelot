/*! \file   EnforceLockStepExecutionPass.cpp
	\date   Wednesday April 18, 2012
	\author Gregory Diamos <gregory.diamos@gatech.edu>
	\brief  The source file for the  EnforceLockStepExecutionPass class.
*/

// Ocelot Includes
#include <ocelot/transforms/EnforceLockStepExecutionPass.h>

#include <ocelot/analysis/ThreadFrontierAnalysis.h>
#include <ocelot/analysis/ConvergentRegionAnalysis.h>
#include <ocelot/analysis/DivergenceAnalysis.h>
#include <ocelot/analysis/DominatorTree.h>

#include <ocelot/ir/IRKernel.h>

// Hydrazine Includes
#include <hydrazine/debug.h>

#ifdef REPORT_BASE
#undef REPORT_BASE
#endif

#define REPORT_BASE 0 

namespace transforms
{

EnforceLockStepExecutionPass::EnforceLockStepExecutionPass()
: KernelPass({}, "EnforceLockStepExecutionPass")
{

}

void EnforceLockStepExecutionPass::initialize(const ir::Module& m)
{
	
}

void EnforceLockStepExecutionPass::runOnKernel(ir::IRKernel& k)
{
	report("Running Enforce-Lock-Step-Execution-Pass on kernel '"
		<< k.name << "'");
	
}

void EnforceLockStepExecutionPass::finalize()
{
	
}

}


