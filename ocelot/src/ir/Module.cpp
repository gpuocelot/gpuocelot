/*! \file Module.cpp
	\author Andrew Kerr <arkerr@gatech.edu>
	\date Jan 15, 2009
	\brief declares a Module loadable from a PTX source file and runable
*/


#include <ocelot/ir/Module.h>
#include <ocelot/ir/PTXKernel.h>
#include <ocelot/parser/PTXParser.h>

#include <hydrazine/debug.h>
#include <hydrazine/Exception.h>

#include <fstream>
#include <cassert>

#include "res_embed.h"

#include <cstring>
#include <streambuf>

#ifdef REPORT_BASE
#undef REPORT_BASE
#endif

////////////////////////////////////////////////////////////////////////////////

// this toggles emitting function prototypes in Module::writeIR()
#define EMIT_FUNCTION_PROTOTYPES 1

#define REPORT_BASE 0

////////////////////////////////////////////////////////////////////////////////

ir::Module::Module(const std::string& path)
{
        //.target sm_21
        _target.directive = PTXStatement::Directive::Target;
        _target.targets.push_back("sm_21");
        //.version 2.3
        _version.directive = PTXStatement::Directive::Version;
        _version.minor = 3;
        _version.major = 2;

        load(path);
}

ir::Module::Module(void* id, const std::string& path)
{
	//.target sm_21
	_target.directive = PTXStatement::Directive::Target;
	_target.targets.push_back("sm_21");
	//.version 2.3
	_version.directive = PTXStatement::Directive::Version;
	_version.minor = 3;
	_version.major = 2;

	load(id, path);
}

ir::Module::Module(void* id, std::istream& stream)
{
	//.target sm_21
	_target.directive = PTXStatement::Directive::Target;
	_target.targets.push_back("sm_21");
	//.version 2.3
	_version.directive = PTXStatement::Directive::Version;
	_version.minor = 3;
	_version.major = 2;

	load(id, stream);
}

ir::Module::Module()
{
	//.target sm_21
	_target.directive = PTXStatement::Directive::Target;
	_target.targets.push_back("sm_21");
	//.version 2.3
	_version.directive = PTXStatement::Directive::Version;
	_version.minor = 3;
	_version.major = 2;
}

ir::Module::Module(const ir::Module& m) : _loaded(false)
{
	*this = m;
}

ir::Module::~Module()
{
	unload();
}


ir::Module::Module(void* id, const StatementVector& statements) : _id(id)
{
	_statements = statements;
	extractPTXKernels();
}

const ir::Module& ir::Module::operator=(const Module& m)
{
	unload();

	_id = m.id();	
	_loaded     = m.loaded();

	if(loaded()) {
		_statements = m._statements;
		
		_textures   = m._textures;
		_prototypes = m._prototypes;
		_globals    = m._globals;
		
		_version = m._version;
		_target  = m._target;
		
		for(KernelMap::const_iterator k = m._kernels.begin();
			k != m._kernels.end(); ++k)
		{
			insertKernel(new PTXKernel(*k->second));
		}
	}
	else {
		_ptx = m._ptx;
	}
	
	return *this;
}

////////////////////////////////////////////////////////////////////////////////

/*!
	Deletes everything associated with this particular module
*/
void ir::Module::unload()
{
	// delete all available kernels
	for (KernelMap::iterator kern_it = _kernels.begin(); 
		kern_it != _kernels.end(); ++kern_it) {
		delete kern_it->second;
	}
	_kernels.clear();
	_statements.clear();
	_textures.clear();
	_globals.clear();
	_id = nullptr;
	
	_loaded = false;
}

/*!
	Unloads module and loads everything in path
*/
bool ir::Module::load(void* id, const std::string& path)
{
	using namespace std;

	_id = id;

	// open file, parse file, extract statements vector

	ifstream file(path.c_str());

	if (file.is_open())
	{
		unload();

		parser::PTXParser parser;
		parser.fileName = path;

		parser.parse( file );

		_statements = std::move( parser.statements() );
		extractPTXKernels();
	}
	else {
		return false;
	}
	
	_loaded = true;

	return true;
}

/*!
	Unloads module and loads everything in path
*/
bool ir::Module::load(const std::string& path)
{
	char* id = (char*)malloc(path.size() + 1);
	strcpy(id, path.c_str());
	return load(id, path);
}

