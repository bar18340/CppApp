/**
 * @file ConnectedApp.cpp
 * @brief Entry point for the Book Search Application.
 */

#include <iostream>
#include <thread>
#include <filesystem>
#include <fstream>
#include "CommonObject.h"
#include "DrawThread.h"
#include "DownloadThread.h"
#include "GuiMain.h"
#include <nlohmann/json.hpp>

/**
 * @brief Main function that initializes and runs the application.
 * 
 * This function initializes shared data structures, starts the drawing and downloading
 * threads, and manages file I/O for user preferences (favorites, notes).
 *
 * @return int Exit status of the application.
 */
int main()
{
    try {
        // Create data directory if it doesn't exist
        std::filesystem::create_directory("data");
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error creating data directory: " << e.what() << std::endl;
        return 1;
    }

    CommonObjects common;

    // Initialize UI and download threads
    DrawThread draw;
    std::thread draw_th([&] { draw(common); });

    DownloadThread down;
    std::thread down_th([&] { down(common); });

    // Set initial configuration
    common.results_per_page = 10;
    common.current_page = 1;

    // Load favorites from file
    std::ifstream favorites_file("data/favorites.txt");
    if (favorites_file.is_open()) {
        std::string line;
        while (std::getline(favorites_file, line)) {
            if (!line.empty()) {
                common.favorite_books.push_back(line);
            }
        }
        favorites_file.close();
    }
    else {
        std::cerr << "Warning: Unable to open favorites.txt. Starting with no favorites." << std::endl;
    }

    // Load notes from file
    std::ifstream notes_file("data/notes.json");
    if (notes_file.is_open()) {
        try {
            nlohmann::json notes_json;
            notes_file >> notes_json;
            for (const auto& [key, note] : notes_json.items()) {
                BookNote book_note;
                book_note.note = note.value("note", "");
                book_note.date = note.value("date", "");
                common.saved_notes[key] = book_note;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error loading notes: " << e.what() << std::endl;
        }
        notes_file.close();
    }
    else {
        std::cerr << "Warning: Unable to open notes.json. Starting with no saved notes." << std::endl;
    }

    std::cout << "Book Search Application Running...\n";

    // Wait for threads to complete
    down_th.join();
    draw_th.join();

    // Save favorites before exit
    std::ofstream save_favorites("data/favorites.txt", std::ios::trunc);
    if (save_favorites.is_open()) {
        for (const auto& book_key : common.favorite_books) {
            save_favorites << book_key << "\n";
        }
        save_favorites.close();
    }
    else {
        std::cerr << "Error: Unable to save favorites to favorites.txt." << std::endl;
    }

    // Save notes before exit
    nlohmann::json notes_json;
    for (const auto& [key, note] : common.saved_notes) {
        notes_json[key] = {
            {"note", note.note},
            {"date", note.date}
        };
    }
    std::ofstream save_notes("data/notes.json", std::ios::trunc);
    if (save_notes.is_open()) {
        save_notes << notes_json.dump(4);
        save_notes.close();
    }
    else {
        std::cerr << "Error: Unable to save notes to notes.json." << std::endl;
    }

    return 0;
}
