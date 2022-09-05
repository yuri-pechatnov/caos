// %%cpp b.cpp
// %run g++ b.cpp -o b.exe

#include <iostream>

int main() {
    std::string s;
    std::cin >> s;
    std::cout << "STDOUT " << s << std::endl;
    std::cerr << "STDERR " << s << std::endl;
}

