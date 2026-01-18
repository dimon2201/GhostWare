// layer.hpp

#pragma once

#include "types.hpp"

namespace arachne
{
    namespace core
    {
        class cScene;

        class cLayer
        {

        public:
            cLayer() {}
            ~cLayer() {}

            virtual void Init(cScene* scene) = 0;
            virtual void Free(cScene* scene) = 0;
            virtual void Update(cScene* scene) = 0;

            void Toggle(types::boolean isEnabled) { m_IsEnabled = ~m_IsEnabled; }

            types::boolean GetState() { return m_IsEnabled; }

        private:
            types::boolean m_IsEnabled = types::K_TRUE;

        };
    }
}