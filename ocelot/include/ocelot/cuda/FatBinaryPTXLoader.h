/*!
	\file FatBinaryContext.h
	\author Andrew Kerr <arkerr@gatech.edu>
	\brief object for interacting with CUDA Fat Binaries
*/

#ifndef FAT_BINARY_PTX_LOADER_H
#define FAT_BINARY_PTX_LOADER_H

#include <string>

namespace cuda {

// Load PTX sources from the provided CUDA Fat Binary.
class FatBinaryPTXLoader
{
	const void* _id;

public :

	FatBinaryPTXLoader(const void *);

	void load();

	void* id() const;

	// Implement in the derived class to consume the provided
	// PTX source as necessary.
	virtual void consumePTX(void* id, const std::string& ptx) = 0;
};

} // namespace cuda

#endif // FAT_BINARY_PTX_LOADER_H

