// Title         viewOverrideCmd.cpp
// Summary       viewOverride Viewport 2.0 render override command
// Copyright     2020 Artineering and/or its licensors
// License       MIT

#include "viewOverrideCmd.h"
#include "viewOverride.h"

/////////////////////////////////////////////////////////////////////
/// Available commands for viewOverride
///
/// viewOverride -t unsigned int
///     changes the active target to show into another
///
/// viewOverride -r
///     refreshes the shaders in the quadRender
///
/// viewOverride -c bool bool bool bool
///     modifies the channels (RGBA) to show
///
/////////////////////////////////////////////////////////////////////

// argument strings
const char *targetSN = "-t";
const char *targetLN = "-target";
const char *refreshSN = "-r";
const char *refreshLN = "-refresh";
const char *channelsSN = "-c";
const char *channelsLN = "-channel";


/// constructor and destructor
Cmd::Cmd() {};
Cmd::~Cmd() {};


/// command creator
void* Cmd::creator() {
    return new Cmd();
};


/// define syntax of command
MSyntax Cmd::newSyntax() {
    MSyntax syntax;
    syntax.enableQuery(true);  // enable query of flags
    syntax.enableEdit(false);  // all flags will "edit" by default
    // get target flag
    syntax.addFlag(targetSN, targetLN, MSyntax::kUnsigned);
    // get refresh flag
    syntax.addFlag(refreshSN, refreshLN, MSyntax::kNoArg);
    // style channel flag
    syntax.addFlag(channelsSN, channelsLN, MSyntax::kBoolean, MSyntax::kBoolean, MSyntax::kBoolean, MSyntax::kBoolean);
    return syntax;
};


/// compute the command
MStatus Cmd::doIt(const MArgList& args) {
    MStatus	status;

    MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
    viewOverride* override = (viewOverride*)theRenderer->findRenderOverride("viewOverride");
    if (!override) {
        cout << "WARNING: No render override instance was found" << endl;
        return MStatus::kFailure;
    }

    // parse arguments
    MArgDatabase argData(syntax(), args);  // so that is works with python
    bool query = argData.isQuery();
    // check for target flag
    if (argData.isFlagSet(targetSN)) {
        if (query) {
            setResult(override->activeTarget());
        }
        else {
            unsigned int targetIndex;
            argData.getFlagArgument(targetSN, 0, targetIndex);
            override->changeActiveTarget(targetIndex);
        }
    }
    // check if shaders need to be refreshed
    if (argData.isFlagSet(refreshSN)) {
        override->resetShaderInstances();
    }
    // check if the present channels are being set
    if (argData.isFlagSet(channelsSN)) {
        int r, g, b, a;
        argData.getFlagArgument(channelsSN, 0, r);
        argData.getFlagArgument(channelsSN, 1, g);
        argData.getFlagArgument(channelsSN, 2, b);
        argData.getFlagArgument(channelsSN, 3, a);
        cout << "( " << r << ", " << g << ", " << b << ", " << a << ")" << endl;
        override->showChannels(r, g, b, a);
    }

    return redoIt();  // normally a command should execute here
};

MStatus Cmd::redoIt() {
    // since the command is not undoable, we just schedule a refresh here
    M3dView::scheduleRefreshAllViews(); ///< refresh all viewports
    return MS::kSuccess;
};

MStatus Cmd::undoIt() { return MS::kSuccess; };

bool Cmd::isUndoable() const { return false; };