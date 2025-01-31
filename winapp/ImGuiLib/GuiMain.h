/*
 * @file GuiMain.h
 * @brief Main GUI interface for the Book Search Application
 */

#pragma once

 /*
  * @typedef drawcallback
  * @brief Function pointer type for the GUI drawing callback
  * @param obj_ptr Pointer to the shared data object
  */
using drawcallback = void(void*);

/*
 * @brief Initializes and runs the main GUI application loop
 * @param drawfunction Callback function that handles GUI rendering
 * @param obj_ptr Pointer to shared data (typically CommonObjects instance)
 * @return 0 on successful execution, 1 on initialization failure
 */
int GuiMain(drawcallback drawfunction, void* obj_ptr);
