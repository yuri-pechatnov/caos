```python
import glob
import os
import subprocess

highlevel_dirs = sum([
    ["../tools"], 
    sorted(glob.glob("../sem17*")),
    sorted(glob.glob("../sem20*")),
], [])

print("Highlevel dirs:", highlevel_dirs)
```

    Highlevel dirs: ['../tools', '../sem17-sockets-tcp-udp', '../sem20-synchronizing']



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

    ../tools ['../tools/set_up_magics.ipynb', '../tools/set_up_magics_dev.ipynb', '../tools/save_them_all.ipynb']
    [NbConvertApp] Converting notebook ../tools/set_up_magics.ipynb to markdown
    [NbConvertApp] Writing 67753 bytes to ../tools/set_up_magics.md
    [NbConvertApp] Converting notebook ../tools/set_up_magics_dev.ipynb to markdown
    [NbConvertApp] Writing 32 bytes to ../tools/set_up_magics_dev.md
    [NbConvertApp] Converting notebook ../tools/save_them_all.ipynb to markdown
    [NbConvertApp] Writing 4286 bytes to ../tools/save_them_all.md
    ../sem17-sockets-tcp-udp ['../sem17-sockets-tcp-udp/sockets-tcp-udp.ipynb']
    [NbConvertApp] Converting notebook ../sem17-sockets-tcp-udp/sockets-tcp-udp.ipynb to markdown
    [NbConvertApp] Writing 61840 bytes to ../sem17-sockets-tcp-udp/README.md
    ../sem20-synchronizing ['../sem20-synchronizing/quiz.ipynb', '../sem20-synchronizing/synchronizing.ipynb']
    [NbConvertApp] Converting notebook ../sem20-synchronizing/quiz.ipynb to markdown
    [NbConvertApp] Writing 34321 bytes to ../sem20-synchronizing/quiz.md
    [NbConvertApp] Converting notebook ../sem20-synchronizing/synchronizing.ipynb to markdown
    [NbConvertApp] Writing 34772 bytes to ../sem20-synchronizing/synchronizing.md



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
    r = r.replace("SUPER_SLASH" + "_N_REPLACER", '\n')
    
    def file_repl(matchobj, path=os.path.dirname(fname)):
        fname = os.path.join(path, matchobj.group(1))
        if fname.find("__FILE__") == -1:
            with open(fname, "r") as f:
                return "\n```\n" + f.read() + "\n```\n"
    
    r = r.replace("</td>", "")
    r = r.replace("</tr>", "")
    
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

    dos2unix: converting file ./../tools/set_up_magics.ipynb to Unix format ...
    dos2unix: converting file ./../tools/set_up_magics_dev.ipynb to Unix format ...
    dos2unix: converting file ./../tools/save_them_all.ipynb to Unix format ...
    dos2unix: converting file ./../sem17-sockets-tcp-udp/sockets-tcp-udp.ipynb to Unix format ...
    dos2unix: converting file ./../sem20-synchronizing/quiz.ipynb to Unix format ...
    dos2unix: converting file ./../sem20-synchronizing/synchronizing.ipynb to Unix format ...
    dos2unix: converting file ./../tools/set_up_magics_dev.md to Unix format ...
    dos2unix: converting file ./../tools/set_up_magics.md to Unix format ...



    ---------------------------------------------------------------------------

    FileNotFoundError                         Traceback (most recent call last)

    <ipython-input-68-dd14b839c0c6> in <module>
          2     for hdir in highlevel_dirs:
          3         for fname in glob.glob("./{}/*".format(hdir) + sfx):
    ----> 4             improve_file(fname)
    

    <ipython-input-67-f947e1e5b248> in improve_file(fname)
         35     basic_improve(fname)
         36     if fname.endswith(".md"):
    ---> 37         improve_md(fname)
    

    <ipython-input-67-f947e1e5b248> in improve_md(fname)
         28     r = r.replace("</tr>", "")
         29 
    ---> 30     r = re.sub(r'\<\!--MD_FROM_FILE (.*?) --\>', file_repl, r)
         31     with open(fname, "w") as f:
         32         f.write(r)


    /usr/lib/python3.5/re.py in sub(pattern, repl, string, count, flags)
        180     a callable, it's passed the match object and must return
        181     a replacement string to be used."""
    --> 182     return _compile(pattern, flags).sub(repl, string, count)
        183 
        184 def subn(pattern, repl, string, count=0, flags=0):


    <ipython-input-67-f947e1e5b248> in file_repl(matchobj, path)
         22         fname = os.path.join(path, matchobj.group(1))
         23         if fname.find("__FILE__") == -1:
    ---> 24             with open(fname, "r") as f:
         25                 return "\n```\n" + f.read() + "\n```\n"
         26 


    FileNotFoundError: [Errno 2] No such file or directory: './../tools/./interactive_launcher_tmp/704228343092166969.log.md'



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


```python

```


```python

```


```python

```


```python

```
