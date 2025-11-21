// render_manager.hpp

#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "id_vec.hpp"
#include "category.hpp"
#include "types.hpp"

namespace realware
{
    namespace app
    {
        class cApplication;
    }

    namespace game
    {
        class cGameObject;
    }

    namespace render
    {
        class cTextureAtlasTexture;
    }

    namespace render
    {
        struct sBuffer;
        struct sVertexArray;
        struct sRenderTarget;
        struct sRenderPass;
        struct sShader;
        class iRenderContext;

        using index = types::u32;

        struct sVertex
        {
            glm::vec3 Position = glm::vec3(0.0f);
            glm::vec2 Texcoord = glm::vec2(0.0f);
            glm::vec3 Normal = glm::vec3(0.0f);
        };

        struct sVertexBufferGeometry
        {
            types::usize VertexCount = 0;
            types::usize IndexCount = 0;
            void* VertexPtr = nullptr;
            void* IndexPtr = nullptr;
            types::usize OffsetVertex = 0;
            types::usize OffsetIndex = 0;
            game::Category Format = game::Category::VERTEX_BUFFER_FORMAT_NONE;
        };

        struct sPrimitive
        {
            sVertex* Vertices = nullptr;
            index* Indices = nullptr;
            types::usize VertexCount = 0;
            types::usize IndexCount = 0;
            types::usize VerticesByteSize = 0;
            types::usize IndicesByteSize = 0;
            game::Category Format = game::Category::VERTEX_BUFFER_FORMAT_NONE;
        };

        struct sModel : sPrimitive
        {
        };

        struct sLight
        {
            glm::vec3 Color = glm::vec3(0.0f);
            glm::vec3 Direction = glm::vec3(0.0f);
            types::f32 Scale = 0.0f;
            types::f32 AttenuationConstant = 0.0f;
            types::f32 AttenuationLinear = 0.0f;
            types::f32 AttenuationQuadratic = 0.0f;
        };

        struct sTransform
        {
            sTransform() = default;
            sTransform(const game::cGameObject* const gameObject);

            void Transform();

            types::boolean Use2D = types::K_FALSE;
            glm::vec3 Position = glm::vec3(0.0f);
            glm::vec3 Rotation = glm::vec3(0.0f);
            glm::vec3 Scale = glm::vec3(1.0f);
            glm::mat4 World = glm::mat4(1.0f);
        };

        class cMaterial : public cIdVecObject
        {
        public:
            explicit cMaterial(const std::string& id, const app::cApplication* const app, const render::cTextureAtlasTexture* const diffuseTexture, const glm::vec4& diffuseColor, const glm::vec4& highlightColor, const sShader* const customShader)
                : cIdVecObject(id, app), _diffuseTexture((render::cTextureAtlasTexture*)diffuseTexture), _diffuseColor(diffuseColor), _highlightColor(highlightColor), _customShader((sShader*)customShader) {}
            ~cMaterial() = default;

            inline render::sShader* GetCustomShader() const { return _customShader; }
            inline render::cTextureAtlasTexture* GetDiffuseTexture() const { return _diffuseTexture; }
            inline const glm::vec4& GetDiffuseColor() const { return _diffuseColor; }
            inline const glm::vec4& GetHighlightColor() const { return _highlightColor; }

        private:
            render::sShader* _customShader = nullptr;
            render::cTextureAtlasTexture* _diffuseTexture = nullptr;
            glm::vec4 _diffuseColor = glm::vec4(1.0f);
            glm::vec4 _highlightColor = glm::vec4(1.0f);
        };

        struct sRenderInstance
        {
            sRenderInstance(const types::s32 materialIndex, const sTransform& transform);

            types::f32 Use2D = 0.0f;
            types::s32 MaterialIndex = -1;
            types::dword _pad[2] = {};
            glm::mat4 World = {};
        };

        struct sTextInstance
        {
            glm::vec4 Info = glm::vec4(0.0f);
            glm::vec4 AtlasInfo = glm::vec4(0.0f);
        };

        class cMaterialInstance
        {
        public:
            cMaterialInstance(const types::s32 materialIndex, const cMaterial* const material);

