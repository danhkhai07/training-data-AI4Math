// ai4math_namer_git_safe.cpp
// Compile: g++ -std=c++17 ai4math_namer_git_safe.cpp -o ai4math_namer_git_safe
// Windows (MSVC): cl /std:c++17 ai4math_namer_git_safe.cpp

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <iomanip>
#include <cstdlib>
#include <csignal>
#include <vector>

namespace fs = std::filesystem;

/*************** GLOBALS FOR CLEANUP ***************/
std::vector<fs::path> created_files; // track created files
fs::path cache_file;
std::map<std::string,int> old_cache; // backup of cache

/*************** SIGNAL HANDLER ***************/
void cleanup_on_signal(int signum) {
    std::cerr << "\nProgram interrupted! Reverting changes...\n";
    for (auto &f : created_files) {
        if (fs::exists(f)) {
            fs::remove(f);
            std::cerr << "Removed: " << f << "\n";
        }
    }
    if (!cache_file.empty() && !old_cache.empty()) {
        std::ofstream out(cache_file, std::ios::trunc);
        out << "WS=" << old_cache["WS"] << "\n";
        out << "NS=" << old_cache["NS"] << "\n";
        std::cerr << "Cache restored.\n";
    }
    std::exit(1);
}

/*************** SMALL HELPERS ***************/
static std::string trim(const std::string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

std::string pad_num(int v, int width) {
    std::ostringstream ss;
    ss << std::setw(width) << std::setfill('0') << v;
    return ss.str();
}

/*************** GIT HELPERS ***************/
bool run_git_command(const fs::path& repo, const std::string &cmd) {
    fs::path old = fs::current_path();
    std::error_code ec;
    fs::current_path(repo, ec);
    if (ec) return false;
    int ret = std::system(cmd.c_str());
    fs::current_path(old, ec);
    return ret == 0;
}

bool has_upstream(const fs::path &repo) {
#ifdef _WIN32
    return run_git_command(repo, "git rev-parse --abbrev-ref --symbolic-full-name @{u} >nul 2>nul");
#else
    return run_git_command(repo, "git rev-parse --abbrev-ref --symbolic-full-name @{u} >/dev/null 2>/dev/null");
#endif
}

/*************** CACHE ***************/
std::map<std::string,int> load_cache(const fs::path &path) {
    std::map<std::string,int> m;
    m["WS"]=0; m["NS"]=0;
    if (!fs::exists(path)) return m;
    std::ifstream in(path);
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        m[line.substr(0,pos)] = std::stoi(line.substr(pos+1));
    }
    return m;
}

void save_cache(const fs::path &path, const std::map<std::string,int> &m) {
    std::ofstream out(path, std::ios::trunc);
    out << "WS=" << m.at("WS") << "\n";
    out << "NS=" << m.at("NS") << "\n";
}

/*************** CREATOR NAME ***************/
std::string get_creator_name(const fs::path& base) {
    fs::path cfg_file = base / ".creator.cfg";
    if (fs::exists(cfg_file)) {
        std::ifstream in(cfg_file);
        std::string name;
        std::getline(in, name);
        name = trim(name);
        if (!name.empty()) return name;
    }
    std::cout << "Enter creator name (will be saved for future runs): ";
    std::string name;
    std::getline(std::cin, name);
    name = trim(name);
    std::ofstream out(cfg_file);
    out << name;
    return name;
}

