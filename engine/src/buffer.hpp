// buffer.hpp

#pragma once

#include "object.hpp"
#include "types.hpp"

namespace arachne
{
    class cDataBuffer : public cObject
    {
        REALWARE_OBJECT(cDataBuffer)

    public:
        explicit cDataBuffer(cContext* context);
        virtual ~cDataBuffer() override;

        void Create(void* data, types::usize byteSize);

        inline void* GetData() const { return _data; }
        inline types::usize GetByteSize() const { return _byteSize; }

    private:
        void* _data = nullptr;
        types::usize _byteSize = 0;
    };
}