/**
 * @file DrawThread.cpp
 * @brief Handles rendering and user interaction through the GUI.
 */

#define _CRT_SECURE_NO_WARNINGS
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "DrawThread.h"
#include "GuiMain.h"
#include "../../shared/ImGuiSrc/imgui.h"
#include "../../shared/HttpSrc/httplib.h"
#include <chrono>
#include <ctime>
#include <unordered_map>
#include <nlohmann/json.hpp>

 /**
  * @brief Global map storing note buffers for each book
  * Used to maintain note text state between edits
  */
static std::unordered_map<std::string, char[1024]> note_buffers;

/**
 * @brief Renders the main application window
 * @param common_ptr Pointer to CommonObjects instance
 * Handles rendering of:
 * - Search interface
 * - Results table
 * - Favorites management
 * - Book details popup
 * - Notes system
 * @note This function is called every frame by the GUI system
 */
void DrawAppWindow(void* common_ptr)
{
    auto common = (CommonObjects*)common_ptr;
    ImGui::Begin("Book Search", nullptr, ImGuiWindowFlags_MenuBar);

    // Search section
    /**
    * @brief Renders the search interface
    * Includes:
    * - Search input field
    * - Search type selector (Title/Author)
    * - Search button
    * - Favorites button
    * - Results per page slider
    */
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));

        // Search bar
        ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.4f);
        static char search_buffer[256] = "";
        if (ImGui::InputText("##search", search_buffer, sizeof(search_buffer),
            ImGuiInputTextFlags_EnterReturnsTrue)) {
            common->search_query = search_buffer;
            common->start_download = true;
        }
        ImGui::SameLine();

        // Search type combo
        static int search_type = 0;
        const char* search_types[] = { "Title", "Author" };
        ImGui::SetNextItemWidth(100);
        if (ImGui::Combo("##searchType", &search_type, search_types, IM_ARRAYSIZE(search_types))) {
            common->search_type = search_type == 0 ? "title" : "author";
        }
        ImGui::SameLine();

        // Search button with custom style
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.7f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.5f, 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.35f, 0.65f, 1.0f));
        if (ImGui::Button("Search")) {
            common->search_query = search_buffer;
            common->start_download = true;
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();

        // Favorites button with custom style
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.4f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.5f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.65f, 0.35f, 0.15f, 1.0f));
        if (ImGui::Button("Show Favorites")) {
            ImGui::OpenPopup("Favorites");
        }
        ImGui::PopStyleColor(3);

        ImGui::PopStyleVar();
    }

    // Results per page slider with custom style
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
    ImGui::SliderInt("Results per page", &common->results_per_page, 5, 50);
    ImGui::PopStyleColor(3);

    // Favorites popup
    /**
    * @brief Manages the favorites list popup
    * Features:
    * - Displays all favorited books
    * - Loads missing book details from API
    * - Allows removal of favorites
    * - Auto-refreshes when needed
    */
    ImGui::SetNextWindowSizeConstraints(ImVec2(400, 400), ImVec2(600, 500));
    if (ImGui::BeginPopupModal("Favorites", nullptr,
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize)) {

        static std::vector<Book> favorite_books_list;
        static bool needs_refresh = true;

        if (needs_refresh) {
            favorite_books_list.clear();
            for (const auto& book : common->books) {
                if (book.is_favorite) {
                    favorite_books_list.push_back(book);
                }
            }

            std::string base_url = "openlibrary.org";
            httplib::SSLClient cli(base_url);
            cli.set_connection_timeout(10);

            for (const auto& key : common->favorite_books) {
                bool found = false;
                for (const auto& book : favorite_books_list) {
                    if (book.key == key) {
                        found = true;
                        break;
                    }
                }
                if (found) continue;

                std::string work_id = key;
                if (work_id.starts_with("/works/")) {
                    work_id = work_id.substr(7);
                }
                std::string path = "/works/" + work_id + ".json";

                auto res = cli.Get(path.c_str());
                if (res && res->status == 200) {
                    try {
                        auto json_result = nlohmann::json::parse(res->body);
                        Book book;
                        book.key = key;
                        book.title = json_result["title"].get<std::string>();
                        book.is_favorite = true;

                        if (json_result.contains("authors")) {
                            for (const auto& author : json_result["authors"]) {
                                if (author.contains("author")) {
                                    std::string author_key = author["author"]["key"].get<std::string>();
                                    std::string author_path = author_key + ".json";
                                    auto author_res = cli.Get(author_path.c_str());
                                    if (author_res && author_res->status == 200) {
                                        try {
                                            auto author_json = nlohmann::json::parse(author_res->body);
                                            if (author_json.contains("name")) {
                                                book.author_names.push_back(author_json["name"].get<std::string>());
                                            }
                                        }
                                        catch (const std::exception& e) {
                                            std::cerr << "Error parsing author: " << e.what() << std::endl;
                                        }
                                    }
                                }
                            }
                        }

                        favorite_books_list.push_back(book);
                    }
                    catch (const std::exception& e) {
                        std::cerr << "Error parsing favorite book: " << e.what() << std::endl;
                    }
                }
            }
            needs_refresh = false;
        }

        ImVec2 content_size = ImGui::GetContentRegionAvail();
        content_size.y -= 30;

        if (ImGui::BeginTable("FavoritesTable", 3,
            ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY, content_size)) {

            ImGui::TableSetupColumn("Title", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Author", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 70.0f);
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < favorite_books_list.size();) {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::TextWrapped("%s", favorite_books_list[i].title.c_str());

                ImGui::TableSetColumnIndex(1);
                if (!favorite_books_list[i].author_names.empty()) {
                    ImGui::TextWrapped("%s", favorite_books_list[i].author_names[0].c_str());
                }

                ImGui::TableSetColumnIndex(2);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.65f, 0.15f, 0.15f, 1.0f));
                if (ImGui::Button(("Remove##" + favorite_books_list[i].key).c_str())) {
                    auto fav_it = std::find(common->favorite_books.begin(),
                        common->favorite_books.end(), favorite_books_list[i].key);
                    if (fav_it != common->favorite_books.end()) {
                        common->favorite_books.erase(fav_it);
                        // Update the main list if the book exists there
                        for (auto& book : common->books) {
                            if (book.key == favorite_books_list[i].key) {
                                book.is_favorite = false;
                                break;
                            }
                        }
                        favorite_books_list.erase(favorite_books_list.begin() + i);
                    }
                }else {
                    i++; // Only increment if we didn't remove an item
                }
                ImGui::PopStyleColor(3);
            }
            ImGui::EndTable();
        }

        if (ImGui::Button("Close")) {
            needs_refresh = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Results table
    /**
    * @brief Displays search results in a table format
    * Shows:
    * - Book title
    * - Author
    * - Publication year
    * - Favorite toggle
    * - Notes access
    * - Detailed information popup
    */
    if (common->data_ready && !common->books.empty())
    {
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5, 5));
        if (ImGui::BeginTable("Books", 6, ImGuiTableFlags_Borders |
            ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable))
        {
            ImGui::TableSetupColumn("Title", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Author", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Year", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Favorite", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Notes", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Details", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();

            for (auto& book : common->books)
            {
                ImGui::TableNextRow();
                ImGui::PushID(book.key.c_str());

                ImGui::TableSetColumnIndex(0);
                ImGui::TextWrapped("%s", book.title.c_str());

                ImGui::TableSetColumnIndex(1);
                if (!book.author_names.empty())
                    ImGui::TextWrapped("%s", book.author_names[0].c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%d", book.first_publish_year);

                ImGui::TableSetColumnIndex(3);
                if (ImGui::Checkbox(("##fav" + book.key).c_str(), &book.is_favorite)) {
                    if (book.is_favorite)
                        common->favorite_books.push_back(book.key);
                    else {
                        auto it = std::find(common->favorite_books.begin(),
                            common->favorite_books.end(), book.key);
                        if (it != common->favorite_books.end())
                            common->favorite_books.erase(it);
                    }
                }

                ImGui::TableSetColumnIndex(4);
                if (ImGui::Button(("Notes##" + book.key).c_str())) {
                    ImGui::OpenPopup(("Notes##popup" + book.key).c_str());
                    if (note_buffers.find(book.key) == note_buffers.end()) {
                        if (book.notes.find(book.key) != book.notes.end()) {
                            strcpy_s(note_buffers[book.key], sizeof(note_buffers[book.key]),
                                book.notes[book.key].note.c_str());
                        }
                        else if (common->saved_notes.find(book.key) != common->saved_notes.end()) {
                            strcpy_s(note_buffers[book.key], sizeof(note_buffers[book.key]),
                                common->saved_notes[book.key].note.c_str());
                            book.notes[book.key] = common->saved_notes[book.key];
                        }
                        else {
                            note_buffers[book.key][0] = '\0';
                        }
                    }
                }

                if (ImGui::BeginPopup(("Notes##popup" + book.key).c_str())) {
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
                    if (ImGui::InputTextMultiline("##note", note_buffers[book.key],
                        sizeof(note_buffers[book.key]),
                        ImVec2(300, 200))) {
                        BookNote note;
                        note.note = note_buffers[book.key];
                        auto now = std::chrono::system_clock::now();
                        auto time = std::chrono::system_clock::to_time_t(now);
                        note.date = std::ctime(&time);
                        book.notes[book.key] = note;
                        common->saved_notes[book.key] = note;
                    }

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.4f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.65f, 0.25f, 1.0f));
                    if (ImGui::Button("Save")) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::PopStyleColor(3);
                    ImGui::PopStyleVar();
                    ImGui::EndPopup();
                }

                ImGui::TableSetColumnIndex(5);
                if (ImGui::Button(("Details##" + book.key).c_str())) {
                    ImGui::OpenPopup(("Details##popup" + book.key).c_str());
                }

                if (ImGui::BeginPopup(("Details##popup" + book.key).c_str())) {
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));

                    ImGui::Text("Languages: %s", book.language.c_str());
                    ImGui::Separator();

                    ImGui::Text("Editions: %d", book.edition_count);
                    ImGui::Separator();

                    if (!book.subject.empty()) {
                        ImGui::Text("Subjects:");
                        ImGui::TextWrapped("%s", book.subject.c_str());
                        ImGui::Separator();
                    }

                    ImGui::Text("Want to read: %d", book.want_to_read_count);
                    ImGui::Separator();
                    ImGui::Text("Currently reading: %d", book.currently_reading_count);
                    ImGui::Separator();
                    ImGui::Text("Already read: %d", book.already_read_count);
                    ImGui::Separator();

                    if (ImGui::Button("Close"))
                        ImGui::CloseCurrentPopup();

                    ImGui::PopStyleVar();
                    ImGui::EndPopup();
                }

                ImGui::PopID();
            }
            ImGui::EndTable();
        }
        ImGui::PopStyleVar();
    }
    ImGui::End();
}

void DrawThread::operator()(CommonObjects& common)
{
    GuiMain(DrawAppWindow, &common);
    common.exit_flag = true;
}