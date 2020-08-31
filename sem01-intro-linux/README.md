


# Вступление: Linux, командная строка, Jupyter notebook

Возможно кому-то Jupyter notebook покажется лишним в этом ряду, но так случилось, что я буду вести у вас АКОС, а мне он кажется очень удобным инструментом :)

<table width=100%> <tr>
    <th width=15%> <b>Видео с семинара &rarr; </b> </th>
    <th>
    <a href="???"><img src="video.jpg" width="320" 
   height="160" align="left" alt="Видео с семинара"></a>
    </th>
    <th> </th>
 </table>


Сегодня в программе:
* <a href="#linux" style="color:#856024"> Очень кратко о Linux </a>
* <a href="#terminal" style="color:#856024"> Часто используемые команды терминала </a>
  * <a href="#task1" style="color:#856024"> Задача 1 </a>
  * <a href="#task2" style="color:#856024"> Задача 2 </a>
  * <a href="#task3" style="color:#856024"> Задача 3 </a>
* <a href="#jupyter" style="color:#856024"> Особенности Jupyter notebok используемые в курсе </a>


## <a name="linux"></a> Очень кратко о Linux

Не будем останавливаться на различиях дистрибутивов, просто далее будем подразумевать под Linux Ubuntu 20.04: 
* Довольно удобная система, легко найти решение проблем на stackoverflow
* Все примеры и инструкции в моих материалах будут проверяться только на ней

Несколько максимально упрощенных тезисов, имеющих отношения к взаимодействию с Linux в рамках этого курса.
* Есть рабочий стол: 
  * Можно запустить браузер и работать в нем так же, как если бы это была Windows или MacOS
  * Легко установить и провести базовую настройку
* Несмотря на наличие магазина приложений, многие операции легче делать из командной строки (а многие только из нее и возможно). Например, просто поставить IDE проще из консоли.

## <a name="terminal"></a> Часто используемые команды терминала

Первая важная особенность Jupyter notebook: можно запускать консольные команды, добавляя `!` в начало.
То есть `!echo Hello` запустит `echo Hello` примерно так же, как это происходило бы в консоли.


```python
# Вывод строки
!echo Hello
```

    Hello



```python
# Перенаправление вывода в файл
!echo "Hello1" > file.txt
!echo "Hello2" > file.txt # Файл перезапишется
# Перенаправление вывода в файл (при этом файл дозапишется, а не перезапишется)
!echo "Hello3" >> file.txt

# Вывод файла 
!cat file.txt
```

    Hello2
    Hello3


Еще особенность Jupyter notebook: можно делать ячейки "магическими", добавляя `%%bash`, `%%python`, `%%time` в начало.

Магия `%%bash` запустит ячейку как bash-скрипт. А bash-скрипт это почти то же самое, что последовательность команд в консоли.


```bash
%%bash 
exec 2>&1 # Настройка bash, чтобы все что пишется в поток ошибок на самом деле писалось в стандартный поток
set -x # Настройка bash, чтобы он выводил все исполняемые команды в поток ошибок

# -e включает восприятие \n как перевода строки у команды echo
echo -e "Hello1\nHello2" > file.txt
cat file.txt
```

    + echo -e 'Hello1\nHello2'
    + cat file.txt
    Hello1
    Hello2



```bash
%%bash 
exec 2>&1 ; set -x # Команды можно писать в одну строчку разделяя с помощью ;

echo -e "Hello1\nHello2\nHello10" > file.txt
# Можно направлять вывод одной команды на вход другой с помощью |
# Команда grep в таком использовании читает все строки из входа, 
#   и выводит только содержащие подстроку`o1`
cat file.txt | grep o1

cat file.txt | grep llo2
```

    + echo -e 'Hello1\nHello2\nHello10'
    + cat file.txt
    + grep o1
    Hello1
    Hello10
    + cat file.txt
    + grep llo2
    Hello2



```bash
%%bash 
# Можно объединять команды с помощью &&
# Тогда вторая выполнится только, если успешно выполнилась первая (как и в C/C++)
echo Hello && echo world!
echo -----------------
echo -n Hello && echo world!
echo -----------------
echo -n "Hello " && echo world!
```

    Hello
    world!
    -----------------
    Helloworld!
    -----------------
    Hello world!



