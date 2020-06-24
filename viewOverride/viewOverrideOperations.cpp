// Title         viewOverrideOperations.cpp
// Summary       viewOverride Viewport 2.0 render override operations
// Copyright     2020 Artineering and/or its licensors
// License       MIT

#include <maya/MShaderManager.h>
#include "viewOverrideOperations.h"

/////////////////////////////////////////////////////////////////////
/// Definition of render operations used in ViewOverride
///
/// 1. Scene render that can render to up to 3 targets
/// 2. Quad render with custom quad shaders
/// 3. Heads Up Display (HUD) operation
/// 4. Present operation
///
/// Of special interest to troubleshoot the transparency object
/// sorting  when rendering to multiple render targets is line 51.
/// Change from listSize 3 to 2 and vice-versa to reproduce the bug
///
/////////////////////////////////////////////////////////////////////

// SCENE RENDER
SceneRender::SceneRender(const MString& name,
    MHWRender::MSceneRender::MSceneFilterOption sceneFilter, unsigned int clearMask)
    : MSceneRender(name)
    , mSceneRenderFilter(sceneFilter){

    // set up the clear operation (mClearOperation)
    float clearColor[4] = { 0.0, 0.0 , 0.0, 0.0 };  // black clear color
    mClearOperation.setClearColor(clearColor);      // set clear color
    mClearOperation.setMask(clearMask);             // set mask
}

SceneRender::~SceneRender() {}

void SceneRender::setTargetOverride(unsigned int i, MHWRender::MRenderTarget *target) {
    if (i < 3) {
        if (target) {
            mTargets[i] = target;
        }
    }
}

MHWRender::MRenderTarget * const * SceneRender::targetOverrideList(unsigned int & listSize) {
    if (mTargets) {
        if (mSceneRenderFilter == MHWRender::MSceneRender::kRenderUIItems) {
            listSize = 2;
            return &mTargets[0];
        } else {
            listSize = 3;
            return &mTargets[0];
        }
    }
    listSize = 0;
    return nullptr;
}

MHWRender::MClearOperation& SceneRender::clearOperation() {
    return mClearOperation;  // value set during construction
}

MHWRender::MSceneRender::MSceneFilterOption SceneRender::renderFilterOverride() {
    return mSceneRenderFilter;  // value set during construction
}

// QUAD RENDER
QuadRender::QuadRender(const MString & name, const MString &shaderFileName, const MString &techniqueName) :
    MQuadRender(name),
    mShaderFileName(shaderFileName),
    mTechniqueName(techniqueName) {
    mClearOperation.setClearGradient(false);
    mClearOperation.setMask(MHWRender::MClearOperation::kClearNone);
}

QuadRender::~QuadRender() {
    clearShaderInstance();
}

const MHWRender::MShaderInstance * QuadRender::shader() {
    if (!mShaderInstance) {
        const MHWRender::MShaderManager* shaderMgr = MHWRender::MRenderer::theRenderer()->getShaderManager();
        mShaderInstance = shaderMgr->getEffectsFileShader(mShaderFileName, mTechniqueName, 0, 0, true);
        if (!mShaderInstance) {
            cerr << mShaderFileName << " could not be initialized" << endl;
        }
    }
    return mShaderInstance;
}

void QuadRender::clearShaderInstance() {
    mShaderInstance = nullptr;
    const MHWRender::MShaderManager* shaderMgr = MHWRender::MRenderer::theRenderer()->getShaderManager();
    shaderMgr->removeEffectFromCache(mShaderFileName, mTechniqueName, 0, 0);
}

void QuadRender::setTargetOverride(unsigned int i, MHWRender::MRenderTarget *target) {
    if (i < 2) {
        if (target) {
            mTargets[i] = target;
        }
    }
}

MHWRender::MRenderTarget * const * QuadRender::targetOverrideList(unsigned int & listSize) {
    if (mTargets) {
        listSize = 2;
        return &mTargets[0];
    }
    listSize = 0;
    return nullptr;
}

// HUD RENDER
HUDOperation::HUDOperation(const MString & rendererName) : mRendererName(rendererName) {
        mPreviousFrame = std::chrono::high_resolution_clock::now();
        mFrameAverage = 0;
        mFrameAccu = 0;
        mTimeAccu = 0LL;
        strcpy(mHUDStatsBuffer, "");
}

HUDOperation::~HUDOperation() {}

bool HUDOperation::hasUIDrawables() const { return true; }

void HUDOperation::addUIDrawables(MHWRender::MUIDrawManager& drawManager2D, const MHWRender::MFrameContext& frameContext) {
    // get viewport information
    int x = 0, y = 0, w = 0, h = 0;
    frameContext.getViewportDimensions(x, y, w, h);

    // setup draw
    drawManager2D.beginDrawable();
    drawManager2D.setColor(MColor(0.3f, 0.3f, 0.3f));
    drawManager2D.setFontSize(MHWRender::MUIDrawManager::kSmallFontSize);

    // draw renderer name
    drawManager2D.text(MPoint(w*0.01f, h*0.97f), mRendererName, MHWRender::MUIDrawManager::kLeft);

    // draw viewport size and FPS information
    mCurrentFrame = std::chrono::high_resolution_clock::now();
    frameDuration = (unsigned int)std::chrono::duration_cast<std::chrono::microseconds>(mCurrentFrame - mPreviousFrame).count();
    mTimeAccu += frameDuration;
    mFrameAccu++;
    if (mTimeAccu > 1000000) {
        mFrameAverage = mFrameAccu;
        mDurationAverage = mTimeAccu / mFrameAccu;
        sprintf(mHUDStatsBuffer, "Resolution [%d, %d]      FPS: %d -> each frame: %d us", w, h, mFrameAverage, mDurationAverage);
        // reset values
        mFrameAccu = 0; mTimeAccu = 0;
    }
    drawManager2D.text(MPoint(w*0.01f, h*0.95f), mHUDStatsBuffer, MHWRender::MUIDrawManager::kLeft);
    mPreviousFrame = mCurrentFrame;

    // end draw UI
    drawManager2D.endDrawable();
}

void HUDOperation::setTargetOverride(unsigned int i, MHWRender::MRenderTarget *target) {
    if (i < 2) {
        if (target) {
            mTargets[i] = target;
        }
    }
}

MHWRender::MRenderTarget* const* HUDOperation::targetOverrideList(unsigned int &listSize) {
    if (mTargets) {
        listSize = 2;
        return &mTargets[0];
    }
    listSize = 0;
    return nullptr;
}


/// PRESENT TARGET
MHWRender::MRenderTarget* const* PresentTarget::targetOverrideList(unsigned int &listSize) {
    if (mTargets) {
        listSize = 2;
        return mTargets;
    }
    listSize = 0;
    return nullptr;
}

void PresentTarget::setTargetOverride(unsigned int i, MHWRender::MRenderTarget *target) {
    if (i < 2) {
        if (target) {
            mTargets[i] = target;
        }
    }
}
