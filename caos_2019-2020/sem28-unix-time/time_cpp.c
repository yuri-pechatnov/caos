// %%cpp time_cpp.c
// %run clang++ -fsanitize=address mutex.c -lpthread -o time_cpp.exe
// %run ./time_cpp.exe

#include <iostream>
#include <sstream>
#include <locale>
#include <iomanip>


int main() {
    { 
        std::tm t = {};
        std::istringstream ss("2011-Februar-18 23:12:34");
        ss.imbue(std::locale("de_DE.utf-8"));
        ss >> std::get_time(&t, "%Y-%b-%d %H:%M:%S");
        if (ss.fail()) {
            std::cout << "Parse failed\n";
        } else {
            std::cout << std::put_time(&t, "%c") << '\n';
        }
    }

    return 0;
}