        private:
            types::s32 _bufferIndex = -1;
            types::f32 _diffuseTextureLayerInfo = 0.0f;
            types::f32 _metallicTextureLayerInfo = 0.0f;
            types::f32 _roughnessTextureLayerInfo = 0.0f;
            types::f32 _userData[4] = {};
            glm::vec4 _diffuseTextureInfo = glm::vec4(0.0f);
            glm::vec4 _diffuseColor = glm::vec4(0.0f);
            glm::vec4 _highlightColor = glm::vec4(0.0f);
        };

        struct sLightInstance
        {
            sLightInstance(const game::cGameObject* const object);

            glm::vec4 Position = glm::vec4(0.0f);
            glm::vec4 Color = glm::vec4(0.0f);
            glm::vec4 DirectionAndScale = glm::vec4(0.0f);
            glm::vec4 Attenuation = glm::vec4(0.0f);
        };

        class mRender
        {
        public:
            explicit mRender(const app::cApplication* const app, const iRenderContext* const context);
            ~mRender();

            cMaterial* CreateMaterial(const std::string& id, const render::cTextureAtlasTexture* const diffuseTexture, const glm::vec4& diffuseColor, const glm::vec4& highlightColor, const game::Category& customShaderRenderPath = game::Category::RENDER_PATH_OPAQUE, const std::string& customVertexFuncPath = "", const std::string& customFragmentFuncPath = "");
            cMaterial* FindMaterial(const std::string& id);
            void DestroyMaterial(const std::string& id);
            sVertexArray* CreateDefaultVertexArray();
            sVertexBufferGeometry* CreateGeometry(const game::Category& format, const types::usize verticesByteSize, const void* const vertices, const types::usize indicesByteSize, const void* const indices);
            void DestroyGeometry(sVertexBufferGeometry* geometry);
            
            void ClearGeometryBuffer();
            void ClearRenderPass(const sRenderPass* const renderPass, const types::boolean clearColor, const types::usize bufferIndex, const glm::vec4& color, const types::boolean clearDepth, const types::f32 depth);
            void ClearRenderPasses(const glm::vec4& clearColor, const types::f32 clearDepth);
            
            void UpdateLights();

            void WriteObjectsToOpaqueBuffers(cIdVec<game::cGameObject>& objects, sRenderPass* const renderPass);
            void WriteObjectsToTransparentBuffers(cIdVec<game::cGameObject>& objects, sRenderPass* const renderPass);
            void DrawGeometryOpaque(const sVertexBufferGeometry* const geometry, const game::cGameObject* const cameraObject, sRenderPass* const renderPass);
            void DrawGeometryOpaque(const sVertexBufferGeometry* const geometry, const game::cGameObject* const cameraObject, sShader* const singleShader = nullptr);
            void DrawGeometryTransparent(const sVertexBufferGeometry* const geometry, const std::vector<game::cGameObject>& objects, const game::cGameObject* const cameraObject, sRenderPass* const renderPass);
            void DrawGeometryTransparent(const sVertexBufferGeometry* const geometry, const game::cGameObject* const cameraObject, sShader* const singleShader = nullptr);
            void DrawTexts(const std::vector<game::cGameObject>& objects);
            
            void CompositeTransparent();
            void CompositeFinal();
            
            sPrimitive* CreatePrimitive(const game::Category& primitive);
            sModel* CreateModel(const std::string& filename);
            void DestroyPrimitive(sPrimitive* primitiveObject);
            
            void LoadShaderFiles(const std::string& vertexFuncPath, const std::string& fragmentFuncPath, std::string& vertexFunc, std::string& fragmentFunc);

            void ResizeWindow(const glm::vec2& size);
            
