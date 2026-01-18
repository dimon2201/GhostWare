// filesystem_manager.cpp

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include "application.hpp"
#include "context.hpp"
#include "filesystem_manager.hpp"
#include "memory_pool.hpp"
#include "buffer.hpp"
#include "engine.hpp"

using namespace types;

namespace arachne
{
    cDataFile::cDataFile(cContext* context) : cObject(context) {}

    cDataFile::~cDataFile()
    {
        if (_data)
        {
            auto memoryAllocator = _context->GetMemoryAllocator();
            memoryAllocator->Deallocate(_data);
        }
    }

    void cDataFile::Open(const std::string& path, types::boolean isText)
    {
        const sEngineCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
        auto memoryAllocator = _context->GetMemoryAllocator();

        if (_data)
            memoryAllocator->Deallocate(_data);

        std::ifstream inputFile(path, std::ios::binary);

        inputFile.seekg(0, std::ios::end);
        const usize byteSize = inputFile.tellg();
        inputFile.seekg(0, std::ios::beg);
        const usize databyteSize = byteSize + (isText == K_TRUE ? 1 : 0);

        _data = (cDataBuffer*)memoryAllocator->Allocate(sizeof(cDataBuffer), caps->memoryAlignment);
        memset(_data, 0, databyteSize);
        inputFile.read((char*)&_data[0], byteSize);
    }

    cFileSystem::cFileSystem(cContext* context) : cObject(context) {}

    cDataFile* cFileSystem::CreateDataFile(const std::string& path, types::boolean isText)
    {
        cDataFile* file = _context->Create<cDataFile>(_context);
        file->Open(path, isText);

        return nullptr;
    }

    void cFileSystem::DestroyDataFile(cDataFile* file)
    {
        if (file == nullptr)
            return;

        _context->Destroy<cDataFile>(file);
    }
}