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
    sorted(glob.glob("../sem09*")),
    #sorted(glob.glob("../sem07*")),
    #sorted(glob.glob("../extra*")),
], [])

print("Highlevel dirs:", highlevel_dirs)
```

    Highlevel dirs: ['../tools', '../sem09-x86-asm-nostdlib']



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
    ../sem09-x86-asm-nostdlib ['../sem09-x86-asm-nostdlib/nostdlib.ipynb']
    jupyter nbconvert ../tools/set_up_magics.ipynb --to markdown --output set_up_magics
    cp ../tools/set_up_magics.ipynb ./tmp_dir/4438040515678393921_set_up_magics.ipynb && jupyter nbconvert ./tmp_dir/4438040515678393921_set_up_magics.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/4438040515678393921_set_up_magics.ipynb --to markdown --output 4438040515678393921_set_up_magics.ipynb && cp ./tmp_dir/4438040515678393921_set_up_magics.ipynb.md ../tools/set_up_magics_no_output.md
    jupyter nbconvert ../tools/save_them_all.ipynb --to markdown --output save_them_all
    cp ../tools/save_them_all.ipynb ./tmp_dir/2745714102162224881_save_them_all.ipynb && jupyter nbconvert ./tmp_dir/2745714102162224881_save_them_all.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/2745714102162224881_save_them_all.ipynb --to markdown --output 2745714102162224881_save_them_all.ipynb && cp ./tmp_dir/2745714102162224881_save_them_all.ipynb.md ../tools/save_them_all_no_output.md
    jupyter nbconvert ../tools/stand.ipynb --to markdown --output stand
    cp ../tools/stand.ipynb ./tmp_dir/6897852096884475976_stand.ipynb && jupyter nbconvert ./tmp_dir/6897852096884475976_stand.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/6897852096884475976_stand.ipynb --to markdown --output 6897852096884475976_stand.ipynb && cp ./tmp_dir/6897852096884475976_stand.ipynb.md ../tools/stand_no_output.md
    jupyter nbconvert ../tools/set_up_magics_dev.ipynb --to markdown --output set_up_magics_dev
    cp ../tools/set_up_magics_dev.ipynb ./tmp_dir/6562605876583730359_set_up_magics_dev.ipynb && jupyter nbconvert ./tmp_dir/6562605876583730359_set_up_magics_dev.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/6562605876583730359_set_up_magics_dev.ipynb --to markdown --output 6562605876583730359_set_up_magics_dev.ipynb && cp ./tmp_dir/6562605876583730359_set_up_magics_dev.ipynb.md ../tools/set_up_magics_dev_no_output.md
    jupyter nbconvert ../sem09-x86-asm-nostdlib/nostdlib.ipynb --to markdown --output README
    cp ../sem09-x86-asm-nostdlib/nostdlib.ipynb ./tmp_dir/4809816577270222998_nostdlib.ipynb && jupyter nbconvert ./tmp_dir/4809816577270222998_nostdlib.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/4809816577270222998_nostdlib.ipynb --to markdown --output 4809816577270222998_nostdlib.ipynb && cp ./tmp_dir/4809816577270222998_nostdlib.ipynb.md ../sem09-x86-asm-nostdlib/README_no_output.md
     [NbConvertApp] Converting notebook ../tools/set_up_magics.ipynb to markdown
    [NbConvertApp] Writing 43889 bytes to ../tools/set_up_magics.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/4438040515678393921_set_up_magics.ipynb to notebook
    [NbConvertApp] Writing 23357 bytes to ./tmp_dir/4438040515678393921_set_up_magics.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/4438040515678393921_set_up_magics.ipynb to markdown
    [NbConvertApp] Writing 16390 bytes to ./tmp_dir/4438040515678393921_set_up_magics.ipynb.md
    
     [NbConvertApp] Converting notebook ../tools/save_them_all.ipynb to markdown
    [NbConvertApp] Writing 18673 bytes to ../tools/save_them_all.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/2745714102162224881_save_them_all.ipynb to notebook
    [NbConvertApp] Writing 10903 bytes to ./tmp_dir/2745714102162224881_save_them_all.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/2745714102162224881_save_them_all.ipynb to markdown
    [NbConvertApp] Writing 6850 bytes to ./tmp_dir/2745714102162224881_save_them_all.ipynb.md
    
     [NbConvertApp] Converting notebook ../tools/stand.ipynb to markdown
    [NbConvertApp] Writing 299 bytes to ../tools/stand.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/6897852096884475976_stand.ipynb to notebook
    [NbConvertApp] Writing 1067 bytes to ./tmp_dir/6897852096884475976_stand.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/6897852096884475976_stand.ipynb to markdown
    [NbConvertApp] Writing 299 bytes to ./tmp_dir/6897852096884475976_stand.ipynb.md
    
     [NbConvertApp] Converting notebook ../tools/set_up_magics_dev.ipynb to markdown
    [NbConvertApp] Writing 32 bytes to ../tools/set_up_magics_dev.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/6562605876583730359_set_up_magics_dev.ipynb to notebook
    [NbConvertApp] Writing 669 bytes to ./tmp_dir/6562605876583730359_set_up_magics_dev.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/6562605876583730359_set_up_magics_dev.ipynb to markdown
    [NbConvertApp] Writing 32 bytes to ./tmp_dir/6562605876583730359_set_up_magics_dev.ipynb.md
    
     [NbConvertApp] Converting notebook ../sem09-x86-asm-nostdlib/nostdlib.ipynb to markdown
    [NbConvertApp] Writing 42542 bytes to ../sem09-x86-asm-nostdlib/README.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/4809816577270222998_nostdlib.ipynb to notebook
    [NbConvertApp] Writing 33010 bytes to ./tmp_dir/4809816577270222998_nostdlib.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/4809816577270222998_nostdlib.ipynb to markdown
    [NbConvertApp] Writing 25162 bytes to ./tmp_dir/4809816577270222998_nostdlib.ipynb.md
    


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
    
     dos2unix: converting file ./../sem09-x86-asm-nostdlib/nostdlib.ipynb to Unix format...
    
     dos2unix: converting file ./../tools/save_them_all.md to Unix format...
    
     dos2unix: converting file ./../tools/save_them_all_no_output.md to Unix format...
    
     dos2unix: converting file ./../tools/set_up_magics_dev_no_output.md to Unix format...
    
     dos2unix: converting file ./../tools/stand_no_output.md to Unix format...
    
     dos2unix: converting file ./../tools/README.md to Unix format...
    
     dos2unix: converting file ./../tools/set_up_magics_dev.md to Unix format...
    
     dos2unix: converting file ./../tools/stand.md to Unix format...
    
     dos2unix: converting file ./../tools/set_up_magics_no_output.md to Unix format...
    
     dos2unix: converting file ./../tools/set_up_magics.md to Unix format...
    
     dos2unix: converting file ./../sem09-x86-asm-nostdlib/README.md to Unix format...
    
     dos2unix: converting file ./../sem09-x86-asm-nostdlib/README_no_output.md to Unix format...
    


### Смотрим изменения


```python
for subdir in highlevel_dirs:
    get_ipython().system("git diff {}/*_no_output*".format(subdir))
