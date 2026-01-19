// event_types.hpp

#include <functional>

namespace harpy
{
    class cDataBuffer;

    using EventFunction = std::function<void(cDataBuffer* const data)>;

    enum class eEventType
    {
        NONE,
        KEY_PRESS
    };
}