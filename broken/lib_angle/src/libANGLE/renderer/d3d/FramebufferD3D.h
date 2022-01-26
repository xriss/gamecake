//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FramebufferD3D.h: Defines the DefaultAttachmentD3D and FramebufferD3D classes.

#ifndef LIBANGLE_RENDERER_D3D_FRAMBUFFERD3D_H_
#define LIBANGLE_RENDERER_D3D_FRAMBUFFERD3D_H_

#include "libANGLE/renderer/FramebufferImpl.h"

#include <vector>
#include <cstdint>

namespace gl
{
struct ClearParameters;
class FramebufferAttachment;
struct PixelPackState;
}

namespace rx
{
class RenderTargetD3D;
class RendererD3D;

class DefaultAttachmentD3D : public DefaultAttachmentImpl
{
  public:
    DefaultAttachmentD3D(RenderTargetD3D *renderTarget);
    virtual ~DefaultAttachmentD3D();

    static DefaultAttachmentD3D *makeDefaultAttachmentD3D(DefaultAttachmentImpl* impl);

    virtual GLsizei getWidth() const override;
    virtual GLsizei getHeight() const override;
    virtual GLenum getInternalFormat() const override;
    virtual GLsizei getSamples() const override;

    RenderTargetD3D *getRenderTarget() const;

  private:
    RenderTargetD3D *mRenderTarget;
};

class FramebufferD3D : public FramebufferImpl
{
  public:
    FramebufferD3D(RendererD3D *renderer);
    virtual ~FramebufferD3D();

    void setColorAttachment(size_t index, const gl::FramebufferAttachment *attachment) override;
    void setDepthttachment(const gl::FramebufferAttachment *attachment) override;
    void setStencilAttachment(const gl::FramebufferAttachment *attachment) override;
    void setDepthStencilAttachment(const gl::FramebufferAttachment *attachment) override;

    void setDrawBuffers(size_t count, const GLenum *buffers) override;
    void setReadBuffer(GLenum buffer) override;

    gl::Error invalidate(size_t count, const GLenum *attachments) override;
    gl::Error invalidateSub(size_t count, const GLenum *attachments, const gl::Rectangle &area) override;

    gl::Error clear(const gl::State &state, GLbitfield mask) override;
    gl::Error clearBufferfv(const gl::State &state, GLenum buffer, GLint drawbuffer, const GLfloat *values) override;
    gl::Error clearBufferuiv(const gl::State &state, GLenum buffer, GLint drawbuffer, const GLuint *values) override;
    gl::Error clearBufferiv(const gl::State &state, GLenum buffer, GLint drawbuffer, const GLint *values) override;
    gl::Error clearBufferfi(const gl::State &state, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) override;

    GLenum getImplementationColorReadFormat() const override;
    GLenum getImplementationColorReadType() const override;
    gl::Error readPixels(const gl::State &state, const gl::Rectangle &area, GLenum format, GLenum type, GLvoid *pixels) const override;

    gl::Error blit(const gl::State &state, const gl::Rectangle &sourceArea, const gl::Rectangle &destArea,
                   GLbitfield mask, GLenum filter, const gl::Framebuffer *sourceFramebuffer) override;

    GLenum checkStatus() const override;

  protected:
    std::vector<const gl::FramebufferAttachment*> mColorBuffers;
    const gl::FramebufferAttachment *mDepthbuffer;
    const gl::FramebufferAttachment *mStencilbuffer;

    std::vector<GLenum> mDrawBuffers;
    GLenum mReadBuffer;

  private:
    RendererD3D *const mRenderer;

    virtual gl::Error clear(const gl::State &state, const gl::ClearParameters &clearParams) = 0;

    virtual gl::Error readPixels(const gl::Rectangle &area, GLenum format, GLenum type, size_t outputPitch,
                                 const gl::PixelPackState &pack, uint8_t *pixels) const = 0;

    virtual gl::Error blit(const gl::Rectangle &sourceArea, const gl::Rectangle &destArea, const gl::Rectangle *scissor,
                           bool blitRenderTarget, bool blitDepth, bool blitStencil, GLenum filter,
                           const gl::Framebuffer *sourceFramebuffer) = 0;

    virtual GLenum getRenderTargetImplementationFormat(RenderTargetD3D *renderTarget) const = 0;
};

gl::Error GetAttachmentRenderTarget(const gl::FramebufferAttachment *attachment, RenderTargetD3D **outRT);
unsigned int GetAttachmentSerial(const gl::FramebufferAttachment *attachment);

}

#endif // LIBANGLE_RENDERER_D3D_FRAMBUFFERD3D_H_
