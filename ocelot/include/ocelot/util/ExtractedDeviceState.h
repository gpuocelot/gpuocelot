/*!
	\file ExtractedDeviceState.h
	\author Andrew Kerr <arkerr@gatech.edu>
	\date 31 Jan 2011
	\brief Data structure describing device state with serialization
		and deserialization procedures
*/

#ifndef OCELOT_UTIL_EXTRACTEDDEVICESTATE_H_INCLUDED
#define OCELOT_UTIL_EXTRACTEDDEVICESTATE_H_INCLUDED

// Ocelot includes
#include <ocelot/ir/Dim3.h>
#include <ocelot/ir/Texture.h>
#include <ocelot/ir/PTXOperand.h>

// C++ includes
#include <fstream>
#include <map>
#include <vector>

// Hydrazine includes
#include <hydrazine/json.h>

namespace util {

	typedef std::vector<char> ByteVector;

	class ExtractedDeviceState {
	public:
	
		class MemoryAllocation {
		public:
			MemoryAllocation(const void *ptr = 0, size_t size = 0);
			
			void serialize(std::ostream &out) const;
			void deserialize(const hydrazine::json::Visitor &visitor);
		
			size_t size() const;
			
		public:
		
			//! \brief references the allocation on the device
			const void *devicePointer;
		
			//! \brief binary representation of data
			ByteVector data;
		};

		class GlobalAllocation {
		public:
			GlobalAllocation(const void *ptr = 0, size_t size = 0,
				void* id = nullptr,
				const std::string& globalName = "");
			
			void serialize(std::ostream &out) const;
			void deserialize(const hydrazine::json::Visitor &visitor);
		
			size_t size() const;
			
			std::string key() const; 
			
		public:
			//! \brief The module that the allocation is stored in
			void* id;
			
			//! \brief The name of the global allocation
			std::string name;
		
			//! \brief binary representation of data
			ByteVector data;
		};
		
		typedef std::map<std::string, ir::Texture*>       TextureMap;
		typedef std::map<std::string, GlobalAllocation*>  GlobalVariableMap;
		typedef std::map<const void *, MemoryAllocation*> GlobalAllocationMap;
		
		/*!
		
		*/
		class Module {
		public:
		
			Module();
			
			~Module();
		
			void clear();
			
			void serialize(std::ostream &out,
				const std::string & prefix = "") const;
			void deserialize(const hydrazine::json::Visitor &visitor);
			
		private:
		
			void serializeTexture(const ir::Texture &texture,
				std::ostream &out, const std::string & prefix = "") const;
			void deserializeTexture(ir::Texture &texture,
				const hydrazine::json::Visitor &visitor);
		
		public:
			//! \brief module id
			void* id;
			
			//! \brief file to which PTX representation of module is written
			std::string ptx;
						
			//! \brief textures contained in the module
			TextureMap textures;
		};
		
		class KernelLaunch {
		public:
		
			void serialize(std::ostream &out) const;
			void deserialize(const hydrazine::json::Visitor &visitor);

		public:
			//! \brief id of module owning kernel
			void* id;
			
			//! \brief name of kernel
			std::string kernelName;
		
			//! \brief dimensions of CTAs
			ir::Dim3 blockDim;
			
			//! \brief dimensions of grid
			ir::Dim3 gridDim;
			
			//! \brief shared memory size
			size_t sharedMemorySize;
			
			//! \brief static shared memory size
			size_t staticSharedMemorySize;
			
			//! \brief block of memory denoting parameter memory
			ByteVector parameterMemory;
		};
		
		class Application {
		public:
		
			Application();
		
			void serialize(std::ostream &out) const;
			void deserialize(const hydrazine::json::Visitor &visitor);
		
		public:
			//! \brief name of application
			std::string name;
			
			//! \brief name of target CUDA device
			std::string cudaDevice;
			
		};
	
		typedef std::map<void*, Module*> ModuleMap;
	
	public:
	
		ExtractedDeviceState(std::istream &in);
		ExtractedDeviceState();
		~ExtractedDeviceState();
		
		void serialize(std::ostream &out) const;
		void deserialize(std::istream &in);
		
		void clear();
		void clearData();
		
	public:
	
		//! \brief data specific to an application
		Application application;
	
		//! \brief maps module names onto structure capturing module state
		ModuleMap modules;
		
		//! \brief values of global allocations before kernel launch
		GlobalAllocationMap globalAllocations;
		
		//! \brief values of global variables before the kernel launch
		GlobalVariableMap globalVariables;
		
		//! \brief values of global allocations after kernel launch
		GlobalAllocationMap postLaunchGlobalAllocations;
		
		//! \brief values of global variables after the kernel launch
		GlobalVariableMap postLaunchGlobalVariables;
		
		//! \brief parameters of CUDA launch
		KernelLaunch launch;
	};

}

#endif