```bash
%%bash 
exec 2>&1 ; set -x
# Создадим пустой файл
touch a.txt
# Выведем список файлов в папке
ls
# Удалим файл
rm a.txt && echo "rm success" || echo "rm fail"
rm a.txt && echo "rm success" || echo "rm fail"
```

    + touch a.txt
    + ls
    a.txt
    file.txt
    intro_linux.ipynb
    lib.c
    lib.so
    README.md
    README_no_output.md
    + rm a.txt
    + echo 'rm success'
    rm success
    + rm a.txt
    rm: cannot remove 'a.txt': No such file or directory
    + echo 'rm fail'
    rm fail



```bash
%%bash 
exec 2>&1 ; set -x
# Начало, конец файла, количество строк
echo -e "1\n2\n3\n4" > a.txt
cat a.txt | head -n 2
cat a.txt | tail -n 2
cat a.txt | wc -l
```

    + echo -e '1\n2\n3\n4'
    + cat a.txt
    + head -n 2
    1
    2
    + cat a.txt
    + tail -n 2
    3
    4
    + cat a.txt
    + wc -l
    4



```python
# Получить документацию по программе легко с помощью команды man
!man head | head -n 10
```

А, важный момент, вообще по умолчанию в ячейках Jupyter notebook пишется код на том языке, с ядром которого запущен ноутбук. Сейчас это python3


```python
with open("b.txt", "w") as f:
    for i in range(100):
        tup = (str(i), "div17" if i % 17 == 0 else "no17")
        f.write("\t".join(tup) + "\n")
```

## <a name="task1"></a> Задача 1


```bash
%%bash
cat b.txt | head -n 5
# Выведите все строчки где есть подстрока div17
# <your line of code here>
```

    0	div17
    1	no17
    2	no17
    3	no17
    4	no17


## <a name="task2"></a> Задача 2


```bash
%%bash
# Выясните, что выведет следующая команда, не запуская ее.
# echo -e "A\nB\nC\nD" | grep -v C -n 
# Используйте только команды man и grep
```


```bash
%%bash
# Подсказка
man grep | grep -e "-C" -C 2
```

                  option, this has no effect and a warning is given.
    
           -C NUM, -NUM, --context=NUM
                  Print NUM lines of output context.  Places a line  containing  a
                  group separator (--) between contiguous groups of matches.  With


## <a name="task3"></a> Задача 3


```bash
%%bash
# Создадим файл на 100 строк
( for i in {0..99} ; do echo line$i ; done ) > a.txt
cat a.txt | head -n 3

# А теперь напишите последовательность команд, которая заменит вторую строчку с line1 на line100
# нельзя использовать replace конструкции, вам достаточно команд, которые были перечислены ранее :)

# <your code here>
```

    line0
    line1
    line2


<details> <summary> Способ в лоб (пожалуйста не смотрите, пока не напишете код решения задачки) </summary>
  <p> cat a.txt | python2 -c 'import sys; lines = list(sys.stdin); lines[1] = "line100\n"; print "".join(lines)' </p>
</details>


```python

```

## <a name="jupyter"></a> Особенности Jupyter notebok используемые в курсе


```python
# код на питоне
[i for i in range(0, 3000, 17) if str(i).endswith("19")] # просто что-то странное
```




    [119, 1819]




```bash
%%bash
echo "Bash code"
```

    Bash code


Jupyter notebook позволяет определять собственную магию. И в первой ячейке каждого моего ноутбука есть одност рочник, который определяет несколько магий:


```python
# Просто вывод строки и ее значения как выражения в питоне
a = 1; b = 2
%p a + b # Sum of a and b
%p (a, b)
```


`a + b = 3`  # Sum of a and b



(a, b) = (1, 2)



```python
%%save_file file.txt 
%# Сохраняет ячейку как файл (А в этой строке просто комментарий)
%run cat file.txt # Выполняет команды следующие после %run в заголовках ячейки
Содержимое файла
```


Run: `cat file.txt # Выполняет команды следующие после %run в заголовках ячейки`


    # %%cpp file.txt 
    # %# Сохраняет ячейку как файл
    # %run cat file.txt # Выполняет команды следующие после %run в заголовках ячейки
    Содержимое файла
    



```python
%%save_file a.sh
%run bash a.sh

echo 123
```


Run: `bash a.sh`


    123



```cpp
%%cpp a.cpp
%# По большому счету тот же save_file, но подсвечивает синтаксис C++
%run g++ a.cpp -o a.exe
%run ./a.exe

#include <iostream>

int main() {
    std::cout << "Hello world!" << std::endl;
}
```


Run: `g++ a.cpp -o a.exe`



Run: `./a.exe`


    Hello world!



```python

```
