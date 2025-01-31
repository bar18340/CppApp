/*
 * @file DownloadThread.h
 * @brief Handles asynchronous book data downloads from Open Library API
 */

#pragma once
#include "CommonObject.h"

 /*
  * @class DownloadThread
  * @brief Thread handler for downloading book data
  * Manages asynchronous downloads from the Open Library API,
  * parsing JSON responses and updating the shared data structure.
  */
class DownloadThread
{
public:
	/*
   * @brief Main thread operation function
   * @param common Reference to shared data object
   * Monitors common.start_download flag and performs searches
   * when triggered. Updates common.books with results.
   */
	void operator()(CommonObjects& common);
};

