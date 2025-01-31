/**
 * @file DrawThread.cpp
 * @brief Handles rendering and user interaction through the GUI.
 */

#pragma once
#include "CommonObject.h"

 /**
  * @class DrawThread
  * @brief Handles the GUI rendering process using ImGui.
  */
class DrawThread
{
public:
    /**
     * @brief Runs the drawing thread.
     * @param common Reference to shared data.
     */
	void operator()(CommonObjects& common);
};

