
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RendererD3D.h: Defines a back-end specific class for the DirectX renderer.

#ifndef LIBANGLE_RENDERER_D3D_RENDERERD3D_H_
#define LIBANGLE_RENDERER_D3D_RENDERERD3D_H_

#include "libANGLE/Data.h"
#include "libANGLE/renderer/Renderer.h"
#include "libANGLE/renderer/d3d/formatutilsD3D.h"
#include "libANGLE/renderer/d3d/MemoryBuffer.h"
#include "libANGLE/renderer/d3d/d3d11/NativeWindow.h"

//FIXME(jmadill): std::array is currently prohibited by Chromium style guide
#include <array>

namespace gl
{
class InfoLog;
struct LinkedVarying;
class Texture;
}

namespace rx
{
class ImageD3D;
class IndexBuffer;
class RenderTargetD3D;
class ShaderExecutableD3D;
class SwapChainD3D;
class TextureStorage;
class UniformStorageD3D;
class VertexBuffer;

enum ShaderType
{
    SHADER_VERTEX,
    SHADER_PIXEL,
    SHADER_GEOMETRY
};

class RendererD3D : public Renderer
{
  public:
    explicit RendererD3D(egl::Display *display);
    virtual ~RendererD3D();

    static RendererD3D *makeRendererD3D(Renderer *renderer);

    virtual std::vector<ConfigDesc> generateConfigs() const = 0;

    gl::Error drawArrays(const gl::Data &data,
                         GLenum mode, GLint first,
                         GLsizei count, GLsizei instances) override;

    gl::Error drawElements(const gl::Data &data,
                           GLenum mode, GLsizei count, GLenum type,
                           const GLvoid *indices, GLsizei instances,
                           const RangeUI &indexRange) override;

    bool isDeviceLost() const override;
    std::string getVendorString() const override;

    virtual int getMinorShaderModel() const = 0;
    virtual std::string getShaderModelSuffix() const = 0;

    DisplayImpl *createDisplay() override;

    // Direct3D Specific methods
    virtual GUID getAdapterIdentifier() const = 0;

    virtual SwapChainD3D *createSwapChain(NativeWindow nativeWindow, HANDLE shareHandle, GLenum backBufferFormat, GLenum depthBufferFormat) = 0;

    virtual gl::Error generateSwizzle(gl::Texture *texture) = 0;
    virtual gl::Error setSamplerState(gl::SamplerType type, int index, gl::Texture *texture, const gl::SamplerState &sampler) = 0;
    virtual gl::Error setTexture(gl::SamplerType type, int index, gl::Texture *texture) = 0;

    virtual gl::Error setUniformBuffers(const gl::Buffer *vertexUniformBuffers[], const gl::Buffer *fragmentUniformBuffers[]) = 0;

    virtual gl::Error setRasterizerState(const gl::RasterizerState &rasterState) = 0;
    virtual gl::Error setBlendState(const gl::Framebuffer *framebuffer, const gl::BlendState &blendState, const gl::ColorF &blendColor,
                                    unsigned int sampleMask) = 0;
    virtual gl::Error setDepthStencilState(const gl::DepthStencilState &depthStencilState, int stencilRef,
                                           int stencilBackRef, bool frontFaceCCW) = 0;

    virtual void setScissorRectangle(const gl::Rectangle &scissor, bool enabled) = 0;
    virtual void setViewport(const gl::Rectangle &viewport, float zNear, float zFar, GLenum drawMode, GLenum frontFace,
                             bool ignoreViewport) = 0;

    virtual gl::Error applyRenderTarget(const gl::Framebuffer *frameBuffer) = 0;
    virtual gl::Error applyShaders(gl::Program *program, const gl::VertexFormat inputLayout[], const gl::Framebuffer *framebuffer,
                                   bool rasterizerDiscard, bool transformFeedbackActive) = 0;
    virtual gl::Error applyUniforms(const ProgramImpl &program, const std::vector<gl::LinkedUniform*> &uniformArray) = 0;
    virtual bool applyPrimitiveType(GLenum primitiveType, GLsizei elementCount, bool usesPointSize) = 0;
    virtual gl::Error applyVertexBuffer(const gl::State &state, GLint first, GLsizei count, GLsizei instances) = 0;
    virtual gl::Error applyIndexBuffer(const GLvoid *indices, gl::Buffer *elementArrayBuffer, GLsizei count, GLenum mode, GLenum type, TranslatedIndexData *indexInfo) = 0;
    virtual void applyTransformFeedbackBuffers(const gl::State& state) = 0;

    virtual void markAllStateDirty() = 0;

    virtual unsigned int getReservedVertexUniformVectors() const = 0;
    virtual unsigned int getReservedFragmentUniformVectors() const = 0;
    virtual unsigned int getReservedVertexUniformBuffers() const = 0;
    virtual unsigned int getReservedFragmentUniformBuffers() const = 0;
    virtual bool getShareHandleSupport() const = 0;
    virtual bool getPostSubBufferSupport() const = 0;

    // Pixel operations
    virtual gl::Error copyImage2D(const gl::Framebuffer *framebuffer, const gl::Rectangle &sourceRect, GLenum destFormat,
                                 const gl::Offset &destOffset, TextureStorage *storage, GLint level) = 0;
    virtual gl::Error copyImageCube(const gl::Framebuffer *framebuffer, const gl::Rectangle &sourceRect, GLenum destFormat,
                                    const gl::Offset &destOffset, TextureStorage *storage, GLenum target, GLint level) = 0;
    virtual gl::Error copyImage3D(const gl::Framebuffer *framebuffer, const gl::Rectangle &sourceRect, GLenum destFormat,
                                  const gl::Offset &destOffset, TextureStorage *storage, GLint level) = 0;
    virtual gl::Error copyImage2DArray(const gl::Framebuffer *framebuffer, const gl::Rectangle &sourceRect, GLenum destFormat,
                                       const gl::Offset &destOffset, TextureStorage *storage, GLint level) = 0;

