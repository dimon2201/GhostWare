// render_context.hpp

#pragma once

#include <vector>
#include <string>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "category.hpp"
#include "object.hpp"
#include "types.hpp"

namespace arachne
{
    class cApplication;
    class cTextureAtlasTexture;

    class cGPUResource : public cObject
    {
        REALWARE_OBJECT(cGPUResource)
        
    protected:
        mutable types::u32 _instance = 0;
        types::u32 _viewInstance = 0;
    };

    class cBuffer : public cGPUResource
    {
        REALWARE_OBJECT(cBuffer)

        friend class cOpenGLGraphicsAPI;

    public:
        enum class eType
        {
            NONE = 0,
            VERTEX = 1,
            INDEX = 2,
            UNIFORM = 3,
            LARGE = 4
        };

        inline eType GetBufferType() const { return _type; }
        inline types::usize GetByteSize() const { return _byteSize; }
        inline types::s32 GetSlot() const { return _slot; }

    private:
        eType _type = eType::NONE;
        types::usize _byteSize = 0;
        types::s32 _slot = -1;
    };

    class cVertexArray : public cGPUResource
    {
        REALWARE_OBJECT(cVertexArray)

        friend class cOpenGLGraphicsAPI;
    };

    class cShader : public cGPUResource
    {
        REALWARE_OBJECT(cShader)

        friend class cOpenGLGraphicsAPI;

    public:
        struct sDefinePair
        {
            sDefinePair(const std::string& name, types::usize index) : _name(name), _index(index) {}
            ~sDefinePair() = default;

            std::string _name = "";
            types::usize _index = 0;
        };

    private:
        std::string _vertex = "";
        std::string _fragment = "";
    };

    class cTexture : public cGPUResource
    {
        REALWARE_OBJECT(cTexture)

        friend class cOpenGLGraphicsAPI;

    public:
        enum class eDimension
        {
            NONE = 0,
            TEXTURE_2D = 1,
            TEXTURE_2D_ARRAY = 2
        };

        enum class eFormat
        {
            NONE = 0,
            R8 = 1,
            R8F = 2,
            RGBA8 = 3,
            RGB16F = 4,
            RGBA16F = 5,
            DEPTH_STENCIL = 6,
            RGBA8_MIPS = 7
        };

        inline types::usize GetWidth() const { return _width; }
        inline types::usize GetHeight() const { return _height; }
        inline types::usize GetDepth() const { return _depth; }
        inline eDimension GetDimension() const { return _dimension; }
        inline eFormat GetFormat() const { return _format; }
        inline types::s32 GetSlot() const { return _slot; }
        inline void SetSlot(types::s32 slot) { _slot = slot; }

    private:
        types::usize _width = 0;
        types::usize _height = 0;
        types::usize _depth = 0;
        eDimension _dimension = eDimension::NONE;
        eFormat _format = eFormat::NONE;
        types::s32 _slot = -1;
    };

    class cRenderTarget : public cGPUResource
    {
        REALWARE_OBJECT(cRenderTarget)

        friend class cOpenGLGraphicsAPI;

    private:
        std::vector<cTexture*> _colorAttachments = {};
        cTexture* _depthAttachment = nullptr;
    };

    class cDepthMode
    {
        REALWARE_OBJECT(cDepthMode)

        friend class cOpenGLGraphicsAPI;

    private:
        types::boolean _useDepthTest = types::K_TRUE;
        types::boolean _useDepthWrite = types::K_TRUE;
    };

    class cBlendMode
    {
        REALWARE_OBJECT(cBlendMode)

        friend class cOpenGLGraphicsAPI;

    public:
        enum class eFactor
        {
            ZERO = 0,
            ONE = 1,
            SRC_COLOR = 2,
            INV_SRC_COLOR = 3,
            SRC_ALPHA = 4,
            INV_SRC_ALPHA = 5
        };

    private:
        types::usize _factorCount = 0;
        eFactor _srcFactors[8] = { eFactor::ZERO };
        eFactor _dstFactors[8] = { eFactor::ZERO };
    };

    class cRenderPass : public cObject
    {
        REALWARE_OBJECT(cRenderPass)

        friend class cOpenGLGraphicsAPI;

    public:
        struct sDescriptor
        {
            cVertexArray* _vertexArray = nullptr;
            eCategory _inputVertexFormat = eCategory::VERTEX_BUFFER_FORMAT_NONE;
            std::vector<cBuffer*> _inputBuffers = {};
            std::vector<cTexture*> _inputTextures = {};
            std::vector<std::string> _inputTextureNames = {};
            std::vector<cTextureAtlasTexture*> _inputTextureAtlasTextures = {};
            std::vector<std::string> _inputTextureAtlasTextureNames = {};
            cShader* _shader = nullptr;
            cShader* _shaderBase = nullptr;
            eCategory _shaderRenderPath = eCategory::RENDER_PATH_OPAQUE;
            std::string _shaderVertexPath = "";
            std::string _shaderFragmentPath = "";
            std::string _shaderVertexFunc = "";
            std::string _shaderFragmentFunc = "";
            cRenderTarget* _renderTarget = nullptr;
            cDepthMode _depthMode = {};
            cBlendMode _blendMode = {};
            glm::vec4 _viewport = glm::vec4(0.0f);
        };

