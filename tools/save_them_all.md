# <a name="how"></a> Как сделать пулл реквест?

0. Выбираем, где хотим провести изменения, в форке репозитория (более предпочтительно, но не принципиально) или в самом репозитории (в этом случае нужно запросить у меня доступ).
1. Нужно произвести все желаемые изменения в семинарском ноутбуке. И убедиться, что эти изменения сохранены (юпитер у меня иногда тупит, поэтому жму трижды `ctrl-s` с интервалом около секунды).
  <br> Постарайтесь ограничиться минимальными изменениями. Так же убедитесь, что у вас актуальная версия репозитория. (Я сам в недавние ноутбуки могу теоретически каждый день коммитить, а мои изменения затирать не надо :) ).
2. Далее в этом ноутбуке (он умеет правильно генерировать `.md` файлы):
  <br>A.  <a href="#what" style="color:#856024"> Здесь </a> выбираем семинар(ы), к которому сделали правку. `../tools` выбирать не надо.
  <br>B.  <a href="#github" style="color:#856024"> Здесь </a> можно написать свой commit message, если есть желание. Можно оставить как есть. В этом репозитории нет культуры хороших сообщений к коммитам :)
  <br>C.  Запускаем этот ноутбук, он сгенерит `.md`-шки и закоммитит изменения на гитхаб.
3. Если изменение было в форке, то делаем пулл реквест.

### <a name="what"></a> Выбираем что коммитить


```python
import glob
import os
import subprocess

highlevel_dirs = sum([
    #["../tools"], 
    sorted(glob.glob("../sem28*")),
    #sorted(glob.glob("../sem28*")),
], [])

print("Highlevel dirs:", highlevel_dirs)
```

    Highlevel dirs: ['../sem28-unix-time']



```python
tmp_dir = "./tmp_dir"
get_ipython().system('rm -r {tmp_dir} ; mkdir {tmp_dir} 2>&1 | grep -v "File exists"'.format(tmp_dir=tmp_dir))
```

### Генерируем все .md-шки стандартными средствами
\+ Делаем .md-шки очищенные для вывода. По этим .md-шкам можно будет смотреть реальную историю изменений. И дифф при пулреквестах.


```python
from multiprocessing import Pool

tasks = []

def convert_tasks(n, d):
    no_output_file = d + "_no_output"
    src_copy = str(abs(hash(n))) + '_' + os.path.basename(n)
    path = os.path.dirname(n)
    return [
        "jupyter nbconvert {} --to markdown --output {}".format(n, d),
        " && ".join([
            "cp {src} {tmp_dir}/{src_copy}",
            "jupyter nbconvert {tmp_dir}/{src_copy} --ClearOutputPreprocessor.enabled=True --inplace",
            "jupyter nbconvert {tmp_dir}/{src_copy} --to markdown --output {src_copy}",
            "cp {tmp_dir}/{src_copy}.md {path}/{no_output_file}.md",
        ]).format(src=n, no_output_file=no_output_file, dst=d, tmp_dir=tmp_dir, src_copy=src_copy, path=path),
    ]
    
for subdir in highlevel_dirs:
    notebooks = glob.glob(subdir + "/*.ipynb")
    print(subdir, notebooks)
    for m in glob.glob(subdir + "/*.md"):
        os.remove(m)
    if len(notebooks) == 1:
        tasks.extend(convert_tasks(notebooks[0], "README"))
    else:
        for n in notebooks:
            tasks.extend(convert_tasks(n, os.path.basename(n.replace(".ipynb", ""))))
        nmds = [os.path.basename(n).replace(".ipynb", ".md") for n in notebooks]
        with open(os.path.join(subdir, "README.md"), "w") as f:
            f.write('\n'.join(
                ['# Ноутбуки семинара'] + 
                ['* [{nmd}]({nmd})'.format(nmd=nmd) for nmd in nmds] + 
                ['']
            ))

print("\n".join(tasks))

def execute_all_in_parallel(tasks):
    ps = []
    for t in tasks:
        ps.append(subprocess.Popen(["bash", "-c", t], stdout=subprocess.PIPE, stderr=subprocess.PIPE))
    for p in ps:
        out, err = p.communicate()
        print(out.decode(), err.decode())

execute_all_in_parallel(tasks)
```

    ../sem28-unix-time ['../sem28-unix-time/time.ipynb']
    jupyter nbconvert ../sem28-unix-time/time.ipynb --to markdown --output README
    cp ../sem28-unix-time/time.ipynb ./tmp_dir/3597715960520351318_time.ipynb && jupyter nbconvert ./tmp_dir/3597715960520351318_time.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/3597715960520351318_time.ipynb --to markdown --output README_no_output && cp ./tmp_dir/README_no_output.md ../sem28-unix-time
     [NbConvertApp] Converting notebook ../sem28-unix-time/time.ipynb to markdown
    [NbConvertApp] Writing 32749 bytes to ../sem28-unix-time/README.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/3597715960520351318_time.ipynb to notebook
    [NbConvertApp] Writing 35263 bytes to ./tmp_dir/3597715960520351318_time.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/3597715960520351318_time.ipynb to markdown
    [NbConvertApp] Writing 28740 bytes to ./tmp_dir/README_no_output.md
    


### Магические исправления

Стандартная конвертилка не учитывает некоторых особенностей маркдауна на гитхабе и некоторых особенностей моих ноутбуков. Поэтому результат надо допилить напильником.


```python
import re


def basic_improve(fname):
    with open(fname, "r") as f:
        r = f.read()
    for b in ["\x00", "\x1B", "\x08"]:
        r = r.replace(b, "")
    with open(fname, "w") as f:
        f.write(r)
    return "dos2unix {}".format(fname)

def improve_md(fname):
    with open(fname, "r") as f:
        r = f.read()
    r = r.replace("```python\n%%cpp", "```cpp\n%%cpp")
    r = r.replace("```python\n%%cmake", "```cmake\n%%cmake")
    r = r.replace('\n', "SUPER_SLASH" + "_N_REPLACER")
    
    r = re.sub(r'\<\!--MD_BEGIN_FILTER--\>.*?\<\!--MD_END_FILTER--\>', "", r)
    #r = re.sub(r'(\#SET_UP_MAGIC_BEGIN.*?\#SET_UP_MAGIC_END)', "<too much code>", r)
    r = re.sub(r'\<\!\-\-\ YANDEX_METRICA_BEGIN\ \-\-\>.*\<\!\-\-\ YANDEX_METRICA_END\ \-\-\>', '', r)
    
    r = r.replace("", '')
    r = r.replace("SUPER_SLASH" + "_N_REPLACER", '\n')
    
    template = "#""MAGICS_SETUP_END"
    bpos = r.rfind(template)
    if bpos != -1:
        r = r[bpos + len(template):]
        template = "```"
        bpos = r.find(template)
        assert bpos >= 0
        r = r[bpos + len(template):]
    
    
    template = "<""!-- MAGICS_SETUP_PRINTING_END -->"
    bpos = r.rfind(template)
    if bpos != -1:
        r = r[bpos + len(template):]
    
    def file_repl(matchobj, path=os.path.dirname(fname)):
        fname = os.path.join(path, matchobj.group(1))
        if fname.find("__FILE__") == -1:
            with open(fname, "r") as f:
                return "\n```\n" + f.read() + "\n```\n"
    
    r = r.replace("", "")
    r = r.replace("", "")
    
    r = re.sub(r'\<\!--MD_FROM_FILE (.*?) --\>', file_repl, r)
    with open(fname, "w") as f:
        f.write(r)
        
def improve_file(fname):
    if fname.endswith(".md"):
        improve_md(fname)

```


```python
tasks = []
shell_tasks = []


for sfx in [".ipynb", ".md"]:
    for hdir in highlevel_dirs:
        for fname in glob.glob("./{}/*".format(hdir) + sfx):
            shell_tasks.append(basic_improve(fname))
            tasks.append(lambda fname=fname: improve_file(fname))
            
execute_all_in_parallel(shell_tasks)
for t in tasks:
    t()
```

     dos2unix: converting file ./../sem28-unix-time/time.ipynb to Unix format...
    
     dos2unix: converting file ./../sem28-unix-time/README.md to Unix format...
    
     dos2unix: converting file ./../sem28-unix-time/README_no_output.md to Unix format...
    


### Смотрим изменения


```python
for subdir in highlevel_dirs:
    get_ipython().system("git diff {}/*_no_output*".format(subdir))
