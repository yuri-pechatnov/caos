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
    #sorted(glob.glob("../sem24*")),
    sorted(glob.glob("../sem28*")),
], [])

print("Highlevel dirs:", highlevel_dirs)
```

    Highlevel dirs: ['../tools', '../sem28-unix-time']



```python
tmp_dir = "./tmp_dir"
get_ipython().system('rm -r {tmp_dir} ; mkdir {tmp_dir} 2>&1 | grep -v "File exists"'.format(tmp_dir=tmp_dir))
```


```python
!rm ../sem28-unix-time/*_no_output* || true
!rm ../tools/*_no_output* || true
```

    rm: cannot remove '../sem28-unix-time/*_no_output*': No such file or directory
    rm: cannot remove '../tools/*_no_output*': No such file or directory



```python
os.path.dirname("../tools/set_up_magics_dev_no_output.ipynb")
```




    '../tools'




```python
!cp ../tools/save_them_all.ipynb ./tmp_dir/save_them_allAAA.ipynb
!jupyter nbconvert ./tmp_dir/save_them_allAAA.ipynb --ClearOutputPreprocessor.enabled=True --inplace 
!jupyter nbconvert ./tmp_dir/save_them_allAAA.ipynb --to markdown --output README_no_output
!cp ./tmp_dir/README_no_output.md ../tools
```

    [NbConvertApp] Converting notebook ./tmp_dir/save_them_allAAA.ipynb to notebook
    [NbConvertApp] Writing 10267 bytes to ./tmp_dir/save_them_allAAA.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/save_them_allAAA.ipynb to markdown
    [NbConvertApp] Writing 6515 bytes to ./tmp_dir/README_no_output.md



```python

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
            "jupyter nbconvert {tmp_dir}/{src_copy} --to markdown --output {no_output_file}",
            "cp {tmp_dir}/{no_output_file}.md {path}",
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

    ../tools ['../tools/set_up_magics.ipynb', '../tools/save_them_all.ipynb', '../tools/stand.ipynb', '../tools/set_up_magics_dev.ipynb']
    ../sem28-unix-time ['../sem28-unix-time/time.ipynb']
    jupyter nbconvert ../tools/set_up_magics.ipynb --to markdown --output set_up_magics
    cp ../tools/set_up_magics.ipynb ./tmp_dir/1809760105274635650_set_up_magics.ipynb && jupyter nbconvert ./tmp_dir/1809760105274635650_set_up_magics.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/1809760105274635650_set_up_magics.ipynb --to markdown --output set_up_magics_no_output && cp ./tmp_dir/set_up_magics_no_output.md ../tools
    jupyter nbconvert ../tools/save_them_all.ipynb --to markdown --output save_them_all
    cp ../tools/save_them_all.ipynb ./tmp_dir/5752926541494098657_save_them_all.ipynb && jupyter nbconvert ./tmp_dir/5752926541494098657_save_them_all.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/5752926541494098657_save_them_all.ipynb --to markdown --output save_them_all_no_output && cp ./tmp_dir/save_them_all_no_output.md ../tools
    jupyter nbconvert ../tools/stand.ipynb --to markdown --output stand
    cp ../tools/stand.ipynb ./tmp_dir/7120860308082601853_stand.ipynb && jupyter nbconvert ./tmp_dir/7120860308082601853_stand.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/7120860308082601853_stand.ipynb --to markdown --output stand_no_output && cp ./tmp_dir/stand_no_output.md ../tools
    jupyter nbconvert ../tools/set_up_magics_dev.ipynb --to markdown --output set_up_magics_dev
    cp ../tools/set_up_magics_dev.ipynb ./tmp_dir/2653528037997578719_set_up_magics_dev.ipynb && jupyter nbconvert ./tmp_dir/2653528037997578719_set_up_magics_dev.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/2653528037997578719_set_up_magics_dev.ipynb --to markdown --output set_up_magics_dev_no_output && cp ./tmp_dir/set_up_magics_dev_no_output.md ../tools
    jupyter nbconvert ../sem28-unix-time/time.ipynb --to markdown --output README
    cp ../sem28-unix-time/time.ipynb ./tmp_dir/3597715960520351318_time.ipynb && jupyter nbconvert ./tmp_dir/3597715960520351318_time.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/3597715960520351318_time.ipynb --to markdown --output README_no_output && cp ./tmp_dir/README_no_output.md ../sem28-unix-time
     [NbConvertApp] Converting notebook ../tools/set_up_magics.ipynb to markdown
    [NbConvertApp] Writing 41310 bytes to ../tools/set_up_magics.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/1809760105274635650_set_up_magics.ipynb to notebook
    [NbConvertApp] Writing 21706 bytes to ./tmp_dir/1809760105274635650_set_up_magics.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/1809760105274635650_set_up_magics.ipynb to markdown
    [NbConvertApp] Writing 15323 bytes to ./tmp_dir/set_up_magics_no_output.md
    
     [NbConvertApp] Converting notebook ../tools/save_them_all.ipynb to markdown
    [NbConvertApp] Writing 15870 bytes to ../tools/save_them_all.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/5752926541494098657_save_them_all.ipynb to notebook
    [NbConvertApp] Writing 10573 bytes to ./tmp_dir/5752926541494098657_save_them_all.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/5752926541494098657_save_them_all.ipynb to markdown
    [NbConvertApp] Writing 6778 bytes to ./tmp_dir/save_them_all_no_output.md
    
     [NbConvertApp] Converting notebook ../tools/stand.ipynb to markdown
    [NbConvertApp] Writing 306 bytes to ../tools/stand.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/7120860308082601853_stand.ipynb to notebook
    [NbConvertApp] Writing 1074 bytes to ./tmp_dir/7120860308082601853_stand.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/7120860308082601853_stand.ipynb to markdown
    [NbConvertApp] Writing 306 bytes to ./tmp_dir/stand_no_output.md
    
     [NbConvertApp] Converting notebook ../tools/set_up_magics_dev.ipynb to markdown
    [NbConvertApp] Writing 32 bytes to ../tools/set_up_magics_dev.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/2653528037997578719_set_up_magics_dev.ipynb to notebook
    [NbConvertApp] Writing 669 bytes to ./tmp_dir/2653528037997578719_set_up_magics_dev.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/2653528037997578719_set_up_magics_dev.ipynb to markdown
    [NbConvertApp] Writing 32 bytes to ./tmp_dir/set_up_magics_dev_no_output.md
    
     [NbConvertApp] Converting notebook ../sem28-unix-time/time.ipynb to markdown
    [NbConvertApp] Writing 32626 bytes to ../sem28-unix-time/README.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/3597715960520351318_time.ipynb to notebook
    [NbConvertApp] Writing 35140 bytes to ./tmp_dir/3597715960520351318_time.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/3597715960520351318_time.ipynb to markdown
    [NbConvertApp] Writing 28617 bytes to ./tmp_dir/README_no_output.md
    



```python
!ls
```

    a.cpp			    set_up_magics_dev.md
    a.py			    set_up_magics_dev_no_output.md
    bash_popen_tmp		    set_up_magics.ipynb
    interactive_launcher_tmp    set_up_magics.md
    launcher.py		    set_up_magics_no_output.md
    README.md		    stand.ipynb
    save_them_all.ipynb	    stand.md
    save_them_all.md	    stand_no_output.md
    save_them_all_no_output.md  tmp_dir
    set_up_magics_dev.ipynb



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

     dos2unix: converting file ./../tools/set_up_magics.ipynb to Unix format...
    
     dos2unix: converting file ./../tools/save_them_all.ipynb to Unix format...
    
     dos2unix: converting file ./../tools/stand.ipynb to Unix format...
    
     dos2unix: converting file ./../tools/set_up_magics_dev.ipynb to Unix format...
    
     dos2unix: converting file ./../sem28-unix-time/time.ipynb to Unix format...
    
     dos2unix: converting file ./../tools/save_them_all.md to Unix format...
    
     dos2unix: converting file ./../tools/save_them_all_no_output.md to Unix format...
    
     dos2unix: converting file ./../tools/set_up_magics_dev_no_output.md to Unix format...
    
     dos2unix: converting file ./../tools/stand_no_output.md to Unix format...
    
     dos2unix: converting file ./../tools/README.md to Unix format...
    
     dos2unix: converting file ./../tools/set_up_magics_dev.md to Unix format...
    
     dos2unix: converting file ./../tools/stand.md to Unix format...
    
     dos2unix: converting file ./../tools/set_up_magics_no_output.md to Unix format...
    
     dos2unix: converting file ./../tools/set_up_magics.md to Unix format...
    
     dos2unix: converting file ./../sem28-unix-time/README.md to Unix format...
    
     dos2unix: converting file ./../sem28-unix-time/README_no_output.md to Unix format...
    


### <a name="github"></a> Коммитим на github


```python

```


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
    [master 1e8f361] yet another update
     5 files changed, 271 insertions(+), 387 deletions(-)
    > git push origin master
    Enumerating objects: 17, done.
    Counting objects: 100% (16/16), done.
    Delta compression using up to 4 threads
    Compressing objects: 100% (9/9), done.
    Writing objects: 100% (9/9), 6.92 KiB | 1.15 MiB/s, done.
    Total 9 (delta 7), reused 0 (delta 0)
    remote: Resolving deltas: 100% (7/7), completed with 6 local objects.[K
    To github.com:yuri-pechatnov/caos_2019-2020.git
       eb1f677..1e8f361  master -> master



```python

```


```python

```


```python

```
