// Title         viewOverride.h
// Summary       viewOverride Viewport 2.0 render override declaration
// Copyright     2020 Artineering and/or its licensors
// License       MIT

#pragma once
#include <vector>
#include <maya/MString.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MRenderTargetManager.h>

// Barebones override class derived from MRenderOverride
class viewOverride : public MHWRender::MRenderOverride
{
public:
    enum renderTargets {
        kColor = 0,
        kDepth,
        kNormals,
        kTargetCount
    };
    enum renderOperations {
        kSceneRender = 0,
        kQuadRender,
        kUIRender,
        kHUDRender,
        kPresentOp,
        kOperationCount
    };

    /// constructors and supported drawAPIs
	viewOverride( const MString & name );
	~viewOverride() override;
	MHWRender::DrawAPI supportedDrawAPIs() const override;

	// Basic setup and cleanup
	MStatus setup( const MString & destination ) override;
	MStatus cleanup() override;

	// Operation iteration methods
	bool startOperationIterator() override;
	MHWRender::MRenderOperation * renderOperation() override;
	bool nextRenderOperation() override;

	// UI name
	MString uiName() const override { return mUIName; }

    // custom functions
    void resetShaderInstances();
    void changeActiveTarget(unsigned int targetIdx);
    unsigned int activeTarget() { return mActiveTarget; };
    void showChannels(bool r, bool g, bool b, bool a);
protected:
    MString mEnvironment;
	MString mUIName;
    unsigned int mActiveTarget = 0;
    std::vector<float> mChannels = std::vector<float>{ 1.0f, 1.0f, 1.0f, 0.0f };

    // Operations and operation names
    MHWRender::MRenderOperation* mOperations[renderOperations::kOperationCount];
    bool mOperationEnabled[renderOperations::kOperationCount];
    int mCurrentOperation;

    // Render Targets
    MHWRender::MRenderTargetDescription mTargetDescriptions[kTargetCount];
    MHWRender::MRenderTarget *mTargets[kTargetCount];
    MStatus mUpdateRenderTargets();
};