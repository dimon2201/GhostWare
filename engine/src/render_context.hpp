// render_context.hpp

#pragma once

#include <vector>
#include <string>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "category.hpp"
#include "types.hpp"

namespace realware
{
    namespace app
    {
        class cApplication;
    }

    namespace render
    {
        class cTextureAtlasTexture;

        struct sGPUResource
        {
            types::u32 Instance = 0;
            types::u32 ViewInstance = 0;
        };

        struct sBuffer : public sGPUResource
        {
            enum class eType
            {
                NONE = 0,
                VERTEX = 1,
                INDEX = 2,
                UNIFORM = 3,
                LARGE = 4
            };

            eType Type = eType::NONE;
            types::usize ByteSize = 0;
            types::s32 Slot = -1;
        };

        struct sVertexArray : public sGPUResource
        {
        };

        struct sShader : public sGPUResource
        {
            struct sDefinePair
            {
                sDefinePair(const std::string& name, const types::usize index) : Name(name), Index(index) {}
                ~sDefinePair() = default;

                std::string Name = "";
                types::usize Index = 0;
            };

            std::string Vertex = "";
            std::string Fragment = "";
        };

        struct sTexture : public sGPUResource
        {
            enum class eType
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

            types::usize Width = 0;
            types::usize Height = 0;
            types::usize Depth = 0;
            eType Type = eType::NONE;
            eFormat Format = eFormat::NONE;
            types::s32 Slot = -1;
        };

        struct sRenderTarget : public sGPUResource
        {
            std::vector<sTexture*> ColorAttachments = {};
            sTexture* DepthAttachment = nullptr;
        };

        struct sDepthMode
        {
            types::boolean UseDepthTest = types::K_TRUE;
            types::boolean UseDepthWrite = types::K_TRUE;
        };

        struct sBlendMode
        {
            enum class eFactor
            {
                ZERO = 0,
                ONE = 1,
                SRC_COLOR = 2,
                INV_SRC_COLOR = 3,
                SRC_ALPHA = 4,
                INV_SRC_ALPHA = 5
            };

            types::usize FactorCount = 0;
            eFactor SrcFactors[8] = { eFactor::ZERO };
            eFactor DstFactors[8] = { eFactor::ZERO };
        };

        struct sRenderPass
        {
            struct sDescriptor
            {
                sVertexArray* VertexArray = nullptr;
                game::Category InputVertexFormat = game::Category::VERTEX_BUFFER_FORMAT_NONE;
                std::vector<sBuffer*> InputBuffers = {};
                std::vector<sTexture*> InputTextures = {};
                std::vector<std::string> InputTextureNames = {};
                std::vector<cTextureAtlasTexture*> InputTextureAtlasTextures = {};
                std::vector<std::string> InputTextureAtlasTextureNames = {};
                sShader* Shader = nullptr;
                sShader* ShaderBase = nullptr;
                game::Category ShaderRenderPath = game::Category::RENDER_PATH_OPAQUE;
                std::string ShaderVertexPath = "";
                std::string ShaderFragmentPath = "";
                std::string ShaderVertexFunc = "";
                std::string ShaderFragmentFunc = "";
                sRenderTarget* RenderTarget = nullptr;
                sDepthMode DepthMode = {};
                sBlendMode BlendMode = {};
                glm::vec4 Viewport = glm::vec4(0.0f);
            };

            sDescriptor Desc;
        };

        class iRenderContext
        {
        public:
            virtual ~iRenderContext() = default;

