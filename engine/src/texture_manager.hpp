// texture_manager.hpp

#pragma once

#include <string>
#include <vector>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "id_vec.hpp"
#include "object.hpp"
#include "types.hpp"

namespace arachne
{
    class cContext;
    class iGraphicsAPI;
    class cApplication;
    class cTexture;

    class cTextureAtlasTexture : public cObject
    {
        REALWARE_OBJECT(cTextureAtlasTexture)

    public:
        cTextureAtlasTexture(cContext* context, types::boolean isNormalized, const glm::vec3& offset, const glm::vec2& size, cTexture* atlas = nullptr);
        ~cTextureAtlasTexture() = default;

        inline types::boolean IsNormalized() const { return _isNormalized; }
        inline const glm::vec3& GetOffset() const { return _offset; }
        inline const glm::vec2& GetSize() const { return _size; }

    private:
        types::boolean _isNormalized = types::K_FALSE;
        glm::vec3 _offset = glm::vec3(0.0f);
        glm::vec2 _size = glm::vec2(0.0f);
    };

    struct sTextureAtlasTextureGPU
    {
        glm::vec4 _textureInfo = glm::vec4(0.0f);
        types::f32 _textureLayerInfo = 0.0f;
    };

    class cTextureAtlas : public cObject
    {
        REALWARE_OBJECT(cTextureAtlas)

    public:
        explicit cTextureAtlas(cContext* context);
        virtual ~cTextureAtlas();

        cTextureAtlasTexture* CreateTexture(const std::string& id, const glm::vec2& size, types::usize channels, const types::u8* data);
        cTextureAtlasTexture* CreateTexture(const std::string& id, const std::string& filename);
        cTextureAtlasTexture* FindTexture(const std::string& id);
        void DestroyTexture(const std::string& id);

        cTexture* GetAtlas() const;
        types::usize GetWidth() const;
        types::usize GetHeight() const;
        types::usize GetDepth() const;

        void SetAtlas(const glm::vec3& size);

    protected:
        iGraphicsAPI* _gfx = nullptr;
        cTexture* _atlas = nullptr;
        cIdVector<cTextureAtlasTexture> _textures;
    };
}