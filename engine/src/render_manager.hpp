// render_manager.hpp
/*
#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "id_vec.hpp"
#include "category.hpp"
#include "object.hpp"
#include "types.hpp"

namespace arachne
{
    class iRenderContext;
    class cApplication;
    class cGameObject;
    class cTextureAtlasTexture;
    struct sBuffer;
    struct sVertexArray;
    struct sRenderTarget;
    struct sRenderPass;
    struct sShader;

    using index = types::u32;

    struct sVertex
    {
        glm::vec3 _position = glm::vec3(0.0f);
        glm::vec2 _texcoord = glm::vec2(0.0f);
        glm::vec3 _normal = glm::vec3(0.0f);
    };

    struct sVertexBufferGeometry
    {
        types::usize _vertexCount = 0;
        types::usize _indexCount = 0;
        void* _vertexPtr = nullptr;
        void* _indexPtr = nullptr;
        types::usize _offsetVertex = 0;
        types::usize _offsetIndex = 0;
        eCategory _format = eCategory::VERTEX_BUFFER_FORMAT_NONE;
    };

    struct sPrimitive
    {
        sVertex* _vertices = nullptr;
        index* _indices = nullptr;
        types::usize _vertexCount = 0;
        types::usize _indexCount = 0;
        types::usize _verticesByteSize = 0;
        types::usize _indicesByteSize = 0;
        eCategory _format = eCategory::VERTEX_BUFFER_FORMAT_NONE;
    };

    struct sModel : sPrimitive
    {
    };

    struct sLight
    {
        glm::vec3 _color = glm::vec3(0.0f);
        glm::vec3 _direction = glm::vec3(0.0f);
        types::f32 _scale = 0.0f;
        types::f32 _attenuationConstant = 0.0f;
        types::f32 _attenuationLinear = 0.0f;
        types::f32 _attenuationQuadratic = 0.0f;
    };

    struct sTransform
    {
        sTransform() = default;
        sTransform(const cGameObject* gameObject);

        void Transform();

        types::boolean _use2D = types::K_FALSE;
        glm::vec3 _position = glm::vec3(0.0f);
        glm::vec3 _rotation = glm::vec3(0.0f);
        glm::vec3 _scale = glm::vec3(1.0f);
        glm::mat4 _world = glm::mat4(1.0f);
    };

    class cMaterial : public cFactoryObject
    {
        REALWARE_CLASS(cMaterial)

    public:
        explicit cMaterial(cContext* context, cTextureAtlasTexture* diffuseTexture, const glm::vec4& diffuseColor, const glm::vec4& highlightColor, sShader* customShader) : cFactoryObject(context), _diffuseTexture(diffuseTexture), _diffuseColor(diffuseColor), _highlightColor(highlightColor), _customShader(customShader) {}
        ~cMaterial() = default;

        inline sShader* GetCustomShader() const { return _customShader; }
        inline cTextureAtlasTexture* GetDiffuseTexture() const { return _diffuseTexture; }
        inline const glm::vec4& GetDiffuseColor() const { return _diffuseColor; }
        inline const glm::vec4& GetHighlightColor() const { return _highlightColor; }

    private:
        sShader* _customShader = nullptr;
        cTextureAtlasTexture* _diffuseTexture = nullptr;
        glm::vec4 _diffuseColor = glm::vec4(1.0f);
        glm::vec4 _highlightColor = glm::vec4(1.0f);
    };

    struct sRenderInstance
    {
        sRenderInstance(types::s32 materialIndex, const sTransform& transform);

        types::f32 _use2D = 0.0f;
        types::s32 _materialIndex = -1;
        types::dword _pad[2] = {};
        glm::mat4 _world = {};
    };

    struct sTextInstance
    {
        glm::vec4 _info = glm::vec4(0.0f);
        glm::vec4 _atlasInfo = glm::vec4(0.0f);
    };

    class cMaterialInstance
    {
    public:
        cMaterialInstance(types::s32 materialIndex, const cMaterial* material);

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
        sLightInstance(const cGameObject* object);

        glm::vec4 _position = glm::vec4(0.0f);
        glm::vec4 _color = glm::vec4(0.0f);
        glm::vec4 _directionAndScale = glm::vec4(0.0f);
        glm::vec4 _attenuation = glm::vec4(0.0f);
    };

    class mRender : public iObject
    {
        REALWARE_CLASS(mRender)

    public:
        explicit mRender(cContext* context, iRenderContext* renderContext);
        ~mRender();

        cMaterial* CreateMaterial(const std::string& id, cTextureAtlasTexture* diffuseTexture, const glm::vec4& diffuseColor, const glm::vec4& highlightColor, eCategory customShaderRenderPath = eCategory::RENDER_PATH_OPAQUE, const std::string& customVertexFuncPath = "", const std::string& customFragmentFuncPath = "");
        cMaterial* FindMaterial(const std::string& id);
        void DestroyMaterial(const std::string& id);
        sVertexArray* CreateDefaultVertexArray();
        sVertexBufferGeometry* CreateGeometry(eCategory format, types::usize verticesByteSize, const void* vertices, types::usize indicesByteSize, const void* indices);
        void DestroyGeometry(sVertexBufferGeometry* geometry);
            
        void ClearGeometryBuffer();
        void ClearRenderPass(const sRenderPass* renderPass, types::boolean clearColor, types::usize bufferIndex, const glm::vec4& color, types::boolean clearDepth, types::f32 depth);
        void ClearRenderPasses(const glm::vec4& clearColor, types::f32 clearDepth);
            
        void UpdateLights();

        void WriteObjectsToOpaqueBuffers(cIdVector<cGameObject>& objects, sRenderPass* renderPass);
        void WriteObjectsToTransparentBuffers(cIdVector<cGameObject>& objects, sRenderPass* renderPass);
        void DrawGeometryOpaque(const sVertexBufferGeometry* geometry, const cGameObject* cameraObject, sRenderPass* renderPass);
        void DrawGeometryOpaque(const sVertexBufferGeometry* geometry, const cGameObject* cameraObject, sShader* singleShader = nullptr);
        void DrawGeometryTransparent(const sVertexBufferGeometry* geometry, const std::vector<cGameObject>& objects, const cGameObject* cameraObject, sRenderPass* renderPass);
        void DrawGeometryTransparent(const sVertexBufferGeometry* geometry, const cGameObject* cameraObject, sShader* singleShader = nullptr);
        void DrawTexts(const std::vector<cGameObject>& objects);
            
        void CompositeTransparent();
        void CompositeFinal();
            
        sPrimitive* CreatePrimitive(eCategory primitive);
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
        iRenderContext* _renderContext = nullptr;
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
        std::unordered_map<cMaterial*, types::s32>* _materialsMap = {};
        sRenderPass* _opaque = nullptr;
        sRenderPass* _transparent = nullptr;
        sRenderPass* _text = nullptr;
        sRenderPass* _compositeTransparent = nullptr;
        sRenderPass* _compositeFinal = nullptr;
        sRenderTarget* _opaqueRenderTarget = nullptr;
        sRenderTarget* _transparentRenderTarget = nullptr;
        types::usize _materialCountCPU = 0;
        cIdVector<cMaterial> _materialsCPU;
    };
}*/