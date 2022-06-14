/*
 * frames.h
 *  Max Peglar-Willis 2021
 */

#pragma once
#include "../common.h"

// forward declare
class FileBrowser;
namespace sketch { class SketchOld; }


class Frames
{
public:
    Frames(Settings& settings) {
        fileBrowser = std::make_unique<FileBrowser>(&settings.p.system.curDir);
    }
    // draws everything imgui related
    void Draw(GRBL& grbl, Settings& settings, Viewer& viewer, sketch::SketchOld& sketcher, float dt);
private:
    void DrawDockSpace(Settings& settings);
    
    std::unique_ptr<FileBrowser> fileBrowser;
    
};
