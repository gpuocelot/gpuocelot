/*!
	\file FatBinaryPTXLoader.h
	\author Andrew Kerr <arkerr@gatech.edu>
	\brief object for interacting with CUDA Fat Binaries
*/


// Ocelot Includes
#include <ocelot/cuda/FatBinaryPTXLoader.h>
#include <ocelot/cuda/cudaFatBinary.h>

// Hydrazine Includes
#include <hydrazine/debug.h>

// Standard Library Includes
#include <cstring>
#include <sstream>
#include <fstream>

// https://github.com/gpuocelot/cuda-fatbin-decompression
extern "C" {
#include "fatbin-decompress.h"
} // extern "C"

// Preprocessor Macros
#ifdef REPORT_BASE
#undef REPORT_BASE
#endif

#define REPORT_BASE 0

// Set the best choice of PTX target architecture while searching
// for PTX sources in CUDA fat binaries. If an exact version
// is not present, choose the closest to it, which is available.
// The original Ocelot code was choosing the most recent architecture.
// Nowadays this is not the best strategy, because Ocelot's PTX grammar
// support yet far behind the actual PTX standard. 
#define PTX_MAJOR 3
#define PTX_MINOR 5

using namespace cuda;

static bool ptxdump_elf_header(fat_elf_header* header, std::string& ptx)
{
	bool seenPtx = false;

	int ptxVersionClosest = 0;
	int ptxVersionBest = PTX_MAJOR * 10 + PTX_MINOR;
	
	char* base = (char*)(header + 1);
	auto entry = (fat_text_header*)(base);
	for (long long unsigned int offset = 0; offset < header->header_size;
		entry = (fat_text_header*)(base + offset), offset += entry->header_size + entry->size) 
	{
		if (!(entry->kind & FATBIN_2_PTX))
			continue;

		seenPtx = true;
		
		int ptxVersion = entry->major * 10 + entry->minor;

		// We always save only the best choice ptx.
		if (std::abs(ptxVersion - ptxVersionBest) >= std::abs(ptxVersionClosest - ptxVersionBest))
			continue;

		ptxVersionClosest = ptxVersion;

		// Deliver uncompressed PTX right away.
		if (!(entry->flags & COMPRESSED_PTX))
		{
			auto ptx = std::string((char*)entry + entry->header_size);
			continue;
		}

		// Decompress the PTX. Note it is not gzip, lzma or anything
		// straight forward. One good soul discovered PTX compression
		// uses some flavor of LZ77 algorithm, more details here:
		// https://reverseengineering.stackexchange.com/a/31806/39732
		ptx.resize(entry->decompressed_size);
		auto size_fact = decompress(
			reinterpret_cast<uint8_t*>(entry) + entry->header_size, entry->compressed_size,
			reinterpret_cast<uint8_t*>(&ptx[0]), ptx.size());

		if (size_fact != entry->decompressed_size)
		{
			report("Decompressed PTX size does not match the entry info: " <<
				size_fact << " != " << entry->decompressed_size);
		}
		
		report(" Selected version " << ptxVersionClosest);
	}
	
	return seenPtx;
}

FatBinaryPTXLoader::FatBinaryPTXLoader(const void *cubin) :
	_id(cubin) { }

void FatBinaryPTXLoader::load()
{
	const void* cubin = _id;

	report("FatBinaryPTXLoader(" << _id << ")");

	if (*(int*)cubin == __cudaFatMAGIC)
	{
		report("Found old fat binary format!");

		int ptxVersionClosest = 0;
		int ptxVersionBest = PTX_MAJOR * 10 + PTX_MINOR;
		
		__cudaFatCudaBinary *binary = (__cudaFatCudaBinary *)cubin;

		assertM(binary->ptx != 0, "binary contains no PTX");
		assertM(binary->ptx->ptx != 0, "binary contains no PTX");

		report("Getting the highest PTX version");
		
		const char* ptx = nullptr;

		for (int i = 0; binary->ptx[i].ptx != 0; i++)
		{
			std::string computeCapability = binary->ptx[i].gpuProfileName;
			std::string versionString(computeCapability.begin() + 8,
				computeCapability.end());
	
			std::stringstream version;
			version << versionString;
			
			int ptxVersion = 0;
			version >> ptxVersion;

			// We always save only the best choice ptx.
			if (std::abs(ptxVersion - ptxVersionBest) >= std::abs(ptxVersionClosest - ptxVersionBest))
				continue;

			ptxVersionClosest = ptxVersion;
			
			// We always save only the best choice ptx.
			ptx = binary->ptx[i].ptx;
		}
		
		report(" Selected version " << ptxVersionClosest);
		
		if (ptx)
		{
			consumePTX(const_cast<void*>(cubin), std::string(ptx));
			return;
		}
	}
	else if (*(int*)cubin == __cudaFatMAGIC2)
	{
		report("Found new fat binary format!");
		
		auto binary = (__cudaFatCudaBinary2*) cubin;

		auto header = (fat_elf_header*) binary->fatbinData;
		if (header->magic != __cudaFatMAGIC2)
		{
			fprintf(stderr, "CUBIN magic does not match the header info: 0x%x != 0x%x\n",
				header->magic, __cudaFatMAGIC2);
			assert(header->magic == __cudaFatMAGIC2);
			return;
		}

		if ((binary->version == 2) && binary->f)
		{
			// Separable compilation is enabled: fatbins are in the other field.
			// Deliver PTX for each prelinked fat binary.
			// Note that PTX target settings might be different for different fat binaries,
			// and we still try to pick the closest target version individually for each fat binary,
			// so in the end they are not guaranteed to be of the same target version.
			auto prelinkedFatbins = (fat_elf_header**)binary->f;
			for (int i = 0; prelinkedFatbins[i] != nullptr; i++)
			{
				std::string ptx;
				auto seenPtx = ptxdump_elf_header(prelinkedFatbins[i], ptx);
				
				if (seenPtx)
				{
					// TODO Advance from id provided by fat cubin to individual cubins in a good way.
					consumePTX(const_cast<void*>(cubin) + i, ptx);
					continue;
				}
				
				report("PTX is not present for one of prelinked fatbins, "
					"our PTX-level linking is likely to fail due to missing dependencies");
			}
		}
		else
		{
			std::string ptx;
			auto seenPtx = ptxdump_elf_header(header, ptx);
			if (seenPtx)
				consumePTX(const_cast<void*>(cubin), ptx);
			else
			{
				report("PTX is not present for one of prelinked fatbins, "
					"our PTX-level linking is likely to fail due to missing dependencies");
			}
		}
	}
	else
	{
		assertM(false, "unknown fat binary magic number " << std::hex << *(int*)cubin);		
	}
	
	report("registered, contains NO PTX");
}

void* FatBinaryPTXLoader::id() const { return const_cast<void*>(_id); }	

