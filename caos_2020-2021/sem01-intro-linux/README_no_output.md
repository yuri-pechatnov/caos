

# Вступление: Linux, командная строка, Jupyter notebook

Возможно кому-то Jupyter notebook покажется лишним в этом ряду, но так случилось, что я буду вести у вас АКОС, а мне он кажется очень удобным инструментом :) И вы в будущем все равно с ним столкнетесь на других курсах.

<table width=100%> <tr>
    <th width=20%> <b>Видеозапись семинара &rarr; </b> </th>
    <th>
    <a href="https://www.youtube.com/watch?v=E0lg8pzzR7o&list=PLjzMm8llUm4AmU6i_hPU0NobgA4VsBowc">
        <img src="video.png" width="320"  height="160" align="left" alt="Видео с семинара"> 
    </a>
    </th>
    <th> </th>
 </table>


[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/blob/master/practice/linux_basics/intro.md) **И сейчас, и в будущем: читайте эти ридинги, там много полезной информации. Я стараюсь ее не дублировать, использование моих ноутбуков подразумевает чтение ридингов Яковлева.**


Сегодня в программе:
* <a href="https://github.com/victor-yacovlev/mipt-diht-caos/blob/master/practice/linux_basics/intro.md" style="color:#856024"> Очень кратко о Linux </a>
* <a href="#terminal" style="color:#856024"> Часто используемые команды терминала </a>
  * <a href="#task1" style="color:#856024"> Задача 1 </a>
  * <a href="#task2" style="color:#856024"> Задача 2 </a>
  * <a href="#task3" style="color:#856024"> Задача 3 </a>
* <a href="#jupyter" style="color:#856024"> Особенности Jupyter notebok используемые в курсе </a>


## <a name="linux"></a> Очень кратко о Linux

Читайте [ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/blob/master/practice/linux_basics/intro.md). Там неплохо написано про линукс и не только.

Еще есть [конспект семинара Александра Пономарева](https://github.com/Alexponomarev7/caos_seminars/tree/master/sem1).

И куча статей в интернете :)

