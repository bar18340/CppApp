/*
 * @file CommonObject.h
 * @brief Shared data structures for the Book Search Application
 */

#pragma once
#include <atomic>
#include <string>
#include <vector>
#include <unordered_map>

 /*
  * @struct BookNote
  * @brief Represents a user's note for a specific book
  */
struct BookNote {
    std::string note; //The content of the note
    std::string date; //The date when the note was created/modified
};

/*
 * @struct Book
 * @brief Represents a book with its metadata from Open Library
 */
struct Book {
    std::string key; //Unique identifier for the book
    std::string title; //Book title
    std::vector<std::string> author_names; //List of author names
    int first_publish_year = 0; //Year of first publication
    int edition_count = 0; //Number of editions
    //std::string cover_id; //ID for book cover image
    bool is_favorite = false; //Whether user marked as favorite
    std::string language; //Book language(s)
    std::string subject; //Book subject categories
    int want_to_read_count = 0; //Number of users wanting to read
    int currently_reading_count = 0; //Number of users currently reading
    int already_read_count = 0; //Number of users who have read
    std::unordered_map<std::string, BookNote> notes; //User notes for this book
};

struct CommonObjects {
    std::atomic_bool exit_flag = false;
    std::atomic_bool start_download = false;
    std::atomic_bool data_ready = false;
    std::string search_query;
    std::string search_type = "title";
    int results_per_page = 10;
    int current_page = 1;
    std::vector<Book> books;
    std::vector<std::string> favorite_books;
    std::unordered_map<std::string, BookNote> saved_notes;
};