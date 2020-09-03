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
    ["../tools"], 
    #sorted(glob.glob("../sem26*")),
    sorted(glob.glob("../sem01*")),
], [])

print("Highlevel dirs:", highlevel_dirs)
```

    Highlevel dirs: ['../tools', '../sem01-intro-linux']



```python
tmp_dir = "./tmp_dir"
get_ipython().system('rm -r {tmp_dir} ; mkdir {tmp_dir} 2>&1 | grep -v "File exists"'.format(tmp_dir=tmp_dir))
```

### Генерируем все .md-шки стандартными средствами
\+ Делаем .md-шки очищенные для вывода. По этим .md-шкам можно будет смотреть реальную историю изменений. И дифф при пулреквестах.


```python
def execute_all_in_parallel(tasks):
    ps = []
    for t in tasks:
        ps.append(subprocess.Popen(["bash", "-c", t], stdout=subprocess.PIPE, stderr=subprocess.PIPE))
    for p in ps:
        out, err = p.communicate()
        print(out.decode(), err.decode())
```


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

execute_all_in_parallel(tasks)
```

    ../tools ['../tools/set_up_magics.ipynb', '../tools/save_them_all.ipynb', '../tools/stand.ipynb', '../tools/set_up_magics_dev.ipynb']
    ../sem01-intro-linux ['../sem01-intro-linux/intro_linux.ipynb']
    jupyter nbconvert ../tools/set_up_magics.ipynb --to markdown --output set_up_magics
    cp ../tools/set_up_magics.ipynb ./tmp_dir/8900933976576672904_set_up_magics.ipynb && jupyter nbconvert ./tmp_dir/8900933976576672904_set_up_magics.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/8900933976576672904_set_up_magics.ipynb --to markdown --output 8900933976576672904_set_up_magics.ipynb && cp ./tmp_dir/8900933976576672904_set_up_magics.ipynb.md ../tools/set_up_magics_no_output.md
    jupyter nbconvert ../tools/save_them_all.ipynb --to markdown --output save_them_all
    cp ../tools/save_them_all.ipynb ./tmp_dir/3178319372717154212_save_them_all.ipynb && jupyter nbconvert ./tmp_dir/3178319372717154212_save_them_all.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/3178319372717154212_save_them_all.ipynb --to markdown --output 3178319372717154212_save_them_all.ipynb && cp ./tmp_dir/3178319372717154212_save_them_all.ipynb.md ../tools/save_them_all_no_output.md
    jupyter nbconvert ../tools/stand.ipynb --to markdown --output stand
    cp ../tools/stand.ipynb ./tmp_dir/6301365062740299122_stand.ipynb && jupyter nbconvert ./tmp_dir/6301365062740299122_stand.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/6301365062740299122_stand.ipynb --to markdown --output 6301365062740299122_stand.ipynb && cp ./tmp_dir/6301365062740299122_stand.ipynb.md ../tools/stand_no_output.md
    jupyter nbconvert ../tools/set_up_magics_dev.ipynb --to markdown --output set_up_magics_dev
    cp ../tools/set_up_magics_dev.ipynb ./tmp_dir/592976144763424110_set_up_magics_dev.ipynb && jupyter nbconvert ./tmp_dir/592976144763424110_set_up_magics_dev.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/592976144763424110_set_up_magics_dev.ipynb --to markdown --output 592976144763424110_set_up_magics_dev.ipynb && cp ./tmp_dir/592976144763424110_set_up_magics_dev.ipynb.md ../tools/set_up_magics_dev_no_output.md
    jupyter nbconvert ../sem01-intro-linux/intro_linux.ipynb --to markdown --output README
    cp ../sem01-intro-linux/intro_linux.ipynb ./tmp_dir/4683768364076222926_intro_linux.ipynb && jupyter nbconvert ./tmp_dir/4683768364076222926_intro_linux.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/4683768364076222926_intro_linux.ipynb --to markdown --output 4683768364076222926_intro_linux.ipynb && cp ./tmp_dir/4683768364076222926_intro_linux.ipynb.md ../sem01-intro-linux/README_no_output.md
     [NbConvertApp] Converting notebook ../tools/set_up_magics.ipynb to markdown
    [NbConvertApp] Writing 41361 bytes to ../tools/set_up_magics.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/8900933976576672904_set_up_magics.ipynb to notebook
    [NbConvertApp] Writing 22048 bytes to ./tmp_dir/8900933976576672904_set_up_magics.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/8900933976576672904_set_up_magics.ipynb to markdown
    [NbConvertApp] Writing 15374 bytes to ./tmp_dir/8900933976576672904_set_up_magics.ipynb.md
    
     [NbConvertApp] Converting notebook ../tools/save_them_all.ipynb to markdown
    [NbConvertApp] Writing 16602 bytes to ../tools/save_them_all.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/3178319372717154212_save_them_all.ipynb to notebook
    [NbConvertApp] Writing 10408 bytes to ./tmp_dir/3178319372717154212_save_them_all.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/3178319372717154212_save_them_all.ipynb to markdown
    [NbConvertApp] Writing 6745 bytes to ./tmp_dir/3178319372717154212_save_them_all.ipynb.md
    
     [NbConvertApp] Converting notebook ../tools/stand.ipynb to markdown
    [NbConvertApp] Writing 299 bytes to ../tools/stand.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/6301365062740299122_stand.ipynb to notebook
    [NbConvertApp] Writing 1067 bytes to ./tmp_dir/6301365062740299122_stand.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/6301365062740299122_stand.ipynb to markdown
    [NbConvertApp] Writing 299 bytes to ./tmp_dir/6301365062740299122_stand.ipynb.md
    
     [NbConvertApp] Converting notebook ../tools/set_up_magics_dev.ipynb to markdown
    [NbConvertApp] Writing 32 bytes to ../tools/set_up_magics_dev.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/592976144763424110_set_up_magics_dev.ipynb to notebook
    [NbConvertApp] Writing 669 bytes to ./tmp_dir/592976144763424110_set_up_magics_dev.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/592976144763424110_set_up_magics_dev.ipynb to markdown
    [NbConvertApp] Writing 32 bytes to ./tmp_dir/592976144763424110_set_up_magics_dev.ipynb.md
    
     [NbConvertApp] Converting notebook ../sem01-intro-linux/intro_linux.ipynb to markdown
    [NbConvertApp] Writing 39083 bytes to ../sem01-intro-linux/README.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/4683768364076222926_intro_linux.ipynb to notebook
    [NbConvertApp] Writing 33124 bytes to ./tmp_dir/4683768364076222926_intro_linux.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/4683768364076222926_intro_linux.ipynb to markdown
    [NbConvertApp] Writing 24282 bytes to ./tmp_dir/4683768364076222926_intro_linux.ipynb.md
    


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
                return "\n<pre>\n" + f.read() + "\n</pre>\n"
    
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

     dos2unix: converting file ./../tools/set_up_magics.ipynb to Unix format...
    
     dos2unix: converting file ./../tools/save_them_all.ipynb to Unix format...
    
     dos2unix: converting file ./../tools/stand.ipynb to Unix format...
    
     dos2unix: converting file ./../tools/set_up_magics_dev.ipynb to Unix format...
    
     dos2unix: converting file ./../sem01-intro-linux/intro_linux.ipynb to Unix format...
    
     dos2unix: converting file ./../tools/save_them_all.md to Unix format...
    
     dos2unix: converting file ./../tools/save_them_all_no_output.md to Unix format...
    
     dos2unix: converting file ./../tools/set_up_magics_dev_no_output.md to Unix format...
    
     dos2unix: converting file ./../tools/stand_no_output.md to Unix format...
    
     dos2unix: converting file ./../tools/README.md to Unix format...
    
     dos2unix: converting file ./../tools/set_up_magics_dev.md to Unix format...
    
     dos2unix: converting file ./../tools/stand.md to Unix format...
    
     dos2unix: converting file ./../tools/set_up_magics_no_output.md to Unix format...
    
     dos2unix: converting file ./../tools/set_up_magics.md to Unix format...
    
     dos2unix: converting file ./../sem01-intro-linux/README.md to Unix format...
    
     dos2unix: converting file ./../sem01-intro-linux/README_no_output.md to Unix format...
    