struct membuf : std::streambuf
{
	membuf(char* begin, char* end) {
		this->setg(begin, begin, end);
	}
};

bool ir::Module::loadEmbedded(const std::string& name)
{
	char* id = (char*)malloc(name.size() + 1);
	strcpy(id, name.c_str());
	return loadEmbedded(id, name);
}

bool ir::Module::loadEmbedded(void* id, const std::string& name)
{
	_id = id;

	size_t szptx = 0;
	char* ptx = const_cast<char*>(res::embed::get(name, &szptx));

	if (ptx && szptx)
	{
		unload();

		membuf sbuf(ptx, ptx + szptx);
		std::istream mem(&sbuf);

		parser::PTXParser parser;

		parser.parse( mem );

		_statements = std::move( parser.statements() );
		extractPTXKernels();
	}
	else {
		return false;
	}

	_loaded = true;

	return true;
}

/*!
        Unloads module and loads everything in path
*/
bool ir::Module::load(std::istream& stream)
{
        char* id = (char*)malloc(sizeof(char));
	id[0] = '\0';
        return load(id, stream);
}

/*!
	Unloads module and loads everything in path
*/
bool ir::Module::load(void* id, std::istream& stream)
{	
	unload();
	
	parser::PTXParser parser;
	_id = id;
	
	parser.parse( stream );
	_statements = std::move( parser.statements() );
	extractPTXKernels();

	_loaded = true;

	return true;
}

bool ir::Module::lazyLoad(void* id, const std::string& source)
{
	unload();
	
	_ptx = std::move( source );
	_id = id;
	
	return true;
}

bool ir::Module::lazyLoad(void* id, const char* source)
{
	unload();
	
	_ptx.assign(source);
	_id = id;
	
	return true;
}

bool ir::Module::lazyLoadEmbedded(void* id, const std::string& name)
{
	unload();

	size_t szptx = 0;
	char* ptx = const_cast<char*>(res::embed::get(name, &szptx));

	_ptx.assign(ptx, ptx + szptx);
	_id = id;

	return true;
}

void ir::Module::loadNow() {
	if (loaded()) return;
	
	if (_ptx.empty())
	{
		report("Module::loadNow() - id: '" << id()
			<< "' contains no PTX");
		return;
	}

	std::stringstream stream( std::move( _ptx ) );
	_ptx.clear();
	
	parser::PTXParser parser;
	
	parser.parse( stream );
	_statements = std::move( parser.statements() );
	extractPTXKernels();

	_loaded = true;
}	
	
bool ir::Module::loaded() const {
	return _loaded;
}

////////////////////////////////////////////////////////////////////////////////

void ir::Module::write( std::ostream& stream ) const {
	assert( loaded() );

	report("Writing module (statements) - " << _id 
		<< " - to output stream.");

	if( _statements.empty() ) {
		return;
	}
		
	PTXStatement::Directive previous = PTXStatement::Directive_invalid;
	
	bool inEntry = false;
	
	for( StatementVector::const_iterator statement = _statements.begin(); 
		statement != _statements.end(); ++statement ) {
		report( "Line " << ( statement - _statements.begin() ) 
			<< ": " << statement->toString() );
		if( statement->directive == PTXStatement::StartScope )
		{
			inEntry = true;
		}
		else if( statement->directive == PTXStatement::EndScope ) {
			inEntry = false;
		}
		
		if( statement->directive == PTXStatement::Param )
		{
			if( !inEntry )
			{
				if( previous != PTXStatement::StartParam )
				{
					stream << ",\n\t" << statement->toString();
				}
				else
				{
					stream << "\n\t" << statement->toString();
				}
			}
			else
			{
				stream << "\n\t" << statement->toString() << ";";
			}
		}
		else
		{
			stream << "\n";
			if( statement->directive == PTXStatement::Instr 
				|| statement->directive == PTXStatement::Loc ) {
				stream << "\t";
			}
			stream << statement->toString();
		}
		previous = statement->directive;
	}
	
	stream << "\n";
}

