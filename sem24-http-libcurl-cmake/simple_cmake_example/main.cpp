// %%cpp simple_cmake_example/main.cpp
// %run mkdir simple_cmake_example/build #// cоздаем директорию для файлов сборки
// %# // переходим в нее, вызываем cmake, чтобы он создал правильный Makefile
// %# // а затем make, который по Makefile правильно все соберет
// %run cd simple_cmake_example/build && cmake .. && make  
// %run simple_cmake_example/build/main #// запускаем собранный бинарь
// %run ls -la simple_cmake_example #// смотрим, а что же теперь есть в основной директории 
// %run ls -la simple_cmake_example/build #// ... и директории сборки
// %run rm -r simple_cmake_example/build #// удаляем директорию с файлами сборки

#include <iostream>
int main(int argc, char** argv)
{
    std::cout << "Hello, World!" << std::endl;
    return 0;
}