```

    [1mdiff --git a/sem28-unix-time/README_no_output.md b/sem28-unix-time/README_no_output.md[m
    [1mindex f804aef..634cb76 100644[m
    [1m--- a/sem28-unix-time/README_no_output.md[m
    [1m+++ b/sem28-unix-time/README_no_output.md[m
    [36m@@ -1,488 +1,370 @@[m
     [m
     [m
    [31m-# Динамические библиотеки[m
    [32m+[m[32m# Опрос для всех, кто зашел на эту страницу[m
     [m
    [32m+[m[32mОн не страшный, там всего два обязательных вопроса на выбор одного варианта из трёх. Извиняюсь за размер, но к сожалению студенты склонны игнорировать опросы :|[m[41m [m
    [32m+[m
    [32m+[m[32mПытаюсь компенсировать :)[m
    [32m+[m
    [32m+[m[32m<a href="https://docs.google.com/forms/d/e/1FAIpQLSdUnBAae8nwdSduZieZv7uatWPOMv9jujCM4meBZcHlTikeXg/viewform?usp=sf_link"><img src="poll.png" width="100%"  align="left" alt="Опрос"></a>[m
    [32m+[m
    [32m+[m
    [32m+[m
    [32m+[m[32m# Работа со временем в С/С++[m
    [32m+[m
    [32m+[m[32mПоговорим о типах времени в C/C++ и функциях для получения текущего времени, парсинга из строк, сериализации в строки.[m
    [32m+[m
    [32m+[m[32mМеня всегда дико напрягало отсутствие одного хорошего типа времени, наличие времени в разных часовых поясах и куча разных типов сериализации. Постараюсь собрать полезную информацию в одном месте, чтобы жилось проще.[m
     [m
     <table width=100%  > <tr>[m
         <th width=15%> <b>Видео с семинара &rarr; </b> </th>[m
         <th>[m
    [31m-    <a href="https://www.youtube.com/watch?v=JLfINSChfOo&list=PLjzMm8llUm4CL-_HgDrmoSTZBCdUk5HQL&index=1"><img src="video.png" width="320" [m
    [32m+[m[32m    <a href="???"><img src="video.jpg" width="320"[m[41m [m
        height="160" align="left" alt="Видео с семинара"></a>[m
         </th>[m
         <th> </th>[m
      </table>[m
     [m
     [m
    [32m+[m
     Сегодня в программе:[m
    [31m-* Создание и подгрузка динамической библиотеки[m
    [31m-  * <a href="#create_dynlib" style="color:#856024">Создание</a>[m
    [31m-  * Подгрузка [m
    [31m-    1. <a href="#load_с" style="color:#856024">При старте средствами OS (динамическая компоновка)</a> [m
    [31m-    <br> Вот [это](https://www.ibm.com/developerworks/ru/library/l-dynamic-libraries/) можно почитать для понимания, что в этом случае происходит.[m
    [31m-    2. В произвольный момент времени:[m
    [31m-      * <a href="#load_python" style="color:#856024">Из python</a> [m
    [31m-      * <a href="#load_с_std" style="color:#856024">Из программы на С (dlopen)</a> [m
    [31m-      * <a href="#load_с_mmap" style="color:#856024">Из программы на С с извращениями (mmap)</a> [m
    [31m-* Нетривиальный пример применения динамических библиотек[m
    [31m-  <br> <a href="#c_interpreter" style="color:#856024">Развлекаемся и пишем простенький интерпретатор языка C (с поблочным выполнением команд)</a> [m
    [31m-  [m
    [31m-  [m
    [31m-Факты:[m
    [31m-* Статическая линковка быстрее динамической. И при запуске программы и при непосредственно работе.[m
    [31m-* https://agner.org/optimize/optimizing_cpp.pdf c155[m
    [31m-  [m
    [31m-<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>[m
    [31m-[m
    [31m-[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/function-pointers)[m
    [31m-[m
    [31m-# <a name="create_dynlib"></a> Создание динамической библиотеки [m
    [32m+[m[32m* <a href="types_c" style="color:#856024"> Типы времени в C </a>[m
    [32m+[m[32m* <a href="funcs_c" style="color:#856024"> Функции для работы со временем в C </a>[m
    [32m+[m[32m* <a href="types_cpp" style="color:#856024"> Типы времени в C++ </a>[m
    [32m+[m[32m* <a href="funcs_cpp" style="color:#856024"> Функции для работы со временем в C++ </a>[m
    [32m+[m[32m<br><br>[m
    [32m+[m[32m* <a href="clocks_and_cpu" style="color:#856024"> Разные часы и процессорное время </a>[m
    [32m+[m[32m* <a href="benchmarking" style="color:#856024"> Время для бенчмарков </a>[m
    [32m+[m[32m<br><br>[m
    [32m+[m[32m* <a href="sleep" style="color:#856024"> Как поспать? </a>[m
    [32m+[m[32m<br><br>[m
    [32m+[m[32m* <a href="problems" style="color:#856024"> Задачки для самостоятельного решения </a>[m
     [m
    [32m+[m[41m [m
     [m
    [31m-```cpp[m
    [31m-%%cpp lib.c[m
    [31m-%# `-shared` - make shared library[m
    [31m-%# `-fPIC` - make Positional Independant Code[m
    [31m-%run gcc -Wall -shared -fPIC lib.c -o libsum.so # compile shared library[m
    [31m-%run objdump -t libsum.so | grep sum # symbols in shared library filtered by 'sum'[m
    [31m-[m
    [31m-int sum(int a, int b) {[m
    [31m-    return a + b;[m
    [31m-}[m
     [m
    [31m-float sum_f(float a, float b) {[m
    [31m-    return a + b;[m
    [31m-}[m
    [31m-```[m
    [32m+[m[32m## <a name="types_c"></a> Типы времени в C[m
     [m
    [31m-# <a name="load_python"></a> Загрузка динамической библиотеки из python'а[m
    [32m+[m[32mЧто у нас есть?[m
     [m
    [32m+[m[32mСобственно типы времени[m
    [32m+[m[32m* `time_t` - целочисленный тип, в котором хранится количество секунд с начала эпохи. В общем таймстемп в секундах. [man](https://www.opennet.ru/man.shtml?topic=time&category=2)[m
    [32m+[m[32m* `struct tm` - структурка в которой хранится год, месяц, ..., секунда [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=ctime&category=3)[m
    [32m+[m[32m* `struct timeval` пара (секунды, миллисекунды) (с начала эпохи, если используется как момент времени) [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=gettimeofday&category=2)[m
    [32m+[m[32m* `struct timespec` пара (секунды, наносекунды) [man](https://www.opennet.ru/man.shtml?topic=select&category=2&russian=)[m
    [32m+[m[32m* `struct timeb` - секунды, миллисекунды, таймзона+информация о летнем времени [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=ftime&category=3) (Я ни разу не сталкивался, но и такая есть)[m
     [m
    [31m-```python[m
    [31m-import ctypes[m
    [32m+[m[32mЧасовой пояс[m
    [32m+[m[32m* `struct timezone` - [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=gettimeofday&category=2)[m
     [m
    [31m-lib = ctypes.CDLL("./libsum.so")[m
    [31m-%p lib.sum(3, 4) # По умолчанию считает типы int'ами, поэтому в этом случае все хорошо[m
    [31m-%p lib.sum_f(3, 4) # А здесь python передает в функцию инты, а она принимает float'ы. Тут может нарушаться соглашение о вызовах и происходить что угодно[m
     [m
    [31m-# Скажем, какие на самом деле типы в сигнатуре функции[m
    [31m-lib.sum_f.argtypes = [ctypes.c_float, ctypes.c_float][m
    [31m-lib.sum_f.restype = ctypes.c_float[m
    [31m-%p lib.sum_f(3, 4) # Теперь все работает хорошо[m
    [31m-```[m
    [32m+[m[32m## <a name="funcs_c"></a> Функции для работы с временем в C[m
     [m
    [31m-# <a name="load_с"></a> Загрузка динамической библиотеки из программы на С. Стандартными средствами, автоматически при старте программы[m
    [32m+[m[32mДо всего последующего хочется напомнить, что многие функции в C не потокобезопасны (если не заканчиваются на `_r`, что означает reentrant, ну и потокобезопасность). Поэтому, перед использованием, стоит посмотреть документацию.[m
     [m
    [32m+[m[32mКонвертация:[m
    [32m+[m[32m<table>[m
    [32m+[m[32m<tr>[m
    [32m+[m[32m  <th>Из чего\Во что</th>[m
    [32m+[m[32m  <th>time_t</th>[m
    [32m+[m[32m  <th>struct tm</th>[m
    [32m+[m[32m  <th>struct timeval</th>[m
    [32m+[m[32m  <th>struct timespec</th>[m
    [32m+[m[41m [m
    [32m+[m[32m<tr> <td>time_t[m
    [32m+[m[32m  <td>=[m
    [32m+[m[32m  <td><a href="https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=ctime&category=3"><code>gmtime_r</code></a>/<a href="https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=ctime&category=3"><code>localtime_r</code></a>[m
    [32m+[m[32m  <td>{.tv_sec = x}[m
    [32m+[m[32m  <td>{.tv_sec = x}[m
     [m
    [31m-```cpp[m
    [31m-%%cpp ld_exec_dynlib_func.c[m
    [31m-%# `-lsum` - подключаем динамическую библиотеку `libsum.so`[m
    [31m-%# `-L.` - во время компиляции ищем библиотеку в директории `.`[m
    [31m-%# `-Wl,-rpath -Wl,'$ORIGIN/'.` - говорим линкеру, чтобы он собрал программу так[m
    [31m-%# чтобы при запуске она искала библиотеку в `'$ORIGIN/'.`. То есть около исполняемого файла программы[m
    [31m-%run gcc -Wall -g ld_exec_dynlib_func.c -L. -lsum -Wl,-rpath -Wl,'$ORIGIN/'. -o ld_exec_dynlib_func.exe[m
    [31m-%run ./ld_exec_dynlib_func.exe[m
    [32m+[m[32m<tr> <td>struct tm[m
    [32m+[m[32m  <td><a href="https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=ctime&category=3"><code>mktime</code></a> [1][m
    [32m+[m[32m  <td>=[m
    [32m+[m[32m  <td>через time_t[m
    [32m+[m[32m  <td>через time_t[m
     [m
    [31m-#include <stdio.h>[m
    [32m+[m[32m<tr> <td>struct timeval[m
    [32m+[m[32m  <td>x.tv_sec[m
    [32m+[m[32m  <td>через time_t[m
    [32m+[m[32m  <td>=[m
    [32m+[m[32m  <td>{.tv_sec = x.tv_sec, .tv_nsec = x.tv_usec * 1000}[m
     [m
    [31m-// объявляем функции[m
    [31m-// ~ #include "sum.h"[m
    [31m-int sum(int a, int b);[m
    [31m-float sum_f(float a, float b);[m
    [32m+[m[32m<tr> <td>struct timespec[m
    [32m+[m[32m  <td>x.tv_sec[m
    [32m+[m[32m  <td>через time_t[m
    [32m+[m[32m  <td>{.tv_sec = x.tv_sec, .tv_usec = x.tv_nsec / 1000}[m
    [32m+[m[32m  <td>=[m
     [m
    [31m-int main() {  [m
    [31m-    #define p(stmt, fmt) printf(#stmt " = " fmt "\n", stmt);[m
    [31m-    p(sum(1, 1), "%d");[m
    [31m-    p(sum(40, 5000), "%d");[m
    [31m-    [m
    [31m-    p(sum_f(1, 1), "%0.2f");[m
    [31m-    p(sum_f(4.0, 500.1), "%0.2f");[m
    [31m-    return 0;[m
    [31m-}[m
    [31m-```[m
    [32m+[m[32m</table>[m
     [m
    [32m+[m[32m[1] - `mktime` неадекватно работает, когда у вас не локальное время. Подробности и как с этим жить - в примерах. https://stackoverflow.com/questions/530519/stdmktime-and-timezone-info[m
     [m
    [31m-```python[m
    [31m-!ldd ld_exec_dynlib_func.exe[m
    [31m-!mkdir tmp[m
    [31m-!cp ld_exec_dynlib_func.exe tmp/ld_exec_dynlib_func.exe[m
    [31m-!ldd tmp/ld_exec_dynlib_func.exe[m
    [31m-```[m
    [32m+[m[32mПолучение:[m
    [32m+[m[32m* `time` - получить время как `time_t` [man](https://www.opennet.ru/man.shtml?topic=time&category=2)[m
    [32m+[m[32m* `clock_gettime` - получить время как `struct timespec` [man](https://www.opennet.ru/man.shtml?topic=clock_gettime&category=3&russian=2)[m
    [32m+[m[32m* `gettimeofday` - получить время как `struct timeval` [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=settimeofday&category=2)[m
     [m
    [31m-# <a name="load_с_std"></a> Загрузка динамической библиотеки из программы на С в произвольный момент времени, используя стандартные функции[m
    [32m+[m[32mПарсинг:[m
    [32m+[m[32m* Если таймстемп - то просто читаем как число.[m
    [32m+[m[32m* `strptime` [man](https://www.opennet.ru/man.shtml?topic=strptime&category=3&russian=0) Не умеет во временные зоны, всегда локальную выставляет[m
    [32m+[m[32m* `getdate` [man](https://opennet.ru/man.shtml?topic=getdate&category=3) Не рекомендую, не очень умная функция.[m
     [m
    [32m+[m[32mСериализация:[m
    [32m+[m[32m* Всегда можно просто записать таймстемп в секундах/миллисекундах.[m
    [32m+[m[32m* `strftime` - позволяет превратить struct tm в строку, используя printf-подобную форматную строку [man](https://www.opennet.ru/man.shtml?topic=strftime&category=3)[m
     [m
    [31m-```cpp[m
    [31m-%%cpp stdload_exec_dynlib_func.c[m
    [31m-%# `-ldl` - пародоксально, но для подгрузки динамических библиотек, нужно подгрузить динамическую библиотеку[m
    [31m-%run gcc -Wall -g stdload_exec_dynlib_func.c -ldl -o stdload_exec_dynlib_func.exe[m
    [31m-%run ./stdload_exec_dynlib_func.exe[m
    [32m+[m[32mАрифметические операции:[m
    [32m+[m[32m* Их нет, все вручную?[m
     [m
    [31m-#include <stdio.h>[m
    [31m-#include <stdlib.h>[m
    [31m-#include <unistd.h>[m
    [31m-#include <sys/types.h>[m
    [31m-#include <sys/stat.h>[m
    [31m-#include <sys/mman.h>[m
    [31m-#include <fcntl.h>[m
    [31m-#include <assert.h>[m
    [31m-#include <dlfcn.h>[m
    [32m+[m[32mРабота с часовыми поясами:[m
    [32m+[m[32m  Прежде всего замечание: в рамках этого семинара считаем, что время в GMT = время в UTC.[m
     [m
    [31m-int main() {  [m
    [31m-    [m
    [31m-    void *lib_handle = dlopen("./libsum.so", RTLD_NOW);[m
    [31m-    if (!lib_handle) {[m
    [31m-        fprintf(stderr, "dlopen: %s\n", dlerror());[m
    [31m-        abort();[m
    [31m-    }[m
    [31m-   [m
    [31m-    int (*sum)(int, int) = dlsym(lib_handle, "sum");[m
    [31m-    float (*sum_f)(float, float) = dlsym(lib_handle, "sum_f");[m
    [31m-    [m
    [31m-    #define p(stmt, fmt) printf(#stmt " = " fmt "\n", stmt);[m
    [31m-    p(sum(1, 1), "%d");[m
    [31m-    p(sum(40, 5000), "%d");[m
    [31m-    [m
    [31m-    p(sum_f(1, 1), "%0.2f");[m
    [31m-    p(sum_f(4.0, 500.1), "%0.2f");[m
    [31m-    [m
    [31m-    dlclose(lib_handle);[m
    [31m-    return 0;[m
    [31m-}[m
    [31m-```[m
    [32m+[m[32m* Сериализация таймстемпа как локального или UTC времени - `localtime_t`/`gmtime_r`.[m
    [32m+[m[32m* Парсинг локального времени - `strptime`.[m
    [32m+[m[32m* Другие часовые пояса и парсинг human-readable строк c заданным часовым поясом только через установку локалей, переменных окружения. В общем избегайте этого[m
     [m
    [31m-# <a name="load_с_std"></a> Загрузка динамической библиотеки из программы на С в произвольный момент времени, используя mmap[m
     [m
    [31m-В примере отсутствует парсинг elf файла, чтобы выцепить адреса функций. Поэтому они просто захардкожены[m
    [32m+[m[32m```python[m
    [32m+[m[32m# В питоне примерно то же самое, что и в С[m
    [32m+[m[32mimport time[m
    [32m+[m[32mprint("* Таймстемп (time_t): ", time.time())[m
    [32m+[m[32mprint("* Дата (struct tm): ", time.localtime(time.time()))[m
    [32m+[m[32mprint("* Дата (struct tm): ", time.gmtime(time.time()), "(обращаем внимание на разницу в часовых поясах)")[m
    [32m+[m[32mprint("* tm_gmtoff для local:", time.localtime(time.time()).tm_gmtoff,[m[41m [m
    [32m+[m[32m      "и для gm: ", time.gmtime(time.time()).tm_gmtoff, "(скрытое поле, но оно используется :) )")[m
    [32m+[m[32mprint("* Дата human-readable (local): ", time.strftime("%Y.%m.%d %H:%M:%S %z", time.localtime(time.time())))[m
    [32m+[m[32mprint("* Дата human-readable (gmt): ", time.strftime("%Y.%m.%d %H:%M:%S %z", time.gmtime(time.time())))[m
    [32m+[m[32m```[m
     [m
     [m
     ```cpp[m
    [31m-%%cpp mmap_exec_dynlib_func.c[m
    [31m-%run gcc -Wall -fsanitize=address -g mmap_exec_dynlib_func.c -o mmap_exec_dynlib_func.exe[m
    [31m-%run ./mmap_exec_dynlib_func.exe[m
    [32m+[m[32m%%cpp time.c[m
    [32m+[m[32m%run gcc -fsanitize=address time.c -lpthread -o time_c.exe[m
    [32m+[m[32m%run ./time_c.exe[m
    [32m+[m
    [32m+[m[32m#define _BSD_SOURCE[m
    [32m+[m[32m#define _GNU_SOURCE  // для strptime[m
     [m
     #include <stdio.h>[m
     #include <stdlib.h>[m
    [31m-#include <unistd.h>[m
    [32m+[m[32m#include <time.h>[m
     #include <sys/types.h>[m
    [31m-#include <sys/stat.h>[m
    [31m-#include <sys/mman.h>[m
    [31m-#include <fcntl.h>[m
    [32m+[m[32m#include <sys/time.h>[m
     #include <assert.h>[m
    [32m+[m[32m#include <string.h>[m
    [32m+[m
    [32m+[m[32m// Я не уверен, что так делать норм[m
    [32m+[m[32mtime_t as_utc_timestamp(struct tm timeTm) {[m
    [32m+[m[32m    time_t timestamp = mktime(&timeTm); // mktime распарсит как локальное время, даже если tm_gmtoff в 0 сбросить[m
    [32m+[m[32m    //               ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ Извращение, чтобы получить нормальный таймстемп UTC[m
    [32m+[m[32m    return timestamp + timeTm.tm_gmtoff; // mktime выставит tm_gmtoff в соответствии с текущей таймзоной[m
    [32m+[m[32m}[m
     [m
     int main() {[m
    [31m-    int fd = open("libsum.so", O_RDWR);[m
    [31m-    struct stat s;[m
    [31m-    assert(fstat(fd, &s) == 0);[m
    [31m-    void* mapped = mmap([m
    [31m-        /* desired addr, addr = */ NULL, [m
    [31m-        /* length = */ s.st_size, [m
    [31m-        /* access attributes, prot = */ PROT_READ | PROT_EXEC | PROT_WRITE, // обратите внимание на PROT_EXEC[m
    [31m-        /* flags = */ MAP_SHARED,[m
    [31m-        /* fd = */ fd,[m
    [31m-        /* offset in file, offset = */ 0[m
    [31m-    );[m
    [31m-    assert(close(fd) == 0); // Не забываем закрывать файл (при закрытии регион памяти остается доступным)[m
    [31m-    if (mapped == MAP_FAILED) {[m
    [31m-        perror("Can't mmap");[m
    [31m-        return -1;[m
    [32m+[m[32m    { // (1)[m
    [32m+[m[32m        struct timespec spec = {0};[m[41m [m
    [32m+[m[32m        clock_gettime(CLOCK_REALTIME, &spec);[m
    [32m+[m[41m        [m
    [32m+[m[32m        time_t timestamp = spec.tv_sec;[m
    [32m+[m[32m        struct tm local_tm = {0};[m
    [32m+[m[32m        localtime_r(&timestamp, &local_tm);[m
    [32m+[m[41m        [m
    [32m+[m[32m        char time_str[100];[m[41m [m
    [32m+[m[32m        size_t time_len = strftime(time_str, sizeof(time_str), "%Y.%m.%d %H:%M:%S", &local_tm);[m
    [32m+[m[32m        time_len += snprintf(time_str + time_len, sizeof(time_str) - time_len, ".%09ld", spec.tv_nsec);[m
    [32m+[m[32m        time_len += strftime(time_str + time_len, sizeof(time_str) - time_len, " %Z", &local_tm);[m
    [32m+[m[32m        printf("(1) Current time: %s\n", time_str);[m
         }[m
    [31m- [m
    [31m-    int (*sum)(int, int) = (void*)((char*)mapped + 0x620); // 0x620 - тот самый оффсет из objdump'a[m
    [31m-    float (*sum_f)(float, float) = (void*)((char*)mapped + 0x634); [m
         [m
    [31m-    #define p(stmt, fmt) printf(#stmt " = " fmt "\n", stmt);[m
    [31m-    [m
    [31m-    p(sum(1, 1), "%d");[m
    [31m-    p(sum(40, 5000), "%d");[m
    [32m+[m[32m    { // (2)[m
    [32m+[m[32m        const char* utc_time = "2020.08.15 12:48:06";[m
    [32m+[m[41m        [m
    [32m+[m[32m        struct tm local_tm = {0};[m
    [32m+[m[32m        strptime(utc_time, "%Y.%m.%d %H:%M:%S", &local_tm); // распарсит как локальное время[m
    [32m+[m[41m        [m
    [32m+[m[32m        time_t timestamp = as_utc_timestamp(local_tm);[m[41m [m
    [32m+[m[32m        localtime_r(&timestamp, &local_tm);[m
    [32m+[m[41m        [m
    [32m+[m[32m        char time_str[100];[m[41m [m
    [32m+[m[32m        size_t time_len = strftime(time_str, sizeof(time_str), "%Y.%m.%d %H:%M:%S%z", &local_tm);[m
    [32m+[m[32m        printf("(2) Recovered time by strptime: %s (given utc time: %s)\n", time_str, utc_time);[m
    [32m+[m[32m    }[m
         [m
    [31m-    p(sum_f(1, 1), "%0.2f");[m
    [31m-    p(sum_f(4.0, 500.1), "%0.2f");[m
    [32m+[m[32m    { // (3)[m
    [32m+[m[32m        time_t timestamps[] = {1589227667, 840124800, -1};[m
    [32m+[m[32m        for (time_t* timestamp = timestamps; *timestamp != -1; ++timestamp) {[m
    [32m+[m[32m            struct tm local_time = {0};[m
    [32m+[m[32m            localtime_r(timestamp, &local_time);[m
    [32m+[m[32m            char time_str[100];[m[41m [m
    [32m+[m[32m            size_t time_len = strftime(time_str, sizeof(time_str), "%Y.%m.%d %H:%M:%S", &local_time);[m
    [32m+[m[32m            printf("(3) Timestamp %ld -> %s\n", *timestamp, time_str);[m
    [32m+[m[32m        }[m
    [32m+[m[32m    }[m
     [m
    [31m-    assert(munmap([m
    [31m-        /* mapped addr, addr = */ mapped, [m
    [31m-        /* length = */ s.st_size[m
    [31m-    ) == 0);[m
         return 0;[m
     }[m
     ```[m
     [m
    [31m-# <a name="c_interpreter"></a> Простенький интерпретатор для С[m
    [31m-[m
    [31m-Идея такая: на каждый кусочек кода будем компилировать динамическую библиотеку, подгружать ее, и выполнять из нее функцию, в которой будет этот самый кусочек.[m
    [31m-[m
    [31m-Взаимодействие между кусочками через глобальные переменные. (Все кусочки кода выполняются в основном процессе.)[m
    [31m-[m
    [31m-Каждая динамическя библиотека компонуется со всеми предыдущими, чтобы видеть их глобальные переменные. Для этого же при загрузке библиотек берется опция RTLD_GLOBAL.[m
    [31m-[m
    [31m-[m
    [31m-```python[m
    [31m-!rm -r tmp a.txt[m
    [31m-!mkdir tmp[m
    [31m-```[m
    [31m-[m
     [m
     ```python[m
    [31m-import os[m
    [31m-import subprocess[m
    [31m-import ctypes[m
    [31m-from textwrap import dedent[m
    [31m-[m
    [31m-uniq_counter = globals().get("uniq_counter", 0) + 1[m
    [31m-libs = [][m
    [31m-all_includes = [][m
    [31m-all_variables = [][m
    [31m-[m
     [m
    [31m-def add_includes_c(includes):[m
    [31m-    global all_includes[m
    [31m-    all_includes = list(set(all_includes) | set(includes.split('\n')))[m
    [31m-[m
    [31m-    [m
    [31m-def declare_c(declaration):[m
    [31m-    assignment_pos = declaration.find('=')[m
    [31m-    assignment_pos = assignment_pos if assignment_pos != -1 else len(declaration)[m
    [31m-    decl_part = declaration[:assignment_pos].rstrip()[m
    [31m-    var_name_begin = decl_part.rfind(' ')[m
    [31m-    var_assignment = declaration[var_name_begin:][m
    [31m-    interprete_c(var_assignment, variables=[decl_part])[m
    [31m-[m
    [31m-    [m
    [31m-def interprete_c(code="", variables=[]):[m
    [31m-    func_name = "lib_func_%d_%d" % (uniq_counter, len(libs))[m
    [31m-    source_name = "./tmp/" + func_name + ".c"[m
    [31m-    lib_name = "lib" + func_name + ".so"[m
    [31m-    lib_file = "./tmp/" + lib_name[m
    [31m-    includes_list = "\n".join(all_includes)[m
    [31m-    variables_list = "; ".join("extern " + v for v in all_variables) + "; " + "; ".join(variables)[m
    [31m-    out_file = "./tmp/" + func_name + ".out" [m
    [31m-    err_file = "./tmp/" + func_name + ".err" [m
    [31m-    lib_code = dedent('''\[m
    [31m-        #include <stdio.h>[m
    [31m-        {includes_list}[m
    [31m-        {variables_list};[m
    [31m-        void {func_name}() {{[m
    [31m-            freopen("{err_file}", "w", stderr);[m
    [31m-            freopen("{out_file}", "w", stdout);[m
    [31m-            {code};[m
    [31m-            fflush(stderr);[m
    [31m-            fflush(stdout);[m
    [31m-        }}[m
    [31m-        ''').format(**locals())[m
    [31m-    with open(source_name, "w") as f:[m
    [31m-        f.write(lib_code)[m
    [31m-    compile_cmd = ([m
    [31m-        ["gcc", "-Wall", "-shared", "-fPIC", source_name, "-Ltmp"] + [m
    [31m-        ['-l' + lib_f for lib_f in libs] + [m
    [31m-        ["-Wl,-rpath", "-Wl," + os.path.join(os.getcwd(), "tmp"), "-o", lib_file][m
    [31m-    )[m
    [31m-    try:[m
    [31m-        subprocess.check_output(compile_cmd)[m
    [31m-    except:[m
    [31m-        print("%s\n%s" % (lib_code, " ".join(compile_cmd)))[m
    [31m-        get_ipython().run_cell("!" + " ".join(compile_cmd))[m
    [31m-        raise[m
    [31m-    [m
    [31m-    lib = ctypes.CDLL(lib_file, ctypes.RTLD_GLOBAL)  # RTLD_GLOBAL - важно! Чтобы позднее загруженные либы видели ранее загруженные[m
    [31m-    func = lib[func_name][m
    [31m-    func()[m
    [31m-    for fname, stream in [(err_file, sys.stderr), (out_file, sys.stdout)]:[m
    [31m-        with open(fname, "r") as f:[m
    [31m-            txt = f.read()[m
    [31m-            if txt:[m
    [31m-                print(txt, file=stream)[m
    [31m-    libs.append(func_name)[m
    [31m-    all_variables.extend(variables)[m
    [31m-    [m
     ```[m
     [m
    [32m+[m[32m## <a name="types_cpp"></a> Типы времени в C++[m
     [m
    [31m-```python[m
    [31m-interprete_c(r'''[m
    [31m-    printf("%d", 40 + 2); [m
    [31m-    dprintf(2, "Hello world!");[m
    [31m-''')[m
    [31m-```[m
    [32m+[m[32mДля начала нам доступно все то же, что было в С.[m
     [m
    [32m+[m[32mНовые типы времени[m
    [32m+[m[32m* `std::tm = struct tm`, `std::time_t = struct tm` - типы старые, но способ написания новый :)[m
    [32m+[m[32m* `std::chrono::time_point` [doc](https://en.cppreference.com/w/cpp/chrono/time_point)[m
    [32m+[m[32m* `std::chrono::duration` [doc](https://en.cppreference.com/w/cpp/chrono/duration)[m
     [m
    [31m-```python[m
    [31m-add_includes_c('''[m
    [31m-    #include <math.h>"[m
    [31m-''')[m
    [31m-interprete_c('''[m
    [31m-    printf("%f", cos(60.0 / 180 * 3.1415))[m
    [31m-''')[m
    [31m-```[m
     [m
    [32m+[m[32mСкажу откровенно, добавились не самые удобные типы. Единственное, что сделано удобно - арифметика времени.[m
     [m
    [31m-```python[m
    [31m-declare_c('''[m
    [31m-   int a = 4242[m
    [31m-''')[m
    [31m-```[m
    [32m+[m[32m## <a name="funcs_cpp"></a> Функции для работы с временем в C++[m
     [m
     [m
    [31m-```python[m
    [31m-interprete_c('''[m
    [31m-    printf("1) %d", a);[m
    [31m-''')[m
    [31m-interprete_c('''[m
    [31m-    printf("2) %06d", a);[m
    [31m-''')[m
    [31m-interprete_c('''[m
    [31m-    printf("3) %6d", a);[m
    [31m-''')[m
    [31m-interprete_c('''[m
    [31m-    printf("4) %0.2f", (float)a);[m
    [31m-''')[m
    [31m-```[m
    [31m-[m
    [32m+[m[32mКонвертация:[m
    [32m+[m[32m* `std::chrono::system_clock::to_time_t`, `std::chrono::system_clock::from_time_t`[m
     [m
    [31m-```python[m
    [31m-add_includes_c('''[m
    [31m-    #include <sys/types.h>[m
    [31m-    #include <sys/stat.h>[m
    [31m-    #include <fcntl.h>[m
    [31m-    #include <unistd.h>[m
    [31m-''')[m
    [31m-declare_c('''[m
    [31m-    int fd = open("./a.txt", O_WRONLY | O_CREAT, 0644)[m
    [31m-''')[m
    [31m-interprete_c('''[m
    [31m-    dprintf(fd, "Hello students! a = %d", a);[m
    [31m-    close(fd);[m
    [31m-    printf("a.txt written and closed!");[m
    [31m-''')[m
    [31m-```[m
    [31m-[m
    [31m-[m
    [31m-```python[m
    [31m-!cat a.txt[m
    [31m-```[m
    [32m+[m[32mСериализация и парсинг:[m
    [32m+[m[32m* `std::get_time` / `std::put_time` - примерно то же самое, что `strftime` и `strptime` в C. Работают с `std::tm`. [doc](https://en.cppreference.com/w/cpp/io/manip/get_time)[m
     [m
    [31m-# <a name="cpp"></a> Особенности с С++[m
    [32m+[m[32mАрифметические операции:[m
    [32m+[m[32m* Из коробки, обычными +/*[m
     [m
    [31m-[Itanium C++ ABI](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling) - тут есть про манглинг[m
     [m
     [m
     ```cpp[m
    [31m-%%cpp libsumcpp.cpp[m
    [31m-%run g++ -std=c++11 -Wall -shared -fPIC libsumcpp.cpp -o libsumcpp.so # compile shared library[m
    [31m-%run objdump -t libsumcpp.so | grep um[m
    [32m+[m[32m%%cpp time.cpp[m
    [32m+[m[32m%run clang++ -std=c++14 -fsanitize=address time.cpp -lpthread -o time_cpp.exe[m
    [32m+[m[32m%run ./time_cpp.exe[m
     [m
    [31m-extern "C" {[m
    [31m-    int sum_c(int a, int b) {[m
    [31m-        return a + b;[m
    [32m+[m[32m#include <iostream>[m
    [32m+[m[32m#include <sstream>[m
    [32m+[m[32m#include <locale>[m
    [32m+[m[32m#include <iomanip>[m
    [32m+[m[32m#include <chrono>[m
    [32m+[m[32m#include <time.h> // localtime_r[m
    [32m+[m
    [32m+[m[32mtime_t as_utc_timestamp(struct tm t) {[m
    [32m+[m[32m    time_t timestamp = mktime(&t); // mktime распарсит как локальное время, даже если tm_gmtoff в 0 сбросить[m
    [32m+[m[32m    //               ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ Извращение, чтобы получить нормальный таймстемп UTC[m
    [32m+[m[32m    return timestamp + t.tm_gmtoff; // mktime выставит tm_gmtoff в соответствии с текущей таймзоной[m
    [32m+[m[32m}[m
    [32m+[m
    [32m+[m[32mint main() {[m
    [32m+[m[32m    { // (0)[m
    [32m+[m[32m        using namespace std::literals;[m
    [32m+[m[32m        auto nowChrono = std::chrono::system_clock::now();[m
    [32m+[m[32m        std::time_t timestamp = std::chrono::system_clock::to_time_t(nowChrono);[m
    [32m+[m[32m        std::tm timeTm = {};[m
    [32m+[m[32m        timestamp = 1589401219;[m
    [32m+[m[32m        localtime_r(&timestamp, &timeTm);[m[41m [m
    [32m+[m[32m        uint64_t nowMs = (nowChrono.time_since_epoch() % 1s) / 1ms;[m
    [32m+[m[32m        std::cout << "(0) Current time: "[m[41m [m
    [32m+[m[32m                  << std::put_time(&timeTm, "%Y.%m.%d %H:%M:%S")[m[41m [m
    [32m+[m[32m                  << "." << std::setfill('0') << std::setw(3) << nowMs << " "[m
    [32m+[m[32m                  << std::put_time(&timeTm, "%z") << " "[m
    [32m+[m[32m                  << ", timestamp = " << timestamp << "'\n";[m
         }[m
    [31m-} [m
     [m
    [31m-int sum_cpp(int a, int b) {[m
    [31m-    return a + b;[m
    [31m-}[m
    [32m+[m[32m    { // (1)[m
    [32m+[m[32m        std::string timeStr = "2011-Jan-18 23:12:34";[m
    [32m+[m[41m        [m
    [32m+[m[32m        std::tm timeTm = {};[m
    [32m+[m[41m        [m
    [32m+[m[32m        std::istringstream timeStrStream{timeStr};[m
    [32m+[m[32m        timeStrStream.imbue(std::locale("en_US.utf-8"));[m
    [32m+[m[32m        timeStrStream >> std::get_time(&timeTm, "%Y-%b-%d %H:%M:%S");[m
    [32m+[m[41m        [m
    [32m+[m[32m        if (timeStrStream.fail()) {[m
    [32m+[m[32m            std::cout << "(1) Parse failed\n";[m
    [32m+[m[32m        } else {[m
    [32m+[m[32m            std::cout << "(1) Parsed time '" << std::put_time(&timeTm, "%Y.%m.%d %H:%M:%S %z") << "'"[m
    [32m+[m[32m                      << " from '" << timeStr << "''\n";[m
    [32m+[m[32m        }[m
    [32m+[m[32m    }[m
    [32m+[m[41m    [m
    [32m+[m[32m    { // (2)[m
    [32m+[m[32m        using namespace std::literals;[m
    [32m+[m[32m        auto nowChrono = std::chrono::system_clock::now();[m
    [32m+[m[32m        for (int i = 0; i < 2; ++i, nowChrono += 23h + 55min) {[m
    [32m+[m[32m            std::time_t nowTimestamp = std::chrono::system_clock::to_time_t(nowChrono);[m
    [32m+[m[32m            std::tm localTm = {};[m
    [32m+[m[32m            localtime_r(&nowTimestamp, &localTm); // кажись в C++ нет потокобезопасной функции[m
    [32m+[m[32m            std::cout << "(2) Composed time: " << std::put_time(&localTm, "%Y.%m.%d %H:%M:%S %z") << "\n";[m
    [32m+[m[32m        }[m
    [32m+[m[32m    }[m
    [32m+[m[41m    [m
    [32m+[m[32m    { // (3)[m
    [32m+[m[32m        using namespace std::literals;[m
    [32m+[m[41m        [m
    [32m+[m[32m        std::string timeStr = "1977.01.11 22:35:22";[m
    [32m+[m[41m        [m
    [32m+[m[32m        std::tm timeTm = {};[m
    [32m+[m[32m        std::istringstream timeStrStream{timeStr};[m
    [32m+[m[32m        timeStrStream >> std::get_time(&timeTm, "%Y.%m.%d %H:%M:%S"); // read as UTC/GMT time[m
    [32m+[m[41m        [m
    [32m+[m[32m        std::cout << "(3) Original time: " << std::put_time(&timeTm, "%Y.%m.%d %H:%M:%S %z") << "\n";[m
    [32m+[m[32m        if (timeStrStream.fail()) {[m
    [32m+[m[32m            std::cout << "(3) Parse failed\n";[m
    [32m+[m[32m        } else {[m
    [32m+[m[32m            std::time_t timestamp = as_utc_timestamp(timeTm);[m
    [32m+[m[32m            auto instantChrono = std::chrono::system_clock::from_time_t(timestamp);[m
    [32m+[m[32m            instantChrono += 23h + 55min;[m
    [32m+[m[32m            std::time_t anotherTimestamp = std::chrono::system_clock::to_time_t(instantChrono);[m
    [32m+[m[32m            std::tm localTm = {};[m
    [32m+[m[32m            gmtime_r(&timestamp, &localTm); // вот эта фигня проинтерпретировала время как локальное[m
    [32m+[m[32m            std::tm anotherLocalTm = {};[m
    [32m+[m[32m            gmtime_r(&anotherTimestamp, &anotherLocalTm);[m[41m [m
    [32m+[m[41m            [m
    [32m+[m[32m            std::cout << "(3) Take '"[m[41m [m
    [32m+[m[32m                      << std::put_time(&localTm, "%Y.%m.%d %H:%M:%S %z") << "', add 23:55, and get '"[m
    [32m+[m[32m                      << std::put_time(&anotherLocalTm, "%Y.%m.%d %H:%M:%S %z") << "'\n";[m
    [32m+[m[32m        }[m
    [32m+[m[32m    }[m
     [m
    [31m-float sum_cpp_f(float a, float b) {[m
    [31m-    return a + b;[m
    [32m+[m[32m    return 0;[m
     }[m
    [31m-[m
    [31m-class TSummer {[m
    [31m-public:[m
    [31m-    TSummer(int a);[m
    [31m-    int SumA(int b);[m
    [31m-    int SumB(int b) { return a + b; } // Обратите внимание, этой функции нет в символах [1][m
    [31m-    template <typename T>[m
    [31m-    int SumC(T b) { return a + b; } // И уж тем более этой [1][m
    [31m-public:[m
    [31m-    int a;[m
    [31m-};[m
    [31m-[m
    [31m-TSummer::TSummer(int a_arg): a(a_arg) {}[m
    [31m-int TSummer::SumA(int b) { return a + b; } [m
     ```[m
     [m
    [31m-##### <a name="odr_inline"></a> Замечание про наличие символов inline-функций в объектных файлах[m
    [32m+[m[32mСтоит обратить внимание, что в С++ не навязывается местный часовой пояс при парсинге времени. Хорошо это или плохо - не знаю.[m
     [m
    [31m-[1] - этих функций нет среди символов в данном запуске. Но в общем случае этого не гарантируется, так как методы класса имеют external linkage (класс не объявлен в анонимном namespace).[m
     [m
    [31m-Но почему же их нет в таблице символов, если у них external linkage? `inline` (в данном случае неявный) позволяет определять функцию в нескольких единицах трансляции при условии, что определение будет одинаковым (смягчается требование [ODR](https://en.cppreference.com/w/cpp/language/definition)). То есть во всех единицах трансляции, где эта, функция используется, она должна быть не просто объявлена, а определена одинаковым образом. Что дает компилятору свободу для оптимизации - он может не создавать символа функции, так как этот символ все равно никому не понадобится для линковки - в других единицах трансляции все равно должно быть такое же определение функции.[m
     [m
    [31m-<details>[m
    [31m-<summary> Больше деталей про <code>inline</code>[m
    [31m-    [m
    [31m-</summary>[m
    [31m-<p>[m
    [31m-    [m
    [31m-`inline` &mdash; это спецификатор [[cppref]](https://en.cppreference.com/w/cpp/language/inline) [[std.dcl.inline]](http://eel.is/c++draft/dcl.inline), используемый для объявления _inline function_ [[cppref]](https://en.cppreference.com/w/cpp/language/inline#Description)[[std.dcl.inline]](http://eel.is/c++draft/dcl.inline#2), и функции, определённые в теле класса, являются inline [[std.class.mcft]](http://eel.is/c++draft/class.mfct#1)[[std.class.friend]](http://eel.is/c++draft/class.friend#6).[m
     [m
    [31m-</p>[m
    [31m-</details>[m
     [m
    [31m-[Issue по которому добавлено замечение](https://github.com/yuri-pechatnov/caos_2019-2020/issues/1)[m
    [32m+[m[32m## <a name="clocks_and_cpu"></a> Разные часы и процессорное время[m
     [m
    [32m+[m[32m[Проблема 2038 года](https://ru.wikipedia.org/wiki/Проблема_2038_года), связанная с переполнением 32-битного time_t. Просто обозначаю, что она есть.[m
     [m
    [31m-```cpp[m
    [31m-%%cpp use_lib_cpp.c[m
    [31m-%run gcc -Wall -g use_lib_cpp.c -ldl -o use_lib_cpp.exe[m
    [31m-%run ./use_lib_cpp.exe[m
    [32m+[m[32m[iana](https://www.iana.org/time-zones) - база данных временных зон.[m
     [m
    [31m-#include <stdio.h>[m
    [31m-#include <stdlib.h>[m
    [31m-#include <unistd.h>[m
    [31m-#include <sys/types.h>[m
    [31m-#include <sys/stat.h>[m
    [31m-#include <sys/mman.h>[m
    [31m-#include <fcntl.h>[m
    [31m-#include <assert.h>[m
    [31m-#include <dlfcn.h>[m
    [31m-[m
    [31m-int main() {  [m
    [31m-    [m
    [31m-    void *lib_handle = dlopen("./libsumcpp.so", RTLD_NOW);[m
    [31m-    if (!lib_handle) {[m
    [31m-        fprintf(stderr, "dlopen: %s\n", dlerror());[m
    [31m-        abort();[m
    [31m-    }[m
    [31m-    [m
    [31m-    int (*sum_c)(int, int) = dlsym(lib_handle, "sum_c");[m
    [31m-    int (*sum)(int, int) = dlsym(lib_handle, "_Z7sum_cppii");[m
    [31m-    float (*sum_f)(float, float) = dlsym(lib_handle, "_Z9sum_cpp_fff");[m
    [31m-    [m
    [31m-    #define p(stmt, fmt) printf(#stmt " = " fmt "\n", stmt);[m
    [31m-    p(sum_c(1, 1), "%d");[m
    [31m-    p(sum_c(40, 5000), "%d");[m
    [31m-    [m
    [31m-    p(sum(1, 1), "%d");[m
    [31m-    p(sum(40, 5000), "%d");[m
    [31m-    [m
    [31m-    p(sum_f(1, 1), "%0.2f");[m
    [31m-    p(sum_f(4.0, 500.1), "%0.2f");[m
    [31m-    [m
    [31m-    char* objStorage[100];[m
    [31m-    void (*constructor)(void*, int) = dlsym(lib_handle, "_ZN7TSummerC1Ei");[m
    [31m-    int (*sumA)(void*, int) = dlsym(lib_handle, "_ZN7TSummer4SumAEi");[m
    [31m-    [m
    [31m-    // f(1, 2, 3) --- , раздяеляет аргументы[m
    [31m-    // (1, 3) + 3 --- , - operator, (итоговое значение 6)[m
    [31m-    // p((1, 3) + 3, "%d"); // == 6[m
    [31m-    p((constructor(objStorage, 10), sumA(objStorage, 1)), "%d"); // operator , - просто делает выполнеяет все команды и берет возвращаемое значение последней[m
    [31m-    p((constructor(objStorage, 4000), sumA(objStorage, 20)), "%d"); [m
    [31m-    [m
    [31m-    dlclose(lib_handle);[m
    [31m-    return 0;[m
    [31m-}[m
    [32m+[m[32mХардверные часы. Обычные кварцевые часы, для которых на материнской плате есть отдельная батарейка. Они не очень точные. А еще разные системы могут хранить там время по-разному. Поэтому при перезагрузках между ubuntu и windows время может прыгать на 3 часа (если выбрано Московское время).[m
     ```[m
    [31m-[m
    [31m-[m
    [31m-```python[m
    [31m-[m
    [32m+[m[32m  -> sudo hwclock[m
    [32m+[m[32mПт 24 апр 2020 00:28:52  .356966 seconds[m
    [32m+[m[32m  -> date[m
    [32m+[m[32mПн май  4 14:28:24 MSK 2020[m
     ```[m
     [m
    [32m+[m[32mПроцессорное время:[m
    [32m+[m[32m* [C/C++: как измерять процессорное время / Хабр](https://habr.com/ru/post/282301/)[m
    [32m+[m[32m* `clock_t clock(void);` - время затраченное процессором на исполнение потока/программы. Измеряется в непонятных единицах, связанных с секундами через CLOCKS_PER_SEC. [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=clock&category=3)[m
    [32m+[m[32m* `clock_gettime` c параметрами `CLOCK_PROCESS_CPUTIME_ID`, `CLOCK_THREAD_CPUTIME_ID` - процессорное время программы и потока.[m
    [32m+[m[32m*[m[41m [m
     [m
    [31m-```python[m
     [m
    [31m-```[m
    [32m+[m[32mТип часов[m
    [32m+[m[32m* `clockid_t` - тип часов [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=clock_gettime&category=3)[m
    [32m+[m[32m* `CLOCK_MONOTONIC` - тип часов, который стоит отдельно выделить. Это монотонные часы, то есть время, которое они показывают всегда возрастает несмотря ни на какие переводы времени. Их правильно использовать для замеров интервалов времени.[m
     [m
     [m
     ```python[m
    [31m-[m
    [32m+[m[32mfor time_type in (time.CLOCK_REALTIME, time.CLOCK_MONOTONIC, time.CLOCK_PROCESS_CPUTIME_ID):[m
    [32m+[m[32m    print(time.clock_gettime(time_type))[m
     ```[m
     [m
     [m
    [36m@@ -490,142 +372,71 @@[m [mint main() {[m
     [m
     ```[m
     [m
    [32m+[m[32m## <a name="benchmarking"></a> Время для бенчмарков[m
     [m
    [31m-```python[m
    [32m+[m[32m#### Что измерять?[m
    [32m+[m[32mСтоит измерять процессорное время. В зависимости от того, делаете ли вы в измеряемой части программы системные вызовы или нет, имеет смысл измерять только пользовательское время или пользовательское и системное вместе.[m
     [m
    [31m-```[m
    [32m+[m[32m#### Как измерять?[m
     [m
    [31m-# <a name="hw"></a> Комментарии к ДЗ[m
    [32m+[m[32mЧтобы замеры были максимально точными, стоит минимизировать влияние среды и максимизировать стабильность измерений.[m[41m [m
     [m
    [31m-* [m
    [31m-*[m
    [31m-* inf19-2-posix/dl/cpp-class-loader[m
    [31m-<br> Задача очень интересная, советую всем сделать :)[m
    [31m-<br>[m
    [31m-<br> А теперь, немного информации, чтобы сделать ее было проще. Прежде всего в системе есть заголовочный файл, который можно исклюдить в решении. И вам нужно написать cpp-шник, в котором реализованы объявленные хедере функции.[m
    [32m+[m[32mКакие есть способы повысить стабильность?[m
     [m
    [31m-<details>[m
    [31m-<summary>interfaces.h</summary>[m
    [32m+[m[32m0. Повторить замер столько раз, сколько можете себе позволить по времени, и усреднить.[m
    [32m+[m[32m1. Увеличить минимальное время, которое шедулер гарантирует процессу, если он сам не отдает управления. Его можно увеличить до 1с.[m
    [32m+[m[32m2. Запускать бенчмарк на выделенном ядре.[m[41m [m
    [32m+[m[32mТо есть запретить шедулеру запускать что-то еще на ядре,[m[41m [m
    [32m+[m[32mгде будет работать бенчмарк, и его парном гипертрединговом.[m
     [m
    [31m-```cpp[m
    [32m+[m[32mА теперь подбробнее[m
    [32m+[m[32m1. `sudo sysctl -w kernel.sched_min_granularity_ns='999999999'` - выкручиваем квант времени шедулера. (Спорная оптимизация, на самом деле. Один раз видел от нее хороший положительный эффект, а один раз слабый отрицательный.)[m
    [32m+[m[32m2. В конфиге grub (`/etc/default/grub`) добавляем `isolcpu=2,3` (у меня это второе физическое ядро) в строку параметров запуска.[m
    [32m+[m[32m  <br> Обновляем grub. `sudo grub-mkconfig`, `sudo grub-mkconfig -o /boot/grub/grub.cfg`. Перезапускаем систему.[m
    [32m+[m[32m  <br> Теперь запускаем бенчмарк как `taskset 0x4 ./my_benchmark`. (4 == 1 << 2, 2 - номер виртуального ядра, на котором запускаем процесс)[m
     [m
    [31m-#include <string>[m
    [31m-[m
    [31m-class AbstractClass[m
    [31m-{[m
    [31m-    friend class ClassLoader;[m
    [31m-public:[m
    [31m-    explicit AbstractClass();[m
    [31m-    ~AbstractClass();[m
    [31m-protected:[m
    [31m-    void* newInstanceWithSize(size_t sizeofClass);[m
    [31m-    struct ClassImpl* pImpl;[m
    [31m-};[m
    [31m-[m
    [31m-template <class T>[m
    [31m-class Class : public AbstractClass[m
    [31m-{[m
    [31m-public:[m
    [31m-    T* newInstance()[m
    [31m-    {[m
    [31m-        size_t classSize = sizeof(T);[m
    [31m-        void* rawPtr = newInstanceWithSize(classSize);[m
    [31m-        return reinterpret_cast<T*>(rawPtr);[m
    [31m-    }[m
    [31m-};[m
    [31m-[m
    [31m-enum class ClassLoaderError {[m
    [31m-    NoError = 0,[m
    [31m-    FileNotFound,[m
    [31m-    LibraryLoadError,[m
    [31m-    NoClassInLibrary[m
    [31m-};[m
    [31m-[m
    [31m-[m
    [31m-class ClassLoader[m
    [31m-{[m
    [31m-public:[m
    [31m-    explicit ClassLoader();[m
    [31m-    AbstractClass* loadClass(const std::string &fullyQualifiedName);[m
    [31m-    ClassLoaderError lastError() const;[m
    [31m-    ~ClassLoader();[m
    [31m-private:[m
    [31m-    struct ClassLoaderImpl* pImpl;[m
    [31m-};[m
    [31m-```[m
    [31m-</details>[m
     [m
    [31m-<br> Что вообще должно у вас получиться:[m
    [31m-<br> Пусть у вас в каком-то динамической библиотеке реализован класс:[m
    [32m+[m[32m#### Чем измерять?[m
    [32m+[m[32m* perf stat[m
     [m
    [31m-<details>[m
    [31m-<summary> module.h </summary>[m
    [32m+[m[32mperf вообще очень мощная штука, помимо бенчмаркинга позволяет профилировать программу, смотреть, какие функции сколько работают.[m
     [m
    [31m-```cpp[m
    [31m-#pragma once[m
    [32m+[m[32mУстанавливается так:[m
     [m
    [31m-class SimpleClass[m
    [31m-{[m
    [31m-public:[m
    [31m-    SimpleClass();[m
    [31m-};[m
    [32m+[m[32m```bash[m
    [32m+[m[32m$ sudo apt install linux-tools-$(uname -r) linux-tools-generic[m
    [32m+[m[32m$ echo -1 > /proc/sys/kernel/perf_event_paranoid # under `sudo -i`[m
     ```[m
    [31m-</details>[m
     [m
    [31m-<details>[m
    [31m-<summary> module.cpp </summary>[m
    [32m+[m[32m* time[m
     [m
    [31m-```cpp[m
    [31m-#include "module.h"[m
     [m
    [31m-#include <iostream>[m
     [m
    [31m-SimpleClass::SimpleClass()[m
    [31m-{[m
    [31m-    std::cout << "Simple Class constructor called" << std::endl;[m
    [31m-}[m
    [32m+[m[32m```bash[m
    [32m+[m[32m%%bash[m
    [32m+[m[32mexec 2>&1 ; set -o xtrace[m
    [32m+[m
    [32m+[m[32mperf stat sleep 1[m
    [32m+[m[32mtime sleep 1[m
     ```[m
    [31m-</details>[m
     [m
    [31m-<br> Вы хотите этот класс загрузить из этой динамической библиотеки:[m
    [32m+[m[32m## <a name="sleep"></a> Как поспать?[m
     [m
    [31m-<details>[m
    [31m-<summary> main.cpp </summary>[m
    [32m+[m[32m`sleep`, `nanosleep` - просто поспать. <s>На практике</s> В хороших продовых проектах такие функции нужны редко, из-за того, что такие ожидания нельзя корректно прервать внешним событием. На деле, конечно, постоянно используется.[m
     [m
    [31m-```cpp[m
    [31m-#include "interfaces.h"[m
    [32m+[m[32m`timerfd` - позволяет создавать таймеры, которые при срабатывании будут приходить записями, которые можно прочесть из файлового дескриптора.[m
     [m
    [31m-#include "module.h"[m
    [32m+[m[32m`select`, `epoll_wait` - одновременное ожидание по таймауту и по файловым дескрипторам.[m
     [m
    [31m-static ClassLoader * Loader = nullptr;[m
    [32m+[m[32m`pthread_cond_timedwait` - одновременное ожидание по таймауту и условной переменной.[m
     [m
    [31m-int testSimpleClass()[m
    [31m-{[m
    [31m-    Class<SimpleClass>* c = reinterpret_cast<Class<SimpleClass>*> ([m
    [31m-		Loader->loadClass("SimpleClass"));[m
    [31m-    if (c) {[m
    [31m-        SimpleClass* instance = c->newInstance(); // тут произошел аналог new SimpleClass()[m
    [31m-        (void)instance; [m
    [31m-        // над уничтожением объекта в этой задаче думать не нужно[m
    [31m-        return 0;[m
    [31m-    }[m
    [31m-    else {[m
    [31m-        return 1;[m
    [31m-    }[m
    [31m-}[m
    [32m+[m[32m`sigtimedwait` - одновременное ожидание по таймауту и сигнала. (Лучше все-таки свести прием сигнала к чтению из файлового дескриптора и не использовать это.)[m
     [m
     [m
    [31m-int main(int argc, char *argv[])[m
    [31m-{[m
    [31m-    Loader = new ClassLoader();[m
    [31m-    int status = testSimpleClass();[m
    [31m-    delete Loader;[m
    [31m-    return status;[m
    [31m-}[m
     [m
    [31m-```[m
    [31m-</details>[m
    [32m+[m[32m```python[m
     [m
    [32m+[m[32m```[m
     [m
     [m
     ```python[m


### <a name="github"></a> Коммитим на github


```python
cmds = []
add_cmd = "git add --ignore-errors "
add_cmd_f = "git add --ignore-errors -f "
for subdir in highlevel_dirs:
    for sfx in [".ipynb", ".md", ".c", ".cpp"]:
        cmds.append(add_cmd + " {}/*{}".format(subdir, sfx))
    cmds.append(add_cmd_f + " -f {}/bash_popen_tmp/*.html".format(subdir))
    cmds.append(add_cmd_f + " -f {}/interactive_launcher_tmp/*.log".format(subdir))
    cmds.append("git add -u {}".format(subdir))
    
def execute_cmd(cmd):
    print(">", cmd)
    get_ipython().system(cmd)
    
for cmd in cmds:
    execute_cmd(cmd)
# execute_cmd("git add -u")
execute_cmd("git commit -m 'yet another update'")
execute_cmd("git push origin master")
```

    > git add --ignore-errors  ../sem01/*.ipynb
    > git add --ignore-errors  ../sem01/*.md
    > git add --ignore-errors  ../sem01/*.c
    > git add --ignore-errors  ../sem01/*.cpp
    fatal: pathspec '../sem01/*.cpp' did not match any files
    > git add --ignore-errors -f  -f ../sem01/bash_popen_tmp/*.html
    warning: could not open directory 'sem01/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem01/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem01/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem01/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem01/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem01
    > git add --ignore-errors  ../sem03-ints-floats/*.ipynb
    > git add --ignore-errors  ../sem03-ints-floats/*.md
    > git add --ignore-errors  ../sem03-ints-floats/*.c
    > git add --ignore-errors  ../sem03-ints-floats/*.cpp
    fatal: pathspec '../sem03-ints-floats/*.cpp' did not match any files
    > git add --ignore-errors -f  -f ../sem03-ints-floats/bash_popen_tmp/*.html
    warning: could not open directory 'sem03-ints-floats/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem03-ints-floats/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem03-ints-floats/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem03-ints-floats/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem03-ints-floats/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem03-ints-floats
    > git add --ignore-errors  ../sem04-asm-arm/*.ipynb
    > git add --ignore-errors  ../sem04-asm-arm/*.md
    > git add --ignore-errors  ../sem04-asm-arm/*.c
    > git add --ignore-errors  ../sem04-asm-arm/*.cpp
    fatal: pathspec '../sem04-asm-arm/*.cpp' did not match any files
    > git add --ignore-errors -f  -f ../sem04-asm-arm/bash_popen_tmp/*.html
    warning: could not open directory 'sem04-asm-arm/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem04-asm-arm/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem04-asm-arm/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem04-asm-arm/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem04-asm-arm/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem04-asm-arm
    > git add --ignore-errors  ../sem05-asm-arm-addressing/*.ipynb
    > git add --ignore-errors  ../sem05-asm-arm-addressing/*.md
    > git add --ignore-errors  ../sem05-asm-arm-addressing/*.c
    > git add --ignore-errors  ../sem05-asm-arm-addressing/*.cpp
    fatal: pathspec '../sem05-asm-arm-addressing/*.cpp' did not match any files
    > git add --ignore-errors -f  -f ../sem05-asm-arm-addressing/bash_popen_tmp/*.html
    warning: could not open directory 'sem05-asm-arm-addressing/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem05-asm-arm-addressing/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem05-asm-arm-addressing/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem05-asm-arm-addressing/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem05-asm-arm-addressing/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem05-asm-arm-addressing
    > git add --ignore-errors  ../sem06-asm-x86/*.ipynb
    > git add --ignore-errors  ../sem06-asm-x86/*.md
    > git add --ignore-errors  ../sem06-asm-x86/*.c
    > git add --ignore-errors  ../sem06-asm-x86/*.cpp
    fatal: pathspec '../sem06-asm-x86/*.cpp' did not match any files
    > git add --ignore-errors -f  -f ../sem06-asm-x86/bash_popen_tmp/*.html
    warning: could not open directory 'sem06-asm-x86/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem06-asm-x86/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem06-asm-x86/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem06-asm-x86/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem06-asm-x86/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem06-asm-x86
    > git add --ignore-errors  ../sem07-asm-x86-x87-sse/*.ipynb
    > git add --ignore-errors  ../sem07-asm-x86-x87-sse/*.md
    > git add --ignore-errors  ../sem07-asm-x86-x87-sse/*.c
    > git add --ignore-errors  ../sem07-asm-x86-x87-sse/*.cpp
    fatal: pathspec '../sem07-asm-x86-x87-sse/*.cpp' did not match any files
    > git add --ignore-errors -f  -f ../sem07-asm-x86-x87-sse/bash_popen_tmp/*.html
    warning: could not open directory 'sem07-asm-x86-x87-sse/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem07-asm-x86-x87-sse/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem07-asm-x86-x87-sse/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem07-asm-x86-x87-sse/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem07-asm-x86-x87-sse/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem07-asm-x86-x87-sse
    > git add --ignore-errors  ../sem08-asm-x86-nostdlib/*.ipynb
    > git add --ignore-errors  ../sem08-asm-x86-nostdlib/*.md
    > git add --ignore-errors  ../sem08-asm-x86-nostdlib/*.c
    > git add --ignore-errors  ../sem08-asm-x86-nostdlib/*.cpp
    fatal: pathspec '../sem08-asm-x86-nostdlib/*.cpp' did not match any files
    > git add --ignore-errors -f  -f ../sem08-asm-x86-nostdlib/bash_popen_tmp/*.html
    warning: could not open directory 'sem08-asm-x86-nostdlib/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem08-asm-x86-nostdlib/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem08-asm-x86-nostdlib/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem08-asm-x86-nostdlib/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem08-asm-x86-nostdlib/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem08-asm-x86-nostdlib
    > git add --ignore-errors  ../sem09-low-level-io/*.ipynb
    > git add --ignore-errors  ../sem09-low-level-io/*.md
    > git add --ignore-errors  ../sem09-low-level-io/*.c
    > git add --ignore-errors  ../sem09-low-level-io/*.cpp
    fatal: pathspec '../sem09-low-level-io/*.cpp' did not match any files
    > git add --ignore-errors -f  -f ../sem09-low-level-io/bash_popen_tmp/*.html
    warning: could not open directory 'sem09-low-level-io/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem09-low-level-io/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem09-low-level-io/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem09-low-level-io/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem09-low-level-io/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem09-low-level-io
    > git add --ignore-errors  ../sem10-file-attributes/*.ipynb
    > git add --ignore-errors  ../sem10-file-attributes/*.md
    > git add --ignore-errors  ../sem10-file-attributes/*.c
    > git add --ignore-errors  ../sem10-file-attributes/*.cpp
    fatal: pathspec '../sem10-file-attributes/*.cpp' did not match any files
    > git add --ignore-errors -f  -f ../sem10-file-attributes/bash_popen_tmp/*.html
    warning: could not open directory 'sem10-file-attributes/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem10-file-attributes/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem10-file-attributes/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem10-file-attributes/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem10-file-attributes/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem10-file-attributes
    > git add --ignore-errors  ../sem11-mmap-instrumentation/*.ipynb
    > git add --ignore-errors  ../sem11-mmap-instrumentation/*.md
    > git add --ignore-errors  ../sem11-mmap-instrumentation/*.c
    > git add --ignore-errors  ../sem11-mmap-instrumentation/*.cpp
    > git add --ignore-errors -f  -f ../sem11-mmap-instrumentation/bash_popen_tmp/*.html
    warning: could not open directory 'sem11-mmap-instrumentation/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem11-mmap-instrumentation/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem11-mmap-instrumentation/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem11-mmap-instrumentation/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem11-mmap-instrumentation/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem11-mmap-instrumentation
    > git add --ignore-errors  ../sem12-fork-exec-pipe/*.ipynb
    > git add --ignore-errors  ../sem12-fork-exec-pipe/*.md
    > git add --ignore-errors  ../sem12-fork-exec-pipe/*.c
    > git add --ignore-errors  ../sem12-fork-exec-pipe/*.cpp
    > git add --ignore-errors -f  -f ../sem12-fork-exec-pipe/bash_popen_tmp/*.html
    warning: could not open directory 'sem12-fork-exec-pipe/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem12-fork-exec-pipe/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem12-fork-exec-pipe/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem12-fork-exec-pipe/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem12-fork-exec-pipe/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem12-fork-exec-pipe
    > git add --ignore-errors  ../sem13-signal/*.ipynb
    > git add --ignore-errors  ../sem13-signal/*.md
    > git add --ignore-errors  ../sem13-signal/*.c
    > git add --ignore-errors  ../sem13-signal/*.cpp
    > git add --ignore-errors -f  -f ../sem13-signal/bash_popen_tmp/*.html
    warning: could not open directory 'sem13-signal/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem13-signal/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem13-signal/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem13-signal/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem13-signal/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem13-signal
    > git add --ignore-errors  ../sem14-fifo-proc/*.ipynb
    > git add --ignore-errors  ../sem14-fifo-proc/*.md
    > git add --ignore-errors  ../sem14-fifo-proc/*.c
    > git add --ignore-errors  ../sem14-fifo-proc/*.cpp
    > git add --ignore-errors -f  -f ../sem14-fifo-proc/bash_popen_tmp/*.html
    warning: could not open directory 'sem14-fifo-proc/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem14-fifo-proc/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem14-fifo-proc/interactive_launcher_tmp/*.log
    > git add -u ../sem14-fifo-proc
    > git add --ignore-errors  ../sem15-ptrace/*.ipynb
    > git add --ignore-errors  ../sem15-ptrace/*.md
    > git add --ignore-errors  ../sem15-ptrace/*.c
    > git add --ignore-errors  ../sem15-ptrace/*.cpp
    > git add --ignore-errors -f  -f ../sem15-ptrace/bash_popen_tmp/*.html
    warning: could not open directory 'sem15-ptrace/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem15-ptrace/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem15-ptrace/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem15-ptrace/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem15-ptrace/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem15-ptrace
    > git add --ignore-errors  ../sem16-fcntl-dup-pipe/*.ipynb
    > git add --ignore-errors  ../sem16-fcntl-dup-pipe/*.md
    > git add --ignore-errors  ../sem16-fcntl-dup-pipe/*.c
    > git add --ignore-errors  ../sem16-fcntl-dup-pipe/*.cpp
    > git add --ignore-errors -f  -f ../sem16-fcntl-dup-pipe/bash_popen_tmp/*.html
    warning: could not open directory 'sem16-fcntl-dup-pipe/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem16-fcntl-dup-pipe/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem16-fcntl-dup-pipe/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem16-fcntl-dup-pipe/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem16-fcntl-dup-pipe/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem16-fcntl-dup-pipe
    > git add --ignore-errors  ../sem17-sockets-tcp-udp/*.ipynb
    > git add --ignore-errors  ../sem17-sockets-tcp-udp/*.md
    > git add --ignore-errors  ../sem17-sockets-tcp-udp/*.c
    > git add --ignore-errors  ../sem17-sockets-tcp-udp/*.cpp
    > git add --ignore-errors -f  -f ../sem17-sockets-tcp-udp/bash_popen_tmp/*.html
    warning: could not open directory 'sem17-sockets-tcp-udp/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem17-sockets-tcp-udp/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem17-sockets-tcp-udp/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem17-sockets-tcp-udp/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem17-sockets-tcp-udp/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem17-sockets-tcp-udp
    > git add --ignore-errors  ../sem18-multiplexing/*.ipynb
    > git add --ignore-errors  ../sem18-multiplexing/*.md
    > git add --ignore-errors  ../sem18-multiplexing/*.c
    fatal: pathspec '../sem18-multiplexing/*.c' did not match any files
    > git add --ignore-errors  ../sem18-multiplexing/*.cpp
    > git add --ignore-errors -f  -f ../sem18-multiplexing/bash_popen_tmp/*.html
    warning: could not open directory 'sem18-multiplexing/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem18-multiplexing/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem18-multiplexing/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem18-multiplexing/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem18-multiplexing/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem18-multiplexing
    > git add --ignore-errors  ../sem19-pthread/*.ipynb
    > git add --ignore-errors  ../sem19-pthread/*.md
    > git add --ignore-errors  ../sem19-pthread/*.c
    > git add --ignore-errors  ../sem19-pthread/*.cpp
    > git add --ignore-errors -f  -f ../sem19-pthread/bash_popen_tmp/*.html
    warning: could not open directory 'sem19-pthread/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem19-pthread/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem19-pthread/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem19-pthread/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem19-pthread/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem19-pthread
    > git add --ignore-errors  ../sem20-synchronizing/*.ipynb
    > git add --ignore-errors  ../sem20-synchronizing/*.md
    > git add --ignore-errors  ../sem20-synchronizing/*.c
    > git add --ignore-errors  ../sem20-synchronizing/*.cpp
    > git add --ignore-errors -f  -f ../sem20-synchronizing/bash_popen_tmp/*.html
    warning: could not open directory 'sem20-synchronizing/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem20-synchronizing/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem20-synchronizing/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem20-synchronizing/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem20-synchronizing/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem20-synchronizing
    > git add --ignore-errors  ../sem21-ipc-synchronizing/*.ipynb
    > git add --ignore-errors  ../sem21-ipc-synchronizing/*.md
    > git add --ignore-errors  ../sem21-ipc-synchronizing/*.c
    > git add --ignore-errors  ../sem21-ipc-synchronizing/*.cpp
    fatal: pathspec '../sem21-ipc-synchronizing/*.cpp' did not match any files
    > git add --ignore-errors -f  -f ../sem21-ipc-synchronizing/bash_popen_tmp/*.html
    warning: could not open directory 'sem21-ipc-synchronizing/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem21-ipc-synchronizing/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem21-ipc-synchronizing/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem21-ipc-synchronizing/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem21-ipc-synchronizing/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem21-ipc-synchronizing
    > git add --ignore-errors  ../sem22-dynamic-lib/*.ipynb
    > git add --ignore-errors  ../sem22-dynamic-lib/*.md
    > git add --ignore-errors  ../sem22-dynamic-lib/*.c
    > git add --ignore-errors  ../sem22-dynamic-lib/*.cpp
    > git add --ignore-errors -f  -f ../sem22-dynamic-lib/bash_popen_tmp/*.html
    warning: could not open directory 'sem22-dynamic-lib/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem22-dynamic-lib/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem22-dynamic-lib/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem22-dynamic-lib/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem22-dynamic-lib/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem22-dynamic-lib
    > git add --ignore-errors  ../sem23-extra-net-protocols/*.ipynb
    > git add --ignore-errors  ../sem23-extra-net-protocols/*.md
    > git add --ignore-errors  ../sem23-extra-net-protocols/*.c
    > git add --ignore-errors  ../sem23-extra-net-protocols/*.cpp
    fatal: pathspec '../sem23-extra-net-protocols/*.cpp' did not match any files
    > git add --ignore-errors -f  -f ../sem23-extra-net-protocols/bash_popen_tmp/*.html
    warning: could not open directory 'sem23-extra-net-protocols/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem23-extra-net-protocols/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem23-extra-net-protocols/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem23-extra-net-protocols/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem23-extra-net-protocols/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem23-extra-net-protocols
    > git add --ignore-errors  ../sem24-http-libcurl-cmake/*.ipynb
    > git add --ignore-errors  ../sem24-http-libcurl-cmake/*.md
    > git add --ignore-errors  ../sem24-http-libcurl-cmake/*.c
    > git add --ignore-errors  ../sem24-http-libcurl-cmake/*.cpp
    > git add --ignore-errors -f  -f ../sem24-http-libcurl-cmake/bash_popen_tmp/*.html
    warning: could not open directory 'sem24-http-libcurl-cmake/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem24-http-libcurl-cmake/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem24-http-libcurl-cmake/interactive_launcher_tmp/*.log
    > git add -u ../sem24-http-libcurl-cmake
    > git add --ignore-errors  ../sem25-crypto-ssl/*.ipynb
    > git add --ignore-errors  ../sem25-crypto-ssl/*.md
    > git add --ignore-errors  ../sem25-crypto-ssl/*.c
    fatal: pathspec '../sem25-crypto-ssl/*.c' did not match any files
    > git add --ignore-errors  ../sem25-crypto-ssl/*.cpp
    > git add --ignore-errors -f  -f ../sem25-crypto-ssl/bash_popen_tmp/*.html
    warning: could not open directory 'sem25-crypto-ssl/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem25-crypto-ssl/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem25-crypto-ssl/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem25-crypto-ssl/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem25-crypto-ssl/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem25-crypto-ssl
    > git add --ignore-errors  ../sem26-fs-fuse/*.ipynb
    > git add --ignore-errors  ../sem26-fs-fuse/*.md
    > git add --ignore-errors  ../sem26-fs-fuse/*.c
    > git add --ignore-errors  ../sem26-fs-fuse/*.cpp
    > git add --ignore-errors -f  -f ../sem26-fs-fuse/bash_popen_tmp/*.html
    warning: could not open directory 'sem26-fs-fuse/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem26-fs-fuse/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem26-fs-fuse/interactive_launcher_tmp/*.log
    > git add -u ../sem26-fs-fuse
    > git add --ignore-errors  ../sem27-python-bindings/*.ipynb
    > git add --ignore-errors  ../sem27-python-bindings/*.md
    > git add --ignore-errors  ../sem27-python-bindings/*.c
    > git add --ignore-errors  ../sem27-python-bindings/*.cpp
    > git add --ignore-errors -f  -f ../sem27-python-bindings/bash_popen_tmp/*.html
    warning: could not open directory 'sem27-python-bindings/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem27-python-bindings/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem27-python-bindings/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem27-python-bindings/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem27-python-bindings/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem27-python-bindings
    > git add --ignore-errors  ../sem28-unix-time/*.ipynb
    > git add --ignore-errors  ../sem28-unix-time/*.md
    > git add --ignore-errors  ../sem28-unix-time/*.c
    > git add --ignore-errors  ../sem28-unix-time/*.cpp
    > git add --ignore-errors -f  -f ../sem28-unix-time/bash_popen_tmp/*.html
    warning: could not open directory 'sem28-unix-time/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem28-unix-time/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem28-unix-time/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem28-unix-time/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem28-unix-time/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem28-unix-time
    > git commit -m 'yet another update'
    [master 40c557d] yet another update
     100 files changed, 9353 insertions(+), 10904 deletions(-)
     rewrite sem06-asm-x86/README_no_output.md (97%)
     rewrite sem08-asm-x86-nostdlib/README_no_output.md (97%)
     copy {sem10-file-attributes => sem09-low-level-io}/README_no_output.md (100%)
     rewrite sem10-file-attributes/README_no_output.md (96%)
     rewrite sem11-mmap-instrumentation/README_no_output.md (94%)
     rewrite sem12-fork-exec-pipe/README_no_output.md (96%)
     rewrite sem14-fifo-proc/README.md (89%)
     copy {sem11-mmap-instrumentation => sem14-fifo-proc}/README_no_output.md (100%)
     rename sem14-fifo-proc/interactive_launcher_tmp/{983043216781878980.log => 127766679693617464.log} (56%)
     rename sem14-fifo-proc/interactive_launcher_tmp/{445842116699040428.log => 130144623552729609.log} (76%)
     rename sem14-fifo-proc/interactive_launcher_tmp/{408174933795177832.log => 131874295726709881.log} (89%)
     delete mode 100644 sem14-fifo-proc/interactive_launcher_tmp/283257014686283392.log
     delete mode 100644 sem14-fifo-proc/interactive_launcher_tmp/33880078996467771.log
     rename sem14-fifo-proc/interactive_launcher_tmp/{168715572125305438.log => 354080172890454133.log} (56%)
     delete mode 100644 sem14-fifo-proc/interactive_launcher_tmp/36680464884053384.log
     rename sem14-fifo-proc/interactive_launcher_tmp/{741034825313819161.log => 401914075815381515.log} (87%)
     rename sem14-fifo-proc/interactive_launcher_tmp/{808161313364762585.log => 52428862673654368.log} (77%)
     delete mode 100644 sem14-fifo-proc/interactive_launcher_tmp/527728542722413092.log
     delete mode 100644 sem14-fifo-proc/interactive_launcher_tmp/636479856332377457.log
     delete mode 100644 sem14-fifo-proc/interactive_launcher_tmp/723675156510583135.log
     delete mode 100644 sem14-fifo-proc/interactive_launcher_tmp/855988039801024707.log
     rewrite sem15-ptrace/README.md (63%)
     rewrite sem15-ptrace/README_no_output.md (98%)
     rewrite sem16-fcntl-dup-pipe/README_no_output.md (97%)
     rewrite sem17-sockets-tcp-udp/README_no_output.md (96%)
     rewrite sem18-multiplexing/README_no_output.md (98%)
     copy {sem16-fcntl-dup-pipe => sem19-pthread}/README_no_output.md (57%)
     rename {sem14-fifo-proc => sem21-ipc-synchronizing}/README_no_output.md (57%)
     rewrite sem23-extra-net-protocols/README_no_output.md (98%)
     rename sem24-http-libcurl-cmake/interactive_launcher_tmp/{560354493218935891.log => 278908528095121695.log} (92%)
     create mode 100644 sem24-http-libcurl-cmake/interactive_launcher_tmp/279871983378398179.log
     create mode 100644 sem24-http-libcurl-cmake/interactive_launcher_tmp/625914360777685168.log
     create mode 100644 sem24-http-libcurl-cmake/interactive_launcher_tmp/825717250500169114.log
     delete mode 100644 sem24-http-libcurl-cmake/interactive_launcher_tmp/860601386309686683.log
     rewrite sem25-crypto-ssl/README_no_output.md (97%)
     rewrite sem26-fs-fuse/README_no_output.md (94%)
     create mode 100644 sem26-fs-fuse/fuse_c_example/build/CMakeFiles/3.16.3/CompilerIdCXX/CMakeCXXCompilerId.cpp
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/156011776327687889.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/182179831592923882.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/192919932314426761.log
     rename sem26-fs-fuse/interactive_launcher_tmp/{116979503829915675.log => 286997408202756564.log} (56%)
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/302856133625005432.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/335751076967978580.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/361456416755900816.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/375169071453270201.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/391525198553748581.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/421856744786994827.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/445080597007145218.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/453638846907414485.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/45732148077862977.log
     rename sem26-fs-fuse/interactive_launcher_tmp/{124782009516790280.log => 470231275623214705.log} (56%)
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/483916945469479113.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/519827888695486851.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/535417739763795536.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/53544415829918814.log
     rename sem26-fs-fuse/interactive_launcher_tmp/{107027058753262637.log => 576155038309430211.log} (56%)
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/59368748222478326.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/600975740645518008.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/620354869924531062.log
     create mode 100644 sem26-fs-fuse/interactive_launcher_tmp/634529996390274655.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/648141190489884267.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/649699121527805609.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/656938869853187950.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/661726051044275334.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/667566771046228386.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/719135738452212765.log
     rename sem26-fs-fuse/interactive_launcher_tmp/{140487424825061744.log => 74592816055267592.log} (56%)
     create mode 100644 sem26-fs-fuse/interactive_launcher_tmp/758593138878100872.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/800742813790693350.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/821532952570932806.log
     create mode 100644 sem26-fs-fuse/interactive_launcher_tmp/861048178809047391.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/949711970034011281.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/958898769296250760.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/962371730647350887.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/971654105879461832.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/98456233821108737.log
     delete mode 100644 sem26-fs-fuse/interactive_launcher_tmp/995128752149981090.log
     rewrite sem28-unix-time/README_no_output.md (97%)
    > git push origin master
    Enumerating objects: 125, done.
    Counting objects: 100% (125/125), done.
    Delta compression using up to 4 threads
    Compressing objects: 100% (76/76), done.
    Writing objects: 100% (78/78), 72.20 KiB | 1.76 MiB/s, done.
    Total 78 (delta 51), reused 0 (delta 0)
    remote: Resolving deltas: 100% (51/51), completed with 35 local objects.[K
    To github.com:yuri-pechatnov/caos_2019-2020.git
       0c5902e..40c557d  master -> master



```python

```


```python

```


```python

```