    private:
        sDescriptor _desc;
    };

    class iGraphicsAPI : public cObject
    {
        REALWARE_OBJECT(iGraphicsAPI)

    public:
        explicit iGraphicsAPI(cContext* context) : cObject(context) {}
        virtual ~iGraphicsAPI() = default;

        virtual cBuffer* CreateBuffer(types::usize byteSize, cBuffer::eType type, const void* data) = 0;
        virtual void BindBuffer(const cBuffer* buffer) = 0;
		virtual void BindBufferNotVAO(const cBuffer* buffer) = 0;
        virtual void UnbindBuffer(const cBuffer* buffer) = 0;
        virtual void WriteBuffer(const cBuffer* buffer, types::usize offset, types::usize byteSize, const void* data) = 0;
        virtual void DestroyBuffer(cBuffer* buffer) = 0;
        virtual cVertexArray* CreateVertexArray() = 0;
        virtual void BindVertexArray(const cVertexArray* vertexArray) = 0;
        virtual void BindDefaultVertexArray(const std::vector<cBuffer*>& buffersToBind) = 0;
        virtual void UnbindVertexArray() = 0;
        virtual void DestroyVertexArray(cVertexArray* vertexArray) = 0;
        virtual void BindShader(const cShader* shader) = 0;
        virtual void UnbindShader() = 0;
        virtual cShader* CreateShader(eCategory renderPath, const std::string& vertexPath, const std::string& fragmentPath, const std::vector<cShader::sDefinePair>& definePairs = {}) = 0;
        virtual cShader* CreateShader(const cShader* baseShader, const std::string& vertexFunc, const std::string& fragmentFunc, const std::vector<cShader::sDefinePair>& definePairs = {}) = 0;
        virtual void DefineInShader(cShader* shader, const std::vector<cShader::sDefinePair>& definePairs) = 0;
        virtual void DestroyShader(cShader* shader) = 0;
        virtual void SetShaderUniform(const cShader* shader, const std::string& name, const glm::mat4& matrix) = 0;
        virtual void SetShaderUniform(const cShader* shader, const std::string& name, types::usize count, const types::f32* values) = 0;
        virtual cTexture* CreateTexture(types::usize width, types::usize height, types::usize depth, cTexture::eDimension dimension, cTexture::eFormat format, const void* data) = 0;
        virtual cTexture* ResizeTexture(cTexture* texture, const glm::vec2& size) = 0;
        virtual void BindTexture(const cShader* shader, const std::string& name, const cTexture* texture, types::s32 slot) = 0;
        virtual void UnbindTexture(const cTexture* texture) = 0;
        virtual void WriteTexture(const cTexture* texture, const glm::vec3& offset, const glm::vec2& size, const void* data) = 0;
        virtual void WriteTextureToFile(const cTexture* texture, const std::string& filename) = 0;
        virtual void GenerateTextureMips(const cTexture* texture) = 0;
        virtual void DestroyTexture(cTexture* texture) = 0;
        virtual cRenderTarget* CreateRenderTarget(const std::vector<cTexture*>& colorAttachments, cTexture* depthAttachment) = 0;
        virtual void ResizeRenderTargetColors(cRenderTarget* renderTarget, const glm::vec2& size) = 0;
        virtual void ResizeRenderTargetDepth(cRenderTarget* renderTarget, const glm::vec2& size) = 0;
        virtual void UpdateRenderTargetBuffers(cRenderTarget* renderTarget) = 0;
        virtual void BindRenderTarget(const cRenderTarget* renderTarget) = 0;
        virtual void UnbindRenderTarget() = 0;
        virtual void DestroyRenderTarget(cRenderTarget* renderTarget) = 0;
        virtual cRenderPass* CreateRenderPass(const cRenderPass::sDescriptor& descriptor) = 0;
        virtual void BindRenderPass(const cRenderPass* renderPass, cShader* customShader = nullptr) = 0;
        virtual void UnbindRenderPass(const cRenderPass* renderPass) = 0;
        virtual void DestroyRenderPass(cRenderPass* renderPass) = 0;
        virtual void BindDefaultInputLayout() = 0;
        virtual void BindDepthMode(const cDepthMode& blendMode) = 0;
        virtual void BindBlendMode(const cBlendMode& blendMode) = 0;
        virtual void Viewport(const glm::vec4& viewport) = 0;
        virtual void ClearColor(const glm::vec4& color) = 0;
        virtual void ClearDepth(types::f32 depth) = 0;
        virtual void ClearFramebufferColor(types::usize bufferIndex, const glm::vec4& color) = 0;
        virtual void ClearFramebufferDepth(types::f32 depth) = 0;
        virtual void Draw(types::usize indexCount, types::usize vertexOffset, types::usize indexOffset, types::usize instanceCount) = 0;
        virtual void DrawQuad() = 0;
        virtual void DrawQuads(types::usize count) = 0;
    };