void ir::Module::writeIR( std::ostream& stream, PTXEmitter::Target emitterTarget) const {
	assert( loaded() );
	report("Writing module (IR) - " << _id << " - to output stream.");

	stream << "/*\n* Ocelot Version : TODO\n*/\n\n";
		
	stream << _version.toString() << "\n";
	stream << _target.toString()  << "\n";

	if((_version.major > 2) ||
		( (_version.major == 2) && (_version.minor >= 3))) {
		stream << ".address_size " << addressSize() << "\n";
	}
	
	stream << "/* Module " << _id << " */\n\n";
	
#if EMIT_FUNCTION_PROTOTYPES == 1
	{
		std::set<std::string> encounteredPrototypes;
		
		stream << "/* Function prototypes */\n";
		for (FunctionPrototypeMap::const_iterator prot_it = _prototypes.begin();
			prot_it != _prototypes.end(); ++prot_it) {
		
			if (prot_it->second.identifier != "") {
				stream << prot_it->second.toString(emitterTarget) << ";\n";
				encounteredPrototypes.insert(prot_it->second.identifier);
			}
		}
		
		for (KernelMap::const_iterator kernel = _kernels.begin();
			kernel != _kernels.end(); ++kernel) {
			if (encounteredPrototypes.count((kernel->second)->name) == 0) {
			
				stream << kernel->second->getPrototype().toString(emitterTarget) << ";\n";
				encounteredPrototypes.insert(kernel->second->name);
			}
		}
	}
#endif

	stream << "\n/* Globals */\n";
	for (GlobalMap::const_iterator global = _globals.begin(); 
		global != _globals.end(); ++global) {
		stream << global->second.statement.toString() << "\n";
	}
	stream << "\n";

	stream << "/* Textures */\n";
	for (TextureMap::const_iterator texture = _textures.begin(); 
		texture != _textures.end(); ++texture) {
		stream << texture->second.toString() << "\n";
	}
	stream << "\n";
	
	stream << "/* Kernels */\n";
	for (KernelMap::const_iterator kernel = _kernels.begin();
		kernel != _kernels.end(); ++kernel) {
		(kernel->second)->writeWithEmitter(stream, emitterTarget);
	}
	
	stream << "\n\n";
}


std::string ir::Module::toString(PTXEmitter::Target emitterTarget) const {
	std::stringstream stream;
	
	writeIR(stream, emitterTarget);

	return stream.str();
}

ir::Texture* ir::Module::getTexture(const std::string& name) {
	loadNow();
	TextureMap::iterator texture = _textures.find(name);
	if (texture != _textures.end()) {
		return &texture->second;
	}
	return 0;
}

ir::Texture* ir::Module::insertTexture(const Texture& texture) {
	typedef std::pair<TextureMap::iterator, bool> Insertion;
	
	loadNow();
	
	Insertion insertion = _textures.insert(
		std::make_pair(texture.demangledName(), texture));
	if(!insertion.second) {
		throw hydrazine::Exception("Inserted duplicated texture - " 
			+ texture.name);
	}
	
	return &insertion.first->second;
}

void ir::Module::removeTexture(const std::string& name) {
	loadNow();
	TextureMap::iterator texture = _textures.find(name);
	if (texture != _textures.end()) {
		_textures.erase(texture);
	}
}

ir::Global* ir::Module::getGlobal(const std::string& name) {
	loadNow();
	GlobalMap::iterator global = _globals.find(name);
	if (global != _globals.end()) {
		return &global->second;
	}
	return 0;
}

const ir::Global* ir::Module::getGlobal(const std::string& name) const {
	GlobalMap::const_iterator global = _globals.find(name);
	if (global != _globals.end()) {
		return &global->second;
	}
	return 0;
}

ir::Global* ir::Module::insertGlobal(const Global& global) {
	typedef std::pair<GlobalMap::iterator, bool> Insertion;
	
	loadNow();
	
	Insertion insertion = _globals.insert(
		std::make_pair(global.name(), global));
	
	if(!insertion.second) {
		throw hydrazine::Exception("Inserted duplicated global - " 
			+ global.name());
	}
	return &insertion.first->second;
}

void ir::Module::removeGlobal(const std::string& name) {
	loadNow();
	GlobalMap::iterator global = _globals.find(name);
	if (global != _globals.end()) {
		_globals.erase(global);
	}
}

void ir::Module::insertGlobalAsStatement(const PTXStatement &statement) {
    loadNow();
   
    if(_globals.find(statement.name) != _globals.end())
        return;
   
    if(!_globals.insert(std::make_pair(statement.name, Global(statement))).second) {
        throw hydrazine::Exception("Inserted duplicated global - "
            + statement.name);
    }
}   

