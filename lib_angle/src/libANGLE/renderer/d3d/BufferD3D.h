//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BufferD3D.h: Defines the rx::BufferD3D class, an implementation of BufferImpl.

#ifndef LIBANGLE_RENDERER_D3D_BUFFERD3D_H_
#define LIBANGLE_RENDERER_D3D_BUFFERD3D_H_

#include "libANGLE/renderer/BufferImpl.h"
#include "libANGLE/angletypes.h"

#include <stdint.h>

namespace rx
{
class RendererD3D;
class StaticIndexBufferInterface;
class StaticVertexBufferInterface;

class BufferD3D : public BufferImpl
{
  public:
    BufferD3D();
    virtual ~BufferD3D();

    static BufferD3D *makeBufferD3D(BufferImpl *buffer);
    static BufferD3D *makeFromBuffer(gl::Buffer *buffer);

    unsigned int getSerial() const { return mSerial; }

    virtual size_t getSize() const = 0;
    virtual bool supportsDirectBinding() const = 0;
    virtual RendererD3D *getRenderer() = 0;

    StaticVertexBufferInterface *getStaticVertexBuffer() { return mStaticVertexBuffer; }
    StaticIndexBufferInterface *getStaticIndexBuffer() { return mStaticIndexBuffer; }

    void initializeStaticData();
    void invalidateStaticData();
    void promoteStaticUsage(int dataSize);

  protected:
    unsigned int mSerial;
    static unsigned int mNextSerial;

    void updateSerial();

    StaticVertexBufferInterface *mStaticVertexBuffer;
    StaticIndexBufferInterface *mStaticIndexBuffer;
    unsigned int mUnmodifiedDataUse;
};

}

#endif // LIBANGLE_RENDERER_D3D_BUFFERD3D_H_
