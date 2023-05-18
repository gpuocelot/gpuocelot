/*! \file   AnalysisFactory.cpp
	\date   Wednesday October 3, 2012
	\author Gregory Diamos <gdiamos@nvidia.com>
	\brief  The source file for the AnalysisFactory class.
	
*/

// Ocelot Includes
#include <ocelot/analysis/AnalysisFactory.h>

#include <ocelot/analysis/DataflowGraph.h>
#include <ocelot/analysis/DivergenceAnalysis.h>
#include <ocelot/analysis/AffineAnalysis.h>
#include <ocelot/analysis/ControlTree.h>
#include <ocelot/analysis/DominatorTree.h>
#include <ocelot/analysis/PostdominatorTree.h>
#include <ocelot/analysis/StructuralAnalysis.h>
#include <ocelot/analysis/ThreadFrontierAnalysis.h>
#include <ocelot/analysis/LoopAnalysis.h>
#include <ocelot/analysis/ConvergentRegionAnalysis.h>
#include <ocelot/analysis/SimpleAliasAnalysis.h>
#include <ocelot/analysis/CycleAnalysis.h>
#include <ocelot/analysis/SafeRegionAnalysis.h>
#include <ocelot/analysis/ControlDependenceAnalysis.h>
#include <ocelot/analysis/DataDependenceAnalysis.h>
#include <ocelot/analysis/DependenceAnalysis.h>
#include <ocelot/analysis/MemoryDependenceAnalysis.h>
#include <ocelot/analysis/HammockGraphAnalysis.h>

namespace analysis 
{

Analysis* AnalysisFactory::createAnalysis(const std::string& name,
	const StringVector& options)
{
	Analysis* analysis = nullptr;

	if(name == "ControlTreeAnalysis")
	{
		analysis = new ControlTree;
	}
	else if(name == "DominatorTreeAnalysis")
	{
		analysis = new DominatorTree;
	}
	else if(name == "PostDominatorTreeAnalysis")
	{
		analysis = new PostdominatorTree;
	}
    else if(name == "DataflowGraphAnalysis")
	{
		auto dfg = new DataflowGraph;
		
		dfg->setPreferredSSAType(analysis::DataflowGraph::None);
				
		analysis = dfg;
	}
	else if(name == "DivergenceAnalysis")
	{
		analysis = new DivergenceAnalysis;
	}
	else if(name == "AffineAnalysis")
	{
		analysis = new AffineAnalysis;
	}
	else if(name == "StructuralAnalysis")
	{
		analysis = new StructuralAnalysis;
	}
	else if(name == "ThreadFrontierAnalysis")
	{
		analysis = new ThreadFrontierAnalysis;
	}
	else if(name == "LoopAnalysis")
	{
		analysis = new LoopAnalysis;
	}
	else if(name == "ConvergentRegionAnalysis")
	{
		analysis = new ConvergentRegionAnalysis;
	}
	else if(name == "SafeRegionAnalysis")
	{
		analysis = new SafeRegionAnalysis;
	}
	else if(name == "CycleAnalysis")
	{
		analysis = new CycleAnalysis;
	}
	else if(name == "SimpleAliasAnalysis")
	{
		analysis = new SimpleAliasAnalysis;
	}
	else if(name == "ControlDependenceAnalysis")
	{
		analysis = new ControlDependenceAnalysis;
	}
	else if(name == "DataDependenceAnalysis")
	{
		analysis = new DataDependenceAnalysis;
	}
	else if(name == "MemoryDependenceAnalysis")
	{
		analysis = new MemoryDependenceAnalysis;
	}
	else if(name == "DependenceAnalysis")
	{
		analysis = new DependenceAnalysis;
	}
	else if(name == "HammockGraphAnalysis")
	{
		analysis = new HammockGraphAnalysis;
	}

	if(analysis != nullptr)
	{
		analysis->configure(options);
	}
	
	return analysis;
}

}

