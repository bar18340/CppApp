/**
 * @file DownloadThread.cpp
 * @brief Implementation of book data downloading and processing from Open Library API
 */

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "DownloadThread.h"
#include "../../shared/HttpSrc/httplib.h"
#include <nlohmann/json.hpp>
#include <iostream>

 /**
  * @brief URL encodes a string for use in HTTP requests
  * @param value The string to encode
  * @return URL-encoded version of the input string
  * Converts special characters to their percent-encoded equivalents
  * according to URL encoding standards.
  */
std::string url_encode(const std::string& value) {
    std::string result;
    for (char c : value) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            result += c;
        }
        else {
            result += '%' + std::string(2, '0') + c;
        }
    }
    return result;
}

/**
 * @brief Main thread operation that handles book data downloads
 * @param common Reference to shared data object
 * Continuously monitors for download requests and when triggered:
 * 1. Constructs API request URL with search parameters
 * 2. Sends HTTP request to Open Library API
 * 3. Parses JSON response into Book objects
 * 4. Updates shared data with results
 * 5. Handles error cases and logging
 */
void DownloadThread::operator()(CommonObjects& common)
{
    while (!common.exit_flag) {
        if (common.start_download) {
            try {
                // Setup API request
                std::string base_url = "openlibrary.org";
                std::string query = url_encode(common.search_query);
                std::string search_type = common.search_type;

                // Initialize HTTPS client
                httplib::SSLClient cli(base_url);
                cli.set_connection_timeout(10);

                // Construct API path with parameters
                std::string path = "/search.json?";
                path += (search_type == "title") ? "title=" : "author=";
                path += query;
                path += "&limit=" + std::to_string(common.results_per_page);
                path += "&page=" + std::to_string(common.current_page);
                path += "&fields=key,title,author_name,first_publish_year,edition_count,cover_i,language,subject,want_to_read_count,currently_reading_count,already_read_count";

                // Make API request
                auto res = cli.Get(path.c_str());
                if (res && res->status == 200) {
                    try {
                        // Parse JSON response
                        auto json_result = nlohmann::json::parse(res->body);
                        common.books.clear();

                        if (json_result.contains("docs")) {
                            for (const auto& doc : json_result["docs"]) {
                                Book book;
                                if (doc.contains("key") && doc.contains("title")) {
                                    // Extract basic book information
                                    book.key = doc["key"].get<std::string>();
                                    book.title = doc["title"].get<std::string>();
                                    // Extract optional fields if present
                                    if (doc.contains("author_name"))
                                        book.author_names = doc["author_name"].get<std::vector<std::string>>();
                                    if (doc.contains("first_publish_year"))
                                        book.first_publish_year = doc["first_publish_year"].get<int>();
                                    if (doc.contains("edition_count"))
                                        book.edition_count = doc["edition_count"].get<int>();
                                    // Process language array
                                    if (doc.contains("language") && doc["language"].is_array()) {
                                        std::string langs;
                                        for (const auto& lang : doc["language"]) {
                                            if (!langs.empty()) langs += ", ";
                                            langs += lang.get<std::string>();
                                        }
                                        book.language = langs;
                                    }
                                    // Process subject array
                                    if (doc.contains("subject") && doc["subject"].is_array()) {
                                        std::string subjects;
                                        for (const auto& subject : doc["subject"]) {
                                            if (!subjects.empty()) subjects += ", ";
                                            subjects += subject.get<std::string>();
                                        }
                                        book.subject = subjects;
                                    }
                                    // Extract reading statistics
                                    if (doc.contains("want_to_read_count"))
                                        book.want_to_read_count = doc["want_to_read_count"].get<int>();
                                    if (doc.contains("currently_reading_count"))
                                        book.currently_reading_count = doc["currently_reading_count"].get<int>();
                                    if (doc.contains("already_read_count"))
                                        book.already_read_count = doc["already_read_count"].get<int>();
                                    // Check if book is in favorites
                                    book.is_favorite = std::find(common.favorite_books.begin(),
                                        common.favorite_books.end(),
                                        book.key) != common.favorite_books.end();
                                    // Load saved notes if they exist
                                    if (common.saved_notes.find(book.key) != common.saved_notes.end()) {
                                        book.notes[book.key] = common.saved_notes[book.key];
                                    }

                                    common.books.push_back(book);
                                }
                            }
                        }
                        common.data_ready = true;
                    }
                    catch (const nlohmann::json::exception& e) {
                        std::cerr << "JSON parsing error: " << e.what() << std::endl;
                    }
                }
                else {
                    std::cerr << "HTTP request failed: "
                        << (res ? std::to_string(res->status) : "no response")
                        << std::endl;
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error in download thread: " << e.what() << std::endl;
            }
            common.start_download = false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
