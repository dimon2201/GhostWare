#pragma once

#include <string>
#include "object.hpp"
#include "types.hpp"

namespace triton
{
    class cTag : public iObject
    {
        TRITON_OBJECT(cTag)

    public:
        static constexpr types::usize kMaxTagByteSize = 32;
        using chars = std::array<types::u8, kMaxTagByteSize>;

    public:
        explicit cTag(cContext* context);
        explicit cTag(cContext* context, const std::string& text);
        explicit cTag(cContext* context, const types::u8* chars, types::usize charsByteSize);
        virtual ~cTag() override final = default;

        types::boolean Compare(const std::string& text);

        inline void* GetData() const { return _data; }
        inline types::usize GetByteSize() const { return _byteSize; }

    private:
        void FillZeros();
        void CopyChars(const types::u8* chars, types::usize charsByteSize);

    private:
        chars _data = {};
        types::usize _byteSize = 0;
    };
}