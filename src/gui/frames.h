/*
 * frames.h
 *  Max Peglar-Willis 2021
 */

#pragma once
#include "../common.h"

namespace Sqeak { 
    
    
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
    void Draw(GRBL& grbl, Settings& settings, Viewer& viewer, sketch::SketchOld& sketcher, Sketch::Sketcher& sketcherNew, float dt);
private:
    void DrawDockSpace(Settings& settings);
    void DrawSketcher(Settings& settings, Sketch::Sketcher& sketcher);
    
    std::unique_ptr<FileBrowser> fileBrowser;
    
};

} // end namespace Sqeak
