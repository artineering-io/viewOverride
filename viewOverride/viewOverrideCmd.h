// Title         viewOverrideCmd.h
// Summary       viewOverride Viewport 2.0 command declaration
// Copyright     2020 Artineering and/or its licensors
// License       MIT

#pragma once
#include <maya/MSyntax.h>
#include <maya/M3dView.h>
#include <maya/MPxCommand.h>
#include <maya/MArgDatabase.h>

class Cmd : public MPxCommand {
public:
    Cmd();
    ~Cmd();
    static void* creator();						 ///< command creator
    static MSyntax newSyntax();					 ///< define syntax of command
    virtual MStatus doIt(const MArgList& args);  ///< compute the command  
    virtual MStatus redoIt();					 ///< compute the command (what should happen when you redo)
    virtual MStatus undoIt();					 ///< command undoer
    bool isUndoable() const;					 ///< can you undo it?
};