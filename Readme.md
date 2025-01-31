# Book Search Application 📚

## Overview
The Book Search Application is a multi-threaded C++ program that allows users to search for books using the Open Library API: https://openlibrary.org/dev/docs/api/search. It provides features such as searching by title or author, managing favorite books, and storing user notes.

## Features
- 🔍 **Book Search**: Users can search for books by title or author.
- ⭐ **Favorites Management**: Save books as favorites for easy access.
- 📝 **User Notes**: Attach personal notes to books.
- ⚡ **Multi-Threaded Performance**: Uses separate threads for downloading book data and rendering the graphical interface.
- 🖥️ **GUI Integration**: Utilizes ImGui for an interactive user interface.

## Dependencies
Ensure you have the following dependencies installed:
- 🛠️ C++17 or later
- 🔒 OpenSSL (for secure HTTP requests)
- 🎨 [ImGui](https://github.com/ocornut/imgui) (for the GUI)
- 📄 [nlohmann/json](https://github.com/nlohmann/json) (for JSON parsing)

## Installation & Compilation
### **1. Clone the Repository**
```bash
git clone <repository-url>
cd book-search-app
```

### **2. Build the Project**
```bash
mkdir build
cd build
cmake ..
make
```

### **3. Run the Application**
```bash
./BookSearchApp
```

## Usage
1. 📂 Launch the application.
2. 📖 Enter a book title or author name in the search bar.
3. 🔎 Click `Search` to fetch book data from Open Library.
4. 📜 View book details, mark books as favorites, or add personal notes.
5. ⭐ Click `Show Favorites` to manage favorite books.
6. 💾 Exit the application to save favorite books and notes automatically.
