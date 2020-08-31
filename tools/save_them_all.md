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
    cp ../tools/set_up_magics.ipynb ./tmp_dir/2194680534572888862_set_up_magics.ipynb && jupyter nbconvert ./tmp_dir/2194680534572888862_set_up_magics.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/2194680534572888862_set_up_magics.ipynb --to markdown --output 2194680534572888862_set_up_magics.ipynb && cp ./tmp_dir/2194680534572888862_set_up_magics.ipynb.md ../tools/set_up_magics_no_output.md
    jupyter nbconvert ../tools/save_them_all.ipynb --to markdown --output save_them_all
    cp ../tools/save_them_all.ipynb ./tmp_dir/7083710874915117445_save_them_all.ipynb && jupyter nbconvert ./tmp_dir/7083710874915117445_save_them_all.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/7083710874915117445_save_them_all.ipynb --to markdown --output 7083710874915117445_save_them_all.ipynb && cp ./tmp_dir/7083710874915117445_save_them_all.ipynb.md ../tools/save_them_all_no_output.md
    jupyter nbconvert ../tools/stand.ipynb --to markdown --output stand
    cp ../tools/stand.ipynb ./tmp_dir/8379913619729260699_stand.ipynb && jupyter nbconvert ./tmp_dir/8379913619729260699_stand.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/8379913619729260699_stand.ipynb --to markdown --output 8379913619729260699_stand.ipynb && cp ./tmp_dir/8379913619729260699_stand.ipynb.md ../tools/stand_no_output.md
    jupyter nbconvert ../tools/set_up_magics_dev.ipynb --to markdown --output set_up_magics_dev
    cp ../tools/set_up_magics_dev.ipynb ./tmp_dir/7869291486245681920_set_up_magics_dev.ipynb && jupyter nbconvert ./tmp_dir/7869291486245681920_set_up_magics_dev.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/7869291486245681920_set_up_magics_dev.ipynb --to markdown --output 7869291486245681920_set_up_magics_dev.ipynb && cp ./tmp_dir/7869291486245681920_set_up_magics_dev.ipynb.md ../tools/set_up_magics_dev_no_output.md
    jupyter nbconvert ../sem01-intro-linux/intro_linux.ipynb --to markdown --output README
    cp ../sem01-intro-linux/intro_linux.ipynb ./tmp_dir/7792754777867098616_intro_linux.ipynb && jupyter nbconvert ./tmp_dir/7792754777867098616_intro_linux.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/7792754777867098616_intro_linux.ipynb --to markdown --output 7792754777867098616_intro_linux.ipynb && cp ./tmp_dir/7792754777867098616_intro_linux.ipynb.md ../sem01-intro-linux/README_no_output.md
     [NbConvertApp] Converting notebook ../tools/set_up_magics.ipynb to markdown
    [NbConvertApp] Writing 41310 bytes to ../tools/set_up_magics.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/2194680534572888862_set_up_magics.ipynb to notebook
    [NbConvertApp] Writing 21706 bytes to ./tmp_dir/2194680534572888862_set_up_magics.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/2194680534572888862_set_up_magics.ipynb to markdown
    [NbConvertApp] Writing 15323 bytes to ./tmp_dir/2194680534572888862_set_up_magics.ipynb.md
    
     [NbConvertApp] Converting notebook ../tools/save_them_all.ipynb to markdown
    [NbConvertApp] Writing 21559 bytes to ../tools/save_them_all.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/7083710874915117445_save_them_all.ipynb to notebook
    [NbConvertApp] Writing 10403 bytes to ./tmp_dir/7083710874915117445_save_them_all.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/7083710874915117445_save_them_all.ipynb to markdown
    [NbConvertApp] Writing 6740 bytes to ./tmp_dir/7083710874915117445_save_them_all.ipynb.md
    
     [NbConvertApp] Converting notebook ../tools/stand.ipynb to markdown
    [NbConvertApp] Writing 306 bytes to ../tools/stand.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/8379913619729260699_stand.ipynb to notebook
    [NbConvertApp] Writing 1074 bytes to ./tmp_dir/8379913619729260699_stand.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/8379913619729260699_stand.ipynb to markdown
    [NbConvertApp] Writing 306 bytes to ./tmp_dir/8379913619729260699_stand.ipynb.md
    
     [NbConvertApp] Converting notebook ../tools/set_up_magics_dev.ipynb to markdown
    [NbConvertApp] Writing 32 bytes to ../tools/set_up_magics_dev.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/7869291486245681920_set_up_magics_dev.ipynb to notebook
    [NbConvertApp] Writing 669 bytes to ./tmp_dir/7869291486245681920_set_up_magics_dev.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/7869291486245681920_set_up_magics_dev.ipynb to markdown
    [NbConvertApp] Writing 32 bytes to ./tmp_dir/7869291486245681920_set_up_magics_dev.ipynb.md
    
     [NbConvertApp] Converting notebook ../sem01-intro-linux/intro_linux.ipynb to markdown
    [NbConvertApp] Writing 23644 bytes to ../sem01-intro-linux/README.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/7792754777867098616_intro_linux.ipynb to notebook
    [NbConvertApp] Writing 26700 bytes to ./tmp_dir/7792754777867098616_intro_linux.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/7792754777867098616_intro_linux.ipynb to markdown
    [NbConvertApp] Writing 21077 bytes to ./tmp_dir/7792754777867098616_intro_linux.ipynb.md
    


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

    [1mdiff --git a/sem01-intro-linux/README_no_output.md b/sem01-intro-linux/README_no_output.md[m
    [1mindex 715e47b..b0504b2 100644[m
    [1m--- a/sem01-intro-linux/README_no_output.md[m
    [1m+++ b/sem01-intro-linux/README_no_output.md[m
    [36m@@ -1,39 +1,247 @@[m
     [m
     [m
    [32m+[m[32m# Вступление: Linux, командная строка, Jupyter notebook[m
     [m
    [31m-```cpp[m
    [31m-%%cpp lib.c[m
    [31m-%run gcc -shared -fPIC lib.c -o lib.so # compile shared library[m
    [32m+[m[32mВозможно кому-то Jupyter notebook покажется лишним в этом ряду, но так случилось, что я буду вести у вас АКОС, а мне он кажется очень удобным инструментом :)[m
     [m
    [31m-int sum(int a, int b) {[m
    [31m-    return a + b;[m
    [31m-}[m
    [32m+[m[32m<table width=100%> <tr>[m
    [32m+[m[32m    <th width=15%> <b>Видео с семинара &rarr; </b> </th>[m
    [32m+[m[32m    <th>[m
    [32m+[m[32m    <a href="???"><img src="video.jpg" width="320"[m[41m [m
    [32m+[m[32m   height="160" align="left" alt="Видео с семинара"></a>[m
    [32m+[m[32m    </th>[m
    [32m+[m[32m    <th> </th>[m
    [32m+[m[32m </table>[m
     [m
    [31m-float sum_f(float a, float b) {[m
    [31m-    return a + b;[m
    [31m-}[m
    [32m+[m
    [32m+[m[32mСегодня в программе:[m
    [32m+[m[32m* <a href="#linux" style="color:#856024"> Очень кратко о Linux </a>[m
    [32m+[m[32m* <a href="#terminal" style="color:#856024"> Часто используемые команды терминала </a>[m
    [32m+[m[32m  * <a href="#task1" style="color:#856024"> Задача 1 </a>[m
    [32m+[m[32m  * <a href="#task2" style="color:#856024"> Задача 2 </a>[m
    [32m+[m[32m  * <a href="#task3" style="color:#856024"> Задача 3 </a>[m
    [32m+[m[32m* <a href="#jupyter" style="color:#856024"> Особенности Jupyter notebok используемые в курсе </a>[m
    [32m+[m
    [32m+[m
    [32m+[m[32m## <a name="linux"></a> Очень кратко о Linux[m
    [32m+[m
    [32m+[m[32mНе будем останавливаться на различиях дистрибутивов, просто далее будем подразумевать под Linux Ubuntu 20.04:[m[41m [m
    [32m+[m[32m* Довольно удобная система, легко найти решение проблем на stackoverflow[m
    [32m+[m[32m* Все примеры и инструкции в моих материалах будут проверяться только на ней[m
    [32m+[m
    [32m+[m[32mНесколько максимально упрощенных тезисов, имеющих отношения к взаимодействию с Linux в рамках этого курса.[m
    [32m+[m[32m* Есть рабочий стол:[m[41m [m
    [32m+[m[32m  * Можно запустить браузер и работать в нем так же, как если бы это была Windows или MacOS[m
    [32m+[m[32m  * Легко установить и провести базовую настройку[m
    [32m+[m[32m* Несмотря на наличие магазина приложений, многие операции легче делать из командной строки (а многие только из нее и возможно). Например, просто поставить IDE проще из консоли.[m
    [32m+[m
    [32m+[m[32m## <a name="terminal"></a> Часто используемые команды терминала[m
    [32m+[m
    [32m+[m[32mПервая важная особенность Jupyter notebook: можно запускать консольные команды, добавляя `!` в начало.[m
    [32m+[m[32mТо есть `!echo Hello` запустит `echo Hello` примерно так же, как это происходило бы в консоли.[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```python[m
    [32m+[m[32m# Вывод строки[m
    [32m+[m[32m!echo Hello[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```python[m
    [32m+[m[32m# Перенаправление вывода в файл[m
    [32m+[m[32m!echo "Hello1" > file.txt[m
    [32m+[m[32m!echo "Hello2" > file.txt # Файл перезапишется[m
    [32m+[m[32m# Перенаправление вывода в файл (при этом файл дозапишется, а не перезапишется)[m
    [32m+[m[32m!echo "Hello3" >> file.txt[m
    [32m+[m
    [32m+[m[32m# Вывод файла[m[41m [m
    [32m+[m[32m!cat file.txt[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m[32mЕще особенность Jupyter notebook: можно делать ячейки "магическими", добавляя `%%bash`, `%%python`, `%%time` в начало.[m
    [32m+[m
    [32m+[m[32mМагия `%%bash` запустит ячейку как bash-скрипт. А bash-скрипт это почти то же самое, что последовательность команд в консоли.[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```bash[m
    [32m+[m[32m%%bash[m[41m [m
    [32m+[m[32mexec 2>&1 # Настройка bash, чтобы все что пишется в поток ошибок на самом деле писалось в стандартный поток[m
    [32m+[m[32mset -x # Настройка bash, чтобы он выводил все исполняемые команды в поток ошибок[m
    [32m+[m
    [32m+[m[32m# -e включает восприятие \n как перевода строки у команды echo[m
    [32m+[m[32mecho -e "Hello1\nHello2" > file.txt[m
    [32m+[m[32mcat file.txt[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```bash[m
    [32m+[m[32m%%bash[m[41m [m
    [32m+[m[32mexec 2>&1 ; set -x # Команды можно писать в одну строчку разделяя с помощью ;[m
    [32m+[m
    [32m+[m[32mecho -e "Hello1\nHello2\nHello10" > file.txt[m
    [32m+[m[32m# Можно направлять вывод одной команды на вход другой с помощью |[m
    [32m+[m[32m# Команда grep в таком использовании читает все строки из входа,[m[41m [m
    [32m+[m[32m#   и выводит только содержащие подстроку`o1`[m
    [32m+[m[32mcat file.txt | grep o1[m
    [32m+[m
    [32m+[m[32mcat file.txt | grep llo2[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```bash[m
    [32m+[m[32m%%bash[m[41m [m
    [32m+[m[32m# Можно объединять команды с помощью &&[m
    [32m+[m[32m# Тогда вторая выполнится только, если успешно выполнилась первая (как и в C/C++)[m
    [32m+[m[32mecho Hello && echo world![m
    [32m+[m[32mecho -----------------[m
    [32m+[m[32mecho -n Hello && echo world![m
    [32m+[m[32mecho -----------------[m
    [32m+[m[32mecho -n "Hello " && echo world![m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```bash[m
    [32m+[m[32m%%bash[m[41m [m
    [32m+[m[32mexec 2>&1 ; set -x[m
    [32m+[m[32m# Создадим пустой файл[m
    [32m+[m[32mtouch a.txt[m
    [32m+[m[32m# Выведем список файлов в папке[m
    [32m+[m[32mls[m
    [32m+[m[32m# Удалим файл[m
    [32m+[m[32mrm a.txt && echo "rm success" || echo "rm fail"[m
    [32m+[m[32mrm a.txt && echo "rm success" || echo "rm fail"[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```bash[m
    [32m+[m[32m%%bash[m[41m [m
    [32m+[m[32mexec 2>&1 ; set -x[m
    [32m+[m[32m# Начало, конец файла, количество строк[m
    [32m+[m[32mecho -e "1\n2\n3\n4" > a.txt[m
    [32m+[m[32mcat a.txt | head -n 2[m
    [32m+[m[32mcat a.txt | tail -n 2[m
    [32m+[m[32mcat a.txt | wc -l[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```python[m
    [32m+[m[32m# Получить документацию по программе легко с помощью команды man[m
    [32m+[m[32m!man head | head -n 10[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m[32mА, важный момент, вообще по умолчанию в ячейках Jupyter notebook пишется код на том языке, с ядром которого запущен ноутбук. Сейчас это python3[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```python[m
    [32m+[m[32mwith open("b.txt", "w") as f:[m
    [32m+[m[32m    for i in range(100):[m
    [32m+[m[32m        tup = (str(i), "div17" if i % 17 == 0 else "no17")[m
    [32m+[m[32m        f.write("\t".join(tup) + "\n")[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m[32m## <a name="task1"></a> Задача 1[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```bash[m
    [32m+[m[32m%%bash[m
    [32m+[m[32mcat b.txt | head -n 5[m
    [32m+[m[32m# Выведите все строчки где есть подстрока div17[m
    [32m+[m[32m# <your line of code here>[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m[32m## <a name="task2"></a> Задача 2[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```bash[m
    [32m+[m[32m%%bash[m
    [32m+[m[32m# Выясните, что выведет следующая команда, не запуская ее.[m
    [32m+[m[32m# echo -e "A\nB\nC\nD" | grep -v C -n[m[41m [m
    [32m+[m[32m# Используйте только команды man и grep[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```bash[m
    [32m+[m[32m%%bash[m
    [32m+[m[32m# Подсказка[m
    [32m+[m[32mman grep | grep -e "-C" -C 2[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m[32m## <a name="task3"></a> Задача 3[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```bash[m
    [32m+[m[32m%%bash[m
    [32m+[m[32m# Создадим файл на 100 строк[m
    [32m+[m[32m( for i in {0..99} ; do echo line$i ; done ) > a.txt[m
    [32m+[m[32mcat a.txt | head -n 3[m
    [32m+[m
    [32m+[m[32m# А теперь напишите последовательность команд, которая заменит вторую строчку с line1 на line100[m
    [32m+[m[32m# нельзя использовать replace конструкции, вам достаточно команд, которые были перечислены ранее :)[m
    [32m+[m
    [32m+[m[32m# <your code here>[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m[32m<details> <summary> Способ в лоб (пожалуйста не смотрите, пока не напишете код решения задачки) </summary>[m
    [32m+[m[32m  <p> cat a.txt | python2 -c 'import sys; lines = list(sys.stdin); lines[1] = "line100\n"; print "".join(lines)' </p>[m
    [32m+[m[32m</details>[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```python[m
    [32m+[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m[32m## <a name="jupyter"></a> Особенности Jupyter notebok используемые в курсе[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```python[m
    [32m+[m[32m# код на питоне[m
    [32m+[m[32m[i for i in range(0, 3000, 17) if str(i).endswith("19")] # просто что-то странное[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
    [32m+[m[32m```bash[m
    [32m+[m[32m%%bash[m
    [32m+[m[32mecho "Bash code"[m
     ```[m
     [m
    [32m+[m[32mJupyter notebook позволяет определять собственную магию. И в первой ячейке каждого моего ноутбука есть одност рочник, который определяет несколько магий:[m
    [32m+[m
     [m
     ```python[m
    [31m-!objdump -t lib.so | grep sum  # symbols in shared library[m
    [32m+[m[32m# Просто вывод строки и ее значения как выражения в питоне[m
    [32m+[m[32ma = 1; b = 2[m
    [32m+[m[32m%p a + b # Sum of a and b[m
    [32m+[m[32m%p (a, b)[m
     ```[m
     [m
     [m
     ```python[m
    [31m-from IPython.display import display[m
    [31m-import ctypes[m
    [32m+[m[32m%%save_file file.txt[m[41m [m
    [32m+[m[32m%# Сохраняет ячейку как файл (А в этой строке просто комментарий)[m
    [32m+[m[32m%run cat file.txt # Выполняет команды следующие после %run в заголовках ячейки[m
    [32m+[m[32mСодержимое файла[m
    [32m+[m[32m```[m
     [m
    [31m-lib = ctypes.CDLL("./lib.so")[m
    [31m-%p lib.sum(3, 4)[m
    [31m-%p lib.sum_f(3, 4)[m
     [m
    [31m-lib.sum_f.restype = ctypes.c_float[m
    [31m-%p lib.sum_f(3, 4) # with set return type[m
    [32m+[m[32m```python[m
    [32m+[m[32m%%save_file a.sh[m
    [32m+[m[32m%run bash a.sh[m
    [32m+[m
    [32m+[m[32mecho 123[m
    [32m+[m[32m```[m
     [m
    [31m-lib.sum_f.argtypes = [ctypes.c_float, ctypes.c_float][m
    [31m-lib.sum_f.restype = ctypes.c_float[m
    [31m-%p lib.sum_f(3, 4) # with set return and arguments types[m
    [32m+[m
    [32m+[m[32m```cpp[m
    [32m+[m[32m%%cpp a.cpp[m
    [32m+[m[32m%# По большому счету тот же save_file, но подсвечивает синтаксис C++[m
    [32m+[m[32m%run g++ a.cpp -o a.exe[m
    [32m+[m[32m%run ./a.exe[m
    [32m+[m
    [32m+[m[32m#include <iostream>[m
    [32m+[m
    [32m+[m[32mint main() {[m
    [32m+[m[32m    std::cout << "Hello world!" << std::endl;[m
    [32m+[m[32m}[m
     ```[m
     [m
     [m


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
    warning: could not open directory 'sem01-intro-linux/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem01-intro-linux/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem01-intro-linux
    > git commit -m 'yet another update'
    On branch master
    Your branch is up to date with 'origin/master'.
    
    Untracked files:
      (use "git add <file>..." to include in what will be committed)
    	[31m../sem01-intro-linux/a.sh[m
    	[31m../sem01-intro-linux/a.txt[m
    	[31m../sem01-intro-linux/b.txt[m
    	[31m../sem01-intro-linux/file.txt[m
    	[31m"../sem01-intro-linux/\320\256"[m
    	[31ma.py[m
    	[31minteractive_launcher_tmp/273640832397559891.log.md[m
    	[31minteractive_launcher_tmp/582664403697483710.log.md[m
    	[31minteractive_launcher_tmp/798219650792581873.log.md[m
    	[31minteractive_launcher_tmp/930644569964515880.log.md[m
    	[31mlauncher.py[m
    	[31mtmp_dir/[m
    
    nothing added to commit but untracked files present (use "git add" to track)
    > git push origin master
    Everything up-to-date



```python

```


```python
!git add ../README.md
!git commit -m "Update readme"
!git push origin master
```

    [master 6af5115] Update readme
     1 file changed, 4 insertions(+), 2 deletions(-)
    Enumerating objects: 5, done.
    Counting objects: 100% (5/5), done.
    Delta compression using up to 4 threads
    Compressing objects: 100% (3/3), done.
    Writing objects: 100% (3/3), 452 bytes | 452.00 KiB/s, done.
    Total 3 (delta 2), reused 0 (delta 0)
    remote: Resolving deltas: 100% (2/2), completed with 2 local objects.[K
    To github.com:yuri-pechatnov/caos.git
       087ef31..6af5115  master -> master



```python

```