### Смотрим изменения


```python
for subdir in highlevel_dirs:
    get_ipython().system("git diff {}/*_no_output*".format(subdir))
```

    [1mdiff --git a/tools/save_them_all_no_output.md b/tools/save_them_all_no_output.md[m
    [1mindex dd6dc35..97c6e11 100644[m
    [1m--- a/tools/save_them_all_no_output.md[m
    [1m+++ b/tools/save_them_all_no_output.md[m
    [36m@@ -137,7 +137,7 @@[m [mdef improve_md(fname):[m
             fname = os.path.join(path, matchobj.group(1))[m
             if fname.find("__FILE__") == -1:[m
                 with open(fname, "r") as f:[m
    [31m-                return "\n```\n" + f.read() + "\n```\n"[m
    [32m+[m[32m                return "\n<pre>\n" + f.read() + "\n</pre>\n"[m
         [m
         r = r.replace("", "")[m
         r = r.replace("", "")[m
    [36m@@ -215,11 +215,6 @@[m [mexecute_cmd("git push origin master")[m
     ```[m
     [m
     [m
    [31m-```python[m
    [31m-[m
    [31m-```[m
    [31m-[m
    [31m-[m
     ```python[m
     [m
     ```[m
    [1mdiff --git a/sem01-intro-linux/README_no_output.md b/sem01-intro-linux/README_no_output.md[m
    [1mindex 3f87da0..91b3bbc 100644[m
    [1m--- a/sem01-intro-linux/README_no_output.md[m
    [1m+++ b/sem01-intro-linux/README_no_output.md[m
    [36m@@ -2,7 +2,7 @@[m
     [m
     # Вступление: Linux, командная строка, Jupyter notebook[m
     [m
    [31m-Возможно кому-то Jupyter notebook покажется лишним в этом ряду, но так случилось, что я буду вести у вас АКОС, а мне он кажется очень удобным инструментом :)[m
    [32m+[m[32mВозможно кому-то Jupyter notebook покажется лишним в этом ряду, но так случилось, что я буду вести у вас АКОС, а мне он кажется очень удобным инструментом :) И вы в будущем все равно с ним столкнетесь на других курсах.[m
     [m
     <table width=100%> <tr>[m
         <th width=20%> <b>Видеозапись семинара &rarr; </b> </th>[m
    [36m@@ -17,8 +17,11 @@[m
      </table>[m
     [m
     [m
    [32m+[m[32m[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/blob/master/practice/linux_basics/intro.md) **И сейчас, и в будущем: читайте эти ридинги, там много полезной информации. Я стараюсь ее не дублировать, использование моих ноутбуков подразумевает чтение ридингов Яковлева.**[m
    [32m+[m
    [32m+[m
     Сегодня в программе:[m
    [31m-* <a href="#linux" style="color:#856024"> Очень кратко о Linux </a>[m
    [32m+[m[32m* <a href="https://github.com/victor-yacovlev/mipt-diht-caos/blob/master/practice/linux_basics/intro.md" style="color:#856024"> Очень кратко о Linux </a>[m
     * <a href="#terminal" style="color:#856024"> Часто используемые команды терминала </a>[m
       * <a href="#task1" style="color:#856024"> Задача 1 </a>[m
       * <a href="#task2" style="color:#856024"> Задача 2 </a>[m
    [36m@@ -28,27 +31,38 @@[m
     [m
     ## <a name="linux"></a> Очень кратко о Linux[m
     [m
    [31m-Не будем останавливаться на различиях дистрибутивов, просто далее будем подразумевать под Linux Ubuntu 20.04: [m
    [31m-* Довольно удобная система, легко найти решение проблем на stackoverflow[m
    [31m-* Все примеры и инструкции в моих материалах будут проверяться только на ней[m
    [32m+[m[32mЧитайте [ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/blob/master/practice/linux_basics/intro.md). Там неплохо написано про линукс и не только.[m
    [32m+[m
    [32m+[m[32mЕще есть [конспект семинара Александра Пономарева](https://github.com/Alexponomarev7/caos_seminars/tree/master/sem1).[m
    [32m+[m
    [32m+[m[32mИ куча статей в интернете :)[m
     [m
    [31m-Несколько максимально упрощенных тезисов, имеющих отношения к взаимодействию с Linux в рамках этого курса.[m
    [31m-* Есть рабочий стол: [m
    [31m-  * Можно запустить браузер и работать в нем так же, как если бы это была Windows или MacOS[m
    [31m-  * Легко установить и провести базовую настройку[m
    [31m-* Несмотря на наличие магазина приложений, многие операции легче делать из командной строки (а многие только из нее и возможно). Например, просто поставить IDE проще из консоли.[m
    [32m+[m[32mЕсли вы все это знаете, можете поиграть в [bandit](https://overthewire.org/wargames/bandit/). Сложненько, но интересно[m
     [m
     ## <a name="terminal"></a> Часто используемые команды терминала[m
     [m
     Первая важная особенность Jupyter notebook: можно запускать консольные команды, добавляя `!` в начало.[m
     То есть `!echo Hello` запустит `echo Hello` примерно так же, как это происходило бы в консоли.[m
     [m
    [32m+[m[32m### `cd`, `pwd`, `cp`, `mv`[m
    [32m+[m
    [32m+[m[32m`cd` - команда оболочки `bash`[m
    [32m+[m
    [32m+[m[32m`export PWD=/home` - альтернатива `cd`[m
    [32m+[m
    [32m+[m[32mПочти все остальные часто используемые команды - на самом деле запускаемые программы.[m
    [32m+[m
    [32m+[m
    [32m+[m[32m### `echo`[m
    [32m+[m
     [m
     ```python[m
     # Вывод строки[m
     !echo Hello[m
     ```[m
     [m
    [32m+[m[32m### `>`, `>>`, `cat`[m
    [32m+[m
     [m
     ```python[m
     # Перенаправление вывода в файл[m
    [36m@@ -61,6 +75,12 @@[m
     !cat file.txt[m
     ```[m
     [m
    [32m+[m
    [32m+[m[32m```python[m
    [32m+[m[32m! >file.txt echo "Hello1" # И так тоже можно :)[m
    [32m+[m[32m!cat file.txt[m
    [32m+[m[32m```[m
    [32m+[m
     Еще особенность Jupyter notebook: можно делать ячейки "магическими", добавляя `%%bash`, `%%python`, `%%time` в начало.[m
     [m
     Магия `%%bash` запустит ячейку как bash-скрипт. А bash-скрипт это почти то же самое, что последовательность команд в консоли.[m
    [36m@@ -76,6 +96,8 @@[m [mecho -e "Hello1\nHello2" > file.txt[m
     cat file.txt[m
     ```[m
     [m
    [32m+[m[32m### `|`, `<`[m
    [32m+[m
     [m
     ```bash[m
     %%bash [m
    [36m@@ -87,7 +109,8 @@[m [mecho -e "Hello1\nHello2\nHello10" > file.txt[m
     #   и выводит только содержащие подстроку`o1`[m
     cat file.txt | grep o1[m
     [m
    [31m-cat file.txt | grep llo2[m
    [32m+[m[32m<file.txt grep llo2[m
    [32m+[m[32mgrep llo2 < file.txt[m
     ```[m
     [m
     [m
    [36m@@ -95,11 +118,13 @@[m [mcat file.txt | grep llo2[m
     %%bash [m
     # Можно объединять команды с помощью &&[m
     # Тогда вторая выполнится только, если успешно выполнилась первая (как и в C/C++)[m
    [31m-echo Hello && echo world![m
    [32m+[m[32mecho -n "Hello " && echo world![m
     echo -----------------[m
    [31m-echo -n Hello && echo world![m
    [32m+[m[32mecho -n "Hello " || echo world! ; echo[m
     echo -----------------[m
    [31m-echo -n "Hello " && echo world![m
    [32m+[m[32mecho -n "Hello " && echBUG1o -n jail && echo -n freedom! ; echo[m
    [32m+[m[32mecho -----------------[m
    [32m+[m[32mecho -n "Hello " && echBUG1o jail || echo freedom![m
     ```[m
     [m
     [m
    [36m@@ -109,7 +134,7 @@[m [mexec 2>&1 ; set -x[m
     # Создадим пустой файл[m
     touch a.txt[m
     # Выведем список файлов в папке[m
    [31m-ls[m
    [32m+[m[32mls *.txt[m
     # Удалим файл[m
     rm a.txt && echo "rm success" || echo "rm fail"[m
     rm a.txt && echo "rm success" || echo "rm fail"[m
    [36m@@ -189,6 +214,104 @@[m [mcat a.txt | head -n 3[m
     </details>[m
     [m
     [m
    [32m+[m[32m```python[m
    [32m+[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m[32m### Стандартные потоки ввода (stdin, 0), вывода (stdout, 1), ошибок (stderr, 2)[m
    [32m+[m
    [32m+[m[32mПо умолчанию, когда мы запускаем программу, то у нее есть три стандартных потока:[m[41m [m
    [32m+[m[32m* ввода (через него все что мы печатаем в терминале передается программе)[m[41m [m
    [32m+[m[32m* вывода (через него то, что выводит программа (printf, std::cout) попадает в терминал)[m
    [32m+[m[32m* ошибок (примерно то же, что и stdout, но по-другому исползуется)[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```bash[m
    [32m+[m[32m%%bash[m
    [32m+[m[32mecho Hello # Успешное завершение в вывод текста в stdout[m
    [32m+[m[32mrm not_existent_file # Ошибка, и текст пишется в поток stderr[m
    [32m+[m[32mtrue # Просто успешно завершающаяся команда, чтобы скрипт не завершился ошибкой.[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m[32mПотоки ввода можно перенаправлять в файлы:[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```bash[m
    [32m+[m[32m%%bash[m
    [32m+[m[32mecho Hello 1> out.txt 2> err.txt[m[41m [m
    [32m+[m[32mecho "stdout: \"`cat out.txt`\" stderr: \"`cat err.txt`\""[m[41m [m
    [32m+[m[32mrm not_existent_file 1> out.txt 2> err.txt[m[41m [m
    [32m+[m[32mecho "stdout: \"`cat out.txt`\" stderr: \"`cat err.txt`\""[m
    [32m+[m
    [32m+[m[32mecho "Hello stXdents!" > file.txt[m
    [32m+[m[32mpython2 -c "import sys; print list(sys.stdin)[0].replace('stXdents', 'students')" 0< file.txt[m
    [32m+[m[32mpython2 -c "import os; print os.read(10, 100)" 10< file.txt # подумайте, что бы это могло быть :)[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m[32mПри этом `>` синоним к `>1` (аналогично `>>`). А `0<` то же самое что и `<`.[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```python[m
    [32m+[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m[32m### `ps`, `top`, `kill`, `killall`, `pidof` - найти, убить, убить попроще, найти по имени[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```bash[m
    [32m+[m[32m%%bash[m
    [32m+[m[32mps aux | grep ipyk[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```cpp[m
    [32m+[m[32m%%cpp bad_program.cpp[m
    [32m+[m[32m%run g++ bad_program.cpp -o bad_program.exe[m
    [32m+[m[32mint main() { while (1) {} }[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m[32m`TInteractiveLauncher` - моя магическая штука для запуска программ в интерактивном режиме из Jupyter notebook[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```python[m
    [32m+[m[32ma = TInteractiveLauncher("./bad_program.exe")[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```python[m
    [32m+[m[32mget_ipython().system("ps aux | grep bad_prog")[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```python[m
    [32m+[m[32mget_ipython().system("kill -9 " + str(a.get_pid()))[m
    [32m+[m[32ma.close()[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```python[m
    [32m+[m[32ma = TInteractiveLauncher("./bad_program.exe")[m
    [32m+[m[32mget_ipython().system("pidof bad_program.exe")[m
    [32m+[m[32mget_ipython().system("killall -9 bad_program.exe")[m
    [32m+[m[32ma.close()[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```python[m
    [32m+[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```python[m
    [32m+[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```python[m
    [32m+[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
     ```python[m
     [m
     ```[m
    [36m@@ -220,7 +343,7 @@[m [ma = 1; b = 2[m
     [m
     ```python[m
     %%save_file file.txt [m
    [31m-%# Сохраняет ячейку как файл (А в этой строке просто комментарий)[m
    [32m+[m[32m%# Сохраняет ячейку как файл, комментируя загаловок (А в этой строке просто комментарий)[m
     %run cat file.txt # Выполняет команды следующие после %run в заголовках ячейки[m
     Содержимое файла[m
     ```[m
    [36m@@ -248,6 +371,42 @@[m [mint main() {[m
     ```[m
     [m
     [m
    [32m+[m[32m```python[m
    [32m+[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```cpp[m
    [32m+[m[32m%%cpp b.cpp[m
    [32m+[m[32m%run g++ b.cpp -o b.exe[m
    [32m+[m
    [32m+[m[32m#include <iostream>[m
    [32m+[m
    [32m+[m[32mint main() {[m
    [32m+[m[32m    std::string s;[m
    [32m+[m[32m    std::cin >> s;[m
    [32m+[m[32m    std::cout << "STDOUT " << s << std::endl;[m
    [32m+[m[32m    std::cerr << "STDERR " << s << std::endl;[m
    [32m+[m[32m}[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```python[m
    [32m+[m[32m# интерактивная запускалка для программ[m
    [32m+[m[32ma = TInteractiveLauncher("./b.exe")[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```python[m
    [32m+[m[32ma.write("Hello\n")[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```python[m
    [32m+[m[32ma.close()[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
     ```python[m
     [m
     ```[m


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

    > git add --ignore-errors  ../tools/*.ipynb
    > git add --ignore-errors  ../tools/*.md
    > git add --ignore-errors  ../tools/*.c
    fatal: pathspec '../tools/*.c' did not match any files
    > git add --ignore-errors  ../tools/*.cpp
    > git add --ignore-errors -f  -f ../tools/bash_popen_tmp/*.html
    > git add --ignore-errors -f  -f ../tools/interactive_launcher_tmp/*.log
    > git add -u ../tools
    > git add --ignore-errors  ../sem01-intro-linux/*.ipynb
    > git add --ignore-errors  ../sem01-intro-linux/*.md
    > git add --ignore-errors  ../sem01-intro-linux/*.c
    > git add --ignore-errors  ../sem01-intro-linux/*.cpp
    > git add --ignore-errors -f  -f ../sem01-intro-linux/bash_popen_tmp/*.html
    warning: could not open directory 'sem01-intro-linux/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem01-intro-linux/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem01-intro-linux/interactive_launcher_tmp/*.log
    > git add -u ../sem01-intro-linux
    > git commit -m 'yet another update'
    [master 014e061] yet another update
     29 files changed, 1563 insertions(+), 764 deletions(-)
     create mode 100644 sem01-intro-linux/b.cpp
     create mode 100644 sem01-intro-linux/bad_program.cpp
     create mode 100644 sem01-intro-linux/interactive_launcher_tmp/134856167677051543.log
     create mode 100644 sem01-intro-linux/interactive_launcher_tmp/159458045408584490.log
     create mode 100644 sem01-intro-linux/interactive_launcher_tmp/160806476576838908.log
     create mode 100644 sem01-intro-linux/interactive_launcher_tmp/237887356681257319.log
     create mode 100644 sem01-intro-linux/interactive_launcher_tmp/272097196292346552.log
     create mode 100644 sem01-intro-linux/interactive_launcher_tmp/291478633046804378.log
     create mode 100644 sem01-intro-linux/interactive_launcher_tmp/299779856288802977.log
     create mode 100644 sem01-intro-linux/interactive_launcher_tmp/308063480967833729.log
     create mode 100644 sem01-intro-linux/interactive_launcher_tmp/33203167529660997.log
     create mode 100644 sem01-intro-linux/interactive_launcher_tmp/368536380590515236.log
     create mode 100644 sem01-intro-linux/interactive_launcher_tmp/44638202710957050.log
     create mode 100644 sem01-intro-linux/interactive_launcher_tmp/488615332501153522.log
     create mode 100644 sem01-intro-linux/interactive_launcher_tmp/547003026116871195.log
     create mode 100644 sem01-intro-linux/interactive_launcher_tmp/551941962051262166.log
     create mode 100644 sem01-intro-linux/interactive_launcher_tmp/580865104873188457.log
     create mode 100644 sem01-intro-linux/interactive_launcher_tmp/58318340108386158.log
     create mode 100644 sem01-intro-linux/interactive_launcher_tmp/718444346696566167.log
     create mode 100644 sem01-intro-linux/interactive_launcher_tmp/906648630641962511.log
     create mode 100644 sem01-intro-linux/interactive_launcher_tmp/93290423175998573.log
     create mode 100644 sem01-intro-linux/interactive_launcher_tmp/965132557118189549.log
     rewrite tools/set_up_magics.md (74%)
    > git push origin master
    Enumerating objects: 44, done.
    Counting objects: 100% (44/44), done.
    Delta compression using up to 4 threads
    Compressing objects: 100% (34/34), done.
    Writing objects: 100% (34/34), 11.37 KiB | 306.00 KiB/s, done.
    Total 34 (delta 22), reused 0 (delta 0)
    remote: Resolving deltas: 100% (22/22), completed with 7 local objects.[K
    To github.com:yuri-pechatnov/caos.git
       7be0e28..014e061  master -> master



```python

```


```python
!git add ../README.md
!git commit -m "Update readme"
!git push origin master
```

    On branch master
    Your branch is up to date with 'origin/master'.
    
    Untracked files:
      (use "git add <file>..." to include in what will be committed)
    	[31m../sem01-intro-linux/2[m
    	[31m../sem01-intro-linux/a.sh[m
    	[31m../sem01-intro-linux/b.txt[m
    	[31m../sem01-intro-linux/err[m
    	[31m../sem01-intro-linux/err.txt[m
    	[31m../sem01-intro-linux/file.txt[m
    	[31m../sem01-intro-linux/file2.txt[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/134856167677051543.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/159458045408584490.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/160806476576838908.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/237887356681257319.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/272097196292346552.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/291478633046804378.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/299779856288802977.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/308063480967833729.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/33203167529660997.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/368536380590515236.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/44638202710957050.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/488615332501153522.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/547003026116871195.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/551941962051262166.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/580865104873188457.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/58318340108386158.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/718444346696566167.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/906648630641962511.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/93290423175998573.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/965132557118189549.log.md[m
    	[31m../sem01-intro-linux/out[m
    	[31m../sem01-intro-linux/out.txt[m
    	[31ma.py[m
    	[31minteractive_launcher_tmp/110047129715984560.log.md[m
    	[31minteractive_launcher_tmp/243546179874885578.log.md[m
    	[31minteractive_launcher_tmp/910964812581195986.log.md[m
    	[31minteractive_launcher_tmp/933917889050860419.log.md[m
    	[31mlauncher.py[m
    	[31mtmp_dir/[m
    
    nothing added to commit but untracked files present (use "git add" to track)
    Everything up-to-date



```python

```


```python

```


```python

```


```python

```


```python

```
