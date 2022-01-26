//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_RENDERER_D3D_MEMORYBUFFER_H_
#define LIBANGLE_RENDERER_D3D_MEMORYBUFFER_H_

#include "common/angleutils.h"

#include <cstddef>
#include <cstdint>

namespace rx
{

class MemoryBuffer
{
  public:
    MemoryBuffer();
    ~MemoryBuffer();

    bool resize(size_t size);
    size_t size() const;
    bool empty() const { return mSize == 0; }

    const uint8_t *data() const;
    uint8_t *data();

  private:
    DISALLOW_COPY_AND_ASSIGN(MemoryBuffer);

    size_t mSize;
    uint8_t *mData;
};

}

#endif // LIBANGLE_RENDERER_D3D_MEMORYBUFFER_H_
