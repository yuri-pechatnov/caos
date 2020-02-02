```python
import glob
import os
import subprocess

def convert(n, d):
    get_ipython().system("jupyter nbconvert {} --to markdown --output {}".format(n, d))
    #subprocess.check_call(["jupyter", "nbconvert", n, "--to", "markdown", "--output", d])

highlevel_dirs = ["tools"] + sorted(glob.glob("./sem*"))

print("Highlevel dirs:", highlevel_dirs)
```

    Highlevel dirs: ['tools', './sem01', './sem03-ints-floats', './sem04-asm-arm', './sem05-asm-arm-addressing', './sem06-asm-x86', './sem07-asm-x86-x87-sse', './sem08-asm-x86-nostdlib', './sem09-low-level-io', './sem10-file-attributes', './sem11-mmap-instrumentation', './sem12-fork-exec-pipe', './sem13-signal', './sem14-fifo-proc', './sem15-ptrace', './sem16-fcntl-dup-pipe']



```python
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

    tools ['tools/set_up_magics.ipynb', 'tools/set_up_magics_dev.ipynb']
    [NbConvertApp] Converting notebook tools/set_up_magics.ipynb to markdown
    [NbConvertApp] Writing 50748 bytes to tools/set_up_magics.md
    [NbConvertApp] Converting notebook tools/set_up_magics_dev.ipynb to markdown
    [NbConvertApp] Writing 32 bytes to tools/set_up_magics_dev.md
    ./sem01 ['./sem01/sem1.ipynb']
    [NbConvertApp] Converting notebook ./sem01/sem1.ipynb to markdown
    [NbConvertApp] Writing 2135 bytes to ./sem01/README.md
    ./sem03-ints-floats ['./sem03-ints-floats/sem3_floats.ipynb', './sem03-ints-floats/sem3_ints.ipynb']
    [NbConvertApp] Converting notebook ./sem03-ints-floats/sem3_floats.ipynb to markdown
    [NbConvertApp] Writing 2857 bytes to ./sem03-ints-floats/sem3_floats.md
    [NbConvertApp] Converting notebook ./sem03-ints-floats/sem3_ints.ipynb to markdown
    [NbConvertApp] Writing 3840 bytes to ./sem03-ints-floats/sem3_ints.md
    ./sem04-asm-arm ['./sem04-asm-arm/arm.ipynb']
    [NbConvertApp] Converting notebook ./sem04-asm-arm/arm.ipynb to markdown
    [NbConvertApp] Writing 7422 bytes to ./sem04-asm-arm/README.md
    ./sem05-asm-arm-addressing ['./sem05-asm-arm-addressing/adressing.ipynb']
    [NbConvertApp] Converting notebook ./sem05-asm-arm-addressing/adressing.ipynb to markdown
    [NbConvertApp] Writing 23552 bytes to ./sem05-asm-arm-addressing/README.md
    ./sem06-asm-x86 ['./sem06-asm-x86/asm_x86.ipynb']
    [NbConvertApp] Converting notebook ./sem06-asm-x86/asm_x86.ipynb to markdown
    [NbConvertApp] Writing 33694 bytes to ./sem06-asm-x86/README.md
    ./sem07-asm-x86-x87-sse ['./sem07-asm-x86-x87-sse/sse_x86.ipynb', './sem07-asm-x86-x87-sse/floating_point_x86.ipynb']
    [NbConvertApp] Converting notebook ./sem07-asm-x86-x87-sse/sse_x86.ipynb to markdown
    [NbConvertApp] Writing 13366 bytes to ./sem07-asm-x86-x87-sse/sse_x86.md
    [NbConvertApp] Converting notebook ./sem07-asm-x86-x87-sse/floating_point_x86.ipynb to markdown
    [NbConvertApp] Writing 8065 bytes to ./sem07-asm-x86-x87-sse/floating_point_x86.md
    ./sem08-asm-x86-nostdlib ['./sem08-asm-x86-nostdlib/nostdlib.ipynb']
    [NbConvertApp] Converting notebook ./sem08-asm-x86-nostdlib/nostdlib.ipynb to markdown
    [NbConvertApp] Writing 23229 bytes to ./sem08-asm-x86-nostdlib/README.md
    ./sem09-low-level-io ['./sem09-low-level-io/low-level-io.ipynb']
    [NbConvertApp] Converting notebook ./sem09-low-level-io/low-level-io.ipynb to markdown
    [NbConvertApp] Writing 17581 bytes to ./sem09-low-level-io/README.md
    ./sem10-file-attributes ['./sem10-file-attributes/file-attrib.ipynb']
    [NbConvertApp] Converting notebook ./sem10-file-attributes/file-attrib.ipynb to markdown
    [NbConvertApp] Writing 16319 bytes to ./sem10-file-attributes/README.md
    ./sem11-mmap-instrumentation ['./sem11-mmap-instrumentation/mmap_and_instrumentation.ipynb']
    [NbConvertApp] Converting notebook ./sem11-mmap-instrumentation/mmap_and_instrumentation.ipynb to markdown
    [NbConvertApp] Writing 23442 bytes to ./sem11-mmap-instrumentation/README.md
    ./sem12-fork-exec-pipe ['./sem12-fork-exec-pipe/fork-pipe-exec.ipynb']
    [NbConvertApp] Converting notebook ./sem12-fork-exec-pipe/fork-pipe-exec.ipynb to markdown
    [NbConvertApp] Writing 20684 bytes to ./sem12-fork-exec-pipe/README.md
    ./sem13-signal ['./sem13-signal/signal.ipynb']
    [NbConvertApp] Converting notebook ./sem13-signal/signal.ipynb to markdown
    [NbConvertApp] Writing 26542 bytes to ./sem13-signal/README.md
    ./sem14-fifo-proc ['./sem14-fifo-proc/fifo-proc.ipynb']
    [NbConvertApp] Converting notebook ./sem14-fifo-proc/fifo-proc.ipynb to markdown
    [NbConvertApp] Writing 56508 bytes to ./sem14-fifo-proc/README.md
    ./sem15-ptrace ['./sem15-ptrace/ptrace.ipynb']
    [NbConvertApp] Converting notebook ./sem15-ptrace/ptrace.ipynb to markdown
    [NbConvertApp] Writing 15768 bytes to ./sem15-ptrace/README.md
    ./sem16-fcntl-dup-pipe ['./sem16-fcntl-dup-pipe/fcntl-dup-pipe.ipynb']
    [NbConvertApp] Converting notebook ./sem16-fcntl-dup-pipe/fcntl-dup-pipe.ipynb to markdown
    [NbConvertApp] Writing 26587 bytes to ./sem16-fcntl-dup-pipe/README.md



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

    dos2unix: converting file ./tools/set_up_magics.ipynb to Unix format ...
    dos2unix: converting file ./tools/set_up_magics_dev.ipynb to Unix format ...
    dos2unix: converting file ././sem01/sem1.ipynb to Unix format ...
    dos2unix: converting file ././sem03-ints-floats/sem3_floats.ipynb to Unix format ...
    dos2unix: converting file ././sem03-ints-floats/sem3_ints.ipynb to Unix format ...
    dos2unix: converting file ././sem04-asm-arm/arm.ipynb to Unix format ...
    dos2unix: converting file ././sem05-asm-arm-addressing/adressing.ipynb to Unix format ...
    dos2unix: converting file ././sem06-asm-x86/asm_x86.ipynb to Unix format ...
    dos2unix: converting file ././sem07-asm-x86-x87-sse/sse_x86.ipynb to Unix format ...
    dos2unix: converting file ././sem07-asm-x86-x87-sse/floating_point_x86.ipynb to Unix format ...
    dos2unix: converting file ././sem08-asm-x86-nostdlib/nostdlib.ipynb to Unix format ...
    dos2unix: converting file ././sem09-low-level-io/low-level-io.ipynb to Unix format ...
    dos2unix: converting file ././sem10-file-attributes/file-attrib.ipynb to Unix format ...
    dos2unix: converting file ././sem11-mmap-instrumentation/mmap_and_instrumentation.ipynb to Unix format ...
    dos2unix: converting file ././sem12-fork-exec-pipe/fork-pipe-exec.ipynb to Unix format ...
    dos2unix: converting file ././sem13-signal/signal.ipynb to Unix format ...
    dos2unix: converting file ././sem14-fifo-proc/fifo-proc.ipynb to Unix format ...
    dos2unix: converting file ././sem15-ptrace/ptrace.ipynb to Unix format ...
    dos2unix: converting file ././sem16-fcntl-dup-pipe/fcntl-dup-pipe.ipynb to Unix format ...
    dos2unix: converting file ./tools/set_up_magics_dev.md to Unix format ...
    dos2unix: converting file ./tools/set_up_magics.md to Unix format ...
    dos2unix: converting file ././sem01/README.md to Unix format ...
    dos2unix: converting file ././sem03-ints-floats/sem3_ints.md to Unix format ...
    dos2unix: converting file ././sem03-ints-floats/sem3_floats.md to Unix format ...
    dos2unix: converting file ././sem04-asm-arm/README.md to Unix format ...
    dos2unix: converting file ././sem05-asm-arm-addressing/README.md to Unix format ...
    dos2unix: converting file ././sem06-asm-x86/README.md to Unix format ...
    dos2unix: converting file ././sem07-asm-x86-x87-sse/floating_point_x86.md to Unix format ...
    dos2unix: converting file ././sem07-asm-x86-x87-sse/sse_x86.md to Unix format ...
    dos2unix: converting file ././sem08-asm-x86-nostdlib/README.md to Unix format ...
    dos2unix: converting file ././sem09-low-level-io/README.md to Unix format ...
    dos2unix: converting file ././sem10-file-attributes/README.md to Unix format ...
    dos2unix: converting file ././sem11-mmap-instrumentation/README.md to Unix format ...
    dos2unix: converting file ././sem12-fork-exec-pipe/README.md to Unix format ...
    dos2unix: converting file ././sem13-signal/README.md to Unix format ...
    dos2unix: converting file ././sem14-fifo-proc/README.md to Unix format ...
    dos2unix: converting file ././sem15-ptrace/README.md to Unix format ...
    dos2unix: converting file ././sem16-fcntl-dup-pipe/README.md to Unix format ...



```python
for subdir in highlevel_dirs:
    for sfx in [".ipynb", ".md", ".c", ".cpp"]:
        get_ipython().system("git add {}/*{}".format(os.path.basename(subdir), sfx))
    get_ipython().system("git add -f {}/bash_popen_tmp/*.html".format(os.path.basename(subdir)))
