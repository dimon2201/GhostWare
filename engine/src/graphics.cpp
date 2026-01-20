// graphics.cpp

#include <GL/glew.h>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include "../../thirdparty/glm/glm/gtc/quaternion.hpp"
#include "../../thirdparty/glm/glm/gtx/quaternion.hpp"
#include "context.hpp"
#include "graphics.hpp"
#include "render_manager.hpp"
#include "render_context.hpp"
#include "texture_manager.hpp"
#include "gameobject_manager.hpp"
#include "font_manager.hpp"
#include "filesystem_manager.hpp"
#include "application.hpp"
#include "memory_pool.hpp"
#include "log.hpp"
#include "graphics.hpp"
#include "render_context.hpp"
#include "log.hpp"

using namespace types;

namespace harpy
{
    sTransform::sTransform(const cGameObject* gameObject)
    {
        const sTransform* transform = gameObject->GetTransform();

        _use2D = gameObject->GetIs2D();
        _position = transform->_position;
        _rotation = transform->_rotation;
        _scale = transform->_scale;
    }

    void sTransform::Transform()
    {
        const glm::quat quatX = glm::angleAxis(_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        const glm::quat quatY = glm::angleAxis(_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::quat quatZ = glm::angleAxis(_rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

        _world = glm::translate(glm::mat4(1.0f), _position) * glm::toMat4(quatZ * quatY * quatX) * glm::scale(glm::mat4(1.0f), _scale);
    }

    sRenderInstance::sRenderInstance(s32 materialIndex, const sTransform& transform)
    {
        _use2D = transform._use2D;
        _materialIndex = materialIndex;
        _world = transform._world;
    }

    cMaterialInstance::cMaterialInstance(s32 materialIndex, const cMaterial* material)
    {
        _bufferIndex = materialIndex;
        _diffuseColor = material->GetDiffuseColor();
        _highlightColor = material->GetHighlightColor();

        const cTextureAtlasTexture* diffuse = material->GetDiffuseTexture();
        if (diffuse)
        {
            _diffuseTextureLayerInfo = diffuse->GetOffset().z;
            _diffuseTextureInfo = glm::vec4(diffuse->GetOffset().x, diffuse->GetOffset().y, diffuse->GetSize().x, diffuse->GetSize().y);
        }
        else
        {
            _diffuseTextureLayerInfo = -1.0f;
        }
    }

    sLightInstance::sLightInstance(const cGameObject* object)
    {
        const sLight* light = object->GetLight();
        _position = glm::vec4(object->GetTransform()->_position, 0.0f);
        _color = glm::vec4(light->_color, 0.0f);
        _directionAndScale = glm::vec4(light->_direction, light->_scale);
        _attenuation = glm::vec4(
            light->_attenuationConstant,
            light->_attenuationLinear,
            light->_attenuationQuadratic,
            0.0f
        );
    }

    cRenderPass::cRenderPass(cContext* context, sRenderPassDescriptor* desc, cRenderPassGPU* renderPass)
        : iObject(context), _desc(desc), _renderPass(renderPass) {}

    void cRenderPass::ResizeViewport(const glm::vec2& size)
    {
        _desc->viewport[2] = size.x;
        _desc->viewport[3] = size.y;
    }

    void cRenderPass::ResizeColorAttachments(const glm::vec2& size)
    {
        iGraphicsAPI* gfx = _context->GetSubsystem<cGraphics>()->GetAPI();
        gfx->ResizeRenderTargetColors(_renderPass->GetRenderTarget(), size);
    }

    void cRenderPass::ResizeDepthAttachment(const glm::vec2& size)
    {
        iGraphicsAPI* gfx = _context->GetSubsystem<cGraphics>()->GetAPI();
        cRenderTarget* renderTarget = _renderPass->GetRenderTarget();
        renderTarget->SetDepthAttachment(gfx->ResizeTexture(renderTarget->GetDepthAttachment(), size));
    }
    
    cGraphics::cGraphics(cContext* context, eAPI api) : iObject(context)
    {
        if (api == eAPI::NONE)
        {
            Print("Error: graphics API not selected!");

            return;
        }
        else if (api == eAPI::OGL)
        {
            _gfx = new cOpenGLGraphicsAPI(_context);
        }
        else if (api == eAPI::D3D11)
        {
        }

        cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
        iGraphicsAPI* gfx = _context->GetSubsystem<cGraphics>()->GetAPI();
        iApplication* app = _context->GetSubsystem<cEngine>()->GetApplication();
        const sApplicationCapabilities* caps = app->GetCapabilities();
        const glm::vec2 windowSize = app->GetWindow()->GetSize();

        _materialsCPU = _context->Create<cIdVector<cMaterial>>(_context, caps->maxRenderMaterialCount);
        _maxOpaqueInstanceBufferByteSize = caps->maxRenderOpaqueInstanceCount * sizeof(sRenderInstance);
        _maxTransparentInstanceBufferByteSize = caps->maxRenderTransparentInstanceCount * sizeof(sRenderInstance);
        _maxTextInstanceBufferByteSize = caps->maxRenderTextInstanceCount * sizeof(sRenderInstance);
        _maxMaterialBufferByteSize = caps->maxRenderMaterialCount * sizeof(cMaterialInstance);
        _maxLightBufferByteSize = caps->maxRenderLightCount * sizeof(sLightInstance);
        _maxTextureAtlasTexturesBufferByteSize = caps->maxRenderTextureAtlasTextureCount * sizeof(sTextureAtlasTextureGPU);

        _vertexBuffer = gfx->CreateBuffer(caps->vertexBufferSize, cBuffer::eType::VERTEX, 0, nullptr);
        _indexBuffer = gfx->CreateBuffer(caps->indexBufferSize, cBuffer::eType::INDEX, 0, nullptr);
        _opaqueInstanceBuffer = gfx->CreateBuffer(_maxOpaqueInstanceBufferByteSize, cBuffer::eType::LARGE, 0, nullptr);
        _transparentInstanceBuffer = gfx->CreateBuffer(_maxTransparentInstanceBufferByteSize, cBuffer::eType::LARGE, 0, nullptr);
        _textInstanceBuffer = gfx->CreateBuffer(_maxTextInstanceBufferByteSize, cBuffer::eType::LARGE, 0, nullptr);
        _opaqueMaterialBuffer = gfx->CreateBuffer(_maxMaterialBufferByteSize, cBuffer::eType::LARGE, 1, nullptr);
        _textMaterialBuffer = gfx->CreateBuffer(_maxMaterialBufferByteSize, cBuffer::eType::LARGE, 1, nullptr);
        _transparentMaterialBuffer = gfx->CreateBuffer(_maxMaterialBufferByteSize, cBuffer::eType::LARGE, 1, nullptr);
        _lightBuffer = gfx->CreateBuffer(_maxLightBufferByteSize, cBuffer::eType::LARGE, 2, nullptr);
        _opaqueTextureAtlasTexturesBuffer = gfx->CreateBuffer(_maxTextureAtlasTexturesBufferByteSize, cBuffer::eType::LARGE, 3, nullptr);
        _transparentTextureAtlasTexturesBuffer = gfx->CreateBuffer(_maxTextureAtlasTexturesBufferByteSize, cBuffer::eType::LARGE, 3, nullptr);

        _vertices = memoryAllocator->Allocate(caps->vertexBufferSize, caps->memoryAlignment);
        _verticesByteSize = 0;
        _indices = memoryAllocator->Allocate(caps->indexBufferSize, caps->memoryAlignment);
        _indicesByteSize = 0;
        _opaqueInstances = memoryAllocator->Allocate(_maxOpaqueInstanceBufferByteSize, caps->memoryAlignment);
        _opaqueInstancesByteSize = 0;
        _transparentInstances = memoryAllocator->Allocate(_maxTransparentInstanceBufferByteSize, caps->memoryAlignment);
        _transparentInstancesByteSize = 0;
        _textInstances = memoryAllocator->Allocate(_maxTextInstanceBufferByteSize, caps->memoryAlignment);
        _textInstancesByteSize = 0;
        _opaqueMaterials = memoryAllocator->Allocate(_maxMaterialBufferByteSize, caps->memoryAlignment);
        _opaqueMaterialsByteSize = 0;
        _transparentMaterials = memoryAllocator->Allocate(_maxMaterialBufferByteSize, caps->memoryAlignment);
        _transparentMaterialsByteSize = 0;
        _textMaterials = memoryAllocator->Allocate(_maxMaterialBufferByteSize, caps->memoryAlignment);
        _textMaterialsByteSize = 0;
        _lights = memoryAllocator->Allocate(_maxLightBufferByteSize, caps->memoryAlignment);
        _lightsByteSize = 0;
        _opaqueTextureAtlasTextures = memoryAllocator->Allocate(_maxTextureAtlasTexturesBufferByteSize, caps->memoryAlignment);
        _opaqueTextureAtlasTexturesByteSize = 0;
        _transparentTextureAtlasTextures = memoryAllocator->Allocate(_maxTextureAtlasTexturesBufferByteSize, caps->memoryAlignment);
        _transparentTextureAtlasTexturesByteSize = 0;
        _materialsMap = _context->Create<std::unordered_map<cMaterial*, s32>>();

        cTexture* color = gfx->CreateTexture(windowSize.x, windowSize.y, 0, cTexture::eDimension::TEXTURE_2D, cTexture::eFormat::RGBA8, nullptr);
        cTexture* accumulation = gfx->CreateTexture(windowSize.x, windowSize.y, 0, cTexture::eDimension::TEXTURE_2D, cTexture::eFormat::RGBA16F, nullptr);
        cTexture* revealage = gfx->CreateTexture(windowSize.x, windowSize.y, 0, cTexture::eDimension::TEXTURE_2D, cTexture::eFormat::R8F, nullptr);
        cTexture* depth = gfx->CreateTexture(windowSize.x, windowSize.y, 0, cTexture::eDimension::TEXTURE_2D, cTexture::eFormat::DEPTH_STENCIL, nullptr);

        _opaqueRenderTarget = gfx->CreateRenderTarget({ color }, depth);
        _transparentRenderTarget = gfx->CreateRenderTarget({ accumulation, revealage }, depth);

        cTextureAtlas* textureAtlas = _context->GetSubsystem<cTextureAtlas>();

        sRenderPassDescriptor opaqueRenderPassDesc;
        opaqueRenderPassDesc.inputVertexFormat = eCategory::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3;
        opaqueRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetVertexBuffer());
        opaqueRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetIndexBuffer());
        opaqueRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetOpaqueInstanceBuffer());
        opaqueRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetOpaqueMaterialBuffer());
        opaqueRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetLightBuffer());
        opaqueRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetOpaqueTextureAtlasTexturesBuffer());
        opaqueRenderPassDesc.inputTextures.emplace_back(textureAtlas->GetAtlas());
        opaqueRenderPassDesc.inputTextureNames.emplace_back("TextureAtlas");
        opaqueRenderPassDesc.shaderBase = nullptr;
        opaqueRenderPassDesc.shaderRenderPath = eCategory::RENDER_PATH_OPAQUE;
        opaqueRenderPassDesc.shaderVertexPath = "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/main_vertex.shader";
        opaqueRenderPassDesc.shaderFragmentPath = "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/main_fragment.shader";
        opaqueRenderPassDesc.viewport = glm::vec4(0.0f, 0.0f, windowSize);
        opaqueRenderPassDesc.depthMode.useDepthTest = K_TRUE;
        opaqueRenderPassDesc.depthMode.useDepthWrite = K_TRUE;
        opaqueRenderPassDesc.blendMode.factorCount = 1;
        opaqueRenderPassDesc.blendMode.srcFactors[0] = sBlendMode::eFactor::ONE;
        opaqueRenderPassDesc.blendMode.dstFactors[0] = sBlendMode::eFactor::ZERO;
        _opaque = CreateRenderPass(&opaqueRenderPassDesc);

        sRenderPassDescriptor transparentRenderPassDesc;
        transparentRenderPassDesc.inputVertexFormat = eCategory::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3;
        transparentRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetVertexBuffer());
        transparentRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetIndexBuffer());
        transparentRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetTransparentInstanceBuffer());
        transparentRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetTransparentMaterialBuffer());
        transparentRenderPassDesc.inputTextures.emplace_back(textureAtlas->GetAtlas());
        transparentRenderPassDesc.inputTextureNames.emplace_back("TextureAtlas");
        transparentRenderPassDesc.shaderBase = nullptr;
        transparentRenderPassDesc.shaderRenderPath = eCategory::RENDER_PATH_TRANSPARENT;
        transparentRenderPassDesc.shaderVertexPath = "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/main_vertex.shader";
        transparentRenderPassDesc.shaderFragmentPath = "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/main_fragment.shader";
        transparentRenderPassDesc.viewport = glm::vec4(0.0f, 0.0f, windowSize);
        transparentRenderPassDesc.depthMode.useDepthTest = K_TRUE;
        transparentRenderPassDesc.depthMode.useDepthWrite = K_FALSE;
        transparentRenderPassDesc.blendMode.factorCount = 2;
        transparentRenderPassDesc.blendMode.srcFactors[0] = sBlendMode::eFactor::ONE;
        transparentRenderPassDesc.blendMode.dstFactors[0] = sBlendMode::eFactor::ONE;
        transparentRenderPassDesc.blendMode.srcFactors[1] = sBlendMode::eFactor::ZERO;
        transparentRenderPassDesc.blendMode.dstFactors[1] = sBlendMode::eFactor::INV_SRC_COLOR;
        _transparent = CreateRenderPass(&transparentRenderPassDesc);
        
        sRenderPassDescriptor textRenderPassDesc;
        textRenderPassDesc.inputVertexFormat = eCategory::VERTEX_BUFFER_FORMAT_NONE;
        textRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetTextInstanceBuffer());
        textRenderPassDesc.inputBuffers.emplace_back(cGraphics::GetTextMaterialBuffer());
        textRenderPassDesc.shaderBase = nullptr;
        textRenderPassDesc.shaderRenderPath = eCategory::RENDER_PATH_TEXT;
        textRenderPassDesc.shaderVertexPath = "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/main_vertex.shader";
        textRenderPassDesc.shaderFragmentPath = "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/main_fragment.shader";
        textRenderPassDesc.viewport = glm::vec4(0.0f, 0.0f, windowSize);
        textRenderPassDesc.depthMode.useDepthTest = K_FALSE;
        textRenderPassDesc.depthMode.useDepthWrite = K_FALSE;
        _text = CreateRenderPass(&textRenderPassDesc);
        
        sRenderPassDescriptor compositeTransparentRenderPassDesc;
        auto& transparentColorAttachments = _transparentRenderTarget->GetColorAttachments();
        compositeTransparentRenderPassDesc.inputVertexFormat = eCategory::VERTEX_BUFFER_FORMAT_NONE;
        compositeTransparentRenderPassDesc.inputTextures.emplace_back(transparentColorAttachments[0]);
        compositeTransparentRenderPassDesc.inputTextureNames.emplace_back("AccumulationTexture");
        compositeTransparentRenderPassDesc.inputTextures.emplace_back(transparentColorAttachments[1]);
        compositeTransparentRenderPassDesc.inputTextureNames.emplace_back("RevealageTexture");
        compositeTransparentRenderPassDesc.shaderBase = nullptr;
        compositeTransparentRenderPassDesc.shaderRenderPath = eCategory::RENDER_PATH_TRANSPARENT_COMPOSITE;
        compositeTransparentRenderPassDesc.shaderVertexPath = "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/main_vertex.shader";
        compositeTransparentRenderPassDesc.shaderFragmentPath = "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/main_fragment.shader";
        compositeTransparentRenderPassDesc.viewport = glm::vec4(0.0f, 0.0f, windowSize);
        compositeTransparentRenderPassDesc.depthMode.useDepthTest = K_FALSE;
        compositeTransparentRenderPassDesc.depthMode.useDepthWrite = K_FALSE;
        compositeTransparentRenderPassDesc.blendMode.factorCount = 1;
        compositeTransparentRenderPassDesc.blendMode.srcFactors[0] = sBlendMode::eFactor::SRC_ALPHA;
        compositeTransparentRenderPassDesc.blendMode.dstFactors[0] = sBlendMode::eFactor::INV_SRC_ALPHA;
        _compositeTransparent = CreateRenderPass(&compositeTransparentRenderPassDesc);
        
        sRenderPassDescriptor compositeFinalRenderPassDesc;
        auto& opaqueColorAttachments = _opaqueRenderTarget->GetColorAttachments();
        compositeFinalRenderPassDesc.inputVertexFormat = eCategory::VERTEX_BUFFER_FORMAT_NONE;
        compositeFinalRenderPassDesc.inputTextures.emplace_back(opaqueColorAttachments[0]);
        compositeFinalRenderPassDesc.inputTextureNames.emplace_back("ColorTexture");
        compositeFinalRenderPassDesc.shaderBase = nullptr;
        compositeFinalRenderPassDesc.shaderRenderPath = eCategory::RENDER_PATH_QUAD;
        compositeFinalRenderPassDesc.shaderVertexPath = "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/main_vertex.shader";
        compositeFinalRenderPassDesc.shaderFragmentPath = "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/main_fragment.shader";
        compositeFinalRenderPassDesc.viewport = glm::vec4(0.0f, 0.0f, windowSize);
        compositeFinalRenderPassDesc.depthMode.useDepthTest = K_FALSE;
        compositeFinalRenderPassDesc.depthMode.useDepthWrite = K_FALSE;
        compositeFinalRenderPassDesc.blendMode.factorCount = 1;
        compositeFinalRenderPassDesc.blendMode.srcFactors[0] = sBlendMode::eFactor::ONE;
        compositeFinalRenderPassDesc.blendMode.dstFactors[0] = sBlendMode::eFactor::ZERO;
        _compositeFinal = CreateRenderPass(&compositeFinalRenderPassDesc);
    }

    cGraphics::~cGraphics()
    {
        cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
        iGraphicsAPI* gfx = _context->GetSubsystem<cGraphics>()->GetAPI();

        DestroyRenderPass(_compositeFinal);
        DestroyRenderPass(_compositeTransparent);
        DestroyRenderPass(_text);
        DestroyRenderPass(_transparent);
        DestroyRenderPass(_opaque);

        gfx->DestroyRenderTarget(_transparentRenderTarget);
        gfx->DestroyRenderTarget(_opaqueRenderTarget);

        gfx->DestroyTexture(_transparentRenderTarget->GetColorAttachments()[0]);
        gfx->DestroyTexture(_transparentRenderTarget->GetColorAttachments()[1]);
        gfx->DestroyTexture(_opaqueRenderTarget->GetColorAttachments()[0]);
        gfx->DestroyTexture(_opaqueRenderTarget->GetDepthAttachment());

        _context->Destroy<std::unordered_map<cMaterial*, s32>>(_materialsMap);

        memoryAllocator->Deallocate(_transparentTextureAtlasTextures);
        memoryAllocator->Deallocate(_opaqueTextureAtlasTextures);
        memoryAllocator->Deallocate(_lights);
        memoryAllocator->Deallocate(_textMaterials);
        memoryAllocator->Deallocate(_transparentMaterials);
        memoryAllocator->Deallocate(_opaqueMaterials);
        memoryAllocator->Deallocate(_textInstances);
        memoryAllocator->Deallocate(_transparentInstances);
        memoryAllocator->Deallocate(_opaqueInstances);
        memoryAllocator->Deallocate(_indices);
        memoryAllocator->Deallocate(_vertices);

        gfx->DestroyBuffer(_transparentTextureAtlasTexturesBuffer);
        gfx->DestroyBuffer(_opaqueTextureAtlasTexturesBuffer);
        gfx->DestroyBuffer(_lightBuffer);
        gfx->DestroyBuffer(_transparentMaterialBuffer);
        gfx->DestroyBuffer(_textMaterialBuffer);
        gfx->DestroyBuffer(_opaqueMaterialBuffer);
        gfx->DestroyBuffer(_textInstanceBuffer);
        gfx->DestroyBuffer(_transparentInstanceBuffer);
        gfx->DestroyBuffer(_opaqueInstanceBuffer);
        gfx->DestroyBuffer(_indexBuffer);
        gfx->DestroyBuffer(_vertexBuffer);

        _context->Destroy<cIdVector<cMaterial>>(_materialsCPU);
    }

    cMaterial* cGraphics::CreateMaterial(const std::string& id, cTextureAtlasTexture* diffuseTexture, const glm::vec4& diffuseColor, const glm::vec4& highlightColor, eCategory customShaderRenderPath, const std::string& customVertexFuncPath, const std::string& customFragmentFuncPath)
    {
        cShader* customShader = nullptr;
        if (customVertexFuncPath != "" || customFragmentFuncPath != "")
        {
            std::string vertexFunc = "";
            std::string fragmentFunc = "";
            LoadShaderFiles(customVertexFuncPath, customFragmentFuncPath, vertexFunc, fragmentFunc);

            if (customShaderRenderPath == eCategory::RENDER_PATH_OPAQUE)
                customShader = _gfx->CreateShader(_opaque->GetShader(), vertexFunc, fragmentFunc);
            else if (customShaderRenderPath == eCategory::RENDER_PATH_TRANSPARENT)
                customShader = _gfx->CreateShader(_transparent->GetShader(), vertexFunc, fragmentFunc);
        }

        return _materialsCPU->Add(id, diffuseTexture, diffuseColor, highlightColor, customShader);
    }

    cVertexArray* cGraphics::CreateDefaultVertexArray()
    {
        cVertexArray* vertexArray = _gfx->CreateVertexArray();
        std::vector<cBuffer*> buffersToBind = { _vertexBuffer, _indexBuffer };

        _gfx->BindVertexArray(vertexArray);
        for (auto buffer : buffersToBind)
            _gfx->BindBuffer(buffer);
        _gfx->BindDefaultInputLayout();
        _gfx->UnbindVertexArray();

        return vertexArray;
    }

    sVertexBufferGeometry* cGraphics::CreateGeometry(eCategory format, usize verticesByteSize, const void* vertices, usize indicesByteSize, const void* indices)
    {
        sVertexBufferGeometry* geometry = _context->Create<sVertexBufferGeometry>();

        memcpy((void*)((usize)_vertices + _verticesByteSize), vertices, verticesByteSize);
        memcpy((void*)((usize)_indices + _indicesByteSize), indices, indicesByteSize);

        _gfx->WriteBuffer(_vertexBuffer, _verticesByteSize, verticesByteSize, vertices);
        _gfx->WriteBuffer(_indexBuffer, _indicesByteSize, indicesByteSize, indices);

        usize vertexCount = verticesByteSize;
        usize vertexOffset = _verticesByteSize;
        switch (format)
        {
        case eCategory::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3:
            vertexCount /= 32;
            vertexOffset /= 32;
            break;

        default:
            Print("Error: unsupported vertex buffer format!");
            return nullptr;
        }

        geometry->_vertexCount = vertexCount;
        geometry->_indexCount = indicesByteSize / sizeof(u32);
        geometry->_vertexPtr = _vertices;
        geometry->_indexPtr = _indices;
        geometry->_offsetVertex = vertexOffset;
        geometry->_offsetIndex = _indicesByteSize;
        geometry->_format = format;

        _verticesByteSize += verticesByteSize;
        _indicesByteSize += indicesByteSize;

        return geometry;
    }

    cRenderPass* cGraphics::CreateRenderPass(const sRenderPassDescriptor* desc)
    {
        cRenderPassGPU* renderPass = _gfx->CreateRenderPass(desc);

        return _context->Create<cRenderPass>(_context, desc, renderPass);
    }

    sPrimitive* cGraphics::CreatePrimitive(eCategory primitive)
    {
        cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
        const sApplicationCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();

        sPrimitive* primitiveObject = (sPrimitive*)_context->Create<sPrimitive>();

        if (primitive == eCategory::PRIMITIVE_TRIANGLE)
        {
            primitiveObject->_format = eCategory::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3;
            primitiveObject->_vertices = (sVertex*)memoryAllocator->Allocate(sizeof(sVertex) * 3, caps->memoryAlignment);
            primitiveObject->_indices = (index*)memoryAllocator->Allocate(sizeof(index) * 3, caps->memoryAlignment);
            primitiveObject->_vertexCount = 3;
            primitiveObject->_indexCount = 3;
            primitiveObject->_verticesByteSize = sizeof(sVertex) * 3;
            primitiveObject->_indicesByteSize = sizeof(index) * 3;
            primitiveObject->_vertices[0]._position[0] = -1.0f; primitiveObject->_vertices[0]._position[1] = -1.0f; primitiveObject->_vertices[0]._position[2] = 0.0f;
            primitiveObject->_vertices[0]._texcoord[0] = 0.0f; primitiveObject->_vertices[0]._texcoord[1] = 0.0f;
            primitiveObject->_vertices[0]._normal[0] = 0.0f; primitiveObject->_vertices[0]._normal[1] = 0.0f; primitiveObject->_vertices[0]._normal[2] = 1.0f;
            primitiveObject->_vertices[1]._position[0] = 0.0f; primitiveObject->_vertices[1]._position[1] = 1.0f; primitiveObject->_vertices[1]._position[2] = 0.0f;
            primitiveObject->_vertices[1]._texcoord[0] = 0.5f; primitiveObject->_vertices[1]._texcoord[1] = 1.0f;
            primitiveObject->_vertices[1]._normal[0] = 0.0f; primitiveObject->_vertices[1]._normal[1] = 0.0f; primitiveObject->_vertices[1]._normal[2] = 1.0f;
            primitiveObject->_vertices[2]._position[0] = 1.0f; primitiveObject->_vertices[2]._position[1] = -1.0f; primitiveObject->_vertices[2]._position[2] = 0.0f;
            primitiveObject->_vertices[2]._texcoord[0] = 1.0f; primitiveObject->_vertices[2]._texcoord[1] = 0.0f;
            primitiveObject->_vertices[2]._normal[0] = 0.0f; primitiveObject->_vertices[2]._normal[1] = 0.0f; primitiveObject->_vertices[2]._normal[2] = 1.0f;
            primitiveObject->_indices[0] = 0;
            primitiveObject->_indices[1] = 1;
            primitiveObject->_indices[2] = 2;
        }
        else if (primitive == eCategory::PRIMITIVE_QUAD)
        {
            primitiveObject->_format = eCategory::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3;
            primitiveObject->_vertices = (sVertex*)memoryAllocator->Allocate(sizeof(sVertex) * 4, caps->memoryAlignment);
            primitiveObject->_indices = (index*)memoryAllocator->Allocate(sizeof(index) * 6, caps->memoryAlignment);
            primitiveObject->_vertexCount = 4;
            primitiveObject->_indexCount = 6;
            primitiveObject->_verticesByteSize = sizeof(sVertex) * 4;
            primitiveObject->_indicesByteSize = sizeof(index) * 6;

            primitiveObject->_vertices[0]._position[0] = -1.0f; primitiveObject->_vertices[0]._position[1] = -1.0f; primitiveObject->_vertices[0]._position[2] = 0.0f;
            primitiveObject->_vertices[0]._texcoord[0] = 0.0f; primitiveObject->_vertices[0]._texcoord[1] = 0.0f;
            primitiveObject->_vertices[0]._normal[0] = 0.0f; primitiveObject->_vertices[0]._normal[1] = 0.0f; primitiveObject->_vertices[0]._normal[2] = 1.0f;
            primitiveObject->_vertices[1]._position[0] = -1.0f; primitiveObject->_vertices[1]._position[1] = 1.0f; primitiveObject->_vertices[1]._position[2] = 0.0f;
            primitiveObject->_vertices[1]._texcoord[0] = 0.0f; primitiveObject->_vertices[1]._texcoord[1] = 1.0f;
            primitiveObject->_vertices[1]._normal[0] = 0.0f; primitiveObject->_vertices[1]._normal[1] = 0.0f; primitiveObject->_vertices[1]._normal[2] = 1.0f;
            primitiveObject->_vertices[2]._position[0] = 1.0f; primitiveObject->_vertices[2]._position[1] = -1.0f; primitiveObject->_vertices[2]._position[2] = 0.0f;
            primitiveObject->_vertices[2]._texcoord[0] = 1.0f; primitiveObject->_vertices[2]._texcoord[1] = 0.0f;
            primitiveObject->_vertices[2]._normal[0] = 0.0f; primitiveObject->_vertices[2]._normal[1] = 0.0f; primitiveObject->_vertices[2]._normal[2] = 1.0f;
            primitiveObject->_vertices[3]._position[0] = 1.0f; primitiveObject->_vertices[3]._position[1] = 1.0f; primitiveObject->_vertices[3]._position[2] = 0.0f;
            primitiveObject->_vertices[3]._texcoord[0] = 1.0f; primitiveObject->_vertices[3]._texcoord[1] = 1.0f;
            primitiveObject->_vertices[3]._normal[0] = 0.0f; primitiveObject->_vertices[3]._normal[1] = 0.0f; primitiveObject->_vertices[3]._normal[2] = 1.0f;
            primitiveObject->_indices[0] = 0;
            primitiveObject->_indices[1] = 1;
            primitiveObject->_indices[2] = 2;
            primitiveObject->_indices[3] = 1;
            primitiveObject->_indices[4] = 3;
            primitiveObject->_indices[5] = 2;
        }

        return primitiveObject;
    }

    sModel* cGraphics::CreateModel(const std::string& filename)
    {
        cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
        const sApplicationCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();

        // Create model
        sModel* pModel = (sModel*)_context->Create<sModel>();

        model->_format = eCategory::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3;

        // Load model
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(
            filename.data(),
            0
        );

        if (scene == nullptr)
            return nullptr;

        // Load vertices
        usize totalVertexCount = 0;
        model->_vertices = (sVertex*)memoryAllocator->Allocate(scene->mMeshes[0]->mNumVertices * sizeof(sVertex), caps->memoryAlignment);
        memset(model->_vertices, 0, scene->mMeshes[0]->mNumVertices * sizeof(sVertex));
        for (usize i = 0; i < scene->mMeshes[0]->mNumVertices; i++)
        {
            const aiVector3D pos = scene->mMeshes[0]->mVertices[i];
            const aiVector3D uv = scene->mMeshes[0]->mTextureCoords[0][i];
            const aiVector3D normal = scene->mMeshes[0]->HasNormals() ? scene->mMeshes[0]->mNormals[i] : aiVector3D(1.0f, 1.0f, 1.0f);

            model->_vertices[totalVertexCount]._position = glm::vec3(pos.x, pos.y, pos.z);
            model->_vertices[totalVertexCount]._texcoord = glm::vec2(uv.x, uv.y);
            model->_vertices[totalVertexCount]._normal = glm::vec3(normal.x, normal.y, normal.z);

            totalVertexCount += 1;
        }

        // Load indices
        usize totalIndexCount = 0;
        model->_indices = (index*)memoryAllocator->Allocate(scene->mMeshes[0]->mNumFaces * 3 * sizeof(index), caps->memoryAlignment);
        memset(model->_indices, 0, scene->mMeshes[0]->mNumFaces * 3 * sizeof(index));
        for (usize i = 0; i < scene->mMeshes[0]->mNumFaces; i++)
        {
            const aiFace face = scene->mMeshes[0]->mFaces[i];
            for (usize j = 0; j < face.mNumIndices; j++)
            {
                model->_indices[totalIndexCount] = face.mIndices[j];

                totalIndexCount += 1;
            }
        }

        model->_vertexCount = totalVertexCount;
        model->_verticesByteSize = totalVertexCount * sizeof(sVertex);
        model->_indexCount = totalIndexCount;
        model->_indicesByteSize = totalIndexCount * sizeof(index);

        return model;
    }

    cMaterial* cGraphics::FindMaterial(const std::string& id)
    {
        return _materialsCPU->Find(id);
    }

    void cGraphics::DestroyMaterial(const std::string& id)
    {
        cMaterial* material = _materialsCPU->Find(id);
        if (material->GetCustomShader() != nullptr)
            _gfx->DestroyShader(material->GetCustomShader());

        _materialsCPU->Delete(id);
    }

    void cGraphics::DestroyGeometry(sVertexBufferGeometry* geometry)
    {
        _context->Destroy<sVertexBufferGeometry>(geometry);
    }

    void cGraphics::DestroyRenderPass(cRenderPass* renderPass)
    {
        _gfx->DestroyRenderPass(renderPass->GetRenderPassGPU());
        _context->Destroy<cRenderPass>(renderPass);
    }

    void cGraphics::DestroyPrimitive(sPrimitive* primitiveObject)
    {
        cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();

        if (primitiveObject->_vertices)
            memoryAllocator->Deallocate(primitiveObject->_vertices);
        if (primitiveObject->_indices)
            memoryAllocator->Deallocate(primitiveObject->_indices);
        _context->Destroy<sPrimitive>(primitiveObject);
    }

    void cGraphics::ClearGeometryBuffer()
    {
        _verticesByteSize = 0;
        _indicesByteSize = 0;
    }

    void cGraphics::ClearRenderPass(const cRenderPass* renderPass, types::boolean clearColor, usize bufferIndex, const glm::vec4& color, types::boolean clearDepth, f32 depth)
    {
        _gfx->BindRenderPass(renderPass);
        if (clearColor == K_TRUE)
            _gfx->ClearFramebufferColor(bufferIndex, color);
        if (clearDepth == K_TRUE)
            _gfx->ClearFramebufferDepth(depth);
        _gfx->UnbindRenderPass(renderPass);
    }

    void cGraphics::ClearRenderPasses(const glm::vec4& clearColor, f32 clearDepth)
    {
        _gfx->BindRenderPass(_opaque);
        _gfx->ClearFramebufferColor(0, clearColor);
        _gfx->ClearFramebufferDepth(clearDepth);

        _gfx->BindRenderPass(_transparent);
        _gfx->ClearFramebufferColor(0, glm::vec4(0.0f));
        _gfx->ClearFramebufferColor(1, glm::vec4(1.0f));

        _gfx->BindRenderPass(_text);
        _gfx->ClearFramebufferColor(0, clearColor);
    }

    void cGraphics::ResizeRenderTargets(const glm::vec2& size)
    {
        _gfx->UnbindRenderPass(_opaque);
        _gfx->UnbindRenderPass(_transparent);

        _opaque->ResizeViewport(size);
        _opaque->ResizeColorAttachments(size);
        _opaque->ResizeDepthAttachment(size);

        _transparent->ResizeViewport(size);
        _transparent->ResizeColorAttachments(size);
        cTexture* opaqueDepthAttachment = _opaque->GetRenderTarget()->GetDepthAttachment();
        _transparent->GetRenderTarget()->SetDepthAttachment(opaqueDepthAttachment);

        _text->ResizeViewport(size);

        _gfx->UpdateRenderTargetBuffers(_opaque->GetRenderTarget());
        _gfx->UpdateRenderTargetBuffers(_transparent->GetRenderTarget());

        _compositeTransparent->ResizeViewport(size);
        auto& transparentColorAttachments = _transparent->GetRenderTarget()->GetColorAttachments();
        _compositeTransparent->SetInputTexture(0, transparentColorAttachments[0]);
        _compositeTransparent->SetInputTexture(1, transparentColorAttachments[1]);
        //_compositeTransparent->_desc._inputTextureNames[0] = "AccumulationTexture";
        //_compositeTransparent->_desc._inputTextureNames[1] = "RevealageTexture";

        _compositeFinal->ResizeViewport(size);
        auto& opaqueColorAttachments = _opaque->GetRenderTarget()->GetColorAttachments();
        _compositeFinal->SetInputTexture(0, opaqueColorAttachments[0]);
        //_compositeFinal->_desc._inputTextureNames[0] = "ColorTexture";
    }

    void cGraphics::LoadShaderFiles(const std::string& vertexFuncPath, const std::string& fragmentFuncPath, std::string& vertexFunc, std::string& fragmentFunc)
    {
        cFileSystem* fileSystem = _context->GetSubsystem<cFileSystem>();

        cDataFile* vertexFuncFile = fileSystem->CreateDataFile(vertexFuncPath, K_TRUE);
        cDataFile* fragmentFuncFile = fileSystem->CreateDataFile(fragmentFuncPath, K_TRUE);

        vertexFunc = std::string((const char*)vertexFuncFile->GetData());
        fragmentFunc = std::string((const char*)fragmentFuncFile->GetData());

        fileSystem->DestroyDataFile(vertexFuncFile);
        fileSystem->DestroyDataFile(fragmentFuncFile);
    }

    void cGraphics::UpdateLights()
    {
        /*_lightsByteSize = 16; // because vec4 (16 bytes) goes first (contains light count)
        memset(_lights, 0, 16 + (sizeof(sLightInstance) * 16));

        glm::uvec4 lightCount = glm::uvec4(0);

        for (auto& it : _app->GetGameObjectManager()->GetObjects())
        {
            if (it.GetLight() != nullptr)
            {
                sLightInstance li(&it);

                memcpy((void*)((usize)_lights + (usize)_lightsByteSize), &li, sizeof(sLightInstance));
                _lightsByteSize += sizeof(sLightInstance);

                lightCount.x += 1;
            }
        }

        memcpy((void*)(usize)_lights, &lightCount, sizeof(glm::uvec4));

        _context->WriteBuffer(_lightBuffer, 0, _lightsByteSize, _lights);*/
    }

    void cGraphics::WriteObjectsToOpaqueBuffers(cIdVector<cGameObject>& objects, cRenderPass* renderPass)
    {
        _opaqueInstanceCount = 0;
        _opaqueInstancesByteSize = 0;
        _opaqueMaterialsByteSize = 0;
        _opaqueTextureAtlasTexturesByteSize = 0;
        _materialsMap->clear();

        cGameObject* objectsArray = objects.GetElements();

        for (usize i = 0; i < objects.GetElementCount(); i++)
        {
            const cGameObject& go = objectsArray[i];

            sTransform transform(&go);
            transform.Transform();

            s32 materialIndex = -1;
            cMaterial* material = go.GetMaterial();
            sVertexBufferGeometry* geometry = go.GetGeometry();

            if (geometry == nullptr)
                continue;

            if (material != nullptr)
            {
                auto it = _materialsMap->find(material);
                if (it == _materialsMap->end())
                {
                    materialIndex = _materialsMap->size();

                    cMaterialInstance mi(materialIndex, material);

                    _materialsMap->insert({ material, materialIndex });

                    memcpy((void*)((usize)_opaqueMaterials + (usize)_opaqueMaterialsByteSize), &mi, sizeof(cMaterialInstance));
                    _opaqueMaterialsByteSize += sizeof(cMaterialInstance);
                }
                else
                {
                    materialIndex = it->second;
                }
            }

            const sRenderInstance ri(materialIndex, transform);

            memcpy((void*)((usize)_opaqueInstances + _opaqueInstancesByteSize), &ri, sizeof(sRenderInstance));
            _opaqueInstancesByteSize += sizeof(sRenderInstance);

            _opaqueInstanceCount += 1;
        }

        const std::vector<cTextureAtlasTexture*>& renderPassTextureAtlasTextures = renderPass->GetInputTextureAtlasTextures();
        for (const auto textureAtlasTexture : renderPassTextureAtlasTextures)
        {
            sTextureAtlasTextureGPU tatGPU;
            tatGPU._textureInfo = glm::vec4(
                textureAtlasTexture->GetOffset().x,
                textureAtlasTexture->GetOffset().y,
                textureAtlasTexture->GetSize().x,
                textureAtlasTexture->GetSize().y
            );
            tatGPU._textureLayerInfo = textureAtlasTexture->GetOffset().z;

            memcpy((void*)((usize)_opaqueTextureAtlasTextures + _opaqueTextureAtlasTexturesByteSize), &tatGPU, sizeof(sTextureAtlasTextureGPU));
            _opaqueTextureAtlasTexturesByteSize += sizeof(sTextureAtlasTextureGPU);
        }

        _gfx->WriteBuffer(_opaqueInstanceBuffer, 0, _opaqueInstancesByteSize, _opaqueInstances);
        _gfx->WriteBuffer(_opaqueMaterialBuffer, 0, _opaqueMaterialsByteSize, _opaqueMaterials);
        _gfx->WriteBuffer(_opaqueTextureAtlasTexturesBuffer, 0, _opaqueTextureAtlasTexturesByteSize, _opaqueTextureAtlasTextures);
    }

    void cGraphics::WriteObjectsToTransparentBuffers(cIdVector<cGameObject>& objects, cRenderPass* renderPass)
    {
        _transparentInstanceCount = 0;
        _transparentInstancesByteSize = 0;
        _transparentMaterialsByteSize = 0;
        _materialsMap->clear();

        cGameObject* objectsArray = objects.GetElements();

        for (usize i = 0; i < objects.GetElementCount(); i++)
        {
            const cGameObject& go = objectsArray[i];

            sTransform transform(&go);
            transform.Transform();

            s32 materialIndex = -1;
            cMaterial* material = go.GetMaterial();
            sVertexBufferGeometry* geometry = go.GetGeometry();

            if (geometry == nullptr)
                continue;

            if (material != nullptr)
            {
                auto it = _materialsMap->find(material);
                if (it == _materialsMap->end())
                {
                    materialIndex = _materialsMap->size();

                    cMaterialInstance mi(materialIndex, material);

                    _materialsMap->insert({ material, materialIndex });

                    memcpy((void*)((usize)_transparentMaterials + _transparentMaterialsByteSize), &mi, sizeof(cMaterialInstance));
                    _transparentMaterialsByteSize += sizeof(cMaterialInstance);
                }
                else
                {
                    materialIndex = it->second;
                }
            }

            const sRenderInstance ri(materialIndex, transform);

            memcpy((void*)((usize)_transparentInstances + _transparentInstancesByteSize), &ri, sizeof(sRenderInstance));
            _transparentInstancesByteSize += sizeof(sRenderInstance);

            _transparentInstanceCount += 1;
        }

        const std::vector<cTextureAtlasTexture*>& renderPassTextureAtlasTextures = renderPass->GetInputTextureAtlasTextures();
        for (const auto textureAtlasTexture : renderPassTextureAtlasTextures)
        {
            sTextureAtlasTextureGPU tatGPU;
            tatGPU._textureInfo = glm::vec4(
                textureAtlasTexture->GetOffset().x,
                textureAtlasTexture->GetOffset().y,
                textureAtlasTexture->GetSize().x,
                textureAtlasTexture->GetSize().y
            );
            tatGPU._textureLayerInfo = textureAtlasTexture->GetOffset().z;

            memcpy((void*)((usize)_transparentTextureAtlasTextures + (usize)_transparentTextureAtlasTexturesByteSize), &tatGPU, sizeof(sTextureAtlasTextureGPU));
            _transparentTextureAtlasTexturesByteSize += sizeof(sTextureAtlasTextureGPU);
        }

        _gfx->WriteBuffer(_transparentInstanceBuffer, 0, _transparentInstancesByteSize, _transparentInstances);
        _gfx->WriteBuffer(_transparentMaterialBuffer, 0, _transparentMaterialsByteSize, _transparentMaterials);
        _gfx->WriteBuffer(_transparentTextureAtlasTexturesBuffer, 0, _transparentTextureAtlasTexturesByteSize, _transparentTextureAtlasTextures);
    }

    void cGraphics::DrawGeometryOpaque(const sVertexBufferGeometry* geometry, const cGameObject* cameraObject, cRenderPass* renderPass)
    {
        if (renderPass == nullptr)
        {
            _gfx->BindRenderPass(_opaque);
            _gfx->SetShaderUniform(_opaque->GetShader(), "ViewProjection", cameraObject->GetViewProjectionMatrix());
        }
        else
        {
            _gfx->BindRenderPass(renderPass);
            _gfx->SetShaderUniform(renderPass->GetShader(), "ViewProjection", cameraObject->GetViewProjectionMatrix());
        }

        _gfx->Draw(
            geometry->_indexCount,
            geometry->_offsetVertex,
            geometry->_offsetIndex,
            _opaqueInstanceCount
        );

        if (renderPass == nullptr)
            _gfx->UnbindRenderPass(_opaque);
        else
            _gfx->UnbindRenderPass(renderPass);
    }

    void cGraphics::DrawGeometryOpaque(const sVertexBufferGeometry* geometry, const cGameObject* cameraObject, cShader* singleShader)
    {
        _gfx->BindRenderPass(_opaque, singleShader);

        if (singleShader == nullptr)
            _gfx->SetShaderUniform(_opaque->GetShader(), "ViewProjection", cameraObject->GetViewProjectionMatrix());
        else
            _gfx->SetShaderUniform(singleShader, "ViewProjection", cameraObject->GetViewProjectionMatrix());

        _gfx->Draw(
            geometry->_indexCount,
            geometry->_offsetVertex,
            geometry->_offsetIndex,
            _opaqueInstanceCount
        );

        _gfx->UnbindRenderPass(_opaque);
    }

    void cGraphics::DrawGeometryTransparent(const sVertexBufferGeometry* geometry, const std::vector<cGameObject>& objects, const cGameObject* cameraObject, cRenderPass* renderPass)
    {
        if (renderPass == nullptr)
        {
            _gfx->BindRenderPass(_transparent);
            _gfx->SetShaderUniform(_transparent->GetShader(), "ViewProjection", cameraObject->GetViewProjectionMatrix());
        }
        else
        {
            _gfx->BindRenderPass(renderPass);
            _gfx->SetShaderUniform(renderPass->GetShader(), "ViewProjection", cameraObject->GetViewProjectionMatrix());
        }

        _gfx->Draw(
            geometry->_indexCount,
            geometry->_offsetVertex,
            geometry->_offsetIndex,
            _transparentInstanceCount
        );

        if (renderPass == nullptr)
            _gfx->UnbindRenderPass(_transparent);
        else
            _gfx->UnbindRenderPass(renderPass);

        CompositeTransparent();
    }

    void cGraphics::DrawGeometryTransparent(const sVertexBufferGeometry* geometry, const cGameObject* cameraObject, cShader* singleShader)
    {
        _gfx->BindRenderPass(_transparent, singleShader);

        if (singleShader != nullptr)
            _gfx->SetShaderUniform(singleShader, "ViewProjection", cameraObject->GetViewProjectionMatrix());
        else
            _gfx->SetShaderUniform(_transparent->GetShader(), "ViewProjection", cameraObject->GetViewProjectionMatrix());

        _gfx->Draw(
            geometry->_indexCount,
            geometry->_offsetVertex,
            geometry->_offsetIndex,
            _transparentInstanceCount
        );

        _gfx->UnbindRenderPass(_transparent);
    }

    void cGraphics::DrawTexts(const std::vector<cGameObject>& objects)
    {
        for (auto& it : objects)
        {
            if (it.GetText() == nullptr)
                continue;

            const cText* text = it.GetText();
            const cFontFace* textFont = text->GetFont();
            const std::string& textString = text->GetText();
            const auto& alphabet = textFont->GetAlphabet();
            const cTexture* atlas = text->GetFont()->GetAtlas();

            _textInstancesByteSize = 0;
            _materialsMap->clear();

            const sTransform transform(&it);

            iApplication* app = _context->GetSubsystem<cEngine>()->GetApplication();
            const glm::vec2 windowSize = app->GetWindow()->GetSize();
            const glm::vec2 textPosition = glm::vec2((transform._position.x * 2.0f) - 1.0f, (transform._position.y * 2.0f) - 1.0f);
            const glm::vec2 textScale = glm::vec2(
                (1.0f / windowSize.x) * it.GetTransform()->_scale.x,
                (1.0f / windowSize.y) * it.GetTransform()->_scale.y
            );

            const usize charCount = textString.length();
            usize actualCharCount = 0;
            glm::vec2 offset = glm::vec2(0.0f);
            for (usize i = 0; i < charCount; i++)
            {
                const u8 glyphChar = textString[i];

                if (glyphChar == '\t')
                {
                    offset.x += textFont->GetOffsetTab() * textScale.x;
                    continue;
                }
                else if (glyphChar == '\n')
                {
                    offset.x = 0.0f;
                    offset.y -= textFont->GetOffsetNewline() * textScale.y;
                    continue;
                }
                else if (glyphChar == ' ')
                {
                    offset.x += textFont->GetOffsetSpace() * textScale.x;
                    continue;
                }

                const auto alphabetEntry = alphabet.find(glyphChar);
                if (alphabetEntry == alphabet.end())
                    continue;
                const sGlyph& glyph = alphabetEntry->second;

                sTextInstance t;
                t._info.x = textPosition.x + offset.x;
                t._info.y = textPosition.y + (offset.y - (float)((glyph._height - glyph._top) * textScale.y));
                t._info.z = (float)glyph._width * textScale.x;
                t._info.w = (float)glyph._height * textScale.y;
                t._atlasInfo.x = (float)glyph._atlasXOffset / (float)atlas->GetWidth();
                t._atlasInfo.y = (float)glyph._atlasYOffset / (float)atlas->GetHeight();
                t._atlasInfo.z = (float)glyph._width / (float)atlas->GetWidth();
                t._atlasInfo.w = (float)glyph._height / (float)atlas->GetHeight();

                offset.x += glyph._advanceX * textScale.x;

                memcpy((void*)((usize)_textInstances + _textInstancesByteSize), &t, sizeof(sTextInstance));
                _textInstancesByteSize += sizeof(sTextInstance);

                actualCharCount += 1;
            }

            const cMaterialInstance mi(0, it.GetMaterial());
            memcpy(_textMaterials, &mi, sizeof(cMaterialInstance));
            _textMaterialsByteSize += sizeof(cMaterialInstance);

            _gfx->WriteBuffer(_textInstanceBuffer, 0, _textInstancesByteSize, _textInstances);
            _gfx->WriteBuffer(_textMaterialBuffer, 0, _textMaterialsByteSize, _textMaterials);

            _gfx->BindRenderPass(_text);
            _gfx->BindTexture(_text->GetShader(), "FontAtlas", atlas, 0);
            _gfx->DrawQuads(actualCharCount);
            _gfx->UnbindRenderPass(_text);
        }
    }

    void cGraphics::CompositeTransparent()
    {
        _gfx->BindRenderPass(_compositeTransparent);
        _gfx->DrawQuad();
        _gfx->UnbindRenderPass(_compositeTransparent);
    }

    void cGraphics::CompositeFinal()
    {
        _gfx->BindRenderPass(_compositeFinal);
        _gfx->DrawQuad();
        _gfx->UnbindRenderPass(_compositeFinal);

        _gfx->UnbindShader();
    }
}