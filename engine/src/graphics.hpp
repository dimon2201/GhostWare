// graphics.hpp

#pragma once

#include <unordered_map>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "object.hpp"
#include "render_context.hpp"
#include "category.hpp"
#include "id_vec.hpp"
#include "types.hpp"

namespace harpy
{
	class cWindow;
	class iGraphicsAPI;
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

    class cMaterial : public iObject
    {
        HARPY_OBJECT(cMaterial)

    public:
        explicit cMaterial(cContext* context, cTextureAtlasTexture* diffuseTexture, const glm::vec4& diffuseColor, const glm::vec4& highlightColor, cShader* customShader) : iObject(context), _diffuseTexture(diffuseTexture), _diffuseColor(diffuseColor), _highlightColor(highlightColor), _customShader(customShader) {}
        ~cMaterial() = default;

        inline cShader* GetCustomShader() const { return _customShader; }
        inline cTextureAtlasTexture* GetDiffuseTexture() const { return _diffuseTexture; }
        inline const glm::vec4& GetDiffuseColor() const { return _diffuseColor; }
        inline const glm::vec4& GetHighlightColor() const { return _highlightColor; }

    private:
        cShader* _customShader = nullptr;
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

    class cRenderPass : public iObject
    {
        HARPY_OBJECT(cRenderPass)

        friend class cOpenGLGraphicsAPI;

    public:
        explicit cRenderPass(cContext* context, sRenderPassDescriptor* desc, cRenderPassGPU* renderPass);
        virtual ~cRenderPass() override final = default;

        void ResizeViewport(const glm::vec2& size);
        void ResizeColorAttachments(const glm::vec2& size);
        void ResizeDepthAttachment(const glm::vec2& size);

        inline const std::vector<cTextureAtlasTexture*>& GetInputTextureAtlasTextures() const { return _desc->inputTextureAtlasTextures; }
        inline cVertexArray* GetVertexArray() const { return _renderPass->GetVertexArray(); }
        inline cShader* GetShader() const { return _renderPass->GetShader(); }
        inline cRenderTarget* GetRenderTarget() const { return _renderPass->GetRenderTarget(); }
        inline cRenderPassGPU* GetRenderPassGPU() const { return _renderPass; }
        inline void SetInputTexture(types::usize textureIndex, cTexture* texture) { _desc->inputTextures[textureIndex] = texture; }

    private:
        sRenderPassDescriptor* _desc = nullptr;
        cRenderPassGPU* _renderPass = nullptr;
    };

	class cGraphics : public iObject
	{
        HARPY_OBJECT(cGraphics)

	public:
		enum class eAPI
		{
			NONE = 0,
			OGL,
			D3D11
		};

	public:
		explicit cGraphics(cContext* context, eAPI api);
		virtual ~cGraphics() override final;

        cMaterial* CreateMaterial(const std::string& id, cTextureAtlasTexture* diffuseTexture, const glm::vec4& diffuseColor, const glm::vec4& highlightColor, eCategory customShaderRenderPath = eCategory::RENDER_PATH_OPAQUE, const std::string& customVertexFuncPath = "", const std::string& customFragmentFuncPath = "");
        cVertexArray* CreateDefaultVertexArray();
        sVertexBufferGeometry* CreateGeometry(eCategory format, types::usize verticesByteSize, const void* vertices, types::usize indicesByteSize, const void* indices);
        cRenderPass* CreateRenderPass(const sRenderPassDescriptor* desc);
        sPrimitive* CreatePrimitive(eCategory primitive);
        sModel* CreateModel(const std::string& filename);

        cMaterial* FindMaterial(const std::string& id);
        
        void DestroyMaterial(const std::string& id);
        void DestroyVertexArray(cVertexArray* vertexArray);
        void DestroyGeometry(sVertexBufferGeometry* geometry);
        void DestroyRenderPass(cRenderPass* renderPass);
        void DestroyPrimitive(sPrimitive* primitiveObject);
        void DestroyModel(sModel* model);
        
        void ClearGeometryBuffer();
        void ClearRenderPass(const cRenderPass* renderPass, types::boolean clearColor, types::usize bufferIndex, const glm::vec4& color, types::boolean clearDepth, types::f32 depth);
        void ClearRenderPasses(const glm::vec4& clearColor, types::f32 clearDepth);
        void ResizeRenderTargets(const glm::vec2& size);
        void LoadShaderFiles(const std::string& vertexFuncPath, const std::string& fragmentFuncPath, std::string& vertexFunc, std::string& fragmentFunc);
        void UpdateLights();
        