    // RenderTarget creation
    virtual gl::Error createRenderTarget(int width, int height, GLenum format, GLsizei samples, RenderTargetD3D **outRT) = 0;

    // Shader operations
    virtual gl::Error loadExecutable(const void *function, size_t length, ShaderType type,
                                     const std::vector<gl::LinkedVarying> &transformFeedbackVaryings,
                                     bool separatedOutputBuffers, ShaderExecutableD3D **outExecutable) = 0;
    virtual gl::Error compileToExecutable(gl::InfoLog &infoLog, const std::string &shaderHLSL, ShaderType type,
                                          const std::vector<gl::LinkedVarying> &transformFeedbackVaryings,
                                          bool separatedOutputBuffers, D3DWorkaroundType workaround,
                                          ShaderExecutableD3D **outExectuable) = 0;
    virtual UniformStorageD3D *createUniformStorage(size_t storageSize) = 0;

    // Image operations
    virtual ImageD3D *createImage() = 0;
    virtual gl::Error generateMipmap(ImageD3D *dest, ImageD3D *source) = 0;
    virtual TextureStorage *createTextureStorage2D(SwapChainD3D *swapChain) = 0;
    virtual TextureStorage *createTextureStorage2D(GLenum internalformat, bool renderTarget, GLsizei width, GLsizei height, int levels, bool hintLevelZeroOnly) = 0;
    virtual TextureStorage *createTextureStorageCube(GLenum internalformat, bool renderTarget, int size, int levels, bool hintLevelZeroOnly) = 0;
    virtual TextureStorage *createTextureStorage3D(GLenum internalformat, bool renderTarget, GLsizei width, GLsizei height, GLsizei depth, int levels) = 0;
    virtual TextureStorage *createTextureStorage2DArray(GLenum internalformat, bool renderTarget, GLsizei width, GLsizei height, GLsizei depth, int levels) = 0;

    // Buffer-to-texture and Texture-to-buffer copies
    virtual bool supportsFastCopyBufferToTexture(GLenum internalFormat) const = 0;
    virtual gl::Error fastCopyBufferToTexture(const gl::PixelUnpackState &unpack, unsigned int offset, RenderTargetD3D *destRenderTarget,
                                              GLenum destinationFormat, GLenum sourcePixelsType, const gl::Box &destArea) = 0;

    virtual VertexConversionType getVertexConversionType(const gl::VertexFormat &vertexFormat) const = 0;
    virtual GLenum getVertexComponentType(const gl::VertexFormat &vertexFormat) const = 0;

    virtual VertexBuffer *createVertexBuffer() = 0;
    virtual IndexBuffer *createIndexBuffer() = 0;

    // Device lost
    void notifyDeviceLost() override;
    virtual bool resetDevice() = 0;

    gl::Error getScratchMemoryBuffer(size_t requestedSize, MemoryBuffer **bufferOut);

  protected:
    virtual gl::Error drawArrays(const gl::Data &data, GLenum mode, GLsizei count, GLsizei instances, bool transformFeedbackActive, bool usesPointSize) = 0;
    virtual gl::Error drawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices,
                                   gl::Buffer *elementArrayBuffer, const TranslatedIndexData &indexInfo, GLsizei instances) = 0;

    virtual bool getLUID(LUID *adapterLuid) const = 0;

    void cleanup();

    egl::Display *mDisplay;
    bool mDeviceLost;

  private:
    DISALLOW_COPY_AND_ASSIGN(RendererD3D);

    //FIXME(jmadill): std::array is currently prohibited by Chromium style guide
    typedef std::array<unsigned int, gl::IMPLEMENTATION_MAX_FRAMEBUFFER_ATTACHMENTS> FramebufferTextureSerialArray;

    gl::Error generateSwizzles(const gl::Data &data, gl::SamplerType type);
    gl::Error generateSwizzles(const gl::Data &data);

    gl::Error applyRenderTarget(const gl::Data &data, GLenum drawMode, bool ignoreViewport);
    gl::Error applyState(const gl::Data &data, GLenum drawMode);
    bool applyTransformFeedbackBuffers(const gl::Data &data);
    gl::Error applyShaders(const gl::Data &data, bool transformFeedbackActive);
    gl::Error applyTextures(const gl::Data &data, gl::SamplerType shaderType,
                            const FramebufferTextureSerialArray &framebufferSerials, size_t framebufferSerialCount);
    gl::Error applyTextures(const gl::Data &data);
    gl::Error applyUniformBuffers(const gl::Data &data);

    bool skipDraw(const gl::Data &data, GLenum drawMode);
    void markTransformFeedbackUsage(const gl::Data &data);

    size_t getBoundFramebufferTextureSerials(const gl::Data &data,
                                             FramebufferTextureSerialArray *outSerialArray);
    gl::Texture *getIncompleteTexture(GLenum type);

    gl::TextureMap mIncompleteTextures;
    MemoryBuffer mScratchMemoryBuffer;
    unsigned int mScratchMemoryBufferResetCounter;
};

struct dx_VertexConstants
{
    float depthRange[4];
    float viewAdjust[4];
    float viewCoords[4];
};

struct dx_PixelConstants
{
    float depthRange[4];
    float viewCoords[4];
    float depthFront[4];
};

}

#endif // LIBANGLE_RENDERER_D3D_RENDERERD3D_H_
