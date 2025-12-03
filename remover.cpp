// Compile: g++ -std=c++17 ai4math_remover_safe_git.cpp -o ai4math_remover_safe_git

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <csignal>
#include <cstdlib>
#include <algorithm>

namespace fs = std::filesystem;

// ---------- GLOBALS FOR CLEANUP ----------
std::vector<std::pair<fs::path, fs::path>> renamed_files; // track files renamed so far (old->new)
fs::path deleted_file;
fs::path cache_file;
std::map<std::string,int> old_cache;

// ---------- SIGNAL HANDLER ----------
void cleanup_on_signal(int signum) {
    std::cerr << "\nProgram interrupted! Reverting changes...\n";

    // revert renamed files
    for (auto it = renamed_files.rbegin(); it != renamed_files.rend(); ++it) {
        auto &pair = *it;
        if (fs::exists(pair.second)) {
            fs::rename(pair.second, pair.first);
            std::cerr << "Restored: " << pair.second.filename()
                      << " -> " << pair.first.filename() << "\n";
        }
    }

    // restore deleted file (cannot restore content automatically)
    if (!deleted_file.empty()) {
        std::cerr << "Deleted file: " << deleted_file.filename() 
                  << " (cannot restore automatically)\n";
    }

    // restore cache
    if (!cache_file.empty() && !old_cache.empty()) {
        std::ofstream out(cache_file, std::ios::trunc);
        out << "WS=" << old_cache["WS"] << "\n";
        out << "NS=" << old_cache["NS"] << "\n";
        std::cerr << "Cache restored.\n";
    }

    std::exit(1);
}

// ---------- HELPERS ----------
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

// load/save cache
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

// git helper
bool run_git_command(const fs::path &repo, const std::string &cmd) {
    fs::path old = fs::current_path();
    std::error_code ec;
    fs::current_path(repo, ec);
    if (ec) return false;
    int ret = std::system(cmd.c_str());
    fs::current_path(old, ec);
    return ret == 0;
}

// ---------- MAIN ----------
int main() {
    std::signal(SIGINT, cleanup_on_signal);
    std::signal(SIGTERM, cleanup_on_signal);

    std::cout << "AI4Math remover + sync + git\n\n";
    fs::path base = fs::current_path();
    std::cout << "Dataset root: " << base << "\n";

    // --- Creator ID ---
    std::string id_input;
    std::cout << "Creator numeric ID: ";
    std::getline(std::cin, id_input);
    int idnum = std::stoi(id_input);
    std::string id_str = pad_num(idnum,2);

    // --- Detect creator folder ---
    fs::path creator_folder;
    bool found = false;
    for (auto &p : fs::directory_iterator(base)) {
        if (p.is_directory() && p.path().filename().string().rfind(id_str+"_")==0) {
            creator_folder = p.path();
            found = true;
            break;
        }
    }
    if (!found) {
        std::cerr << "Error: cannot find folder starting with ID_" << id_str << "\n";
        return 1;
    }
    std::cout << "Detected creator folder: " << creator_folder << "\n";

    // --- Load cache ---
    cache_file = creator_folder / ".cache";
    old_cache = load_cache(cache_file);
    auto cache = old_cache;

    // --- Prompt full filename ---
    std::string filename;
    std::cout << "Enter full filename to remove (e.g. WS010001_C02_L3_MATH.tex): ";
    std::getline(std::cin, filename);
    filename = trim(filename);

    fs::path file_path = creator_folder / filename;
    if (!fs::exists(file_path)) {
        std::cerr << "Error: file does not exist.\n";
        return 1;
    }

    // --- Parse WS/NS and STT ---
    std::regex re(R"((WS|NS)(\d{6})_C\d{2}_L[1-5]_(MATH|LEAN)\.(tex|lean))", std::regex::icase);
    std::smatch match;
    if (!std::regex_match(filename, match, re)) {
        std::cerr << "Error: filename does not match pattern\n";
        return 1;
    }
    std::string wsns = match[1];
    int stt_to_remove = std::stoi(match[2].str().substr(2));

    // --- Delete file ---
    fs::remove(file_path);
    deleted_file = file_path;
    std::cout << "Deleted: " << file_path << "\n";

    // --- Renumber subsequent files ---
    std::vector<std::pair<fs::path, fs::path>> renames; // old->new
    for (auto &p : fs::directory_iterator(creator_folder)) {
        std::string f = p.path().filename().string();
        std::smatch m;
        if (std::regex_match(f, m, re)) {
            std::string type = m[1];
            int stt = std::stoi(m[2].str().substr(2));
            if (type==wsns && stt > stt_to_remove) {
                std::string new_stt_str = pad_num(stt-1,4);
                std::string new_name = wsns + id_str + new_stt_str + f.substr(8);
                fs::path new_path = creator_folder / new_name;
                renames.emplace_back(p.path(), new_path);
            }
        }
    }

    // sort ascending STT
    std::sort(renames.begin(), renames.end(), [](auto &a, auto &b){ return a.first.filename() < b.first.filename(); });

    // apply renames and record for cleanup
    for (auto &p : renames) {
        fs::rename(p.first, p.second);
        renamed_files.push_back(p); // record old->new
        std::cout << "Renamed: " << p.first.filename() << " -> " << p.second.filename() << "\n";
    }

    // --- Update cache ---
    cache[wsns]--;
    save_cache(cache_file, cache);
    std::cout << "Cache updated. " << wsns << " now ends at " << cache[wsns] << "\n";

    // --- Git add/commit/push ---
    if (fs::exists(creator_folder / ".git") || fs::exists(base / ".git")) {
        fs::path repo = fs::exists(base / ".git") ? base : creator_folder;
        std::string msg = "Removed file data: " + filename;
        run_git_command(repo, ("git add -A \"" + creator_folder.string() + "\"").c_str());
        run_git_command(repo, ("git commit -m \"" + msg + "\"").c_str());
        run_git_command(repo, "git push");
        std::cout << "Git push completed.\n";
    }

    // Clear tracking
    renamed_files.clear();
    deleted_file.clear();

    return 0;
}

