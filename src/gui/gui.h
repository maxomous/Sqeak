
/*
 * gui.hpp
 *  Max Peglar-Willis 2021
 */

#pragma once

#define GUI_WINDOW_NAME 	"XYZ Table"
#define GUI_WINDOW_W 		1280
#define GUI_WINDOW_H 		720
#define GUI_WINDOW_WMIN 	200
#define GUI_WINDOW_HMIN 	200

#define GUI_CONFIG_FILE 	"XYZTable.ini"	// created in the directory of the executable

#define GUI_IMG_ICON 		"/img/img_restart.png"

extern int gui(GRBL& grbl);