            virtual sBuffer* CreateBuffer(const types::usize byteSize, const sBuffer::eType& type, const void* const data) = 0;
            virtual void BindBuffer(const sBuffer* const buffer) = 0;
			virtual void BindBufferNotVAO(const sBuffer* const buffer) = 0;
            virtual void UnbindBuffer(const sBuffer* const buffer) = 0;
            virtual void WriteBuffer(const sBuffer* const buffer, const types::usize offset, const types::usize byteSize, const void* const data) = 0;
            virtual void DestroyBuffer(sBuffer* buffer) = 0;
            virtual sVertexArray* CreateVertexArray() = 0;
            virtual void BindVertexArray(const sVertexArray* const vertexArray) = 0;
            virtual void BindDefaultVertexArray(const std::vector<sBuffer*>& buffersToBind) = 0;
            virtual void UnbindVertexArray() = 0;
            virtual void DestroyVertexArray(sVertexArray* vertexArray) = 0;
            virtual void BindShader(const sShader* const shader) = 0;
            virtual void UnbindShader() = 0;
            virtual sShader* CreateShader(const game::Category& renderPath, const std::string& vertexPath, const std::string& fragmentPath, const std::vector<sShader::sDefinePair>& definePairs = {}) = 0;
            virtual sShader* CreateShader(const sShader* const baseShader, const std::string& vertexFunc, const std::string& fragmentFunc, const std::vector<sShader::sDefinePair>& definePairs = {}) = 0;
            virtual void DefineInShader(sShader* const shader, const std::vector<sShader::sDefinePair>& definePairs) = 0;
            virtual void DestroyShader(sShader* shader) = 0;
            virtual void SetShaderUniform(const sShader* const shader, const std::string& name, const glm::mat4& matrix) = 0;
            virtual void SetShaderUniform(const sShader* const shader, const std::string& name, const types::usize count, const types::f32* const values) = 0;
            virtual sTexture* CreateTexture(const types::usize width, const types::usize height, const types::usize depth, const sTexture::eType& type, const sTexture::eFormat& format, const void* const data) = 0;
            virtual sTexture* ResizeTexture(sTexture* const texture, const glm::vec2& size) = 0;
            virtual void BindTexture(const sShader* const shader, const std::string& name, const sTexture* const texture, const types::s32 slot) = 0;
            virtual void UnbindTexture(const sTexture* const texture) = 0;
            virtual void WriteTexture(sTexture* const texture, const glm::vec3& offset, const glm::vec2& size, const void* const data) = 0;
            virtual void WriteTextureToFile(const sTexture* const texture, const std::string& filename) = 0;
            virtual void GenerateTextureMips(sTexture* const texture) = 0;
            virtual void DestroyTexture(sTexture* texture) = 0;
            virtual sRenderTarget* CreateRenderTarget(const std::vector<sTexture*>& colorAttachments, const sTexture* const depthAttachment) = 0;
            virtual void ResizeRenderTargetColors(sRenderTarget* const renderTarget, const glm::vec2& size) = 0;
            virtual void ResizeRenderTargetDepth(sRenderTarget* const renderTarget, const glm::vec2& size) = 0;
            virtual void UpdateRenderTargetBuffers(const sRenderTarget* const renderTarget) = 0;
            virtual void BindRenderTarget(const sRenderTarget* const renderTarget) = 0;
            virtual void UnbindRenderTarget() = 0;
            virtual void DestroyRenderTarget(sRenderTarget* renderTarget) = 0;
            virtual sRenderPass* CreateRenderPass(const sRenderPass::sDescriptor& descriptor) = 0;
            virtual void BindRenderPass(const sRenderPass* const renderPass, const sShader* const customShader = nullptr) = 0;
            virtual void UnbindRenderPass(const sRenderPass* const renderPass) = 0;
            virtual void DestroyRenderPass(sRenderPass* renderPass) = 0;
            virtual void BindDefaultInputLayout() = 0;
            virtual void BindDepthMode(const sDepthMode& blendMode) = 0;
            virtual void BindBlendMode(const sBlendMode& blendMode) = 0;
            virtual void Viewport(const glm::vec4& viewport) = 0;
            virtual void ClearColor(const glm::vec4& color) = 0;
            virtual void ClearDepth(const types::f32 depth) = 0;
            virtual void ClearFramebufferColor(const types::usize bufferIndex, const glm::vec4& color) = 0;
            virtual void ClearFramebufferDepth(const types::f32 depth) = 0;
            virtual void Draw(types::usize indexCount, types::usize vertexOffset, types::usize indexOffset, types::usize instanceCount) = 0;
            virtual void DrawQuad() = 0;
            virtual void DrawQuads(types::usize count) = 0;
        };

        class cOpenGLRenderContext : public iRenderContext
        {
        public:
            cOpenGLRenderContext(const app::cApplication* const app);
            virtual ~cOpenGLRenderContext() override final;

