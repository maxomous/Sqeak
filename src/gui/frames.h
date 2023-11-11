/*
 * frames.h
 *  Max Peglar-Willis 2021
 */

#pragma once
#include "../common.h"

namespace Sqeak { 
    
    
// forward declare
class Functions;
namespace sketch { class SketchOld; }


class Frames
{
public:
    Frames() {}
    // draws everything imgui related
    void Draw(GRBL& grbl, Settings* settings, Viewer& viewer, sketch::SketchOld& sketcher, Sketch::Sketcher* sketcherNew, Functions& functions, float dt);
private:
    void DrawDockSpace(Settings& settings);
    void DrawSketcher(Settings& settings, Sketch::Sketcher& sketcher);
    
    
};

} // end namespace Sqeak