        void WriteObjectsToOpaqueBuffers(cIdVector<cGameObject>& objects, cRenderPass* renderPass);
        void WriteObjectsToTransparentBuffers(cIdVector<cGameObject>& objects, cRenderPass* renderPass);
        
        void DrawGeometryOpaque(const sVertexBufferGeometry* geometry, const cGameObject* cameraObject, cRenderPass* renderPass);
        void DrawGeometryOpaque(const sVertexBufferGeometry* geometry, const cGameObject* cameraObject, cShader* singleShader = nullptr);
        void DrawGeometryTransparent(const sVertexBufferGeometry* geometry, const std::vector<cGameObject>& objects, const cGameObject* cameraObject, cRenderPass* renderPass);
        void DrawGeometryTransparent(const sVertexBufferGeometry* geometry, const cGameObject* cameraObject, cShader* singleShader = nullptr);
        void DrawTexts(const std::vector<cGameObject>& objects);
        
        void CompositeTransparent();
        void CompositeFinal();

        inline iGraphicsAPI* GetAPI() const { return _gfx; }
        inline cBuffer* GetVertexBuffer() const { return _vertexBuffer; }
        inline cBuffer* GetIndexBuffer() const { return _indexBuffer; }
        inline cBuffer* GetOpaqueInstanceBuffer() const { return _opaqueInstanceBuffer; }
        inline cBuffer* GetTextInstanceBuffer() const { return _textInstanceBuffer; }
        inline cBuffer* GetOpaqueMaterialBuffer() const { return _opaqueMaterialBuffer; }
        inline cBuffer* GetTransparentInstanceBuffer() const { return _transparentInstanceBuffer; }
        inline cBuffer* GetTransparentMaterialBuffer() const { return _transparentMaterialBuffer; }
        inline cBuffer* GetTextMaterialBuffer() const { return _textMaterialBuffer; }
        inline cBuffer* GetLightBuffer() const { return _lightBuffer; }
        inline cBuffer* GetOpaqueTextureAtlasTexturesBuffer() const { return _opaqueTextureAtlasTexturesBuffer; }
        inline cBuffer* GetTransparentTextureAtlasTexturesBuffer() const { return _transparentTextureAtlasTexturesBuffer; }
        inline cRenderPass* GetOpaqueRenderPass() const { return _opaque; }
        inline cRenderPass* GetTransparentRenderPass() const { return _transparent; }
        inline cRenderPass* GetTextRenderPass() const { return _text; }
        inline cRenderPass* GetCompositeTransparentRenderPass() const { return _compositeTransparent; }
        inline cRenderPass* GetCompositeFinalRenderPass() const { return _compositeFinal; }
        inline cRenderTarget* GetOpaqueRenderTarget() const { return _opaqueRenderTarget; }
        inline cRenderTarget* GetTransparentRenderTarget() const { return _transparentRenderTarget; }

	private:
		iGraphicsAPI* _gfx = nullptr;
        types::usize _maxOpaqueInstanceBufferByteSize = 0;
        types::usize _maxTransparentInstanceBufferByteSize = 0;
        types::usize _maxTextInstanceBufferByteSize = 0;
        types::usize _maxMaterialBufferByteSize = 0;
        types::usize _maxLightBufferByteSize = 0;
        types::usize _maxTextureAtlasTexturesBufferByteSize = 0;
        cBuffer* _vertexBuffer = nullptr;
        cBuffer* _indexBuffer = nullptr;
        cBuffer* _opaqueInstanceBuffer = nullptr;
        cBuffer* _transparentInstanceBuffer = nullptr;
        cBuffer* _textInstanceBuffer = nullptr;
        cBuffer* _opaqueMaterialBuffer = nullptr;
        cBuffer* _transparentMaterialBuffer = nullptr;
        cBuffer* _textMaterialBuffer = nullptr;
        cBuffer* _lightBuffer = nullptr;
        cBuffer* _opaqueTextureAtlasTexturesBuffer = nullptr;
        cBuffer* _transparentTextureAtlasTexturesBuffer = nullptr;
        cBuffer* _textTextureAtlasTexturesBuffer = nullptr;
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
        cRenderPass* _opaque = nullptr;
        cRenderPass* _transparent = nullptr;
        cRenderPass* _text = nullptr;
        cRenderPass* _compositeTransparent = nullptr;
        cRenderPass* _compositeFinal = nullptr;
        cRenderTarget* _opaqueRenderTarget = nullptr;
        cRenderTarget* _transparentRenderTarget = nullptr;
        types::usize _materialCountCPU = 0;
        cIdVector<cMaterial>* _materialsCPU;
	};
}