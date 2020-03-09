```python
import glob
import os
import subprocess

highlevel_dirs = sum([
    #["../tools"], 
    #sorted(glob.glob("../sem17*")),
    sorted(glob.glob("../sem20*")),
], [])

print("Highlevel dirs:", highlevel_dirs)
```

    Highlevel dirs: ['../sem20-synchronizing']



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

    ../sem20-synchronizing ['../sem20-synchronizing/quiz.ipynb', '../sem20-synchronizing/synchronizing.ipynb']
    [NbConvertApp] Converting notebook ../sem20-synchronizing/quiz.ipynb to markdown
    [NbConvertApp] Writing 34405 bytes to ../sem20-synchronizing/quiz.md
    [NbConvertApp] Converting notebook ../sem20-synchronizing/synchronizing.ipynb to markdown
    [NbConvertApp] Writing 33989 bytes to ../sem20-synchronizing/synchronizing.md



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
    r = r.replace('\n', "SUPER_SLASH" + "_N_REPLACER")
    
    r = re.sub(r'\<\!--MD_BEGIN_FILTER--\>.*?\<\!--MD_END_FILTER--\>', "", r)
    r = re.sub(r'(\#SET_UP_MAGIC_BEGIN.*?\#SET_UP_MAGIC_END)', "<too much code>", r)
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

    dos2unix: converting file ./../sem20-synchronizing/quiz.ipynb to Unix format ...
    dos2unix: converting file ./../sem20-synchronizing/synchronizing.ipynb to Unix format ...
    dos2unix: converting file ./../sem20-synchronizing/README.md to Unix format ...
    dos2unix: converting file ./../sem20-synchronizing/quiz.md to Unix format ...
    dos2unix: converting file ./../sem20-synchronizing/synchronizing.md to Unix format ...



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

    > git add --ignore-errors  ../sem20-synchronizing/*.ipynb
    warning: LF will be replaced by CRLF in sem20-synchronizing/quiz.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem20-synchronizing/synchronizing.ipynb.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../sem20-synchronizing/*.md
    warning: LF will be replaced by CRLF in sem20-synchronizing/README.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem20-synchronizing/quiz.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem20-synchronizing/synchronizing.md.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../sem20-synchronizing/*.c
    > git add --ignore-errors  ../sem20-synchronizing/*.cpp
    > git add --ignore-errors -f  -f ../sem20-synchronizing/bash_popen_tmp/*.html
    fatal: pathspec '../sem20-synchronizing/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem20-synchronizing/interactive_launcher_tmp/*.log
    fatal: pathspec '../sem20-synchronizing/interactive_launcher_tmp/*.log' did not match any files
    > git add -u
    warning: LF will be replaced by CRLF in tools/save_them_all.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/set_up_magics.ipynb.
    The file will have its original line endings in your working directory.
    > git commit -m 'yet another update'
    warning: LF will be replaced by CRLF in sem20-synchronizing/synchronizing.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem20-synchronizing/synchronizing.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/save_them_all.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/set_up_magics.ipynb.
    The file will have its original line endings in your working directory.
    [master 59f6bf8] yet another update
    warning: LF will be replaced by CRLF in sem20-synchronizing/synchronizing.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem20-synchronizing/synchronizing.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/save_them_all.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/set_up_magics.ipynb.
    The file will have its original line endings in your working directory.
     4 files changed, 47 insertions(+), 49 deletions(-)
    > git push origin master
    Counting objects: 8, done.
    Compressing objects: 100% (8/8), done.
    Writing objects: 100% (8/8), 1.34 KiB | 0 bytes/s, done.
    Total 8 (delta 7), reused 0 (delta 0)
    remote: Resolving deltas: 100% (7/7), completed with 7 local objects.[K
    To git@github.com:yuri-pechatnov/caos_2019-2020.git
       2d3582b..59f6bf8  master -> master



```python

```


```python

```


```python

```


```python

```