void* ir::Module::id() const {
	return _id;
}

const ir::Module::KernelMap& ir::Module::kernels() const {
	assert( loaded() );
	return _kernels;
}

const ir::Module::GlobalMap& ir::Module::globals() const {
	assert( loaded() );
	return _globals;
}

const ir::Module::TextureMap& ir::Module::textures() const {
	assert( loaded() );
	return _textures;
}

const ir::Module::StatementVector& ir::Module::statements() const {
	assert( loaded() );
	return _statements;
}

const ir::Module::FunctionPrototypeMap& ir::Module::prototypes() const {
	assert( loaded() );
	return _prototypes;
}

unsigned int ir::Module::addressSize() const {
	return _addressSize;
}

void ir::Module::addPrototype(const std::string &identifier,
	const PTXKernel::Prototype &prototype) {
	report("adding prototype: " << prototype.toString());
	
	_prototypes.insert(std::make_pair(identifier, prototype));
}

void ir::Module::removePrototype(const std::string& name) {
	loadNow();
	FunctionPrototypeMap::iterator prototype = _prototypes.find(name);
	if (prototype != _prototypes.end()) {
		_prototypes.erase(prototype);
	}
}
		
ir::PTXKernel* ir::Module::getKernel(const std::string& kernelName) {
	loadNow();
	KernelMap::iterator kernel = _kernels.find(kernelName);
	if (kernel != _kernels.end()) {
		return kernel->second;
	}
	return 0;
}
		
const ir::PTXKernel* ir::Module::getKernel(
	const std::string& kernelName) const {

	KernelMap::const_iterator kernel = _kernels.find(kernelName);
	if (kernel != _kernels.end()) {
		return kernel->second;
	}
	return 0;
}

void ir::Module::removeKernel(const std::string& name) {
	loadNow();
	KernelMap::iterator kernel = _kernels.find(name);
	if (kernel != _kernels.end()) {
		delete kernel->second;
		_kernels.erase(kernel);
	}
}

ir::PTXKernel* ir::Module::insertKernel(PTXKernel* kernel) {
	loadNow();
	
	kernel->module = this;
	
	if(!_kernels.insert(std::make_pair(kernel->name, kernel)).second) {
		throw hydrazine::Exception("Inserted duplicated kernel - " 
			+ kernel->name);
	}
	
	return kernel;
}

void ir::Module::setVersion(unsigned int major, unsigned int minor) {
	_version.major = major;
	_version.minor = minor;
}
		
ir::PTXStatement ir::Module::version() const { return _version; }
ir::PTXStatement ir::Module::target()  const { return _target;  }

