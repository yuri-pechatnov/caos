```python
import glob
import os
import subprocess

highlevel_dirs = sum([
    ["../tools"], 
    #sorted(glob.glob("../sem19*")),
    sorted(glob.glob("../sem24*")),
    #sorted(glob.glob("../sem22*")),
], [])

print("Highlevel dirs:", highlevel_dirs)
```

    Highlevel dirs: ['../tools', '../sem24-http-libcurl-cmake']



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

    ../tools ['../tools/set_up_magics.ipynb', '../tools/set_up_magics_dev.ipynb', '../tools/save_them_all.ipynb']
    [NbConvertApp] Converting notebook ../tools/set_up_magics.ipynb to markdown
    [NbConvertApp] Writing 54496 bytes to ../tools/set_up_magics.md
    [NbConvertApp] Converting notebook ../tools/set_up_magics_dev.ipynb to markdown
    [NbConvertApp] Writing 32 bytes to ../tools/set_up_magics_dev.md
    [NbConvertApp] Converting notebook ../tools/save_them_all.ipynb to markdown
    [NbConvertApp] Writing 3849 bytes to ../tools/save_them_all.md
    ../sem24-http-libcurl-cmake ['../sem24-http-libcurl-cmake/http_libcurl_cmake.ipynb']
    [NbConvertApp] Converting notebook ../sem24-http-libcurl-cmake/http_libcurl_cmake.ipynb to markdown
    [NbConvertApp] Writing 30587 bytes to ../sem24-http-libcurl-cmake/README.md



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

    dos2unix: converting file ./../tools/set_up_magics.ipynb to Unix format ...
    dos2unix: converting file ./../tools/set_up_magics_dev.ipynb to Unix format ...
    dos2unix: converting file ./../tools/save_them_all.ipynb to Unix format ...
    dos2unix: converting file ./../sem24-http-libcurl-cmake/http_libcurl_cmake.ipynb to Unix format ...
    dos2unix: converting file ./../tools/README.md to Unix format ...
    dos2unix: converting file ./../tools/set_up_magics_dev.md to Unix format ...
    dos2unix: converting file ./../tools/set_up_magics.md to Unix format ...



    ---------------------------------------------------------------------------

    FileNotFoundError                         Traceback (most recent call last)

    <ipython-input-6-dd14b839c0c6> in <module>
          2     for hdir in highlevel_dirs:
          3         for fname in glob.glob("./{}/*".format(hdir) + sfx):
    ----> 4             improve_file(fname)
    

    <ipython-input-5-9435c1df1baa> in improve_file(fname)
         54     basic_improve(fname)
         55     if fname.endswith(".md"):
    ---> 56         improve_md(fname)
    

    <ipython-input-5-9435c1df1baa> in improve_md(fname)
         47     r = r.replace("", "")
         48 
    ---> 49     r = re.sub(r'\<\!--MD_FROM_FILE (.*?) --\>', file_repl, r)
         50     with open(fname, "w") as f:
         51         f.write(r)


    /usr/lib/python3.5/re.py in sub(pattern, repl, string, count, flags)
        180     a callable, it's passed the match object and must return
        181     a replacement string to be used."""
    --> 182     return _compile(pattern, flags).sub(repl, string, count)
        183 
        184 def subn(pattern, repl, string, count=0, flags=0):


    <ipython-input-5-9435c1df1baa> in file_repl(matchobj, path)
         41         fname = os.path.join(path, matchobj.group(1))
         42         if fname.find("__FILE__") == -1:
    ---> 43             with open(fname, "r") as f:
         44                 return "\n```\n" + f.read() + "\n```\n"
         45 


    FileNotFoundError: [Errno 2] No such file or directory: './../tools/./interactive_launcher_tmp/668453912241038847.log.md'



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
