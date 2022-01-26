//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceD3D.h: D3D implementation of an EGL surface

#ifndef LIBANGLE_RENDERER_D3D_SURFACED3D_H_
#define LIBANGLE_RENDERER_D3D_SURFACED3D_H_

#include "libANGLE/renderer/SurfaceImpl.h"
#include "libANGLE/renderer/d3d/d3d11/NativeWindow.h"

namespace egl
{
class Surface;
}

namespace rx
{
class SwapChainD3D;
class RendererD3D;

class SurfaceD3D : public SurfaceImpl
{
  public:
    static SurfaceD3D *createFromWindow(egl::Display *display, const egl::Config *config,
                                        EGLNativeWindowType window, EGLint fixedSize,
                                        EGLint width, EGLint height, EGLint postSubBufferSupported);
    static SurfaceD3D *createOffscreen(egl::Display *display, const egl::Config *config,
                                       EGLClientBuffer shareHandle, EGLint width, EGLint height,
                                       EGLenum textureFormat, EGLenum textureTarget);
    ~SurfaceD3D() override;
    void releaseSwapChain();

    static SurfaceD3D *makeSurfaceD3D(SurfaceImpl *impl);
    static SurfaceD3D *makeSurfaceD3D(egl::Surface *surface);

    egl::Error initialize() override;
    egl::Error swap() override;
    egl::Error postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height) override;
    egl::Error querySurfacePointerANGLE(EGLint attribute, void **value) override;
    egl::Error bindTexImage(EGLint buffer) override;
    egl::Error releaseTexImage(EGLint buffer) override;
    void setSwapInterval(EGLint interval) override;

    // D3D implementations (some virtual to hack across DLL boundaries)
    virtual SwapChainD3D *getSwapChain() const;

    egl::Error resetSwapChain();

    // Returns true if swapchain changed due to resize or interval update
    bool checkForOutOfDateSwapChain();

    EGLNativeWindowType getWindowHandle() const override;

  private:
    DISALLOW_COPY_AND_ASSIGN(SurfaceD3D);

    SurfaceD3D(egl::Display *display, const egl::Config *config, EGLint width, EGLint height,
               EGLint fixedSize, EGLint postSubBufferSupported, EGLenum textureFormat,
               EGLenum textureType, EGLClientBuffer shareHandle, EGLNativeWindowType window);
    egl::Error swapRect(EGLint x, EGLint y, EGLint width, EGLint height);
    egl::Error resetSwapChain(int backbufferWidth, int backbufferHeight);
    egl::Error resizeSwapChain(int backbufferWidth, int backbufferHeight);

    void subclassWindow();
    void unsubclassWindow();

    RendererD3D *mRenderer;

    SwapChainD3D *mSwapChain;
    bool mSwapIntervalDirty;
    bool mWindowSubclassed;        // Indicates whether we successfully subclassed mWindow for WM_RESIZE hooking

    NativeWindow mNativeWindow;   // Handler for the Window that the surface is created for.
};


}

#endif // LIBANGLE_RENDERER_D3D_SURFACED3D_H_
