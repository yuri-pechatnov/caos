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

    > git commit -m 'yet another update'
    On branch master
    Your branch is up to date with 'origin/master'.
    
    Changes not staged for commit:
      (use "git add <file>..." to update what will be committed)
      (use "git restore <file>..." to discard changes in working directory)
    	[31mmodified:   ../README.md[m
    
    no changes added to commit (use "git add" and/or "git commit -a")
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