/*************** MAIN PROGRAM ***************/
int main() {
    // Register signal handlers for crash safety
    std::signal(SIGINT, cleanup_on_signal);
    std::signal(SIGTERM, cleanup_on_signal);

    std::cout << "AI4Math filename helper + Git automation\n\n";

    // --- Base dataset folder (always current directory)
    fs::path base = fs::current_path();
    std::cout << "Dataset root: " << base << "\n";

    // --- Git pull if possible ---
    bool has_git = fs::exists(base / ".git");
    if (has_git) {
        std::cout << "\nGit detected.\n";
        if (has_upstream(base)) {
            std::cout << "Upstream found → running: git pull --rebase\n";
            run_git_command(base, "git pull --rebase");
        } else {
            std::cout << "No upstream branch → skipping git pull\n";
        }
    }

    // --- Creator info ---
    std::string id_input;
    std::cout << "\nCreator numeric ID: ";
    std::getline(std::cin, id_input);
    int idnum = std::stoi(id_input);
    std::string id_str = pad_num(idnum, 2);

    std::string creator_name = get_creator_name(base);

    fs::path creator_folder = base / (id_str + "_" + creator_name);
    fs::create_directories(creator_folder);

    // --- Load cache ---
    cache_file = creator_folder / ".cache";
    old_cache = load_cache(cache_file); // backup
    auto cache = old_cache;

    // --- WS/NS ---
    std::string wsns;
    while (true) {
        std::cout << "WS or NS: ";
        std::getline(std::cin, wsns);
        for (auto &c : wsns) c = toupper(c);
        if (wsns == "WS" || wsns == "NS") break;
    }
    int next_stt = cache[wsns] + 1;

    // --- Chapter ---
    std::string chapter;
    std::cout << "Chapter (C02 or 2): ";
    std::getline(std::cin, chapter);
    chapter = trim(chapter);
    if (chapter[0] == 'C' || chapter[0] == 'c')
        chapter = "C" + pad_num(std::stoi(chapter.substr(1)), 2);
    else
        chapter = "C" + pad_num(std::stoi(chapter), 2);

    // --- Difficulty ---
    std::string difficulty;
    while (true) {
        std::cout << "Difficulty L1..L5: ";
        std::getline(std::cin, difficulty);
        for (auto &c : difficulty) c = toupper(c);
        if (std::regex_match(difficulty, std::regex("L[1-5]"))) break;
    }

    // --- Classification ---
    std::string cls;
    while (true) {
        std::cout << "MATH or LEAN: ";
        std::getline(std::cin, cls);
        for (auto &c : cls) c = toupper(c);
        if (cls == "MATH" || cls == "LEAN") break;
    }
    std::string ext = (cls == "MATH") ? ".tex" : ".lean";

    // --- Build filename ---
    auto make_name = [&](int stt)->std::string {
        return wsns + id_str + pad_num(stt,4) + "_" + chapter + "_" + difficulty + "_" + cls + ext;
    };
    int stt = next_stt;
    std::string filename;
    while (true) {
        filename = make_name(stt);
        if (!fs::exists(creator_folder / filename)) break;
        stt++;
    }
    std::cout << "\nFinal filename: " << filename << "\n";

    // --- Content input ---
    std::ostringstream content;
    std::cout << "Enter file content (end with . on a line):\n";
    while (true) {
        std::string line;
        std::getline(std::cin, line);
        if (trim(line) == ".") break;
        content << line << "\n";
    }

    // --- Write temp file then rename atomically ---
    fs::path temp_file = creator_folder / (filename + ".tmp");
    std::ofstream out(temp_file);
    out << content.str();
    out.close();

    fs::path fullpath = creator_folder / filename;
    fs::rename(temp_file, fullpath);
    created_files.push_back(fullpath);

    // --- Update cache ---
    cache[wsns] = stt;
    save_cache(cache_file, cache);

    // --- Clear created_files on success ---
    created_files.clear();

    std::cout << "Saved: " << fullpath << "\n";

    // --- Git add/commit/push ---
    if (has_git) {
        std::cout << "\n--- Git add/commit/push ---\n";
        std::string msg = "New file data: " + filename;
        std::cout << "Auto commit message: " << msg << "\n";

        if (msg.empty()) msg = "Add " + filename;
        run_git_command(base, ("git add \"" + fullpath.string() + "\"").c_str());
        run_git_command(base, ("git commit -m \"" + msg + "\"").c_str());
        run_git_command(base, "git push");
        std::cout << "Git push completed.\n";
    }

    return 0;
}