```

    [1mdiff --git a/tools/save_them_all_no_output.md b/tools/save_them_all_no_output.md[m
    [1mindex d09949c..4c081eb 100644[m
    [1m--- a/tools/save_them_all_no_output.md[m
    [1m+++ b/tools/save_them_all_no_output.md[m
    [36m@@ -19,7 +19,8 @@[m [mimport subprocess[m
     [m
     highlevel_dirs = sum([[m
         #["../tools"], [m
    [31m-    sorted(glob.glob("../sem03*")),[m
    [32m+[m[32m    sorted(glob.glob("../sem08*")),[m
    [32m+[m[32m    #sorted(glob.glob("../sem07*")),[m
         #sorted(glob.glob("../extra*")),[m
     ], [])[m
     [m
    [36m@@ -157,7 +158,6 @@[m [mdef improve_file(fname):[m
     tasks = [][m
     shell_tasks = [][m
     [m
    [31m-[m
     for sfx in [".ipynb", ".md"]:[m
         for hdir in highlevel_dirs:[m
             for fname in glob.glob("./{}/*".format(hdir) + sfx):[m
    [1mdiff --git a/tools/set_up_magics_no_output.md b/tools/set_up_magics_no_output.md[m
    [1mindex f8e596b..2fa0536 100644[m
    [1m--- a/tools/set_up_magics_no_output.md[m
    [1m+++ b/tools/set_up_magics_no_output.md[m
    [36m@@ -22,7 +22,8 @@[m [mget_ipython().run_cell_magic('javascript', '',[m
     from IPython.core.magic import register_cell_magic, register_line_magic[m
     from IPython.display import display, Markdown, HTML[m
     import argparse[m
    [31m-from subprocess import Popen, PIPE[m
    [32m+[m[32mfrom subprocess import Popen, PIPE, STDOUT, check_output[m
    [32m+[m[32mimport html[m
     import random[m
     import sys[m
     import os[m
    [36m@@ -38,6 +39,7 @@[m [mdef save_file(args_str, cell, line_comment_start="#"):[m
         parser = argparse.ArgumentParser()[m
         parser.add_argument("fname")[m
         parser.add_argument("--ejudge-style", action="store_true")[m
    [32m+[m[32m    parser.add_argument("--under-spoiler-threshold", type=int, default=None)[m
         args = parser.parse_args(args_str.split())[m
         [m
         cell = cell if cell[-1] == '\n' or args.no_eof_newline else cell + "\n"[m
    [36m@@ -52,7 +54,9 @@[m [mdef save_file(args_str, cell, line_comment_start="#"):[m
                         cmds.append(line[len(run_prefix):].strip())[m
                         f.write(line_comment_start + " " + line_to_write)[m
                         continue[m
    [31m-                if line.startswith("%" + line_comment_start + " "):[m
    [32m+[m[32m                comment_prefix = "%" + line_comment_start[m
    [32m+[m[32m                if line.startswith(comment_prefix):[m
    [32m+[m[32m                    cmds.append('#' + line[len(comment_prefix):].strip())[m
                         f.write(line_comment_start + " " + line_to_write)[m
                         continue[m
                     raise Exception("Unknown %%save_file subcommand: '%s'" % line)[m
    [36m@@ -60,8 +64,22 @@[m [mdef save_file(args_str, cell, line_comment_start="#"):[m
                     f.write(line_to_write)[m
             f.write("" if not args.ejudge_style else line_comment_start + r" line without \n")[m
         for cmd in cmds:[m
    [31m-        display(Markdown("Run: `%s`" % cmd))[m
    [31m-        get_ipython().system(cmd)[m
    [32m+[m[32m        if cmd.startswith('#'):[m
    [32m+[m[32m            display(Markdown("\#\#\#\# `%s`" % cmd[1:]))[m
    [32m+[m[32m        else:[m
    [32m+[m[32m            display(Markdown("Run: `%s`" % cmd))[m
    [32m+[m[32m            if args.under_spoiler_threshold:[m
    [32m+[m[32m                out = check_output(cmd, stderr=STDOUT, shell=True, universal_newlines=True)[m
    [32m+[m[32m                out = out[:-1] if out.endswith('\n') else out[m
    [32m+[m[32m                out = html.escape(out)[m
    [32m+[m[32m                if len(out.split('\n')) > args.under_spoiler_threshold:[m
    [32m+[m[32m                    out = "<details> <summary> output </summary> <pre><code>%s</code></pre></details>" % out[m
    [32m+[m[32m                elif out:[m
    [32m+[m[32m                    out = "<pre><code>%s</code></pre>" % out[m
    [32m+[m[32m                if out:[m
    [32m+[m[32m                    display(HTML(out))[m
    [32m+[m[32m            else:[m
    [32m+[m[32m                get_ipython().system(cmd)[m
     [m
     @register_cell_magic[m
     def cpp(fname, cell):[m
    [36m@@ -353,6 +371,11 @@[m [ma.close()[m
     ```[m
     [m
     [m
    [32m+[m[32m```python[m
    [32m+[m[32mhelp(get_ipython().system)[m
    [32m+[m[32m```[m
    [32m+[m
    [32m+[m
     ```python[m
     a = TInteractiveLauncher("echo 1 ; echo 2 1>&2 ; read XX ; echo \"A${XX}B\" ")[m
     os.kill(a.get_pid(), 9)[m


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
    > git add --ignore-errors  ../sem09-x86-asm-nostdlib/*.ipynb
    > git add --ignore-errors  ../sem09-x86-asm-nostdlib/*.md
    > git add --ignore-errors  ../sem09-x86-asm-nostdlib/*.c
    > git add --ignore-errors  ../sem09-x86-asm-nostdlib/*.cpp
    fatal: pathspec '../sem09-x86-asm-nostdlib/*.cpp' did not match any files
    > git add --ignore-errors -f  -f ../sem09-x86-asm-nostdlib/bash_popen_tmp/*.html
    warning: could not open directory 'sem09-x86-asm-nostdlib/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem09-x86-asm-nostdlib/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem09-x86-asm-nostdlib/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem09-x86-asm-nostdlib/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem09-x86-asm-nostdlib/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem09-x86-asm-nostdlib
    > git commit -m 'yet another update'
    [master eba985c] yet another update
     21 files changed, 4451 insertions(+), 257 deletions(-)
     create mode 100644 sem09-x86-asm-nostdlib/README.md
     create mode 100644 sem09-x86-asm-nostdlib/README_no_output.md
     create mode 100644 sem09-x86-asm-nostdlib/example1.c
     create mode 100644 sem09-x86-asm-nostdlib/example2.c
     create mode 100644 sem09-x86-asm-nostdlib/example3.c
     create mode 100644 sem09-x86-asm-nostdlib/exit.c
     create mode 100644 sem09-x86-asm-nostdlib/look_at_addresses.c
     create mode 100644 sem09-x86-asm-nostdlib/main.c
     create mode 100644 sem09-x86-asm-nostdlib/main2.c
     create mode 100644 sem09-x86-asm-nostdlib/minimal.c
     create mode 100644 sem09-x86-asm-nostdlib/nostdlib.ipynb
     rename tools/interactive_launcher_tmp/{621662941835523211.log => 161300355223537194.log} (82%)
     rename tools/interactive_launcher_tmp/{355906436253704745.log => 371757449732314597.log} (56%)
     rename tools/interactive_launcher_tmp/{325927153848681636.log => 519997700123128471.log} (89%)
     rename tools/interactive_launcher_tmp/{372627075579716753.log => 688225086340752142.log} (89%)
    > git push origin master
    Enumerating objects: 35, done.
    Counting objects: 100% (35/35), done.
    Delta compression using up to 4 threads
    Compressing objects: 100% (26/26), done.
    Writing objects: 100% (26/26), 29.31 KiB | 652.00 KiB/s, done.
    Total 26 (delta 17), reused 0 (delta 0)
    remote: Resolving deltas: 100% (17/17), completed with 8 local objects.[K
    To github.com:yuri-pechatnov/caos.git
       bb56e2a..eba985c  master -> master



```python

```


```python
!git add ../README.md
!git commit -m "Update readme"
!git push origin master
```


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