get_ipython().system("git add -u")
get_ipython().system("git commit -m 'yet another update'")
get_ipython().system("git push origin master")
```

    warning: LF will be replaced by CRLF in tools/set_up_magics.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/set_up_magics_dev.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/set_up_magics.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/set_up_magics_dev.md.
    The file will have its original line endings in your working directory.
    fatal: pathspec 'tools/*.c' did not match any files
    fatal: pathspec 'tools/*.cpp' did not match any files
    warning: LF will be replaced by CRLF in sem01/sem1.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem01/README.md.
    The file will have its original line endings in your working directory.
    fatal: pathspec 'sem01/*.cpp' did not match any files
    fatal: pathspec 'sem01/bash_popen_tmp/*.html' did not match any files
    warning: LF will be replaced by CRLF in sem03-ints-floats/sem3_floats.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem03-ints-floats/sem3_ints.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem03-ints-floats/sem3_floats.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem03-ints-floats/sem3_ints.md.
    The file will have its original line endings in your working directory.
    fatal: pathspec 'sem03-ints-floats/*.cpp' did not match any files
    fatal: pathspec 'sem03-ints-floats/bash_popen_tmp/*.html' did not match any files
    warning: LF will be replaced by CRLF in sem04-asm-arm/arm.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem04-asm-arm/README.md.
    The file will have its original line endings in your working directory.
    fatal: pathspec 'sem04-asm-arm/*.cpp' did not match any files
    fatal: pathspec 'sem04-asm-arm/bash_popen_tmp/*.html' did not match any files
    warning: LF will be replaced by CRLF in sem05-asm-arm-addressing/adressing.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem05-asm-arm-addressing/README.md.
    The file will have its original line endings in your working directory.
    fatal: pathspec 'sem05-asm-arm-addressing/*.cpp' did not match any files
    fatal: pathspec 'sem05-asm-arm-addressing/bash_popen_tmp/*.html' did not match any files
    warning: LF will be replaced by CRLF in sem06-asm-x86/asm_x86.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem06-asm-x86/README.md.
    The file will have its original line endings in your working directory.
    fatal: pathspec 'sem06-asm-x86/*.cpp' did not match any files
    fatal: pathspec 'sem06-asm-x86/bash_popen_tmp/*.html' did not match any files
    warning: LF will be replaced by CRLF in sem07-asm-x86-x87-sse/floating_point_x86.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem07-asm-x86-x87-sse/sse_x86.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem07-asm-x86-x87-sse/floating_point_x86.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem07-asm-x86-x87-sse/sse_x86.md.
    The file will have its original line endings in your working directory.
    fatal: pathspec 'sem07-asm-x86-x87-sse/*.cpp' did not match any files
    fatal: pathspec 'sem07-asm-x86-x87-sse/bash_popen_tmp/*.html' did not match any files
    warning: LF will be replaced by CRLF in sem08-asm-x86-nostdlib/nostdlib.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem08-asm-x86-nostdlib/README.md.
    The file will have its original line endings in your working directory.
    fatal: pathspec 'sem08-asm-x86-nostdlib/*.cpp' did not match any files
    fatal: pathspec 'sem08-asm-x86-nostdlib/bash_popen_tmp/*.html' did not match any files
    warning: LF will be replaced by CRLF in sem09-low-level-io/low-level-io.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem09-low-level-io/README.md.
    The file will have its original line endings in your working directory.
    fatal: pathspec 'sem09-low-level-io/*.cpp' did not match any files
    fatal: pathspec 'sem09-low-level-io/bash_popen_tmp/*.html' did not match any files
    warning: LF will be replaced by CRLF in sem10-file-attributes/file-attrib.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem10-file-attributes/README.md.
    The file will have its original line endings in your working directory.
    fatal: pathspec 'sem10-file-attributes/*.cpp' did not match any files
    fatal: pathspec 'sem10-file-attributes/bash_popen_tmp/*.html' did not match any files
    warning: LF will be replaced by CRLF in sem11-mmap-instrumentation/mmap_and_instrumentation.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem11-mmap-instrumentation/README.md.
    The file will have its original line endings in your working directory.
    fatal: pathspec 'sem11-mmap-instrumentation/bash_popen_tmp/*.html' did not match any files
    warning: LF will be replaced by CRLF in sem12-fork-exec-pipe/fork-pipe-exec.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem12-fork-exec-pipe/README.md.
    The file will have its original line endings in your working directory.
    fatal: pathspec 'sem12-fork-exec-pipe/bash_popen_tmp/*.html' did not match any files
    warning: LF will be replaced by CRLF in sem13-signal/signal.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem13-signal/README.md.
    The file will have its original line endings in your working directory.
    fatal: pathspec 'sem13-signal/bash_popen_tmp/*.html' did not match any files
    warning: LF will be replaced by CRLF in sem14-fifo-proc/fifo-proc.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem14-fifo-proc/README.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem15-ptrace/ptrace.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem15-ptrace/README.md.
    The file will have its original line endings in your working directory.
    fatal: pathspec 'sem15-ptrace/bash_popen_tmp/*.html' did not match any files
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/fcntl-dup-pipe.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/README.md.
    The file will have its original line endings in your working directory.
    fatal: pathspec 'sem16-fcntl-dup-pipe/*.c' did not match any files
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/dup.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/dup2.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/fcntl_dup.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/fcntl_flags.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/fcntl_open_flags.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/pipe.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/pipe2.cpp.
    The file will have its original line endings in your working directory.
    fatal: pathspec 'sem16-fcntl-dup-pipe/bash_popen_tmp/*.html' did not match any files
    warning: LF will be replaced by CRLF in save_them_all.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in save_them_all.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in save_them_all.ipynb.
    The file will have its original line endings in your working directory.
    [master 82eb774] yet another update
    warning: LF will be replaced by CRLF in save_them_all.ipynb.
    The file will have its original line endings in your working directory.
     1 file changed, 62 insertions(+), 95 deletions(-)
    Counting objects: 3, done.
    Compressing objects: 100% (3/3), done.
    Writing objects: 100% (3/3), 1013 bytes | 0 bytes/s, done.
    Total 3 (delta 2), reused 0 (delta 0)
    remote: Resolving deltas: 100% (2/2), completed with 2 local objects.[K
    To git@github.com:yuri-pechatnov/caos_2019-2020.git
       f2b872f..82eb774  master -> master



```python

```


```python

```


```python

```