Если вы все это знаете, можете поиграть в [bandit](https://overthewire.org/wargames/bandit/). Сложненько, но интересно

## <a name="terminal"></a> Часто используемые команды терминала

Первая важная особенность Jupyter notebook: можно запускать консольные команды, добавляя `!` в начало.
То есть `!echo Hello` запустит `echo Hello` примерно так же, как это происходило бы в консоли.

### `cd`, `pwd`, `cp`, `mv`

`cd` - команда оболочки `bash`

`export PWD=/home` - альтернатива `cd`

Почти все остальные часто используемые команды - на самом деле запускаемые программы.


### `echo`


```python
# Вывод строки
!echo Hello
```

### `>`, `>>`, `cat`


```python
# Перенаправление вывода в файл
!echo "Hello1" > file.txt
!echo "Hello2" > file.txt # Файл перезапишется
# Перенаправление вывода в файл (при этом файл дозапишется, а не перезапишется)
!echo "Hello3" >> file.txt

# Вывод файла 
!cat file.txt
```


```python
! >file.txt echo "Hello1" # И так тоже можно :)
!cat file.txt
```

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

### `|`, `<`


```bash
%%bash 
exec 2>&1 ; set -x # Команды можно писать в одну строчку разделяя с помощью ;

echo -e "Hello1\nHello2\nHello10" > file.txt
# Можно направлять вывод одной команды на вход другой с помощью |
# Команда grep в таком использовании читает все строки из входа, 
#   и выводит только содержащие подстроку`o1`
cat file.txt | grep o1

<file.txt grep llo2
grep llo2 < file.txt
```


```bash
%%bash 
# Можно объединять команды с помощью &&
# Тогда вторая выполнится только, если успешно выполнилась первая (как и в C/C++)
echo -n "Hello " && echo world!
echo -----------------
echo -n "Hello " || echo world! ; echo
echo -----------------
echo -n "Hello " && echBUG1o -n jail && echo -n freedom! ; echo
echo -----------------
echo -n "Hello " && echBUG1o jail || echo freedom!
```


```bash
%%bash 
exec 2>&1 ; set -x
# Создадим пустой файл
touch a.txt
# Выведем список файлов в папке
ls *.txt
# Удалим файл
rm a.txt && echo "rm success" || echo "rm fail"
rm a.txt && echo "rm success" || echo "rm fail"
```


```bash
%%bash 
exec 2>&1 ; set -x
# Начало, конец файла, количество строк
echo -e "1\n2\n3\n4" > a.txt
cat a.txt | head -n 2
cat a.txt | tail -n 2
cat a.txt | wc -l
```


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

<details> <summary> Способ в лоб (пожалуйста не смотрите, пока не напишете код решения задачки) </summary>
  <pre> <code> cat a.txt | python2 -c 'import sys; lines = list(sys.stdin); lines[1] = "line100\n"; print "".join(lines)' </code> </pre>
</details>


```python

```

### Стандартные потоки ввода (stdin, 0), вывода (stdout, 1), ошибок (stderr, 2)

По умолчанию, когда мы запускаем программу, то у нее есть три стандартных потока: 
* ввода (через него все что мы печатаем в терминале передается программе) 
* вывода (через него то, что выводит программа (printf, std::cout) попадает в терминал)
* ошибок (примерно то же, что и stdout, но по-другому исползуется)


```bash
%%bash
echo Hello # Успешное завершение в вывод текста в stdout
rm not_existent_file # Ошибка, и текст пишется в поток stderr
true # Просто успешно завершающаяся команда, чтобы скрипт не завершился ошибкой.
```

Потоки ввода можно перенаправлять в файлы:


```bash
%%bash
echo Hello 1> out.txt 2> err.txt 
echo "stdout: \"`cat out.txt`\" stderr: \"`cat err.txt`\"" 
rm not_existent_file 1> out.txt 2> err.txt 
echo "stdout: \"`cat out.txt`\" stderr: \"`cat err.txt`\""

echo "Hello stXdents!" > file.txt
python2 -c "import sys; print list(sys.stdin)[0].replace('stXdents', 'students')" 0< file.txt
python2 -c "import os; print os.read(10, 100)" 10< file.txt # подумайте, что бы это могло быть :)
```

При этом `>` синоним к `>1` (аналогично `>>`). А `0<` то же самое что и `<`.


```python

```

### `ps`, `top`, `kill`, `killall`, `pidof` - найти, убить, убить попроще, найти по имени


```bash
%%bash
ps aux | grep ipyk
```


```cpp
%%cpp bad_program.cpp
%run g++ -O3 -Os bad_program.cpp -o bad_program.exe
int main() { while (1) {} }
```

`TInteractiveLauncher` - моя магическая штука для запуска программ в интерактивном режиме из Jupyter notebook


```python
a = TInteractiveLauncher("./bad_program.exe")
```


```python
get_ipython().system("ps aux | grep bad_prog")
```


```python
get_ipython().system("kill -9 " + str(a.get_pid()))
a.close()
```


```python
a = TInteractiveLauncher("./bad_program.exe")
get_ipython().system("pidof bad_program.exe")
get_ipython().system("killall -9 bad_program.exe")
a.close()
```


```python

```


```python

```


```python

```


```python

```

## <a name="jupyter"></a> Особенности Jupyter notebok используемые в курсе


```python
# код на питоне
[i for i in range(0, 3000, 17) if str(i).endswith("19")] # просто что-то странное
```


```bash
%%bash
echo "Bash code"
```

Jupyter notebook позволяет определять собственную магию. И в первой ячейке каждого моего ноутбука есть однострочник, который определяет несколько магий:


```python
# Просто вывод строки и ее значения как выражения в питоне
a = 1; b = 2
%p a + b # Sum of a and b
%p (a, b)
```


```python
%%save_file file.txt 
%# Сохраняет ячейку как файл, комментируя загаловок (А в этой строке просто комментарий)
%run cat file.txt # Выполняет команды следующие после %run в заголовках ячейки
Содержимое файла
```


```python
%%save_file a.sh
%run bash a.sh

echo 123
```


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


```python
!cat a.cpp
```


```python

```


```cpp
%%cpp b.cpp
%run g++ b.cpp -o b.exe

#include <iostream>

int main() {
    std::string s;
    std::cin >> s;
    std::cout << "STDOUT " << s << std::endl;
    std::cerr << "STDERR " << s << std::endl;
}
```


```python
# интерактивная запускалка для программ
a = TInteractiveLauncher("./b.exe")
```


```python
a.write("Hello\n")
```


```python
a.close()
```


```python

```