            inline sBuffer* GetVertexBuffer() const { return _vertexBuffer; }
            inline sBuffer* GetIndexBuffer() const { return _indexBuffer; }
            inline sBuffer* GetOpaqueInstanceBuffer() const { return _opaqueInstanceBuffer; }
            inline sBuffer* GetTextInstanceBuffer() const { return _textInstanceBuffer; }
            inline sBuffer* GetOpaqueMaterialBuffer() const { return _opaqueMaterialBuffer; }
            inline sBuffer* GetTransparentInstanceBuffer() const { return _transparentInstanceBuffer; }
            inline sBuffer* GetTransparentMaterialBuffer() const { return _transparentMaterialBuffer; }
            inline sBuffer* GetTextMaterialBuffer() const { return _textMaterialBuffer; }
            inline sBuffer* GetLightBuffer() const { return _lightBuffer; }
            inline sBuffer* GetOpaqueTextureAtlasTexturesBuffer() const { return _opaqueTextureAtlasTexturesBuffer; }
            inline sBuffer* GetTransparentTextureAtlasTexturesBuffer() const { return _transparentTextureAtlasTexturesBuffer; }
            inline sRenderPass* GetOpaqueRenderPass() const { return _opaque; }
            inline sRenderPass* GetTransparentRenderPass() const { return _transparent; }
            inline sRenderPass* GetTextRenderPass() const { return _text; }
            inline sRenderPass* GetCompositeTransparentRenderPass() const { return _compositeTransparent; }
            inline sRenderPass* GetCompositeFinalRenderPass() const { return _compositeFinal; }
            inline sRenderTarget* GetOpaqueRenderTarget() const { return _opaqueRenderTarget; }
            inline sRenderTarget* GetTransparentRenderTarget() const { return _transparentRenderTarget; }

        private:
            app::cApplication* _app = nullptr;
            iRenderContext* _context = nullptr;
            types::usize _maxOpaqueInstanceBufferByteSize = 0;
            types::usize _maxTransparentInstanceBufferByteSize = 0;
            types::usize _maxTextInstanceBufferByteSize = 0;
            types::usize _maxMaterialBufferByteSize = 0;
            types::usize _maxLightBufferByteSize = 0;
            types::usize _maxTextureAtlasTexturesBufferByteSize = 0;
            sBuffer* _vertexBuffer = nullptr;
            sBuffer* _indexBuffer = nullptr;
            sBuffer* _opaqueInstanceBuffer = nullptr;
            sBuffer* _transparentInstanceBuffer = nullptr;
            sBuffer* _textInstanceBuffer = nullptr;
            sBuffer* _opaqueMaterialBuffer = nullptr;
            sBuffer* _transparentMaterialBuffer = nullptr;
            sBuffer* _textMaterialBuffer = nullptr;
            sBuffer* _lightBuffer = nullptr;
            sBuffer* _opaqueTextureAtlasTexturesBuffer = nullptr;
            sBuffer* _transparentTextureAtlasTexturesBuffer = nullptr;
            sBuffer* _textTextureAtlasTexturesBuffer = nullptr;
            types::usize _opaqueInstanceCount = 0;
            types::usize _transparentInstanceCount = 0;
            void* _vertices = nullptr;
            types::usize _verticesByteSize = 0;
            void* _indices = nullptr;
            types::usize _indicesByteSize = 0;
            void* _opaqueInstances = nullptr;
            types::usize _opaqueInstancesByteSize = 0;
            void* _transparentInstances = nullptr;
            types::usize _transparentInstancesByteSize = 0;
            void* _textInstances = nullptr;
            types::usize _textInstancesByteSize = 0;
            void* _opaqueMaterials = nullptr;
            types::usize _opaqueMaterialsByteSize = 0;
            void* _transparentMaterials = nullptr;
            types::usize _transparentMaterialsByteSize = 0;
            void* _textMaterials = nullptr;
            types::usize _textMaterialsByteSize = 0;
            void* _lights = nullptr;
            types::usize _lightsByteSize = 0;
            void* _opaqueTextureAtlasTextures = nullptr;
            types::usize _opaqueTextureAtlasTexturesByteSize = 0;
            void* _transparentTextureAtlasTextures = nullptr;
            types::usize _transparentTextureAtlasTexturesByteSize = 0;
            void* _textTextureAtlasTextures = nullptr;
            types::usize _textTextureAtlasTexturesByteSize = 0;
            std::unordered_map<render::cMaterial*, types::s32>* _materialsMap = {};
            sRenderPass* _opaque = nullptr;
            sRenderPass* _transparent = nullptr;
            sRenderPass* _text = nullptr;
            sRenderPass* _compositeTransparent = nullptr;
            sRenderPass* _compositeFinal = nullptr;
            sRenderTarget* _opaqueRenderTarget = nullptr;
            sRenderTarget* _transparentRenderTarget = nullptr;
            types::usize _materialCountCPU = 0;
            cIdVec<cMaterial> _materialsCPU;
        };
    }
}