            virtual sBuffer* CreateBuffer(const types::usize byteSize, const sBuffer::eType& type, const void* const data) override final;
            virtual void BindBuffer(const sBuffer* const buffer) override final;
            virtual void BindBufferNotVAO(const sBuffer* const buffer) override final;
            virtual void UnbindBuffer(const sBuffer* const buffer) override final;
            virtual void WriteBuffer(const sBuffer* const buffer, const types::usize offset, const types::usize byteSize, const void* const data) override final;
            virtual void DestroyBuffer(sBuffer* buffer) override final;
            virtual sVertexArray* CreateVertexArray() override final;
            virtual void BindVertexArray(const sVertexArray* const vertexArray) override final;
            virtual void BindDefaultVertexArray(const std::vector<sBuffer*>& buffersToBind) override final;
            virtual void UnbindVertexArray() override final;
            virtual void DestroyVertexArray(sVertexArray* vertexArray) override final;
            virtual void BindShader(const sShader* const shader) override final;
            virtual void UnbindShader() override final;
            virtual sShader* CreateShader(const game::Category& renderPath, const std::string& vertexPath, const std::string& fragmentPath, const std::vector<sShader::sDefinePair>& definePairs = {}) override final;
            virtual sShader* CreateShader(const sShader* const baseShader, const std::string& vertexFunc, const std::string& fragmentFunc, const std::vector<sShader::sDefinePair>& definePairs = {}) override final;
            virtual void DefineInShader(sShader* const shader, const std::vector<sShader::sDefinePair>& definePairs) override final;
            virtual void DestroyShader(sShader* shader) override final;
            virtual void SetShaderUniform(const sShader* const shader, const std::string& name, const glm::mat4& matrix) override final;
            virtual void SetShaderUniform(const sShader* const shader, const std::string& name, const types::usize count, const types::f32* const values) override final;
            virtual sTexture* CreateTexture(const types::usize width, const types::usize height, const types::usize depth, const sTexture::eType& type, const sTexture::eFormat& format, const void* const data) override final;
            virtual sTexture* ResizeTexture(sTexture* const texture, const glm::vec2& size) override final;
            virtual void BindTexture(const sShader* const shader, const std::string& name, const sTexture* const texture, const types::s32 slot) override final;
            virtual void UnbindTexture(const sTexture* const texture) override final;
            virtual void WriteTexture(sTexture* const texture, const glm::vec3& offset, const glm::vec2& size, const void* const data) override final;
            virtual void WriteTextureToFile(const sTexture* const texture, const std::string& filename) override final;
            virtual void GenerateTextureMips(sTexture* const texture) override final;
            virtual void DestroyTexture(sTexture* texture) override final;
            virtual sRenderTarget* CreateRenderTarget(const std::vector<sTexture*>& colorAttachments, const sTexture* const depthAttachment) override final;
            virtual void ResizeRenderTargetColors(sRenderTarget* const renderTarget, const glm::vec2& size) override final;
            virtual void ResizeRenderTargetDepth(sRenderTarget* const renderTarget, const glm::vec2& size) override final;
            virtual void UpdateRenderTargetBuffers(const sRenderTarget* const renderTarget) override final;
            virtual void BindRenderTarget(const sRenderTarget* const renderTarget) override final;
            virtual void UnbindRenderTarget() override final;
            virtual void DestroyRenderTarget(sRenderTarget* renderTarget) override final;
            virtual sRenderPass* CreateRenderPass(const sRenderPass::sDescriptor& descriptor) override final;
            virtual void BindRenderPass(const sRenderPass* const renderPass, const sShader* const customShader = nullptr) override final;
            virtual void UnbindRenderPass(const sRenderPass* const renderPass) override final;
            virtual void DestroyRenderPass(sRenderPass* renderPass) override final;
            virtual void BindDefaultInputLayout() override final;
            virtual void BindDepthMode(const sDepthMode& blendMode) override final;
            virtual void BindBlendMode(const sBlendMode& blendMode) override final;
            virtual void Viewport(const glm::vec4& viewport) override final;
            virtual void ClearColor(const glm::vec4& color) override final;
            virtual void ClearDepth(const types::f32 depth) override final;
            virtual void ClearFramebufferColor(const types::usize bufferIndex, const glm::vec4& color) override final;
            virtual void ClearFramebufferDepth(const types::f32 depth) override final;
            virtual void Draw(types::usize indexCount, types::usize vertexOffset, types::usize indexOffset, types::usize instanceCount) override final;
            virtual void DrawQuad() override final;
            virtual void DrawQuads(types::usize count) override final;

        private:
            app::cApplication* _app = nullptr;
        };
    }
}