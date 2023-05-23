/*!
	\file FatBinaryContext.h
	\author Andrew Kerr <arkerr@gatech.edu>
	\brief object for interacting with CUDA Fat Binaries
*/

#ifndef FAT_BINARY_CONTEXT_H
#define FAT_BINARY_CONTEXT_H

#include "FatBinaryPTXLoader.h"
#include <ocelot/ir/Module.h>

namespace cuda {

// Load PTX sources from the provided CUDA Fat Binary.
class FatBinaryContext : public FatBinaryPTXLoader
{
	// Could be more than one module, if separable compilation is enabled.
	std::vector<ir::Module> modules;

public :

	const ir::Module& getFirstModule() const;

	FatBinaryContext(const void* );

	virtual void consumePTX(void* id, const std::string& ptx);
};

} // namespace cuda

#endif // FAT_BINARY_CONTEXT_H