    class cOpenGLGraphicsAPI : public iGraphicsAPI
    {
        REALWARE_OBJECT(cOpenGLGraphicsAPI)

    public:
        explicit cOpenGLGraphicsAPI(cContext* context);
        virtual ~cOpenGLGraphicsAPI() override final;

        virtual cBuffer* CreateBuffer(types::usize byteSize, cBuffer::eType type, const void* data) override final;
        virtual void BindBuffer(const cBuffer* buffer) override final;
        virtual void BindBufferNotVAO(const cBuffer* buffer) override final;
        virtual void UnbindBuffer(const cBuffer* buffer) override final;
        virtual void WriteBuffer(const cBuffer* buffer, types::usize offset, types::usize byteSize, const void* data) override final;
        virtual void DestroyBuffer(cBuffer* buffer) override final;
        virtual cVertexArray* CreateVertexArray() override final;
        virtual void BindVertexArray(const cVertexArray* vertexArray) override final;
        virtual void BindDefaultVertexArray(const std::vector<cBuffer*>& buffersToBind) override final;
        virtual void UnbindVertexArray() override final;
        virtual void DestroyVertexArray(cVertexArray* vertexArray) override final;
        virtual void BindShader(const cShader* shader) override final;
        virtual void UnbindShader() override final;
        virtual cShader* CreateShader(eCategory renderPath, const std::string& vertexPath, const std::string& fragmentPath, const std::vector<cShader::sDefinePair>& definePairs = {}) override final;
        virtual cShader* CreateShader(const cShader* baseShader, const std::string& vertexFunc, const std::string& fragmentFunc, const std::vector<cShader::sDefinePair>& definePairs = {}) override final;
        virtual void DefineInShader(cShader* shader, const std::vector<cShader::sDefinePair>& definePairs) override final;
        virtual void DestroyShader(cShader* shader) override final;
        virtual void SetShaderUniform(const cShader* shader, const std::string& name, const glm::mat4& matrix) override final;
        virtual void SetShaderUniform(const cShader* shader, const std::string& name, types::usize count, const types::f32* values) override final;
        virtual cTexture* CreateTexture(types::usize width, types::usize height, types::usize depth, cTexture::eDimension dimension, cTexture::eFormat format, const void* data) override final;
        virtual cTexture* ResizeTexture(cTexture* texture, const glm::vec2& size) override final;
        virtual void BindTexture(const cShader* shader, const std::string& name, const cTexture* texture, types::s32 slot) override final;
        virtual void UnbindTexture(const cTexture* texture) override final;
        virtual void WriteTexture(const cTexture* texture, const glm::vec3& offset, const glm::vec2& size, const void* data) override final;
        virtual void WriteTextureToFile(const cTexture* texture, const std::string& filename) override final;
        virtual void GenerateTextureMips(const cTexture* texture) override final;
        virtual void DestroyTexture(cTexture* texture) override final;
        virtual cRenderTarget* CreateRenderTarget(const std::vector<cTexture*>& colorAttachments, cTexture* depthAttachment) override final;
        virtual void ResizeRenderTargetColors(cRenderTarget* renderTarget, const glm::vec2& size) override final;
        virtual void ResizeRenderTargetDepth(cRenderTarget* renderTarget, const glm::vec2& size) override final;
        virtual void UpdateRenderTargetBuffers(cRenderTarget* renderTarget) override final;
        virtual void BindRenderTarget(const cRenderTarget* renderTarget) override final;
        virtual void UnbindRenderTarget() override final;
        virtual void DestroyRenderTarget(cRenderTarget* renderTarget) override final;
        virtual cRenderPass* CreateRenderPass(const cRenderPass::sDescriptor& descriptor) override final;
        virtual void BindRenderPass(const cRenderPass* renderPass, cShader* customShader = nullptr) override final;
        virtual void UnbindRenderPass(const cRenderPass* renderPass) override final;
        virtual void DestroyRenderPass(cRenderPass* renderPass) override final;
        virtual void BindDefaultInputLayout() override final;
        virtual void BindDepthMode(const cDepthMode& blendMode) override final;
        virtual void BindBlendMode(const cBlendMode& blendMode) override final;
        virtual void Viewport(const glm::vec4& viewport) override final;
        virtual void ClearColor(const glm::vec4& color) override final;
        virtual void ClearDepth(types::f32 depth) override final;
        virtual void ClearFramebufferColor(types::usize bufferIndex, const glm::vec4& color) override final;
        virtual void ClearFramebufferDepth(types::f32 depth) override final;
        virtual void Draw(types::usize indexCount, types::usize vertexOffset, types::usize indexOffset, types::usize instanceCount) override final;
        virtual void DrawQuad() override final;
        virtual void DrawQuads(types::usize count) override final;
    };
}