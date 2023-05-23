/*!
	\file FatBinaryPTXLoader.h
	\author Andrew Kerr <arkerr@gatech.edu>
	\brief object for interacting with CUDA Fat Binaries
*/


// Ocelot Includes
#include <ocelot/cuda/FatBinaryContext.h>

// Hydrazine Includes
#include <hydrazine/debug.h>

// Preprocessor Macros
#ifdef REPORT_BASE
#undef REPORT_BASE
#endif

#define REPORT_BASE 0

using namespace cuda;

FatBinaryContext::FatBinaryContext(const void *cubin) :
	FatBinaryPTXLoader(cubin) { }

void FatBinaryContext::consumePTX(void* id, const std::string& ptx)
{
	ir::Module module;
	module.lazyLoad(id, ptx);
	modules.push_back(module);
}

const ir::Module& FatBinaryContext::getFirstModule() const
{
	return modules[0];
}
