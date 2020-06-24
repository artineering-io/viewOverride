// Title         viewOverrideOperations.h
// Summary       viewOverride Viewport 2.0 operations declaration
// Copyright     2020 Artineering and/or its licensors
// License       MIT

#pragma once
#include <chrono>
#include <maya/MViewport2Renderer.h>

/// Declaration of all override operations
/// 1. SceneRender
/// 2. QuadRender
/// 3. HUDRender
/// 4. PresentTarget
///===========================================
class SceneRender : public MHWRender::MSceneRender {
public:
    SceneRender(const MString &name,
        MHWRender::MSceneRender::MSceneFilterOption sceneFilter,
        unsigned int clearMask);
    ~SceneRender();

    /// set custom render target list
    void setTargetOverride(unsigned int i, MHWRender::MRenderTarget* target);
    /// return custom render target list
    MHWRender::MRenderTarget* const* targetOverrideList(unsigned int &listSize) override;
    /// set a custom clear operation before the scene render starts
    MHWRender::MClearOperation& SceneRender::clearOperation() override;
    /// set a custom scene filter (e.g., opaque, transparent)
    MHWRender::MSceneRender::MSceneFilterOption SceneRender::renderFilterOverride() override;

protected:
    MHWRender::MSceneRender::MSceneFilterOption mSceneRenderFilter;  ///< scene draw filter override (onlyShaded, etc)
    MHWRender::MRenderTarget* mTargets[3];  ///< target list that is presented on the viewport
};


class QuadRender : public MHWRender::MQuadRender {
public:
    QuadRender(const MString &name, const MString &shaderFileName, const MString &techniqueName);
    ~QuadRender();

    /// custom quad shader
    virtual const MHWRender::MShaderInstance* shader();
    MHWRender::MShaderInstance* shaderInstance() {
        return mShaderInstance;
    }
    void clearShaderInstance();
    /// set custom render target list
    void setTargetOverride(unsigned int i, MHWRender::MRenderTarget* target);
    /// set custom render target
    virtual MHWRender::MRenderTarget* const* targetOverrideList(unsigned int &listSize);

protected:
    MString mShaderFileName;
    MString mTechniqueName;
    MHWRender::MShaderInstance* mShaderInstance = nullptr;     ///< shader instance
    MHWRender::MRenderTarget* mTargets[2];  ///< target list that is presented on the viewport
};


class HUDOperation : public MHWRender::MHUDRender {
public:
    HUDOperation(const MString& t_rendererName);
    ~HUDOperation();

    virtual bool hasUIDrawables() const;  ///< enables UI drawables
    virtual void addUIDrawables(MHWRender::MUIDrawManager& drawManager2D, const MHWRender::MFrameContext& frameContext);  ///< sets up custom HUD elements

    /// set custom render target list
    void setTargetOverride(unsigned int i, MHWRender::MRenderTarget* target);
    virtual MHWRender::MRenderTarget* const* targetOverrideList(unsigned int &listSize);  ///< targets to render operation to

protected:
    const MString mRendererName;			   ///< render override name
    MHWRender::MRenderTarget* mTargets[2];  ///< target list that is presented on the viewport

    /// variables for time statistics
    std::chrono::high_resolution_clock::time_point mPreviousFrame;
    std::chrono::high_resolution_clock::time_point mCurrentFrame;
    unsigned int mTimeAccu;
    unsigned int mFrameAccu;
    unsigned int frameDuration;
    unsigned int mFrameAverage;
    unsigned int mDurationAverage;
    char mHUDStatsBuffer[120];
};


class PresentTarget : public MHWRender::MPresentTarget {
public:
    PresentTarget(const MString &t_name) : MPresentTarget(t_name) { }
    ~PresentTarget() {}

    /// set custom render target list
    void setTargetOverride(unsigned int i, MHWRender::MRenderTarget* target);
    /// target override list
    MHWRender::MRenderTarget* const* targetOverrideList(unsigned int &listSize);

protected:
    MHWRender::MRenderTarget* mTargets[2];  ///< target list that is presented on the viewport
};