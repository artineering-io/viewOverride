// Title         viewOverride.cpp
// Summary       viewOverride Viewport 2.0 render override
// Copyright     2020 Artineering and/or its licensors
// License       MIT

#include <maya/MGlobal.h>
#include <maya/MShaderManager.h>
#include "viewOverride.h"
#include "viewOverrideOperations.h"

/////////////////////////////////////////////////////////////////////
/// viewOverride Viewport 2.0 renderer
///
/// A simple Viewport 2.0 render override showing the object sorting
/// bug within Maya when rendering transparent objects to multiple
/// render targets.
///
/// The SceneRender renders all shaded items to three targets i.e.,
/// color, depth and normals. The quad operation is then in charge
/// of showing the different render targets through a command:
/// viewOverride -t 2;  // index of normals target
/// viewOverride -t 0;  // to revert to the the index of color target
///
/// As it can be seen, transparent objects are not sorted according
/// to depth, as opaque objects are. If the Scene Render is changed
/// to only output to two textures, the transparent objects are 
/// sorted correctly. To reproduce and test this, change line 51 in
/// viewOverrideOperations.cpp
///
/////////////////////////////////////////////////////////////////////

viewOverride::viewOverride(const MString & name)
    : MRenderOverride(name)
    , mUIName("View Override")
    , mCurrentOperation(-1)
{
    // set plugin environment (root folder in which plugin is located)
    MString command = "pluginInfo -query -path \"" + name + "\";";
    MString pluginDir = MGlobal::executeCommandStringResult(command);
    int mpos = pluginDir.rindexW("plug-ins");
    mEnvironment = pluginDir.substringW(0, mpos - 1);
    cout << "-> Plugin environment set to: " << mEnvironment << endl;

    // set shader path in plugin (shaders folder in plugin environment)
    const MHWRender::MShaderManager* shaderMgr = MHWRender::MRenderer::theRenderer()->getShaderManager();
    if (shaderMgr) {
        MString shaderPath = mEnvironment + "shaders/";
        //check if shaderPath has been included before
        MStringArray shaderPathsArray;
        shaderMgr->shaderPaths(shaderPathsArray);
        bool pathFound = false;
        for (unsigned int i = 0; i < shaderPathsArray.length(); i++) {
            if (shaderPathsArray[i] == shaderPath) {
                pathFound = true;
                cout << "-> Shader directory has previously been added" << endl;
                break;
            }
        }
        //add shader path if not found
        if (pathFound == false) {
            shaderMgr->addShaderPath(shaderPath);
            cout << "-> Shader directory added: " << shaderPath << endl;
        }
    }

    // initialize operations
    for (unsigned int i = 0; i < renderOperations::kOperationCount; i++) {
        mOperations[i] = nullptr;
    }
    cout << "Operations initialized" << endl;

    // initialize render targets
    unsigned int tWidth = 1;
    unsigned int tHeight = 1;
    int MSAA = 0;
    unsigned arraySliceCount = 1;
    bool isCubeMap = false;
    mTargetDescriptions[renderTargets::kColor] = MHWRender::MRenderTargetDescription("colorTarget", tWidth, tHeight, MSAA, MHWRender::kR16G16B16A16_FLOAT, arraySliceCount, isCubeMap);
    mTargetDescriptions[renderTargets::kDepth] = MHWRender::MRenderTargetDescription("depthTarget", tWidth, tHeight, MSAA, MHWRender::kD24S8, arraySliceCount, isCubeMap);
    mTargetDescriptions[renderTargets::kNormals] = MHWRender::MRenderTargetDescription("normalsTarget", tWidth, tHeight, MSAA, MHWRender::kR32G32B32A32_FLOAT, arraySliceCount, isCubeMap);
    // acquire render targets
    MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
    if (theRenderer) {
        const MRenderTargetManager *targetManager = theRenderer->getRenderTargetManager();
        if (targetManager) {
            for (unsigned int i = 0; i < renderTargets::kTargetCount; i++) {
                mTargets[i] = targetManager->acquireRenderTarget(mTargetDescriptions[i]);
            }
        }
    }
    cout << "Render targets initialized" << endl;
}

// On destruction all operations are deleted.
viewOverride::~viewOverride() {
    // get renderer and target manager
    MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
    const MHWRender::MRenderTargetManager* targetManager = theRenderer->getRenderTargetManager();
    // delete operations
	for (unsigned int i=0; i<renderOperations::kOperationCount; i++){
		if (mOperations[i]) {
			delete mOperations[i];
			mOperations[i] = nullptr;
		}
	}
    // delete targets
    for (unsigned int i = 0; i < renderTargets::kTargetCount; i++) {
        if (mTargets[i]) {
            if (targetManager) {
                targetManager->releaseRenderTarget(mTargets[i]);
            }
        }
    }
}
	
// Drawing uses all internal code so will support all draw APIs
//
MHWRender::DrawAPI viewOverride::supportedDrawAPIs() const {
	return MHWRender::kAllDevices;
}

// Basic iterator methods which returns a list of operations in order
// The operations are not executed at this time only queued for execution
//
// - startOperationIterator() : to start iterating
// - renderOperation() : will be called to return the current operation
// - nextRenderOperation() : when this returns false we've returned all operations
//
bool viewOverride::startOperationIterator() {
	mCurrentOperation = 0;
	return true;
}

MHWRender::MRenderOperation*
viewOverride::renderOperation() {
	if (mCurrentOperation >= 0 && mCurrentOperation < renderOperations::kOperationCount) {
		if (mOperations[mCurrentOperation]) {
			return mOperations[mCurrentOperation];
		}
	}
	return NULL;
}

