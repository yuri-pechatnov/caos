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
    sorted(glob.glob("../sem03*")),
    #sorted(glob.glob("../extra*")),
], [])

print("Highlevel dirs:", highlevel_dirs)
```

    Highlevel dirs: ['../sem03-cmake-python-bindings']



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

    ../sem03-cmake-python-bindings ['../sem03-cmake-python-bindings/bindings.ipynb']
    jupyter nbconvert ../sem03-cmake-python-bindings/bindings.ipynb --to markdown --output README
    cp ../sem03-cmake-python-bindings/bindings.ipynb ./tmp_dir/2378846515383418137_bindings.ipynb && jupyter nbconvert ./tmp_dir/2378846515383418137_bindings.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/2378846515383418137_bindings.ipynb --to markdown --output 2378846515383418137_bindings.ipynb && cp ./tmp_dir/2378846515383418137_bindings.ipynb.md ../sem03-cmake-python-bindings/README_no_output.md
     [NbConvertApp] Converting notebook ../sem03-cmake-python-bindings/bindings.ipynb to markdown
    [NbConvertApp] Writing 45540 bytes to ../sem03-cmake-python-bindings/README.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/2378846515383418137_bindings.ipynb to notebook
    [NbConvertApp] Writing 49100 bytes to ./tmp_dir/2378846515383418137_bindings.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/2378846515383418137_bindings.ipynb to markdown
    [NbConvertApp] Writing 34965 bytes to ./tmp_dir/2378846515383418137_bindings.ipynb.md
    


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

     dos2unix: converting file ./../sem03-cmake-python-bindings/bindings.ipynb to Unix format...
    
     dos2unix: converting file ./../sem03-cmake-python-bindings/README.md to Unix format...
    
     dos2unix: converting file ./../sem03-cmake-python-bindings/README_no_output.md to Unix format...
    


### Смотрим изменения


```python
for subdir in highlevel_dirs:
    get_ipython().system("git diff {}/*_no_output*".format(subdir))
```

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

    > git add --ignore-errors  ../sem03-cmake-python-bindings/*.ipynb
    > git add --ignore-errors  ../sem03-cmake-python-bindings/*.md
    > git add --ignore-errors  ../sem03-cmake-python-bindings/*.c
    > git add --ignore-errors  ../sem03-cmake-python-bindings/*.cpp
    > git add --ignore-errors -f  -f ../sem03-cmake-python-bindings/bash_popen_tmp/*.html
    warning: could not open directory 'sem03-cmake-python-bindings/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem03-cmake-python-bindings/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem03-cmake-python-bindings/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem03-cmake-python-bindings/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem03-cmake-python-bindings/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem03-cmake-python-bindings
    > git commit -m 'yet another update'
    [master 5251720] yet another update
     9 files changed, 9113 insertions(+)
     create mode 100644 sem03-cmake-python-bindings/README.md
     create mode 100644 sem03-cmake-python-bindings/README_no_output.md
     create mode 100644 sem03-cmake-python-bindings/bindings.ipynb
     create mode 100644 sem03-cmake-python-bindings/c_api_module.c
     create mode 100644 sem03-cmake-python-bindings/c_api_module_2.c
     create mode 100644 sem03-cmake-python-bindings/ctypes_lib.c
     create mode 100644 sem03-cmake-python-bindings/pairs.cpp
     create mode 100644 sem03-cmake-python-bindings/pairs_pybind.cpp
     create mode 100644 sem03-cmake-python-bindings/use_interpreter.c
    > git push origin master
    Enumerating objects: 9, done.
    Counting objects: 100% (9/9), done.
    Delta compression using up to 4 threads
    Compressing objects: 100% (8/8), done.
    Writing objects: 100% (8/8), 23.59 KiB | 561.00 KiB/s, done.
    Total 8 (delta 5), reused 0 (delta 0)
    remote: Resolving deltas: 100% (5/5), completed with 1 local object.[K
    To github.com:yuri-pechatnov/caos.git
       66637a8..5251720  master -> master



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
      (use "git add <file>..." to update what will be committed)
      (use "git restore <file>..." to discard changes in working directory)
    	[31mmodified:   ../sem01-intro-linux/interactive_launcher_tmp/104342371564797143.log[m
    	[31mmodified:   save_them_all.ipynb[m
    
    Untracked files:
      (use "git add <file>..." to include in what will be committed)
    	[31m../extra-c-basics/001.in[m
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
    	[31ma.py[m
    	[31minteractive_launcher_tmp/110047129715984560.log.md[m
    	[31minteractive_launcher_tmp/243546179874885578.log.md[m
    	[31minteractive_launcher_tmp/910964812581195986.log.md[m
    	[31minteractive_launcher_tmp/933917889050860419.log.md[m
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
