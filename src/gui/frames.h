/*
 * frames.hpp
 *  Max Peglar-Willis 2021
 */

#pragma once

void BeginDisableWidgets(GRBLVals &grblVals);
void EndDisableWidgets(GRBLVals &grblVals);
void HelpMarker(const char *desc);
void HereButton(GRBLVals &grblVals, glm::vec3& p);

void drawFrames(GRBL& grbl, GRBLVals& grblVals);
