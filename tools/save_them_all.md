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
    sorted(glob.glob("../sem08*")),
    #sorted(glob.glob("../sem07*")),
    #sorted(glob.glob("../extra*")),
], [])

print("Highlevel dirs:", highlevel_dirs)
```

    Highlevel dirs: ['../sem08-x86-fpmath-sse']



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

    ../sem08-x86-fpmath-sse ['../sem08-x86-fpmath-sse/fpmath_sse.ipynb']
    jupyter nbconvert ../sem08-x86-fpmath-sse/fpmath_sse.ipynb --to markdown --output README
    cp ../sem08-x86-fpmath-sse/fpmath_sse.ipynb ./tmp_dir/2826881759286590356_fpmath_sse.ipynb && jupyter nbconvert ./tmp_dir/2826881759286590356_fpmath_sse.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/2826881759286590356_fpmath_sse.ipynb --to markdown --output 2826881759286590356_fpmath_sse.ipynb && cp ./tmp_dir/2826881759286590356_fpmath_sse.ipynb.md ../sem08-x86-fpmath-sse/README_no_output.md
     [NbConvertApp] Converting notebook ../sem08-x86-fpmath-sse/fpmath_sse.ipynb to markdown
    [NbConvertApp] Writing 28429 bytes to ../sem08-x86-fpmath-sse/README.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/2826881759286590356_fpmath_sse.ipynb to notebook
    [NbConvertApp] Writing 27347 bytes to ./tmp_dir/2826881759286590356_fpmath_sse.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/2826881759286590356_fpmath_sse.ipynb to markdown
    [NbConvertApp] Writing 20966 bytes to ./tmp_dir/2826881759286590356_fpmath_sse.ipynb.md
    


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

     dos2unix: converting file ./../sem08-x86-fpmath-sse/fpmath_sse.ipynb to Unix format...
    
     dos2unix: converting file ./../sem08-x86-fpmath-sse/README.md to Unix format...
    
     dos2unix: converting file ./../sem08-x86-fpmath-sse/README_no_output.md to Unix format...
    


### Смотрим изменения


```python
for subdir in highlevel_dirs:
    get_ipython().system("git diff {}/*_no_output*".format(subdir))
```

    [1mdiff --git a/sem08-x86-fpmath-sse/README_no_output.md b/sem08-x86-fpmath-sse/README_no_output.md[m
    [1mindex d8d923d..6039c03 100644[m
    [1m--- a/sem08-x86-fpmath-sse/README_no_output.md[m
    [1m+++ b/sem08-x86-fpmath-sse/README_no_output.md[m
    [36m@@ -2,16 +2,10 @@[m
     [m
     # Вещественная арифметика на x86 и SSE[m
     [m
    [31m-<table width=100%> <tr>[m
    [31m-    <th width=20%> <b>Видеозапись семинара &rarr; </b> </th>[m
    [31m-    <th>[m
    [31m-    <a href="https://www.youtube.com/watch?v=i_eeouEiXnI&list=PLjzMm8llUm4AmU6i_hPU0NobgA4VsBowc&index=8">[m
    [31m-        <img src="video.jpg" width="320"  height="160" align="left" alt="Видео с семинара"> [m
    [31m-    </a>[m
    [31m-    </th>[m
    [31m-    <th> </th>[m
    [31m- </table>[m
     [m
    [32m+[m[32m<p><a href="https://www.youtube.com/watch?v=obufMgdWPKI&list=PLjzMm8llUm4AmU6i_hPU0NobgA4VsBowc&index=9" target="_blank">[m
    [32m+[m[32m    <h3>Видеозапись семинара</h3>[m
    [32m+[m[32m</a></p>[m
     [m
     [Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/asm/x86_fpmath) [m
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

    > git add --ignore-errors  ../sem08-x86-fpmath-sse/*.ipynb
    > git add --ignore-errors  ../sem08-x86-fpmath-sse/*.md
    > git add --ignore-errors  ../sem08-x86-fpmath-sse/*.c
    > git add --ignore-errors  ../sem08-x86-fpmath-sse/*.cpp
    fatal: pathspec '../sem08-x86-fpmath-sse/*.cpp' did not match any files
    > git add --ignore-errors -f  -f ../sem08-x86-fpmath-sse/bash_popen_tmp/*.html
    warning: could not open directory 'sem08-x86-fpmath-sse/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem08-x86-fpmath-sse/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem08-x86-fpmath-sse/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem08-x86-fpmath-sse/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem08-x86-fpmath-sse/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem08-x86-fpmath-sse
    > git commit -m 'yet another update'
    [master bb56e2a] yet another update
     3 files changed, 9 insertions(+), 27 deletions(-)
    > git push origin master
    Enumerating objects: 11, done.
    Counting objects: 100% (11/11), done.
    Delta compression using up to 4 threads
    Compressing objects: 100% (6/6), done.
    Writing objects: 100% (6/6), 746 bytes | 746.00 KiB/s, done.
    Total 6 (delta 5), reused 0 (delta 0)
    remote: Resolving deltas: 100% (5/5), completed with 5 local objects.[K
    To github.com:yuri-pechatnov/caos.git
       a7ca2f8..bb56e2a  master -> master



```python

