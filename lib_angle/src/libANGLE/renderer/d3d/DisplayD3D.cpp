//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayD3D.cpp: D3D implementation of egl::Display

#include "libANGLE/renderer/d3d/DisplayD3D.h"

#include "libANGLE/Surface.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"
#include "libANGLE/renderer/d3d/SurfaceD3D.h"
#include "libANGLE/renderer/d3d/SwapChainD3D.h"

namespace rx
{

SurfaceImpl *DisplayD3D::createWindowSurface(egl::Display *display, const egl::Config *config,
                                             EGLNativeWindowType window, EGLint fixedSize,
                                             EGLint width, EGLint height, EGLint postSubBufferSupported)
{
    return SurfaceD3D::createFromWindow(display, config, window, fixedSize,
                                        width, height, postSubBufferSupported);
}

SurfaceImpl *DisplayD3D::createOffscreenSurface(egl::Display *display, const egl::Config *config,
                                                EGLClientBuffer shareHandle, EGLint width, EGLint height,
                                                EGLenum textureFormat, EGLenum textureTarget)
{
    return SurfaceD3D::createOffscreen(display, config, shareHandle,
                                       width, height, textureFormat, textureTarget);
}

DisplayD3D::DisplayD3D(rx::RendererD3D *renderer)
    : mRenderer(renderer)
{
}

std::vector<ConfigDesc> DisplayD3D::generateConfigs() const
{
    return mRenderer->generateConfigs();
}

egl::Error DisplayD3D::restoreLostDevice()
{
    // Release surface resources to make the Reset() succeed
    for (auto &surface : mSurfaceSet)
    {
        if (surface->getBoundTexture())
        {
            surface->releaseTexImage(EGL_BACK_BUFFER);
        }
        SurfaceD3D *surfaceD3D = SurfaceD3D::makeSurfaceD3D(surface);
        surfaceD3D->releaseSwapChain();
    }

    if (!mRenderer->resetDevice())
    {
        return egl::Error(EGL_BAD_ALLOC);
    }

    // Restore any surfaces that may have been lost
    for (const auto &surface : mSurfaceSet)
    {
        SurfaceD3D *surfaceD3D = SurfaceD3D::makeSurfaceD3D(surface);

        egl::Error error = surfaceD3D->resetSwapChain();
        if (error.isError())
        {
            return error;
        }
    }

    return egl::Error(EGL_SUCCESS);
}

bool DisplayD3D::isValidNativeWindow(EGLNativeWindowType window) const
{
    return NativeWindow::isValidNativeWindow(window);
}

void DisplayD3D::generateExtensions(egl::DisplayExtensions *outExtensions) const
{
    outExtensions->createContextRobustness = true;

    // ANGLE-specific extensions
    if (mRenderer->getShareHandleSupport())
    {
        outExtensions->d3dShareHandleClientBuffer = true;
        outExtensions->surfaceD3DTexture2DShareHandle = true;
    }

    outExtensions->querySurfacePointer = true;
    outExtensions->windowFixedSize = true;

    if (mRenderer->getPostSubBufferSupport())
    {
        outExtensions->postSubBuffer = true;
    }

    outExtensions->createContext = true;
}

}
