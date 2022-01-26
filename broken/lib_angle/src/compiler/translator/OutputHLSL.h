//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_OUTPUTHLSL_H_
#define COMPILER_TRANSLATOR_OUTPUTHLSL_H_

#include <list>
#include <set>
#include <map>
#include <stack>

#include "angle_gl.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/ParseContext.h"

class BuiltInFunctionEmulatorHLSL;

namespace sh
{
class UnfoldShortCircuit;
class StructureHLSL;
class UniformHLSL;

typedef std::map<TString, TIntermSymbol*> ReferencedSymbols;

class OutputHLSL : public TIntermTraverser
{
  public:
    OutputHLSL(TParseContext &context, TranslatorHLSL *parentTranslator);
    ~OutputHLSL();

    void output();

    const std::map<std::string, unsigned int> &getInterfaceBlockRegisterMap() const;
    const std::map<std::string, unsigned int> &getUniformRegisterMap() const;

    static TString initializer(const TType &type);

    TInfoSinkBase &getInfoSink() { ASSERT(!mInfoSinkStack.empty()); return *mInfoSinkStack.top(); }

  protected:
    void header(const BuiltInFunctionEmulatorHLSL *builtInFunctionEmulator);

    // Visit AST nodes and output their code to the body stream
    void visitSymbol(TIntermSymbol*);
    void visitRaw(TIntermRaw*);
    void visitConstantUnion(TIntermConstantUnion*);
    bool visitBinary(Visit visit, TIntermBinary*);
    bool visitUnary(Visit visit, TIntermUnary*);
    bool visitSelection(Visit visit, TIntermSelection*);
    bool visitAggregate(Visit visit, TIntermAggregate*);
    bool visitLoop(Visit visit, TIntermLoop*);
    bool visitBranch(Visit visit, TIntermBranch*);

    void traverseStatements(TIntermNode *node);
    bool isSingleStatement(TIntermNode *node);
    bool handleExcessiveLoop(TIntermLoop *node);
    void outputTriplet(Visit visit, const TString &preString, const TString &inString, const TString &postString);
    void outputLineDirective(int line);
    TString argumentString(const TIntermSymbol *symbol);
    int vectorSize(const TType &type) const;

    void outputConstructor(Visit visit, const TType &type, const TString &name, const TIntermSequence *parameters);
    const ConstantUnion *writeConstantUnion(const TType &type, const ConstantUnion *constUnion);

    void writeEmulatedFunctionTriplet(Visit visit, const char *preStr);
    void makeFlaggedStructMaps(const std::vector<TIntermTyped *> &flaggedStructs);

    // Returns true if it found a 'same symbol' initializer (initializer that references the variable it's initting)
    bool writeSameSymbolInitializer(TInfoSinkBase &out, TIntermSymbol *symbolNode, TIntermTyped *expression);
    void writeDeferredGlobalInitializers(TInfoSinkBase &out);

    TParseContext &mContext;
    const ShShaderOutput mOutputType;
    UnfoldShortCircuit *mUnfoldShortCircuit;
    bool mInsideFunction;

    // Output streams
    TInfoSinkBase mHeader;
    TInfoSinkBase mBody;
    TInfoSinkBase mFooter;

    // A stack is useful when we want to traverse in the header, or in helper functions, but not always
    // write to the body. Instead use an InfoSink stack to keep our current state intact.
    std::stack<TInfoSinkBase *> mInfoSinkStack;

    ReferencedSymbols mReferencedUniforms;
    ReferencedSymbols mReferencedInterfaceBlocks;
    ReferencedSymbols mReferencedAttributes;
    ReferencedSymbols mReferencedVaryings;
    ReferencedSymbols mReferencedOutputVariables;

    StructureHLSL *mStructureHLSL;
    UniformHLSL *mUniformHLSL;

    struct TextureFunction
    {
        enum Method
        {
            IMPLICIT,   // Mipmap LOD determined implicitly (standard lookup)
            BIAS,
            LOD,
            LOD0,
            LOD0BIAS,
            SIZE,   // textureSize()
            FETCH,
            GRAD
        };

        TBasicType sampler;
        int coords;
        bool proj;
        bool offset;
        Method method;

        TString name() const;

        bool operator<(const TextureFunction &rhs) const;
    };

    typedef std::set<TextureFunction> TextureFunctionSet;

    // Parameters determining what goes in the header output
    TextureFunctionSet mUsesTexture;
    bool mUsesFragColor;
    bool mUsesFragData;
    bool mUsesDepthRange;
    bool mUsesFragCoord;
    bool mUsesPointCoord;
    bool mUsesFrontFacing;
    bool mUsesPointSize;
    bool mUsesFragDepth;
    bool mUsesXor;
    bool mUsesDiscardRewriting;
    bool mUsesNestedBreak;

    int mNumRenderTargets;

    int mUniqueIndex;   // For creating unique names

    bool mContainsLoopDiscontinuity;
    bool mContainsAnyLoop;
    bool mOutputLod0Function;
    bool mInsideDiscontinuousLoop;
    int mNestedLoopDepth;

    TIntermSymbol *mExcessiveLoopIndex;

    TString structInitializerString(int indent, const TStructure &structure, const TString &rhsStructName);

    std::map<TIntermTyped*, TString> mFlaggedStructMappedNames;
    std::map<TIntermTyped*, TString> mFlaggedStructOriginalNames;

    // Some initializers use varyings, uniforms or attributes, thus we can't evaluate some variables
    // at global static scope in HLSL. These variables depend on values which we retrieve from the
    // shader input structure, which we set in the D3D main function. Instead, we can initialize
    // these static globals after we initialize our other globals.
    std::vector<std::pair<TIntermSymbol*, TIntermTyped*>> mDeferredGlobalInitializers;
};

}

#endif   // COMPILER_TRANSLATOR_OUTPUTHLSL_H_