```


```python
!git add ../README.md
!git commit -m "Update readme"
!git push origin master
```

    On branch master
    Your branch is up to date with 'origin/master'.
    
    Changes not staged for commit:
      (use "git add/rm <file>..." to update what will be committed)
      (use "git restore <file>..." to discard changes in working directory)
    	[31mmodified:   ../caos_2019-2020/sem04-asm-arm/arm.ipynb[m
    	[31mmodified:   ../caos_2019-2020/sem05-asm-arm-addressing/adressing.ipynb[m
    	[31mmodified:   ../sem01-intro-linux/intro_linux.ipynb[m
    	[31mmodified:   ../sem02-instruments-compilation/instruments_compilation.ipynb[m
    	[31mmodified:   ../sem02-instruments-compilation/macro_example_0.c[m
    	[31mmodified:   ../sem05-arm-asm/arm_asm.ipynb[m
    	[31mmodified:   ../sem05-arm-asm/lib.c[m
    	[31mmodified:   ../sem07-x86-asm/asm_x86-64.ipynb[m
    	[31mmodified:   ../sem07-x86-asm/intel_example.c[m
    	[31mdeleted:    interactive_launcher_tmp/325927153848681636.log[m
    	[31mdeleted:    interactive_launcher_tmp/355906436253704745.log[m
    	[31mdeleted:    interactive_launcher_tmp/372627075579716753.log[m
    	[31mdeleted:    interactive_launcher_tmp/621662941835523211.log[m
    	[31mmodified:   save_them_all.ipynb[m
    	[31mmodified:   set_up_magics.ipynb[m
    
    Untracked files:
      (use "git add <file>..." to include in what will be committed)
    	[31m../extra-c-basics/001.expected[m
    	[31m../extra-c-basics/001.in[m
    	[31m../extra-c-basics/001.out[m
    	[31m../extra-c-basics/out[m
    	[31m../extra-c-basics/test.h[m
    	[31m../extra-c-basics/test.sh[m
    	[31m../sem01-intro-linux/2[m
    	[31m../sem01-intro-linux/a.sh[m
    	[31m../sem01-intro-linux/a.txt[m
    	[31m../sem01-intro-linux/b.txt[m
    	[31m../sem01-intro-linux/err[m
    	[31m../sem01-intro-linux/err.txt[m
    	[31m../sem01-intro-linux/file.txt[m
    	[31m../sem01-intro-linux/file2.txt[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/104342371564797143.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/134856167677051543.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/159458045408584490.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/160806476576838908.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/237887356681257319.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/272097196292346552.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/291478633046804378.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/299779856288802977.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/308063480967833729.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/33203167529660997.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/357027856339287279.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/357412744100799582.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/363736236628022803.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/368536380590515236.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/38653968711948138.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/390229348646733600.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/44638202710957050.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/488615332501153522.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/547003026116871195.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/551941962051262166.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/577835561503179454.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/580865104873188457.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/58318340108386158.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/60572178042054264.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/659941140088179386.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/718444346696566167.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/731534507899320722.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/761344427069245362.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/777467580737092300.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/799265701962172890.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/887167192337308461.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/906648630641962511.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/928497432930519081.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/93290423175998573.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/965132557118189549.log.md[m
    	[31m../sem01-intro-linux/out[m
    	[31m../sem01-intro-linux/out.txt[m
    	[31m../sem02-instruments-compilation/a.out[m
    	[31m../sem02-instruments-compilation/a.txt[m
    	[31m../sem02-instruments-compilation/g.h[m
    	[31m../sem02-instruments-compilation/lib.[m
    	[31m../sem02-instruments-compilation/lib.a[m
    	[31m../sem02-instruments-compilation/out[m
    	[31m../sem02-instruments-compilation/preprocessing_max.h[m
    	[31m../sem03-cmake-python-bindings/._poll.png[m
    	[31m../sem03-cmake-python-bindings/api_module_example.py[m
    	[31m../sem03-cmake-python-bindings/c_api_module_2_example.py[m
    	[31m../sem03-cmake-python-bindings/c_api_own_type_module_example.py[m
    	[31m../sem03-cmake-python-bindings/count_1e8_cython.py[m
    	[31m../sem03-cmake-python-bindings/count_1e8_native.py[m
    	[31m../sem03-cmake-python-bindings/ctypes_example.py[m
    	[31m../sem03-cmake-python-bindings/cython_setup.py[m
    	[31m../sem03-cmake-python-bindings/make_example/[m
    	[31m../sem03-cmake-python-bindings/pairs.h[m
    	[31m../sem03-cmake-python-bindings/pairs.pxd[m
    	[31m../sem03-cmake-python-bindings/pairs.pyx[m
    	[31m../sem03-cmake-python-bindings/pybind_setup.py[m
    	[31m../sem03-cmake-python-bindings/python_cmake_example/[m
    	[31m../sem03-cmake-python-bindings/simple_cmake_example/[m
    	[31m../sem03-cmake-python-bindings/test_pairs.py[m
    	[31m../sem03-cmake-python-bindings/test_pybind_pairs.py[m
    	[31m../sem04-int-float/code_sample[m
    	[31m../sem04-int-float/run_ub.py[m
    	[31m../sem04-int-float/stand.h[m
    	[31m../sem04-int-float/text[m
    	[31m../sem05-arm-asm/code_sample[m
    	[31m../sem05-arm-asm/run_ub.py[m
    	[31m../sem05-arm-asm/stand.h[m
    	[31m../sem06-arm-asm-addressing/structs_in_memory_common.h[m
    	[31m../sem07-x86-asm/asm_filter_useless[m
    	[31m../sem07-x86-asm/example.c[m
    	[31m../sem07-x86-asm/print.c[m
    	[31m../sem08-x86-fpmath-sse/asm_filter_useless[m
    	[31ma.py[m
    	[31minteractive_launcher_tmp/214843656435050573.log[m
    	[31minteractive_launcher_tmp/214843656435050573.log.md[m
    	[31minteractive_launcher_tmp/483965652334385005.log[m
    	[31minteractive_launcher_tmp/483965652334385005.log.md[m
    	[31minteractive_launcher_tmp/564738347608472818.log[m
    	[31minteractive_launcher_tmp/564738347608472818.log.md[m
    	[31minteractive_launcher_tmp/832756156005154650.log[m
    	[31minteractive_launcher_tmp/832756156005154650.log.md[m
    	[31mlauncher.py[m
    	[31mtmp_dir/[m
    
    no changes added to commit (use "git add" and/or "git commit -a")
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
