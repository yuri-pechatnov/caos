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
    #sorted(glob.glob("../sem24*")),
    sorted(glob.glob("../sem28*")),
], [])

print("Highlevel dirs:", highlevel_dirs)
```

    Highlevel dirs: ['../sem28-unix-time']



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

    ../sem28-unix-time ['../sem28-unix-time/time.ipynb']
    [NbConvertApp] Converting notebook ../sem28-unix-time/time.ipynb to markdown
    [NbConvertApp] Writing 28656 bytes to ../sem28-unix-time/README.md



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

    dos2unix: converting file ./../sem28-unix-time/time.ipynb to Unix format ...
    dos2unix: converting file ./../sem28-unix-time/README.md to Unix format ...


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
    
def execute_cmd(cmd):
    print(">", cmd)
    get_ipython().system(cmd)
    
for cmd in cmds:
    execute_cmd(cmd)
execute_cmd("git add -u")
execute_cmd("git commit -m 'yet another update'")
execute_cmd("git push origin master")
```

    > git add --ignore-errors  ../sem28-unix-time/*.ipynb
    warning: LF will be replaced by CRLF in sem28-unix-time/time.ipynb.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../sem28-unix-time/*.md
    warning: LF will be replaced by CRLF in sem28-unix-time/README.md.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../sem28-unix-time/*.c
    warning: LF will be replaced by CRLF in sem28-unix-time/time.c.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../sem28-unix-time/*.cpp
    warning: LF will be replaced by CRLF in sem28-unix-time/time.cpp.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors -f  -f ../sem28-unix-time/bash_popen_tmp/*.html
    fatal: pathspec '../sem28-unix-time/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem28-unix-time/interactive_launcher_tmp/*.log
    fatal: pathspec '../sem28-unix-time/interactive_launcher_tmp/*.log' did not match any files
    > git add -u
    warning: LF will be replaced by CRLF in sem28-unix-time/README.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem28-unix-time/time.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/save_them_all.ipynb.
    The file will have its original line endings in your working directory.
    > git commit -m 'yet another update'
    warning: LF will be replaced by CRLF in sem28-unix-time/README.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem28-unix-time/time.c.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem28-unix-time/time.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem28-unix-time/time.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/save_them_all.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem28-unix-time/README.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem28-unix-time/time.c.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem28-unix-time/time.cpp.
    The file will have its original line endings in your working directory.
    [master 34c419f] yet another update
    warning: LF will be replaced by CRLF in sem28-unix-time/README.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem28-unix-time/time.c.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem28-unix-time/time.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem28-unix-time/time.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/save_them_all.ipynb.
    The file will have its original line endings in your working directory.
     5 files changed, 280 insertions(+), 592 deletions(-)
    > git push origin master
    Counting objects: 9, done.
    Delta compression using up to 2 threads.
    Compressing objects: 100% (9/9), done.
    Writing objects: 100% (9/9), 4.43 KiB | 0 bytes/s, done.
    Total 9 (delta 7), reused 0 (delta 0)
    remote: Resolving deltas: 100% (7/7), completed with 6 local objects.[K
    To git@github.com:yuri-pechatnov/caos_2019-2020.git
       773e77d..34c419f  master -> master



```python

```


```python

```


```python

```


```python

```
