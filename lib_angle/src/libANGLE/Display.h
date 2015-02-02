//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Display.h: Defines the egl::Display class, representing the abstract
// display on which graphics are drawn. Implements EGLDisplay.
// [EGL 1.4] section 2.1.2 page 3.

#ifndef LIBANGLE_DISPLAY_H_
#define LIBANGLE_DISPLAY_H_

#include <set>
#include <vector>

#include "libANGLE/Error.h"
#include "libANGLE/Config.h"
#include "libANGLE/AttributeMap.h"

namespace gl
{
class Context;
}

namespace rx
{
class DisplayImpl;
}

namespace egl
{
class Surface;

class Display final
{
  public:
    ~Display();

    Error initialize();
    void terminate();

    static egl::Display *getDisplay(EGLNativeDisplayType displayId, const AttributeMap &attribMap);

    static const ClientExtensions &getClientExtensions();
    static const std::string &getClientExtensionString();

    bool getConfigs(EGLConfig *configs, const EGLint *attribList, EGLint configSize, EGLint *numConfig);
    bool getConfigAttrib(EGLConfig config, EGLint attribute, EGLint *value);

    Error createWindowSurface(EGLNativeWindowType window, EGLConfig config, const EGLint *attribList, EGLSurface *outSurface);
    Error createOffscreenSurface(EGLConfig config, EGLClientBuffer shareHandle, const EGLint *attribList, EGLSurface *outSurface);
    Error createContext(EGLConfig configHandle, EGLint clientVersion, const gl::Context *shareContext, bool notifyResets,
                        bool robustAccess, EGLContext *outContext);

    void destroySurface(egl::Surface *surface);
    void destroyContext(gl::Context *context);

    bool isInitialized() const;
    bool isValidConfig(EGLConfig config);
    bool isValidContext(gl::Context *context);
    bool isValidSurface(egl::Surface *surface);
    bool hasExistingWindowSurface(EGLNativeWindowType window);
    bool isValidNativeWindow(EGLNativeWindowType window) const;
    bool isValidNativeDisplay(EGLNativeDisplayType display) const;

    rx::Renderer *getRenderer() { return mRenderer; };

    void notifyDeviceLost();

    const DisplayExtensions &getExtensions() const;
    const std::string &getExtensionString() const;
    const std::string &getVendorString() const;

  private:
    DISALLOW_COPY_AND_ASSIGN(Display);

    Display(EGLNativeDisplayType displayId);

    void setAttributes(const AttributeMap &attribMap);

    Error restoreLostDevice();

    void initDisplayExtensions();
    void initVendorString();

    rx::DisplayImpl *mImplementation;

    EGLNativeDisplayType mDisplayId;
    AttributeMap mAttributeMap;

    ConfigSet mConfigSet;

    typedef std::set<gl::Context*> ContextSet;
    ContextSet mContextSet;

    rx::Renderer *mRenderer;

    DisplayExtensions mDisplayExtensions;
    std::string mDisplayExtensionString;

    std::string mVendorString;
};

}

#endif   // LIBANGLE_DISPLAY_H_