/*!
	After parsing, construct a set of Kernels with ISA equal to PTX
	 from the statements vector.
*/
void ir::Module::extractPTXKernels() {

	using namespace std;
	StatementVector::const_iterator startIterator = _statements.end(), 
		endIterator = _statements.end();

	bool inKernel = false;
	unsigned int instructionCount = 0;
	unsigned int kernelInstance = 1;
	bool isFunction = false;
	unsigned int depth = 0;
	PTXKernel::Prototype functionPrototype;
	
	report("extractPTXKernels()");
	
	enum {
		PS_NoState,
		PS_ReturnParams,
		PS_Params,
		PS_End
	} prototypeState = PS_NoState;

	for (StatementVector::const_iterator it = _statements.begin(); 
		it != _statements.end(); ++it) {
		const PTXStatement &statement = (*it);
	
		if (statement.directive != PTXStatement::Instr &&
			statement.directive != PTXStatement::Loc) {
			report("directive: "
				<< PTXStatement::toString(statement.directive));
		}
	
		switch (statement.directive) {
			case PTXStatement::Version:
			{
				_version = statement;
			}
			break;
			case PTXStatement::Target:
			{
				_target = statement;
			}
			break;
			case PTXStatement::Entry:	// fallthrough
			case PTXStatement::Func:
			{
				// new kernel
				assert(!inKernel);
				startIterator = it;
				inKernel = true;
				isFunction = statement.directive == PTXStatement::Func;
				instructionCount = 0;
				functionPrototype.clear();
				
				if (prototypeState == PS_NoState) {
					if (!isFunction) {
						functionPrototype.identifier = statement.name;
					}
					functionPrototype.callType = 
						(isFunction ? 
							PTXKernel::Prototype::Func : 
							PTXKernel::Prototype::Entry);
					functionPrototype.linkingDirective =
						(PTXKernel::Prototype::LinkingDirective)
						statement.attribute;
					static_assert((int)PTXKernel::Prototype::Visible ==
						(int)ir::PTXStatement::Visible, "Mismatched flag");
					static_assert((int)PTXKernel::Prototype::Extern ==
						(int)ir::PTXStatement::Extern, "Mismatched flag");
					static_assert((int)PTXKernel::Prototype::Weak ==
						(int)ir::PTXStatement::Weak, "Mismatched flag");
					static_assert((int)PTXKernel::Prototype::InternalHidden ==
						(int)ir::PTXStatement::NoAttribute, "Mismatched flag");
					prototypeState = PS_Params;
				}
			}
			break;
			case PTXStatement::FunctionName:
			{
				report("  function name: " << statement.name);
				functionPrototype.identifier = statement.name;
				functionPrototype.returnArguments = functionPrototype.arguments;
				functionPrototype.arguments.clear();
				prototypeState = PS_Params;
			}
			break;
			case PTXStatement::EndScope:
			{
				report("  end scope: '}'");
				assert(inKernel);
				assert(depth != 0);
				
				--depth;
				
				if (depth == 0) {
					// construct the kernel and push it onto something
					inKernel = false;
					endIterator = ++StatementVector::const_iterator(it);
					if (instructionCount) {
						PTXKernel *kernel = new PTXKernel(startIterator, 
							    endIterator, isFunction, kernelInstance++);
						kernel->module = this;
						_kernels[kernel->name] = (kernel);
					}
				}
			}
			break;
			case PTXStatement::EndFuncDec:
			{
				report("  end func dec:");
				
				assert(inKernel);
				inKernel   = false;
				isFunction = false;
				if (prototypeState != PS_NoState) {
					addPrototype(functionPrototype.identifier,
						functionPrototype);
					prototypeState = PS_NoState;
				}
				
			}
			break;
			case PTXStatement::StartScope:
			{
				report("  start scope: '{'");
				if (prototypeState != PS_NoState) {
					addPrototype(functionPrototype.identifier,
						functionPrototype);
					prototypeState = PS_NoState;
				}
				
				++depth;
			}
			break;
			case PTXStatement::Param:
			{						
				if (prototypeState == PS_ReturnParams || PS_Params) {
					ir::Parameter argument(statement, false);
					if (prototypeState == PS_ReturnParams) {
						report("  appending " << argument.name
							<< " to returnArguments");
						functionPrototype.returnArguments.push_back(argument);
					}
					else {
						report("  appending " << argument.name
							<< " to arguments");
						functionPrototype.arguments.push_back(argument);
					}					
				}
				else {
				
				}
			}
			break;
				
			case PTXStatement::Instr:
			{
				if (inKernel) {
					instructionCount++;
				}
			}
			break;
			case PTXStatement::Const:  // fallthrough
			case PTXStatement::Global: // fallthrough
			case PTXStatement::Shared: // fallthrough
			case PTXStatement::Local:
			{
				if (!inKernel) {
					assertM(_globals.count(statement.name) == 0,
						"Global operand '" 
						<< statement.name << "' declared more than once." );

					_globals.insert(std::make_pair(statement.name,
						Global(statement)));
				}
			}
			break;

			case PTXStatement::AddressSize:
			{
				_addressSize = statement.addressSize;
			}
			break;

			case PTXStatement::Texref:
			{
				if (!inKernel) {
					assert(_textures.count(statement.name) == 0);
					Texture texture(statement.name, Texture::Texref);
					
					_textures.insert(std::make_pair(texture.demangledName(), 
									texture));
				}
			}
			break;
			case PTXStatement::Surfref:
			{
				if (!inKernel) {
					Texture texture(statement.name, Texture::Surfref);
					
					assert(_textures.count(statement.name) == 0);
					_textures.insert(std::make_pair(texture.demangledName(), 
									texture));
				}
			}
			break;
			case PTXStatement::Samplerref:
			{
				if (!inKernel) {
					Texture texture(statement.name, Texture::Samplerref);
					
					assert(_textures.count(statement.name) == 0);
					_textures.insert(std::make_pair(texture.demangledName(), 
									texture));
				}
			}
			break;
				
			default:
				break;
		}
	}
}

