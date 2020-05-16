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

```


```python

def convert(n, d):
    get_ipython().system("jupyter nbconvert {} --to markdown --output {}".format(n, d))

for subdir in highlevel_dirs:
    notebooks = glob.glob(subdir + "/*.ipynb")
    print(subdir, notebooks)
    for m in glob.glob(subdir + "/*.md"):
        os.remove(m)
    if len(notebooks) == 1:
        convert(notebooks[0], "README")
    else:
        for n in notebooks:
            convert(n, os.path.basename(n.replace(".ipynb", "")))
        nmds = [os.path.basename(n).replace(".ipynb", ".md") for n in notebooks]
        with open(os.path.join(subdir, "README.md"), "w") as f:
            f.write('\n'.join(
                ['# Ноутбуки семинара'] + 
                ['* [{nmd}]({nmd})'.format(nmd=nmd) for nmd in nmds] + 
                ['']
            ))
        
```

    ../tools ['../tools/set_up_magics.ipynb', '../tools/save_them_all.ipynb', '../tools/stand.ipynb', '../tools/set_up_magics_dev.ipynb']
    [NbConvertApp] Converting notebook ../tools/set_up_magics.ipynb to markdown
    [NbConvertApp] Writing 41266 bytes to ../tools/set_up_magics.md
    [NbConvertApp] Converting notebook ../tools/save_them_all.ipynb to markdown
    [NbConvertApp] Writing 9473 bytes to ../tools/save_them_all.md
    [NbConvertApp] Converting notebook ../tools/stand.ipynb to markdown
    [NbConvertApp] Writing 308 bytes to ../tools/stand.md
    [NbConvertApp] Converting notebook ../tools/set_up_magics_dev.ipynb to markdown
    [NbConvertApp] Writing 32 bytes to ../tools/set_up_magics_dev.md
    ../sem28-unix-time ['../sem28-unix-time/time.ipynb']
    [NbConvertApp] Converting notebook ../sem28-unix-time/time.ipynb to markdown
    [NbConvertApp] Writing 32539 bytes to ../sem28-unix-time/README.md



```python

```


```python
import re

def basic_improve(fname):
    with open(fname, "r") as f:
        r = f.read()
    for b in ["\x00", "\x1B", "\x08"]:
        r = r.replace(b, "")
    with open(fname, "w") as f:
        f.write(r)
    get_ipython().system("dos2unix {}".format(fname))

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
    basic_improve(fname)
    if fname.endswith(".md"):
        improve_md(fname)

```


```python
for sfx in [".ipynb", ".md"]:
    for hdir in highlevel_dirs:
        for fname in glob.glob("./{}/*".format(hdir) + sfx):
            improve_file(fname)
```

    dos2unix: converting file ./../tools/set_up_magics.ipynb to Unix format...
    dos2unix: converting file ./../tools/save_them_all.ipynb to Unix format...
    dos2unix: converting file ./../tools/stand.ipynb to Unix format...
    dos2unix: converting file ./../tools/set_up_magics_dev.ipynb to Unix format...
    dos2unix: converting file ./../sem28-unix-time/time.ipynb to Unix format...
    dos2unix: converting file ./../tools/save_them_all.md to Unix format...
    dos2unix: converting file ./../tools/README.md to Unix format...
    dos2unix: converting file ./../tools/set_up_magics_dev.md to Unix format...
    dos2unix: converting file ./../tools/stand.md to Unix format...
    dos2unix: converting file ./../tools/set_up_magics.md to Unix format...



    ---------------------------------------------------------------------------

    FileNotFoundError                         Traceback (most recent call last)

    <ipython-input-7-dd14b839c0c6> in <module>
          2     for hdir in highlevel_dirs:
          3         for fname in glob.glob("./{}/*".format(hdir) + sfx):
    ----> 4             improve_file(fname)
    

    <ipython-input-3-c3cb15f9c42c> in improve_file(fname)
         55     basic_improve(fname)
         56     if fname.endswith(".md"):
    ---> 57         improve_md(fname)
    

    <ipython-input-3-c3cb15f9c42c> in improve_md(fname)
         48     r = r.replace("", "")
         49 
    ---> 50     r = re.sub(r'\<\!--MD_FROM_FILE (.*?) --\>', file_repl, r)
         51     with open(fname, "w") as f:
         52         f.write(r)


    /usr/lib/python3.8/re.py in sub(pattern, repl, string, count, flags)
        206     a callable, it's passed the Match object and must return
        207     a replacement string to be used."""
    --> 208     return _compile(pattern, flags).sub(repl, string, count)
        209 
        210 def subn(pattern, repl, string, count=0, flags=0):


    <ipython-input-3-c3cb15f9c42c> in file_repl(matchobj, path)
         42         fname = os.path.join(path, matchobj.group(1))
         43         if fname.find("__FILE__") == -1:
    ---> 44             with open(fname, "r") as f:
         45                 return "\n```\n" + f.read() + "\n```\n"
         46 


    FileNotFoundError: [Errno 2] No such file or directory: './../tools/./interactive_launcher_tmp/670388572452524161.log.md'


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
    warning: LF will be replaced by CRLF in tools/save_them_all.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/set_up_magics.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/set_up_magics_dev.ipynb.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../tools/*.md
    warning: LF will be replaced by CRLF in tools/README.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/save_them_all.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/set_up_magics.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/set_up_magics_dev.md.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../tools/*.c
    fatal: pathspec '../tools/*.c' did not match any files
    > git add --ignore-errors  ../tools/*.cpp
    > git add --ignore-errors -f  -f ../tools/bash_popen_tmp/*.html
    > git add --ignore-errors -f  -f ../tools/interactive_launcher_tmp/*.log
    > git add -u
    warning: LF will be replaced by CRLF in tools/save_them_all.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/set_up_magics.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/set_up_magics_dev.md.
    The file will have its original line endings in your working directory.
    > git commit -m 'yet another update'
    warning: LF will be replaced by CRLF in tools/save_them_all.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/save_them_all.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/save_them_all.md.
    The file will have its original line endings in your working directory.
    [master aa6116f] yet another update
    warning: LF will be replaced by CRLF in tools/save_them_all.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/save_them_all.md.
    The file will have its original line endings in your working directory.
     2 files changed, 269 insertions(+), 364 deletions(-)
     rewrite tools/save_them_all.md (62%)
    > git push origin master
    Counting objects: 5, done.
    Delta compression using up to 2 threads.
    Compressing objects: 100% (5/5), done.
    Writing objects: 100% (5/5), 1.45 KiB | 0 bytes/s, done.
    Total 5 (delta 4), reused 0 (delta 0)
    remote: Resolving deltas: 100% (4/4), completed with 4 local objects.[K
    To git@github.com:yuri-pechatnov/caos_2019-2020.git
       34c419f..aa6116f  master -> master



```python

```


```python

```


```python

```


```python

```