bool viewOverride::nextRenderOperation() {
	mCurrentOperation++;
	if (mCurrentOperation < renderOperations::kOperationCount) {
		return true;
	}
	return false;
}

void viewOverride::resetShaderInstances() {
    for (unsigned i = 0; i < renderOperations::kOperationCount; i++) {
        if (mOperations[i]->operationType() == MRenderOperation::kQuadRender) {
            static_cast<QuadRender*>(mOperations[i])->clearShaderInstance();
        }
    }
}

void viewOverride::changeActiveTarget(unsigned int targetIdx) {
    if (targetIdx < renderTargets::kTargetCount) {
        mActiveTarget = targetIdx;
    }
}

void viewOverride::showChannels(bool r, bool g, bool b, bool a) {

}


// Updates the render targets based on the current frame context (viewport)
MStatus viewOverride::mUpdateRenderTargets(){
    const MFrameContext *frameContext = this->getFrameContext();
    
    int x, y, width, height;
    frameContext->getViewportDimensions(x, y, width, height);
    for (unsigned int i = 0; i < renderTargets::kTargetCount; i++) {
        mTargetDescriptions[i].setWidth(width);
        mTargetDescriptions[i].setHeight(height);
        mTargets[i]->updateDescription(mTargetDescriptions[i]);
    }

    return MS::kSuccess;
}

// setup() runs every frame and we can make sure that the rendering
// pipeline is properly set up and ready for rendering
//
//	- One scene render operation to draw the scene.
//  - One quad operator to debug the scene render targets
//	- One HUD render operation to draw the HUD over the scene
//	- One presentation operation to be able to see the results in the viewport
MStatus viewOverride::setup( const MString & destination ) {
	MHWRender::MRenderer *theRenderer = MHWRender::MRenderer::theRenderer();
	if (!theRenderer)
		return MStatus::kFailure;

    // setup targets
    mUpdateRenderTargets();

	// Create a new set of operations as required
    if (!mOperations[0]) {
        cout << "Defining render operations" << endl;
        // Scene Operations
        mOperations[0] = new SceneRender("viewOverride_Scene",
            MHWRender::MSceneRender::kRenderShadedItems,
            MHWRender::MClearOperation::kClearAll);
        SceneRender * sceneOp = dynamic_cast<SceneRender*>(mOperations[0]);
        if (sceneOp) {
            sceneOp->setTargetOverride(0, mTargets[renderTargets::kColor]);
            sceneOp->setTargetOverride(1, mTargets[renderTargets::kDepth]);
            sceneOp->setTargetOverride(2, mTargets[renderTargets::kNormals]);
        }
        // Quad Operations
        mOperations[1] = new QuadRender("viewOverride_Quad", "quadDebug", "debug");
        QuadRender * quadOp = dynamic_cast<QuadRender*>(mOperations[1]);
        if (quadOp) {
            quadOp->setTargetOverride(0, mTargets[renderTargets::kColor]);
            quadOp->setTargetOverride(1, mTargets[renderTargets::kDepth]);
            //quadOp->setEnabled(false);
        }
        // Scene UI Operation
        mOperations[2] = new SceneRender("viewOverride_Scene_UI",
            MHWRender::MSceneRender::kRenderUIItems,
            MHWRender::MClearOperation::kClearNone);
        sceneOp = dynamic_cast<SceneRender*>(mOperations[2]);
        if (sceneOp) {
            sceneOp->setTargetOverride(0, mTargets[renderTargets::kColor]);
            sceneOp->setTargetOverride(1, mTargets[renderTargets::kDepth]);
        }
        // HUD Operation
        MString API = MGlobal::executeCommandStringResult("optionVar -q vp2RenderingEngine");
        mOperations[3] = new HUDOperation(mUIName + " - " + API);
        HUDOperation * hudOp = dynamic_cast<HUDOperation*>(mOperations[3]);
        if (hudOp) {
            hudOp->setTargetOverride(0, mTargets[renderTargets::kColor]);
            hudOp->setTargetOverride(1, mTargets[renderTargets::kDepth]);
        }
        // Present Operation
        mOperations[4] = new PresentTarget("viewOverride_Present");
        PresentTarget * presentOp = dynamic_cast<PresentTarget*>(mOperations[4]);
        if (presentOp) {
            presentOp->setTargetOverride(0, mTargets[renderTargets::kColor]);
            presentOp->setTargetOverride(1, mTargets[renderTargets::kDepth]);
        }
        cout << "Render operations defined successfully" << endl;
    }
    // Test if all operations are initialized successfully
    for (unsigned int i = 0; i < renderOperations::kOperationCount; i++) {
        if (!mOperations[i]) {
            cerr << "mOperation[" << i << "] could not be initialized correctly" << endl;
            return MStatus::kFailure;
        }
	}
    // update shaders
    QuadRender * quadOp = (QuadRender*)mOperations[renderOperations::kQuadRender];
    // set shader parameters
    MShaderInstance *shader = quadOp->shaderInstance();
    if (shader) {
        MRenderTargetAssignment targetAssignment{ mTargets[mActiveTarget] };
        shader->setParameter("gInputTex", targetAssignment);
        shader->setParameter("gColorChannels", mChannels[0]);
    }

    /*
    /// testing
    MStringArray oT = mOperations[0]->outputTargets();
    cout << "Scene render outputs to: " << endl;
    for (unsigned int i = 0; i < oT.length(); i++) {
        cout << oT[i] << endl;
    }
    */
	return MStatus::kSuccess;
}

// On cleanup we just return for returning the list of operations for
// the next render
//
MStatus viewOverride::cleanup() {
	mCurrentOperation = -1;
	return MStatus::kSuccess;
}