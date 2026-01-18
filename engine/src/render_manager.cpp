// render_manager.cpp
/*
#include <GL/glew.h>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include "../../thirdparty/glm/glm/gtc/quaternion.hpp"
#include "../../thirdparty/glm/glm/gtx/quaternion.hpp"
#include "render_manager.hpp"
#include "render_context.hpp"
#include "texture_manager.hpp"
#include "gameobject_manager.hpp"
#include "font_manager.hpp"
#include "filesystem_manager.hpp"
#include "application.hpp"
#include "memory_pool.hpp"
#include "log.hpp"

using namespace types;

namespace arachne
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

    mRender::mRender(cContext* context, iRenderContext* renderContext) : iObject(context), _renderContext(renderContext), _materialsCPU(app)
    {
        cMemoryPool* memoryPool = app->GetMemoryPool();

        const sApplicationDescriptor* desc = app->GetDesc();
        const glm::vec2 windowSize = app->GetWindowSize();

        _maxOpaqueInstanceBufferByteSize = desc->_maxRenderOpaqueInstanceCount * sizeof(sRenderInstance);
        _maxTransparentInstanceBufferByteSize = desc->_maxRenderTransparentInstanceCount * sizeof(sRenderInstance);
        _maxTextInstanceBufferByteSize = desc->_maxRenderTextInstanceCount * sizeof(sRenderInstance);
        _maxMaterialBufferByteSize = desc->_maxMaterialCount * sizeof(cMaterialInstance);
        _maxLightBufferByteSize = desc->_maxLightCount * sizeof(sLightInstance);
        _maxTextureAtlasTexturesBufferByteSize = desc->_maxTextureAtlasTextureCount * sizeof(sTextureAtlasTextureGPU);

        _vertexBuffer = _renderContext->CreateBuffer(desc->_vertexBufferSize, sBuffer::eType::VERTEX, nullptr);
        _indexBuffer = _renderContext->CreateBuffer(desc->_indexBufferSize, sBuffer::eType::INDEX, nullptr);
        _opaqueInstanceBuffer = _renderContext->CreateBuffer(_maxOpaqueInstanceBufferByteSize, sBuffer::eType::LARGE, nullptr);
        _opaqueInstanceBuffer->_slot = 0;
        _transparentInstanceBuffer = _renderContext->CreateBuffer(_maxTransparentInstanceBufferByteSize, sBuffer::eType::LARGE, nullptr);
        _transparentInstanceBuffer->_slot = 0;
        _textInstanceBuffer = _renderContext->CreateBuffer(_maxTextInstanceBufferByteSize, sBuffer::eType::LARGE, nullptr);
        _textInstanceBuffer->_slot = 0;
        _opaqueMaterialBuffer = _renderContext->CreateBuffer(_maxMaterialBufferByteSize, sBuffer::eType::LARGE, nullptr);
        _opaqueMaterialBuffer->_slot = 1;
        _textMaterialBuffer = _renderContext->CreateBuffer(_maxMaterialBufferByteSize, sBuffer::eType::LARGE, nullptr);
        _textMaterialBuffer->_slot = 1;
        _transparentMaterialBuffer = _renderContext->CreateBuffer(_maxMaterialBufferByteSize, sBuffer::eType::LARGE, nullptr);
        _transparentMaterialBuffer->_slot = 1;
        _lightBuffer = _renderContext->CreateBuffer(_maxLightBufferByteSize, sBuffer::eType::LARGE, nullptr);
        _lightBuffer->_slot = 2;
        _opaqueTextureAtlasTexturesBuffer = _renderContext->CreateBuffer(_maxTextureAtlasTexturesBufferByteSize, sBuffer::eType::LARGE, nullptr);
        _opaqueTextureAtlasTexturesBuffer->_slot = 3;
        _transparentTextureAtlasTexturesBuffer = _renderContext->CreateBuffer(_maxTextureAtlasTexturesBufferByteSize, sBuffer::eType::LARGE, nullptr);
        _transparentTextureAtlasTexturesBuffer->_slot = 3;
        _vertices = memoryPool->Allocate(desc->_vertexBufferSize);
        _verticesByteSize = 0;
        _indices = memoryPool->Allocate(desc->_indexBufferSize);
        _indicesByteSize = 0;
        _opaqueInstances = memoryPool->Allocate(_maxOpaqueInstanceBufferByteSize);
        _opaqueInstancesByteSize = 0;
        _transparentInstances = memoryPool->Allocate(_maxTransparentInstanceBufferByteSize);
        _transparentInstancesByteSize = 0;
        _textInstances = memoryPool->Allocate(_maxTextInstanceBufferByteSize);
        _textInstancesByteSize = 0;
        _opaqueMaterials = memoryPool->Allocate(_maxMaterialBufferByteSize);
        _opaqueMaterialsByteSize = 0;
        _transparentMaterials = memoryPool->Allocate(_maxMaterialBufferByteSize);
        _transparentMaterialsByteSize = 0;
        _textMaterials = memoryPool->Allocate(_maxMaterialBufferByteSize);
        _textMaterialsByteSize = 0;
        _lights = memoryPool->Allocate(_maxLightBufferByteSize);
        _lightsByteSize = 0;
        _opaqueTextureAtlasTextures = memoryPool->Allocate(_maxTextureAtlasTexturesBufferByteSize);
        _opaqueTextureAtlasTexturesByteSize = 0;
        _transparentTextureAtlasTextures = memoryPool->Allocate(_maxTextureAtlasTexturesBufferByteSize);
        _transparentTextureAtlasTexturesByteSize = 0;
        std::unordered_map<cMaterial*, s32>* pMaterialsMap = (std::unordered_map<cMaterial*, s32>*)app->GetMemoryPool()->Allocate(sizeof(std::unordered_map<cMaterial*, s32>));
        _materialsMap = new (pMaterialsMap) std::unordered_map<cMaterial*, s32>();

        sTexture* color = _renderContext->CreateTexture(windowSize.x, windowSize.y, 0, sTexture::eType::TEXTURE_2D, sTexture::eFormat::RGBA8, nullptr);
        sTexture* accumulation = _renderContext->CreateTexture(windowSize.x, windowSize.y, 0, sTexture::eType::TEXTURE_2D, sTexture::eFormat::RGBA16F, nullptr);
        sTexture* ui = _renderContext->CreateTexture(windowSize.x, windowSize.y, 0, sTexture::eType::TEXTURE_2D, sTexture::eFormat::RGBA8, nullptr);
        sTexture* revealage = _renderContext->CreateTexture(windowSize.x, windowSize.y, 0, sTexture::eType::TEXTURE_2D, sTexture::eFormat::R8F, nullptr);
        sTexture* depth = _renderContext->CreateTexture(windowSize.x, windowSize.y, 0, sTexture::eType::TEXTURE_2D, sTexture::eFormat::DEPTH_STENCIL, nullptr);

        _opaqueRenderTarget = _renderContext->CreateRenderTarget({ color }, depth);
        _transparentRenderTarget = _renderContext->CreateRenderTarget({ accumulation, revealage }, depth);

        {
            sRenderPass::sDescriptor renderPassDesc;
            renderPassDesc._inputVertexFormat = eCategory::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3;
            renderPassDesc._inputBuffers.emplace_back(mRender::GetVertexBuffer());
            renderPassDesc._inputBuffers.emplace_back(mRender::GetIndexBuffer());
            renderPassDesc._inputBuffers.emplace_back(mRender::GetOpaqueInstanceBuffer());
            renderPassDesc._inputBuffers.emplace_back(mRender::GetOpaqueMaterialBuffer());
            renderPassDesc._inputBuffers.emplace_back(mRender::GetLightBuffer());
            renderPassDesc._inputBuffers.emplace_back(mRender::GetOpaqueTextureAtlasTexturesBuffer());
            renderPassDesc._inputTextures.emplace_back(app->GetTextureManager()->GetAtlas());
            renderPassDesc._inputTextureNames.emplace_back("TextureAtlas");
            renderPassDesc._shaderBase = nullptr;
            renderPassDesc._shaderRenderPath = eCategory::RENDER_PATH_OPAQUE;
            renderPassDesc._shaderVertexPath = "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/main_vertex.shader";
            renderPassDesc._shaderFragmentPath = "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/main_fragment.shader";
            renderPassDesc._renderTarget = _opaqueRenderTarget;
            renderPassDesc._viewport = glm::vec4(0.0f, 0.0f, windowSize);
            renderPassDesc._depthMode._useDepthTest = K_TRUE;
            renderPassDesc._depthMode._useDepthWrite = K_TRUE;
            renderPassDesc._blendMode._factorCount = 1;
            renderPassDesc._blendMode._srcFactors[0] = sBlendMode::eFactor::ONE;
            renderPassDesc._blendMode._dstFactors[0] = sBlendMode::eFactor::ZERO;
            _opaque = _renderContext->CreateRenderPass(renderPassDesc);
        }
        {
            sRenderPass::sDescriptor renderPassDesc;
            renderPassDesc._inputVertexFormat = eCategory::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3;
            renderPassDesc._inputBuffers.emplace_back(mRender::GetVertexBuffer());
            renderPassDesc._inputBuffers.emplace_back(mRender::GetIndexBuffer());
            renderPassDesc._inputBuffers.emplace_back(mRender::GetTransparentInstanceBuffer());
            renderPassDesc._inputBuffers.emplace_back(mRender::GetTransparentMaterialBuffer());
            renderPassDesc._inputTextures.emplace_back(app->GetTextureManager()->GetAtlas());
            renderPassDesc._inputTextureNames.emplace_back("TextureAtlas");
            renderPassDesc._shaderBase = nullptr;
            renderPassDesc._shaderRenderPath = eCategory::RENDER_PATH_TRANSPARENT;
            renderPassDesc._shaderVertexPath = "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/main_vertex.shader";
            renderPassDesc._shaderFragmentPath = "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/main_fragment.shader";
            renderPassDesc._renderTarget = _transparentRenderTarget;
            renderPassDesc._viewport = glm::vec4(0.0f, 0.0f, windowSize);
            renderPassDesc._depthMode._useDepthTest = K_TRUE;
            renderPassDesc._depthMode._useDepthWrite = K_FALSE;
            renderPassDesc._blendMode._factorCount = 2;
            renderPassDesc._blendMode._srcFactors[0] = sBlendMode::eFactor::ONE;
            renderPassDesc._blendMode._dstFactors[0] = sBlendMode::eFactor::ONE;
            renderPassDesc._blendMode._srcFactors[1] = sBlendMode::eFactor::ZERO;
            renderPassDesc._blendMode._dstFactors[1] = sBlendMode::eFactor::INV_SRC_COLOR;
            _transparent = _renderContext->CreateRenderPass(renderPassDesc);
        }
        {
            sRenderPass::sDescriptor renderPassDesc;
            renderPassDesc._inputVertexFormat = eCategory::VERTEX_BUFFER_FORMAT_NONE;
            renderPassDesc._inputBuffers.emplace_back(mRender::GetTextInstanceBuffer());
            renderPassDesc._inputBuffers.emplace_back(mRender::GetTextMaterialBuffer());
            renderPassDesc._shaderBase = nullptr;
            renderPassDesc._shaderRenderPath = eCategory::RENDER_PATH_TEXT;
            renderPassDesc._shaderVertexPath = "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/main_vertex.shader";
            renderPassDesc._shaderFragmentPath = "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/main_fragment.shader";
            renderPassDesc._renderTarget = _opaqueRenderTarget;
            renderPassDesc._viewport = glm::vec4(0.0f, 0.0f, windowSize);
            renderPassDesc._depthMode._useDepthTest = K_FALSE;
            renderPassDesc._depthMode._useDepthWrite = K_FALSE;
            _text = _renderContext->CreateRenderPass(renderPassDesc);
        }
        {
            sRenderPass::sDescriptor renderPassDesc;
            renderPassDesc._inputVertexFormat = eCategory::VERTEX_BUFFER_FORMAT_NONE;
            renderPassDesc._inputTextures.emplace_back(_transparentRenderTarget->_colorAttachments[0]);
            renderPassDesc._inputTextureNames.emplace_back("AccumulationTexture");
            renderPassDesc._inputTextures.emplace_back(_transparentRenderTarget->_colorAttachments[1]);
            renderPassDesc._inputTextureNames.emplace_back("RevealageTexture");
            renderPassDesc._shaderBase = nullptr;
            renderPassDesc._shaderRenderPath = eCategory::RENDER_PATH_TRANSPARENT_COMPOSITE;
            renderPassDesc._shaderVertexPath = "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/main_vertex.shader";
            renderPassDesc._shaderFragmentPath = "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/main_fragment.shader";
            renderPassDesc._renderTarget = _opaqueRenderTarget;
            renderPassDesc._viewport = glm::vec4(0.0f, 0.0f, windowSize);
            renderPassDesc._depthMode._useDepthTest = K_FALSE;
            renderPassDesc._depthMode._useDepthWrite = K_FALSE;
            renderPassDesc._blendMode._factorCount = 1;
            renderPassDesc._blendMode._srcFactors[0] = sBlendMode::eFactor::SRC_ALPHA;
            renderPassDesc._blendMode._dstFactors[0] = sBlendMode::eFactor::INV_SRC_ALPHA;
            _compositeTransparent = _renderContext->CreateRenderPass(renderPassDesc);
        }
        {
            sRenderPass::sDescriptor renderPassDesc;
            renderPassDesc._inputVertexFormat = eCategory::VERTEX_BUFFER_FORMAT_NONE;
            renderPassDesc._inputTextures.emplace_back(_opaqueRenderTarget->_colorAttachments[0]);
            renderPassDesc._inputTextureNames.emplace_back("ColorTexture");
            renderPassDesc._shaderBase = nullptr;
            renderPassDesc._shaderRenderPath = eCategory::RENDER_PATH_QUAD;
            renderPassDesc._shaderVertexPath = "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/main_vertex.shader";
            renderPassDesc._shaderFragmentPath = "C:/DDD/RealWare/out/build/x64-Debug/samples/Sample01/data/shaders/main_fragment.shader";
            renderPassDesc._renderTarget = nullptr;
            renderPassDesc._viewport = glm::vec4(0.0f, 0.0f, windowSize);
            renderPassDesc._depthMode._useDepthTest = K_FALSE;
            renderPassDesc._depthMode._useDepthWrite = K_FALSE;
            renderPassDesc._blendMode._factorCount = 1;
            renderPassDesc._blendMode._srcFactors[0] = sBlendMode::eFactor::ONE;
            renderPassDesc._blendMode._dstFactors[0] = sBlendMode::eFactor::ZERO;
            _compositeFinal = _renderContext->CreateRenderPass(renderPassDesc);
        }
    }

    mRender::~mRender()
    {
        cMemoryPool* memoryPool = GetApplication()->GetMemoryPool();

        _renderContext->DestroyBuffer(_vertexBuffer);
        _renderContext->DestroyBuffer(_indexBuffer);
        _renderContext->DestroyBuffer(_textInstanceBuffer);
        _renderContext->DestroyBuffer(_transparentInstanceBuffer);
        _renderContext->DestroyBuffer(_opaqueInstanceBuffer);
        _materialsMap->~unordered_map<cMaterial*, s32>();
        memoryPool->Free(_materialsMap);
        memoryPool->Free(_vertices);
        memoryPool->Free(_indices);
        memoryPool->Free(_textInstances);
        memoryPool->Free(_transparentInstances);
        memoryPool->Free(_opaqueInstances);
    }

    cMaterial* mRender::CreateMaterial(const std::string& id, cTextureAtlasTexture* diffuseTexture, const glm::vec4& diffuseColor, const glm::vec4& highlightColor, eCategory customShaderRenderPath, const std::string& customVertexFuncPath, const std::string& customFragmentFuncPath)
    {
        sShader* customShader = nullptr;
        if (customVertexFuncPath != "" || customFragmentFuncPath != "")
        {
            std::string vertexFunc = "";
            std::string fragmentFunc = "";
            LoadShaderFiles(customVertexFuncPath, customFragmentFuncPath, vertexFunc, fragmentFunc);

            if (customShaderRenderPath == eCategory::RENDER_PATH_OPAQUE)
                customShader = _renderContext->CreateShader(_opaque->_desc._shader, vertexFunc, fragmentFunc);
            else if (customShaderRenderPath == eCategory::RENDER_PATH_TRANSPARENT)
                customShader = _renderContext->CreateShader(_transparent->_desc._shader, vertexFunc, fragmentFunc);
        }

        return _materialsCPU.Add(id, diffuseTexture, diffuseColor, highlightColor, customShader);
    }

    cMaterial* mRender::FindMaterial(const std::string& id)
    {
        return _materialsCPU.Find(id);
    }

    void mRender::DestroyMaterial(const std::string& id)
    {
        cMaterial* material = _materialsCPU.Find(id);
        if (material->GetCustomShader() != nullptr)
            _renderContext->DestroyShader(material->GetCustomShader());

        _materialsCPU.Delete(id);
    }

    sVertexArray* mRender::CreateDefaultVertexArray()
    {
        sVertexArray* vertexArray = _renderContext->CreateVertexArray();
        std::vector<sBuffer*> buffersToBind = { _vertexBuffer, _indexBuffer };

        _renderContext->BindVertexArray(vertexArray);
        for (auto buffer : buffersToBind)
            _renderContext->BindBuffer(buffer);
        _renderContext->BindDefaultInputLayout();
        _renderContext->UnbindVertexArray();

        return vertexArray;
    }

    sVertexBufferGeometry* mRender::CreateGeometry(eCategory format, usize verticesByteSize, const void* vertices, usize indicesByteSize, const void* indices)
    {
        sVertexBufferGeometry* pGeometry = (sVertexBufferGeometry*)GetApplication()->GetMemoryPool()->Allocate(sizeof(sVertexBufferGeometry));
        sVertexBufferGeometry* geometry = new (pGeometry) sVertexBufferGeometry();

        memcpy((void*)((usize)_vertices + _verticesByteSize), vertices, verticesByteSize);
        memcpy((void*)((usize)_indices + _indicesByteSize), indices, indicesByteSize);

        _renderContext->WriteBuffer(_vertexBuffer, _verticesByteSize, verticesByteSize, vertices);
        _renderContext->WriteBuffer(_indexBuffer, _indicesByteSize, indicesByteSize, indices);

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

    void mRender::DestroyGeometry(sVertexBufferGeometry* geometry)
    {
        geometry->~sVertexBufferGeometry();
        GetApplication()->GetMemoryPool()->Free(geometry);
    }

    void mRender::ClearGeometryBuffer()
    {
        _verticesByteSize = 0;
        _indicesByteSize = 0;
    }

    void mRender::ClearRenderPass(const sRenderPass* renderPass, types::boolean clearColor, usize bufferIndex, const glm::vec4& color, types::boolean clearDepth, f32 depth)
    {
        _renderContext->BindRenderPass(renderPass);
        if (clearColor == K_TRUE)
            _renderContext->ClearFramebufferColor(bufferIndex, color);
        if (clearDepth == K_TRUE)
            _renderContext->ClearFramebufferDepth(depth);
        _renderContext->UnbindRenderPass(renderPass);
    }

    void mRender::ClearRenderPasses(const glm::vec4& clearColor, f32 clearDepth)
    {
        _renderContext->BindRenderPass(_opaque);
        _renderContext->ClearFramebufferColor(0, clearColor);
        _renderContext->ClearFramebufferDepth(clearDepth);
			
        _renderContext->BindRenderPass(_transparent);
        _renderContext->ClearFramebufferColor(0, glm::vec4(0.0f));
        _renderContext->ClearFramebufferColor(1, glm::vec4(1.0f));

        _renderContext->BindRenderPass(_text);
        _renderContext->ClearFramebufferColor(0, clearColor);
    }

    void mRender::UpdateLights()
    {
        _lightsByteSize = 16; // because vec4 (16 bytes) goes first (contains light count)
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

        _context->WriteBuffer(_lightBuffer, 0, _lightsByteSize, _lights);
    }

    void mRender::WriteObjectsToOpaqueBuffers(cIdVector<cGameObject>& objects, sRenderPass* renderPass)
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

        const std::vector<cTextureAtlasTexture*>& renderPassTextureAtlasTextures = renderPass->_desc._inputTextureAtlasTextures;
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

        _renderContext->WriteBuffer(_opaqueInstanceBuffer, 0, _opaqueInstancesByteSize, _opaqueInstances);
        _renderContext->WriteBuffer(_opaqueMaterialBuffer, 0, _opaqueMaterialsByteSize, _opaqueMaterials);
        _renderContext->WriteBuffer(_opaqueTextureAtlasTexturesBuffer, 0, _opaqueTextureAtlasTexturesByteSize, _opaqueTextureAtlasTextures);
    }

    void mRender::WriteObjectsToTransparentBuffers(cIdVector<cGameObject>& objects, sRenderPass* renderPass)
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

        const std::vector<cTextureAtlasTexture*>& renderPassTextureAtlasTextures = renderPass->_desc._inputTextureAtlasTextures;
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

        _renderContext->WriteBuffer(_transparentInstanceBuffer, 0, _transparentInstancesByteSize, _transparentInstances);
        _renderContext->WriteBuffer(_transparentMaterialBuffer, 0, _transparentMaterialsByteSize, _transparentMaterials);
        _renderContext->WriteBuffer(_transparentTextureAtlasTexturesBuffer, 0, _transparentTextureAtlasTexturesByteSize, _transparentTextureAtlasTextures);
    }

    void mRender::DrawGeometryOpaque(const sVertexBufferGeometry* geometry, const cGameObject* cameraObject, sRenderPass* renderPass)
    {
        if (renderPass == nullptr)
        {
            _renderContext->BindRenderPass(_opaque);
            _renderContext->SetShaderUniform(_opaque->_desc._shader, "ViewProjection", cameraObject->GetViewProjectionMatrix());
        }
        else
        {
            _renderContext->BindRenderPass(renderPass);
            _renderContext->SetShaderUniform(renderPass->_desc._shader, "ViewProjection", cameraObject->GetViewProjectionMatrix());
        }

        _renderContext->Draw(
            geometry->_indexCount,
            geometry->_offsetVertex,
            geometry->_offsetIndex,
            _opaqueInstanceCount
        );

        if (renderPass == nullptr)
            _renderContext->UnbindRenderPass(_opaque);
        else
            _renderContext->UnbindRenderPass(renderPass);
    }

    void mRender::DrawGeometryOpaque(const sVertexBufferGeometry* geometry, const cGameObject* cameraObject, sShader* singleShader)
    {
        _renderContext->BindRenderPass(_opaque, singleShader);

        if (singleShader == nullptr)
            _renderContext->SetShaderUniform(_opaque->_desc._shader, "ViewProjection", cameraObject->GetViewProjectionMatrix());
        else
            _renderContext->SetShaderUniform(singleShader, "ViewProjection", cameraObject->GetViewProjectionMatrix());
            
        _renderContext->Draw(
            geometry->_indexCount,
            geometry->_offsetVertex,
            geometry->_offsetIndex,
            _opaqueInstanceCount
        );

        _renderContext->UnbindRenderPass(_opaque);
    }

    void mRender::DrawGeometryTransparent(const sVertexBufferGeometry* geometry, const std::vector<cGameObject>& objects, const cGameObject* cameraObject, sRenderPass* renderPass)
    {
        if (renderPass == nullptr)
        {
            _renderContext->BindRenderPass(_transparent);
            _renderContext->SetShaderUniform(_transparent->_desc._shader, "ViewProjection", cameraObject->GetViewProjectionMatrix());
        }
        else
        {
            _renderContext->BindRenderPass(renderPass);
            _renderContext->SetShaderUniform(renderPass->_desc._shader, "ViewProjection", cameraObject->GetViewProjectionMatrix());
        }

        _renderContext->Draw(
            geometry->_indexCount,
            geometry->_offsetVertex,
            geometry->_offsetIndex,
            _transparentInstanceCount
        );

        if (renderPass == nullptr)
            _renderContext->UnbindRenderPass(_transparent);
        else
            _renderContext->UnbindRenderPass(renderPass);

        CompositeTransparent();
    }

    void mRender::DrawGeometryTransparent(const sVertexBufferGeometry* geometry, const cGameObject* cameraObject, sShader* singleShader)
    {
        _renderContext->BindRenderPass(_transparent, singleShader);

        if (singleShader != nullptr)
            _renderContext->SetShaderUniform(singleShader, "ViewProjection", cameraObject->GetViewProjectionMatrix());
        else
            _renderContext->SetShaderUniform(_transparent->_desc._shader, "ViewProjection", cameraObject->GetViewProjectionMatrix());

        _renderContext->Draw(
            geometry->_indexCount,
            geometry->_offsetVertex,
            geometry->_offsetIndex,
            _transparentInstanceCount
        );

        _renderContext->UnbindRenderPass(_transparent);
    }

    void mRender::DrawTexts(const std::vector<cGameObject>& objects)
    {
        for (auto& it : objects)
        {
            if (it.GetText() == nullptr)
                continue;

            const sText* text = it.GetText();
            const sFont* textFont = text->_font;
            const std::string& textString = text->_text;
            const auto& alphabet = textFont->_alphabet;
            const sTexture* atlas = text->_font->_atlas;

            _textInstancesByteSize = 0;
            _materialsMap->clear();

            const sTransform transform(&it);

            const glm::vec2 windowSize = GetApplication()->GetWindowSize();
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
                    offset.x += textFont->_offsetTab * textScale.x;
                    continue;
                }
                else if (glyphChar == '\n')
                {
                    offset.x = 0.0f;
                    offset.y -= textFont->_offsetNewline * textScale.y;
                    continue;
                }
                else if (glyphChar == ' ')
                {
                    offset.x += textFont->_offsetSpace * textScale.x;
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
                t._atlasInfo.x = (float)glyph._atlasXOffset / (float)atlas->_width;
                t._atlasInfo.y = (float)glyph._atlasYOffset / (float)atlas->_height;
                t._atlasInfo.z = (float)glyph._width / (float)atlas->_width;
                t._atlasInfo.w = (float)glyph._height / (float)atlas->_height;

                offset.x += glyph._advanceX * textScale.x;

                memcpy((void*)((usize)_textInstances + _textInstancesByteSize), &t, sizeof(sTextInstance));
                _textInstancesByteSize += sizeof(sTextInstance);

                actualCharCount += 1;
            }

            const cMaterialInstance mi(0, it.GetMaterial());
            memcpy(_textMaterials, &mi, sizeof(cMaterialInstance));
            _textMaterialsByteSize += sizeof(cMaterialInstance);

            _renderContext->WriteBuffer(_textInstanceBuffer, 0, _textInstancesByteSize, _textInstances);
            _renderContext->WriteBuffer(_textMaterialBuffer, 0, _textMaterialsByteSize, _textMaterials);

            _renderContext->BindRenderPass(_text);
            _renderContext->BindTexture(_text->_desc._shader, "FontAtlas", atlas, 0);
            _renderContext->DrawQuads(actualCharCount);
            _renderContext->UnbindRenderPass(_text);
        }
    }

    void mRender::CompositeTransparent()
    {
        _renderContext->BindRenderPass(_compositeTransparent);
        _renderContext->DrawQuad();
        _renderContext->UnbindRenderPass(_compositeTransparent);
    }

    void mRender::CompositeFinal()
    {
        _renderContext->BindRenderPass(_compositeFinal);
        _renderContext->DrawQuad();
        _renderContext->UnbindRenderPass(_compositeFinal);
			
        _renderContext->UnbindShader();
    }

    sPrimitive* mRender::CreatePrimitive(eCategory primitive)
    {
        cApplication* app = GetApplication();
        cMemoryPool* memoryPool = app->GetMemoryPool();

        sPrimitive* pPrimitiveObject = (sPrimitive*)app->GetMemoryPool()->Allocate(sizeof(sPrimitive));
        sPrimitive* primitiveObject = new (pPrimitiveObject) sPrimitive();

        if (primitive == eCategory::PRIMITIVE_TRIANGLE)
        {
            primitiveObject->_format = eCategory::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3;
            primitiveObject->_vertices = (sVertex*)memoryPool->Allocate(sizeof(sVertex) * 3);
            primitiveObject->_indices = (index*)memoryPool->Allocate(sizeof(index) * 3);
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
            primitiveObject->_vertices = (sVertex*)memoryPool->Allocate(sizeof(sVertex) * 4);
            primitiveObject->_indices = (index*)memoryPool->Allocate(sizeof(index) * 6);
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

    sModel* mRender::CreateModel(const std::string& filename)
    {
        cApplication* app = GetApplication();
        cMemoryPool* memoryPool = app->GetMemoryPool();

        // Create model
        sModel* pModel = (sModel*)app->GetMemoryPool()->Allocate(sizeof(sModel));
        sModel* model = new (pModel) sModel();

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
        model->_vertices = (sVertex*)memoryPool->Allocate(scene->mMeshes[0]->mNumVertices * sizeof(sVertex));
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
        model->_indices = (index*)memoryPool->Allocate(scene->mMeshes[0]->mNumFaces * 3 * sizeof(index));
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

    void mRender::DestroyPrimitive(sPrimitive* primitiveObject)
    {
        cMemoryPool* memoryPool = GetApplication()->GetMemoryPool();

        if (primitiveObject->_vertices)
            memoryPool->Free(primitiveObject->_vertices);
        if (primitiveObject->_indices)
            memoryPool->Free(primitiveObject->_indices);
        primitiveObject->~sPrimitive();
        memoryPool->Free(primitiveObject);
    }

    void mRender::LoadShaderFiles(const std::string& vertexFuncPath, const std::string& fragmentFuncPath, std::string& vertexFunc, std::string& fragmentFunc)
    {
        cApplication* app = GetApplication();
        sFile* vertexFuncFile = app->GetFileSystemManager()->CreateDataFile(vertexFuncPath, K_TRUE);
        sFile* fragmentFuncFile = app->GetFileSystemManager()->CreateDataFile(fragmentFuncPath, K_TRUE);
        vertexFunc = std::string((const char*)vertexFuncFile->_data);
        fragmentFunc = std::string((const char*)fragmentFuncFile->_data);
        app->GetFileSystemManager()->DestroyDataFile(vertexFuncFile);
        app->GetFileSystemManager()->DestroyDataFile(fragmentFuncFile);
    }

    void mRender::ResizeWindow(const glm::vec2& size)
    {
        _renderContext->UnbindRenderPass(_opaque);
        _renderContext->UnbindRenderPass(_transparent);

        _opaque->_desc._viewport[2] = size.x;
        _opaque->_desc._viewport[3] = size.y;
        _renderContext->ResizeRenderTargetColors(_opaque->_desc._renderTarget, size);
        _transparent->_desc._viewport[2] = size.x;
        _transparent->_desc._viewport[3] = size.y;
        _renderContext->ResizeRenderTargetColors(_transparent->_desc._renderTarget, size);
        _text->_desc._viewport[2] = size.x;
        _text->_desc._viewport[3] = size.y;
        _opaque->_desc._renderTarget->_depthAttachment = _renderContext->ResizeTexture(_opaque->_desc._renderTarget->_depthAttachment, size);
        _transparent->_desc._renderTarget->_depthAttachment = _opaque->_desc._renderTarget->_depthAttachment;
        _renderContext->UpdateRenderTargetBuffers(_opaque->_desc._renderTarget);
        _renderContext->UpdateRenderTargetBuffers(_transparent->_desc._renderTarget);

        _compositeTransparent->_desc._viewport[2] = size.x;
        _compositeTransparent->_desc._viewport[3] = size.y;
        _compositeTransparent->_desc._inputTextures[0] = _transparent->_desc._renderTarget->_colorAttachments[0];
        _compositeTransparent->_desc._inputTextureNames[0] = "AccumulationTexture";
        _compositeTransparent->_desc._inputTextures[1] = _transparent->_desc._renderTarget->_colorAttachments[1];
        _compositeTransparent->_desc._inputTextureNames[1] = "RevealageTexture";

        _compositeFinal->_desc._viewport[2] = size.x;
        _compositeFinal->_desc._viewport[3] = size.y;
        _compositeFinal->_desc._inputTextures[0] = _opaque->_desc._renderTarget->_colorAttachments[0];
        _compositeFinal->_desc._inputTextureNames[0] = "ColorTexture";
    }
}*/