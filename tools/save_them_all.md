```python
import glob
import os
import subprocess

highlevel_dirs = sum([
    #["../tools"], 
    #sorted(glob.glob("../sem16*")),
    sorted(glob.glob("../sem19*")),
], [])

print("Highlevel dirs:", highlevel_dirs)
```

    Highlevel dirs: ['../sem19-pthread']



```python

def convert(n, d):
    get_ipython().system("jupyter nbconvert {} --to markdown --output {}".format(n, d))
    #subprocess.check_call(["jupyter", "nbconvert", n, "--to", "markdown", "--output", d])

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
        
```

    ../sem19-pthread ['../sem19-pthread/pthread.ipynb']
    [NbConvertApp] Converting notebook ../sem19-pthread/pthread.ipynb to markdown
    [NbConvertApp] Writing 28511 bytes to ../sem19-pthread/README.md



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
    r = re.sub(r'(\<too much code>)', "<too much code>", r)
    r = r.replace("SUPER_SLASH" + "_N_REPLACER", '\n')
    
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

    dos2unix: converting file ./../sem19-pthread/pthread.ipynb to Unix format ...
    dos2unix: converting file ./../sem19-pthread/README.md to Unix format ...



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

    > git add --ignore-errors  ../sem19-pthread/*.ipynb
    warning: LF will be replaced by CRLF in sem19-pthread/pthread.ipynb.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../sem19-pthread/*.md
    warning: LF will be replaced by CRLF in sem19-pthread/README.md.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../sem19-pthread/*.c
    fatal: pathspec '../sem19-pthread/*.c' did not match any files
    > git add --ignore-errors  ../sem19-pthread/*.cpp
    warning: LF will be replaced by CRLF in sem19-pthread/pthread_cancel.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem19-pthread/pthread_cancel_fail.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem19-pthread/pthread_create.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem19-pthread/pthread_stack_size.cpp.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors -f  -f ../sem19-pthread/bash_popen_tmp/*.html
    fatal: pathspec '../sem19-pthread/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem19-pthread/interactive_launcher_tmp/*.log
    fatal: pathspec '../sem19-pthread/interactive_launcher_tmp/*.log' did not match any files
    > git add -u
    warning: LF will be replaced by CRLF in sem17-sockets-tcp-udp/sockets-tcp-udp.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem19-pthread/README.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/save_them_all.ipynb.
    The file will have its original line endings in your working directory.
    > git commit -m 'yet another update'
    warning: LF will be replaced by CRLF in sem17-sockets-tcp-udp/sockets-tcp-udp.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/save_them_all.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem19-pthread/README.md.
    The file will have its original line endings in your working directory.
    [master 3f7a6a4] yet another update
    warning: LF will be replaced by CRLF in sem17-sockets-tcp-udp/sockets-tcp-udp.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem19-pthread/README.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem19-pthread/pthread.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem19-pthread/pthread_cancel.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem19-pthread/pthread_cancel_fail.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem19-pthread/pthread_create.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem19-pthread/pthread_stack_size.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/save_them_all.ipynb.
    The file will have its original line endings in your working directory.
     8 files changed, 1724 insertions(+), 182 deletions(-)
     create mode 100644 sem19-pthread/README.md
     create mode 100644 sem19-pthread/pthread.ipynb
     create mode 100644 sem19-pthread/pthread_cancel.cpp
     create mode 100644 sem19-pthread/pthread_cancel_fail.cpp
     create mode 100644 sem19-pthread/pthread_create.cpp
     create mode 100644 sem19-pthread/pthread_stack_size.cpp
    > git push origin master
    Counting objects: 13, done.
    Compressing objects: 100% (13/13), done.
    Writing objects: 100% (13/13), 10.30 KiB | 0 bytes/s, done.
    Total 13 (delta 9), reused 0 (delta 0)
    remote: Resolving deltas: 100% (9/9), completed with 5 local objects.[K
    To git@github.com:yuri-pechatnov/caos_2019-2020.git
       d7f2273..3f7a6a4  master -> master



```python

```


```python

```


```python

```
