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
    [NbConvertApp] Writing 10578 bytes to ../tools/save_them_all.md
    
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
    [NbConvertApp] Writing 16715 bytes to ../sem01-intro-linux/README.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/7792754777867098616_intro_linux.ipynb to notebook
    [NbConvertApp] Writing 17295 bytes to ./tmp_dir/7792754777867098616_intro_linux.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/7792754777867098616_intro_linux.ipynb to markdown
    [NbConvertApp] Writing 15418 bytes to ./tmp_dir/7792754777867098616_intro_linux.ipynb.md
    


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

    [1mdiff --git a/tools/save_them_all_no_output.md b/tools/save_them_all_no_output.md[m
    [1mindex 25737f6..311b936 100644[m
    [1m--- a/tools/save_them_all_no_output.md[m
    [1m+++ b/tools/save_them_all_no_output.md[m
    [36m@@ -18,9 +18,9 @@[m [mimport os[m
     import subprocess[m
     [m
     highlevel_dirs = sum([[m
    [31m-    #["../tools"], [m
    [32m+[m[32m    ["../tools"],[m[41m [m
         #sorted(glob.glob("../sem26*")),[m
    [31m-    sorted(glob.glob("../sem17*")),[m
    [32m+[m[32m    sorted(glob.glob("../sem01*")),[m
     ], [])[m
     [m
     print("Highlevel dirs:", highlevel_dirs)[m
    [36m@@ -36,6 +36,17 @@[m [mget_ipython().system('rm -r {tmp_dir} ; mkdir {tmp_dir} 2>&1 | grep -v "File exi[m
     \+ Делаем .md-шки очищенные для вывода. По этим .md-шкам можно будет смотреть реальную историю изменений. И дифф при пулреквестах.[m
     [m
     [m
    [32m+[m[32m```python[m
    [32m+[m[32mdef execute_all_in_parallel(tasks):[m
    [32m+[m[32m    ps = [][m
    [32m+[m[32m    for t in tasks:[m
    [32m+[m[32m        ps.append(subprocess.Popen(["bash", "-c", t], stdout=subprocess.PIPE, stderr=subprocess.PIPE))[m
    [32m+[m[32m    for p in ps:[m
    [32m+[m[32m        out, err = p.communicate()[m
    [32m+[m[32m        print(out.decode(), err.decode())[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
     ```python[m
     from multiprocessing import Pool[m
     [m
    [36m@@ -75,14 +86,6 @@[m [mfor subdir in highlevel_dirs:[m
     [m
     print("\n".join(tasks))[m
     [m
    [31m-def execute_all_in_parallel(tasks):[m
    [31m-    ps = [][m
    [31m-    for t in tasks:[m
    [31m-        ps.append(subprocess.Popen(["bash", "-c", t], stdout=subprocess.PIPE, stderr=subprocess.PIPE))[m
    [31m-    for p in ps:[m
    [31m-        out, err = p.communicate()[m
    [31m-        print(out.decode(), err.decode())[m
    [31m-[m
     execute_all_in_parallel(tasks)[m
     ```[m
     [m
    [36m@@ -112,10 +115,7 @@[m [mdef improve_md(fname):[m
         r = r.replace('\n', "SUPER_SLASH" + "_N_REPLACER")[m
         [m
         r = re.sub(r'\<\!--MD_BEGIN_FILTER--\>.*?\<\!--MD_END_FILTER--\>', "", r)[m
    [31m-    #r = re.sub(r'(\#SET_UP_MAGIC_BEGIN.*?\#SET_UP_MAGIC_END)', "<too much code>", r)[m
    [31m-    r = re.sub(r'\<\!\-\-\ YANDEX_METRICA_BEGIN\ \-\-\>.*\<\!\-\-\ YANDEX_METRICA_END\ \-\-\>', '', r)[m
         [m
    [31m-    r = r.replace("", '')[m
         r = r.replace("SUPER_SLASH" + "_N_REPLACER", '\n')[m
         [m
         template = "#""MAGICS_SETUP_END"[m
    [36m@@ -209,7 +209,9 @@[m [mexecute_cmd("git push origin master")[m
     [m
     [m
     ```python[m
    [31m-[m
    [32m+[m[32m!git add ../README.md[m
    [32m+[m[32m!git commit -m "Update readme"[m
    [32m+[m[32m!git push origin master[m
     ```[m
     [m
     [m
    [1mdiff --git a/tools/set_up_magics_no_output.md b/tools/set_up_magics_no_output.md[m
    [1mindex a80837f..a2c3b77 100644[m
    [1m--- a/tools/set_up_magics_no_output.md[m
    [1m+++ b/tools/set_up_magics_no_output.md[m
    [36m@@ -184,7 +184,20 @@[m [mTInteractiveLauncher.terminate_all()[m
        [m
     yandex_metrica_allowed = bool(globals().get("yandex_metrica_allowed", False))[m
     if yandex_metrica_allowed:[m
    [31m-    display(HTML(''''''))[m
    [32m+[m[32m    display(HTML('''<!-- YANDEX_METRICA_BEGIN -->[m
    [32m+[m[32m    <script type="text/javascript" >[m
    [32m+[m[32m       (function(m,e,t,r,i,k,a){m[i]=m[i]||function(){(m[i].a=m[i].a||[]).push(arguments)};[m
    [32m+[m[32m       m[i].l=1*new Date();k=e.createElement(t),a=e.getElementsByTagName(t)[0],k.async=1,k.src=r,a.parentNode.insertBefore(k,a)})[m
    [32m+[m[32m       (window, document, "script", "https://mc.yandex.ru/metrika/tag.js", "ym");[m
    [32m+[m
    [32m+[m[32m       ym(59260609, "init", {[m
    [32m+[m[32m            clickmap:true,[m
    [32m+[m[32m            trackLinks:true,[m
    [32m+[m[32m            accurateTrackBounce:true[m
    [32m+[m[32m       });[m
    [32m+[m[32m    </script>[m
    [32m+[m[32m    <noscript><div><img src="https://mc.yandex.ru/watch/59260609" style="position:absolute; left:-9999px;" alt="" /></div></noscript>[m
    [32m+[m[32m    <!-- YANDEX_METRICA_END -->'''))[m
     [m
     def make_oneliner():[m
         html_text = '("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований. Если вы не хотите, чтобы по вам собиралась статистика, исправьте: yandex_metrica_allowed = False" if yandex_metrica_allowed else "")'[m


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
    fatal: pathspec '../sem01-intro-linux/*.cpp' did not match any files
    > git add --ignore-errors -f  -f ../sem01-intro-linux/bash_popen_tmp/*.html
    warning: could not open directory 'sem01-intro-linux/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem01-intro-linux/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem01-intro-linux/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem01-intro-linux/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem01-intro-linux/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem01-intro-linux
    > git commit -m 'yet another update'
    [master 807ac3e] yet another update
     14 files changed, 763 insertions(+), 504 deletions(-)
     create mode 100644 sem01-intro-linux/README.md
     create mode 100644 sem01-intro-linux/README_no_output.md
     create mode 100644 sem01-intro-linux/intro_linux.ipynb
     create mode 100644 sem01-intro-linux/lib.c
     rename tools/interactive_launcher_tmp/{981384515921563800.log => 152093000614822661.log} (89%)
     rename tools/interactive_launcher_tmp/{341853987367548759.log => 404502961379090749.log} (82%)
     rename tools/interactive_launcher_tmp/{978838368375159155.log => 556287154257559006.log} (56%)
     rename tools/interactive_launcher_tmp/{937518918408435731.log => 988648069798698224.log} (89%)
    > git push origin master
    Enumerating objects: 25, done.
    Counting objects: 100% (25/25), done.
    Delta compression using up to 4 threads
    Compressing objects: 100% (16/16), done.
    Writing objects: 100% (16/16), 11.28 KiB | 1.03 MiB/s, done.
    Total 16 (delta 11), reused 0 (delta 0)
    remote: Resolving deltas: 100% (11/11), completed with 8 local objects.[K
    To github.com:yuri-pechatnov/caos.git
       6af5115..807ac3e  master -> master



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
