// buffer.cpp

#pragma once

#include "buffer.hpp"

namespace arachne
{
    cDataBuffer::cDataBuffer(cContext* context) : cObject(context) {}

    cDataBuffer::~cDataBuffer()
    {
    }

    void cDataBuffer::Create(void* data, types::usize byteSize)
    {
    }
}