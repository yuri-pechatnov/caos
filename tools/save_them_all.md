```python
import glob
import os
import subprocess

highlevel_dirs = sum([
    #["../tools"], 
    #sorted(glob.glob("../sem16*")),
    sorted(glob.glob("../sem1*")),
], [])

print("Highlevel dirs:", highlevel_dirs)
```

    Highlevel dirs: ['../sem10-file-attributes', '../sem11-mmap-instrumentation', '../sem12-fork-exec-pipe', '../sem13-signal', '../sem14-fifo-proc', '../sem15-ptrace', '../sem16-fcntl-dup-pipe', '../sem17-sockets-tcp-udp', '../sem18-multiplexing']



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

    ../sem10-file-attributes ['../sem10-file-attributes/file-attrib.ipynb']
    [NbConvertApp] Converting notebook ../sem10-file-attributes/file-attrib.ipynb to markdown
    [NbConvertApp] Writing 16319 bytes to ../sem10-file-attributes/README.md
    ../sem11-mmap-instrumentation ['../sem11-mmap-instrumentation/mmap_and_instrumentation.ipynb']
    [NbConvertApp] Converting notebook ../sem11-mmap-instrumentation/mmap_and_instrumentation.ipynb to markdown
    [NbConvertApp] Writing 23442 bytes to ../sem11-mmap-instrumentation/README.md
    ../sem12-fork-exec-pipe ['../sem12-fork-exec-pipe/fork-pipe-exec.ipynb']
    [NbConvertApp] Converting notebook ../sem12-fork-exec-pipe/fork-pipe-exec.ipynb to markdown
    [NbConvertApp] Writing 20684 bytes to ../sem12-fork-exec-pipe/README.md
    ../sem13-signal ['../sem13-signal/signal.ipynb']
    [NbConvertApp] Converting notebook ../sem13-signal/signal.ipynb to markdown
    [NbConvertApp] Writing 26542 bytes to ../sem13-signal/README.md
    ../sem14-fifo-proc ['../sem14-fifo-proc/fifo-proc.ipynb']
    [NbConvertApp] Converting notebook ../sem14-fifo-proc/fifo-proc.ipynb to markdown
    [NbConvertApp] Writing 56508 bytes to ../sem14-fifo-proc/README.md
    ../sem15-ptrace ['../sem15-ptrace/ptrace.ipynb']
    [NbConvertApp] Converting notebook ../sem15-ptrace/ptrace.ipynb to markdown
    [NbConvertApp] Writing 15768 bytes to ../sem15-ptrace/README.md
    ../sem16-fcntl-dup-pipe ['../sem16-fcntl-dup-pipe/fcntl-dup-pipe.ipynb']
    [NbConvertApp] Converting notebook ../sem16-fcntl-dup-pipe/fcntl-dup-pipe.ipynb to markdown
    [NbConvertApp] Writing 109425 bytes to ../sem16-fcntl-dup-pipe/README.md
    ../sem17-sockets-tcp-udp ['../sem17-sockets-tcp-udp/sockets-tcp-udp.ipynb']
    [NbConvertApp] Converting notebook ../sem17-sockets-tcp-udp/sockets-tcp-udp.ipynb to markdown
    [NbConvertApp] Writing 56193 bytes to ../sem17-sockets-tcp-udp/README.md
    ../sem18-multiplexing ['../sem18-multiplexing/epoll-poll-select-linuxaio.ipynb']
    [NbConvertApp] Converting notebook ../sem18-multiplexing/epoll-poll-select-linuxaio.ipynb to markdown
    [NbConvertApp] Writing 30813 bytes to ../sem18-multiplexing/README.md



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

    dos2unix: converting file ./../sem10-file-attributes/file-attrib.ipynb to Unix format ...
    dos2unix: converting file ./../sem11-mmap-instrumentation/mmap_and_instrumentation.ipynb to Unix format ...
    dos2unix: converting file ./../sem12-fork-exec-pipe/fork-pipe-exec.ipynb to Unix format ...
    dos2unix: converting file ./../sem13-signal/signal.ipynb to Unix format ...
    dos2unix: converting file ./../sem14-fifo-proc/fifo-proc.ipynb to Unix format ...
    dos2unix: converting file ./../sem15-ptrace/ptrace.ipynb to Unix format ...
    dos2unix: converting file ./../sem16-fcntl-dup-pipe/fcntl-dup-pipe.ipynb to Unix format ...
    dos2unix: converting file ./../sem17-sockets-tcp-udp/sockets-tcp-udp.ipynb to Unix format ...
    dos2unix: converting file ./../sem18-multiplexing/epoll-poll-select-linuxaio.ipynb to Unix format ...
    dos2unix: converting file ./../sem10-file-attributes/README.md to Unix format ...
    dos2unix: converting file ./../sem11-mmap-instrumentation/README.md to Unix format ...
    dos2unix: converting file ./../sem12-fork-exec-pipe/README.md to Unix format ...
    dos2unix: converting file ./../sem13-signal/README.md to Unix format ...
    dos2unix: converting file ./../sem14-fifo-proc/README.md to Unix format ...
    dos2unix: converting file ./../sem15-ptrace/README.md to Unix format ...
    dos2unix: converting file ./../sem16-fcntl-dup-pipe/README.md to Unix format ...
    dos2unix: converting file ./../sem17-sockets-tcp-udp/README.md to Unix format ...
    dos2unix: Binary symbol 0x02 found at line 458
    dos2unix: Skipping binary file ./../sem18-multiplexing/README.md



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

    > git add --ignore-errors  ../sem10-file-attributes/*.ipynb
    warning: LF will be replaced by CRLF in sem10-file-attributes/file-attrib.ipynb.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../sem10-file-attributes/*.md
    warning: LF will be replaced by CRLF in sem10-file-attributes/README.md.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../sem10-file-attributes/*.c
    > git add --ignore-errors  ../sem10-file-attributes/*.cpp
    fatal: pathspec '../sem10-file-attributes/*.cpp' did not match any files
    > git add --ignore-errors -f  -f ../sem10-file-attributes/bash_popen_tmp/*.html
    fatal: pathspec '../sem10-file-attributes/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem10-file-attributes/interactive_launcher_tmp/*.log
    fatal: pathspec '../sem10-file-attributes/interactive_launcher_tmp/*.log' did not match any files
    > git add --ignore-errors  ../sem11-mmap-instrumentation/*.ipynb
    warning: LF will be replaced by CRLF in sem11-mmap-instrumentation/mmap_and_instrumentation.ipynb.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../sem11-mmap-instrumentation/*.md
    warning: LF will be replaced by CRLF in sem11-mmap-instrumentation/README.md.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../sem11-mmap-instrumentation/*.c
    > git add --ignore-errors  ../sem11-mmap-instrumentation/*.cpp
    > git add --ignore-errors -f  -f ../sem11-mmap-instrumentation/bash_popen_tmp/*.html
    fatal: pathspec '../sem11-mmap-instrumentation/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem11-mmap-instrumentation/interactive_launcher_tmp/*.log
    fatal: pathspec '../sem11-mmap-instrumentation/interactive_launcher_tmp/*.log' did not match any files
    > git add --ignore-errors  ../sem12-fork-exec-pipe/*.ipynb
    warning: LF will be replaced by CRLF in sem12-fork-exec-pipe/fork-pipe-exec.ipynb.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../sem12-fork-exec-pipe/*.md
    warning: LF will be replaced by CRLF in sem12-fork-exec-pipe/README.md.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../sem12-fork-exec-pipe/*.c
    > git add --ignore-errors  ../sem12-fork-exec-pipe/*.cpp
    > git add --ignore-errors -f  -f ../sem12-fork-exec-pipe/bash_popen_tmp/*.html
    fatal: pathspec '../sem12-fork-exec-pipe/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem12-fork-exec-pipe/interactive_launcher_tmp/*.log
    fatal: pathspec '../sem12-fork-exec-pipe/interactive_launcher_tmp/*.log' did not match any files
    > git add --ignore-errors  ../sem13-signal/*.ipynb
    warning: LF will be replaced by CRLF in sem13-signal/signal.ipynb.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../sem13-signal/*.md
    warning: LF will be replaced by CRLF in sem13-signal/README.md.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../sem13-signal/*.c
    > git add --ignore-errors  ../sem13-signal/*.cpp
    > git add --ignore-errors -f  -f ../sem13-signal/bash_popen_tmp/*.html
    fatal: pathspec '../sem13-signal/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem13-signal/interactive_launcher_tmp/*.log
    fatal: pathspec '../sem13-signal/interactive_launcher_tmp/*.log' did not match any files
    > git add --ignore-errors  ../sem14-fifo-proc/*.ipynb
    warning: LF will be replaced by CRLF in sem14-fifo-proc/fifo-proc.ipynb.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../sem14-fifo-proc/*.md
    warning: LF will be replaced by CRLF in sem14-fifo-proc/README.md.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../sem14-fifo-proc/*.c
    > git add --ignore-errors  ../sem14-fifo-proc/*.cpp
    > git add --ignore-errors -f  -f ../sem14-fifo-proc/bash_popen_tmp/*.html
    > git add --ignore-errors -f  -f ../sem14-fifo-proc/interactive_launcher_tmp/*.log
    fatal: pathspec '../sem14-fifo-proc/interactive_launcher_tmp/*.log' did not match any files
    > git add --ignore-errors  ../sem15-ptrace/*.ipynb
    warning: LF will be replaced by CRLF in sem15-ptrace/ptrace.ipynb.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../sem15-ptrace/*.md
    warning: LF will be replaced by CRLF in sem15-ptrace/README.md.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../sem15-ptrace/*.c
    > git add --ignore-errors  ../sem15-ptrace/*.cpp
    > git add --ignore-errors -f  -f ../sem15-ptrace/bash_popen_tmp/*.html
    fatal: pathspec '../sem15-ptrace/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem15-ptrace/interactive_launcher_tmp/*.log
    fatal: pathspec '../sem15-ptrace/interactive_launcher_tmp/*.log' did not match any files
    > git add --ignore-errors  ../sem16-fcntl-dup-pipe/*.ipynb
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/fcntl-dup-pipe.ipynb.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../sem16-fcntl-dup-pipe/*.md
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/README.md.
    The file will have its original line endings in your working directory.
    > git add --ignore-errors  ../sem16-fcntl-dup-pipe/*.c
    > git add --ignore-errors  ../sem16-fcntl-dup-pipe/*.cpp
    > git add --ignore-errors -f  -f ../sem16-fcntl-dup-pipe/bash_popen_tmp/*.html
    fatal: pathspec '../sem16-fcntl-dup-pipe/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem16-fcntl-dup-pipe/interactive_launcher_tmp/*.log
    fatal: pathspec '../sem16-fcntl-dup-pipe/interactive_launcher_tmp/*.log' did not match any files
    > git add --ignore-errors  ../sem17-sockets-tcp-udp/*.ipynb
    warning: LF will be replaced by CRLF in sem17-sockets-tcp-udp/sockets-tcp-udp.ipynb.
    The file will have its original line endings in your working directory.



```python

```


```python

```


```python

```
