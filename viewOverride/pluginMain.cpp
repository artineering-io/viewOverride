// Title         pluginMain.cpp
// Summary       viewOverride plugin declaration
// Copyright     2020 Artineering and/or its licensors
// License       MIT

#include <stdio.h>
#include <maya/MFnPlugin.h>
#include <maya/MStreamUtils.h>
#include "viewOverride.h"
#include "viewOverrideCmd.h"

// On plug-in initialization we register a new override
MStatus initializePlugin(MObject obj) {
    MStatus status;
    MFnPlugin plugin(obj, "Artineering", "1.0", "Any");

    #if defined(NT_PLUGIN)
        std::cout.set_rdbuf(MStreamUtils::stdOutStream().rdbuf());
        std::cerr.set_rdbuf(MStreamUtils::stdErrorStream().rdbuf());
    #endif

    MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
    if (theRenderer) {
        // register the render override
        viewOverride *overridePtr = new viewOverride("viewOverride");
        if (overridePtr) {
            status = theRenderer->registerOverride(overridePtr);
            CHECK_MSTATUS_AND_RETURN_IT(status);
        }
    }
    // register command
    status = plugin.registerCommand("viewOverride", Cmd::creator, Cmd::newSyntax);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}


// On plug-in de-initialization we deregister a new override
MStatus uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj);

    MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
    if (renderer) {
        // Find override with the given name and deregister
        const MHWRender::MRenderOverride* overridePtr = renderer->findRenderOverride("viewOverride");
        if (overridePtr) {
            renderer->deregisterOverride(overridePtr);
            delete overridePtr;
        }
    }
    // deregister Command
    status = plugin.deregisterCommand("viewOverride");
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}
