# viewOverride
A simple Viewport 2.0 render override showing the object depth sorting bug within _Maya_ when rendering transparent objects to multiple render targets (MRT).

## Bug description
When MSceneRender renders to multiple render targets (more than just color and depth), object depth sorting of transparent objects stops working. This plugin allows for an easy reproduction of the issue using coding conventions found in the viewOverride plugins within the devkit.

In the _viewRenderOperations.cpp_ file at **line 51**, the scene render operation outputs can be changed between 2 (normal Viewport 2.0) and 3 (rendering color, depth and normals)--the plugin needs to be recompiled for this change to take effect. 
* When rendering to two targets, object depth sorting of both opaque and transparent objects work.
* __When rendering to three or more targets, object depth sorting works for opaque objects, but transparent objects' depth sorting stops working.__

Different targets can be easily visualized using the `viewOverride -t` mel command provided by the plugin. This is useful for viewing the multiple render targets that are rendered.
`viewOverride -t 0` will show the color target, whereas `viewOverride -t 2` will show the normals target in materials that support writing to MRT.

## Build instructions
1. Open the viewOverride folder within the repository
2. Double click on the build.bat to build in DEBUG mode
3. The plugin should build on Windows in the plug-ins folder at the root of the repository

## Installing the plugin
1. Once built, in the version of Maya that you build the plugin for, open the _Plug-in Manager_ and load the _viewOverride_ plugin in the plug-ins folder of the cloned repo.
2. Switch from _Viewport 2.0_ to _View Override_ renderer in the viewport
3. Open the _TransparencyTest.ma_ file and use the _View Override_ renderer.
4. (Optional) For best results when viewing the _TransparencyTest.ma_ file, switch the _Viewport 2.0 Rendering engine_ to _DirectX 11_ (Windows->Settings/Preferences->Preferences->Display->Viewport 2.0->Rendering engine->DirectX 11).