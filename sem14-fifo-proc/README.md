```python
get_ipython().run_cell_magic('javascript', '', '// setup cpp code highlighting\nIPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\'reg\':[/^%%cpp/]} ;')

# creating magics
from IPython.core.magic import register_cell_magic, register_line_magic
from IPython.display import display, Markdown, HTML
import argparse
from subprocess import Popen, PIPE
import random
import sys

@register_cell_magic
def save_file(args_str, cell, line_comment_start="#"):
    parser = argparse.ArgumentParser()
    parser.add_argument("fname")
    parser.add_argument("--ejudge-style", action="store_true")
    args = parser.parse_args(args_str.split())
    
    cell = cell if cell[-1] == '\n' or args.no_eof_newline else cell + "\n"
    cmds = []
    with open(args.fname, "w") as f:
        f.write(line_comment_start + " %%cpp " + args_str + "\n")
        for line in cell.split("\n"):
            if line.startswith("%"):
                run_prefix = "%run "
                assert line.startswith(run_prefix)
                cmds.append(line[len(run_prefix):].strip())
            else:
                f.write((line if not args.ejudge_style else line.rstrip()) + "\n")
        f.write("" if not args.ejudge_style else line_comment_start + r" line without \n")
    for cmd in cmds:
        display(Markdown("Run: `%s`" % cmd))
        get_ipython().system(cmd)

@register_cell_magic
def cpp(fname, cell):
    save_file(fname, cell, "//")

@register_cell_magic
def asm(fname, cell):
    save_file(fname, cell, "//")
    
@register_cell_magic
def makefile(fname, cell):
    assert not fname
    save_file("makefile", cell.replace(" " * 4, "\t"))
        
@register_line_magic
def p(line):
    try:
        expr, comment = line.split(" #")
        display(Markdown("`{} = {}`  # {}".format(expr.strip(), eval(expr), comment.strip())))
    except:
        display(Markdown("{} = {}".format(line, eval(line))))
        
def show_file(file, clear_at_begin=False):
    if clear_at_begin:
        get_ipython().system("truncate --size 0 " + file)
    obj = file.replace('.', '_').replace('/', '_') + "_obj"
    display(HTML('''
        <script type=text/javascript>
        function refresh__OBJ__()
        {
            var elem = document.getElementById("__OBJ__");
            if (elem) {
                var xmlhttp=new XMLHttpRequest();
                xmlhttp.onreadystatechange=function()
                {
                    var elem2 = document.getElementById("__OBJ__1");
                    if (xmlhttp.readyState==4 && xmlhttp.status==200)
                    {
                        elem2.innerText = xmlhttp.responseText;
                        // console.log(xmlhttp.responseText);
                        return xmlhttp.responseText;
                    }
                }
                xmlhttp.open("GET", elem.data, true);
                xmlhttp.send();    
                elem.hidden = "hidden";
                window.setTimeout("refresh__OBJ__()", 300); 
            }
        }
        window.setTimeout("refresh__OBJ__()", 300); 
        </script>
        <div id="__OBJ__1"></div>
        <div><object id="__OBJ__" data="__FILE__", hidden="hidden"></object></div>
        '''.replace("__OBJ__", obj)
           .replace("__FILE__", file)))
    
def bash_popen_terminate_all():
    for p in globals().get("bash_popen_list", []):
        print("Terminate pid=" + str(p.pid), file=sys.stderr)
        p.terminate()
        globals()["bash_popen_list"] = []

bash_popen_terminate_all()  

def bash_popen(cmd):
    original_cmd = cmd
    h = "tmp/" + str(random.randint(0, 1e18))
    
    stdout = open(h + ".out.html", "wb")
    display(Markdown("**STDOUT** (interactive)"))
    show_file(h + ".out.html", clear_at_begin=True)
    
    stdout = open(h + ".err.html", "wb")
    display(Markdown("**STDERR** (interactive)"))
    show_file(h + ".err.html", clear_at_begin=True)
    
    fin_file = h + ".fin.html"
    cmd = "echo 'Process started!' > " + fin_file + "; " + cmd + " ; echo \"Process finished! code=$?\" >> " + fin_file
    display(Markdown("**RUN LOG** (interactive, `" + original_cmd + "`)"))
    show_file(h + ".fin.html", clear_at_begin=True)
    
    p = Popen(["bash", "-c", cmd], stdin=PIPE, stdout=stdout, stderr=stdout)
    bash_popen_list.append(p)
    return p


@register_line_magic
def bash_async(line):
    bash_popen(line)
```


    <IPython.core.display.Javascript object>


# named FIFO

Ранее мы познакомились с пайпами (анонимными fifo (далее буду называть просто pipe'ами)). Теперь же посмотрим на именованые.
Отличие в том, что именоваванные fifo (дальше буду называть просто fifo) являются файлами в файловой системе linux. Соответственно они могут существовать, не будучи открытыми какой-либо программой. Как и файл их можно удалить.

Как создать из консоли - `man mkfifo`, как создать из кода на C - `man 3 mkfifo`. Чтение и запись в fifo происходит так же как и с обычным файлом.


```python
!rm -f my_fifo
!mkfifo my_fifo
```


```python
!echo "Hello" > my_fifo
```

    ^C
    /bin/sh: 1: cannot create my_fifo: Interrupted system call



```python
%bash_async echo "Hello" > my_fifo ; echo "After writing to my_fifo"
```


**STDOUT** (interactive)




<script type=text/javascript>
function refreshtmp_69928660144861329_out_html_obj()
{
    var elem = document.getElementById("tmp_69928660144861329_out_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_69928660144861329_out_html_obj1");
            if (xmlhttp.readyState==4 && xmlhttp.status==200)
            {
                elem2.innerText = xmlhttp.responseText;
                // console.log(xmlhttp.responseText);
                return xmlhttp.responseText;
            }
        }
        xmlhttp.open("GET", elem.data, true);
        xmlhttp.send();    
        elem.hidden = "hidden";
        window.setTimeout("refreshtmp_69928660144861329_out_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_69928660144861329_out_html_obj()", 300); 
</script>
<div id="tmp_69928660144861329_out_html_obj1"></div>
<div><object id="tmp_69928660144861329_out_html_obj" data="tmp/69928660144861329.out.html", hidden="hidden"></object></div>




**STDERR** (interactive)




<script type=text/javascript>
function refreshtmp_69928660144861329_err_html_obj()
{
    var elem = document.getElementById("tmp_69928660144861329_err_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_69928660144861329_err_html_obj1");
            if (xmlhttp.readyState==4 && xmlhttp.status==200)
            {
                elem2.innerText = xmlhttp.responseText;
                // console.log(xmlhttp.responseText);
                return xmlhttp.responseText;
            }
        }
        xmlhttp.open("GET", elem.data, true);
        xmlhttp.send();    
        elem.hidden = "hidden";
        window.setTimeout("refreshtmp_69928660144861329_err_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_69928660144861329_err_html_obj()", 300); 
</script>
<div id="tmp_69928660144861329_err_html_obj1"></div>
<div><object id="tmp_69928660144861329_err_html_obj" data="tmp/69928660144861329.err.html", hidden="hidden"></object></div>




**RUN LOG** (interactive, `echo "Hello" > my_fifo ; echo "After writing to my_fifo"`)




<script type=text/javascript>
function refreshtmp_69928660144861329_fin_html_obj()
{
    var elem = document.getElementById("tmp_69928660144861329_fin_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_69928660144861329_fin_html_obj1");
            if (xmlhttp.readyState==4 && xmlhttp.status==200)
            {
                elem2.innerText = xmlhttp.responseText;
                // console.log(xmlhttp.responseText);
                return xmlhttp.responseText;
            }
        }
        xmlhttp.open("GET", elem.data, true);
        xmlhttp.send();    
        elem.hidden = "hidden";
        window.setTimeout("refreshtmp_69928660144861329_fin_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_69928660144861329_fin_html_obj()", 300); 
</script>
<div id="tmp_69928660144861329_fin_html_obj1"></div>
<div><object id="tmp_69928660144861329_fin_html_obj" data="tmp/69928660144861329.fin.html", hidden="hidden"></object></div>




```python
!cat my_fifo
```

    Hello


# Теперь на С
Обратите внимание, что fifo не может открыться на запись, пока ее не начнут читать.


```cpp
%%cpp write_fifo.cpp
%run gcc write_fifo.cpp -o write_fifo.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <fcntl.h>

int main() {
    fprintf(stderr, "Started\n"); fflush(stderr);
    int fd = open("my_fifo", O_WRONLY);
    assert(fd >= 0);
    fprintf(stderr, "Opened\n"); fflush(stderr);
    const char str[] = "Hello from C!\n";
    assert(write(fd, str, sizeof(str)) == sizeof(str));
    fprintf(stderr, "Written\n"); fflush(stderr);
    assert(close(fd) == 0);
    fprintf(stderr, "Closed\n"); fflush(stderr);
    return 0;
}
```


Run: `gcc write_fifo.cpp -o write_fifo.exe`



```python
%bash_async ./write_fifo.exe
```


**STDOUT** (interactive)




<script type=text/javascript>
function refreshtmp_141103302578553425_out_html_obj()
{
    var elem = document.getElementById("tmp_141103302578553425_out_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_141103302578553425_out_html_obj1");
            if (xmlhttp.readyState==4 && xmlhttp.status==200)
            {
                elem2.innerText = xmlhttp.responseText;
                // console.log(xmlhttp.responseText);
                return xmlhttp.responseText;
            }
        }
        xmlhttp.open("GET", elem.data, true);
        xmlhttp.send();    
        elem.hidden = "hidden";
        window.setTimeout("refreshtmp_141103302578553425_out_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_141103302578553425_out_html_obj()", 300); 
</script>
<div id="tmp_141103302578553425_out_html_obj1"></div>
<div><object id="tmp_141103302578553425_out_html_obj" data="tmp/141103302578553425.out.html", hidden="hidden"></object></div>




**STDERR** (interactive)




<script type=text/javascript>
function refreshtmp_141103302578553425_err_html_obj()
{
    var elem = document.getElementById("tmp_141103302578553425_err_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_141103302578553425_err_html_obj1");
            if (xmlhttp.readyState==4 && xmlhttp.status==200)
            {
                elem2.innerText = xmlhttp.responseText;
                // console.log(xmlhttp.responseText);
                return xmlhttp.responseText;
            }
        }
        xmlhttp.open("GET", elem.data, true);
        xmlhttp.send();    
        elem.hidden = "hidden";
        window.setTimeout("refreshtmp_141103302578553425_err_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_141103302578553425_err_html_obj()", 300); 
</script>
<div id="tmp_141103302578553425_err_html_obj1"></div>
<div><object id="tmp_141103302578553425_err_html_obj" data="tmp/141103302578553425.err.html", hidden="hidden"></object></div>




**RUN LOG** (interactive, `./write_fifo.exe`)




<script type=text/javascript>
function refreshtmp_141103302578553425_fin_html_obj()
{
    var elem = document.getElementById("tmp_141103302578553425_fin_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_141103302578553425_fin_html_obj1");
            if (xmlhttp.readyState==4 && xmlhttp.status==200)
            {
                elem2.innerText = xmlhttp.responseText;
                // console.log(xmlhttp.responseText);
                return xmlhttp.responseText;
            }
        }
        xmlhttp.open("GET", elem.data, true);
        xmlhttp.send();    
        elem.hidden = "hidden";
        window.setTimeout("refreshtmp_141103302578553425_fin_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_141103302578553425_fin_html_obj()", 300); 
</script>
<div id="tmp_141103302578553425_fin_html_obj1"></div>
<div><object id="tmp_141103302578553425_fin_html_obj" data="tmp/141103302578553425.fin.html", hidden="hidden"></object></div>




```python
!cat my_fifo
```

    Hello from C!
    

# Директория /proc/<pid\>/*

Интересная штука директория `/proc` это виртулаьная файловая система в которой можно получать сведения о процессах, читая из из файлов. (Это не обычные файлы на диске, а скорее некоторое view на сведения о процессах из ядра системы).

Что есть в proc: http://man7.org/linux/man-pages/man5/proc.5.html

Имеющая отношение к делу статья на хабре: https://habr.com/ru/post/209446/

Посмотрим, что можно узнать о запущенном процессе:


```python
# запустим процесс в фоне
%bash_async echo "Hello" > my_fifo 
```


**STDOUT** (interactive)




<script type=text/javascript>
function refreshtmp_216195087402002832_out_html_obj()
{
    var elem = document.getElementById("tmp_216195087402002832_out_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_216195087402002832_out_html_obj1");
            if (xmlhttp.readyState==4 && xmlhttp.status==200)
            {
                elem2.innerText = xmlhttp.responseText;
                // console.log(xmlhttp.responseText);
                return xmlhttp.responseText;
            }
        }
        xmlhttp.open("GET", elem.data, true);
        xmlhttp.send();    
        elem.hidden = "hidden";
        window.setTimeout("refreshtmp_216195087402002832_out_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_216195087402002832_out_html_obj()", 300); 
</script>
<div id="tmp_216195087402002832_out_html_obj1"></div>
<div><object id="tmp_216195087402002832_out_html_obj" data="tmp/216195087402002832.out.html", hidden="hidden"></object></div>




**STDERR** (interactive)




<script type=text/javascript>
function refreshtmp_216195087402002832_err_html_obj()
{
    var elem = document.getElementById("tmp_216195087402002832_err_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_216195087402002832_err_html_obj1");
            if (xmlhttp.readyState==4 && xmlhttp.status==200)
            {
                elem2.innerText = xmlhttp.responseText;
                // console.log(xmlhttp.responseText);
                return xmlhttp.responseText;
            }
        }
        xmlhttp.open("GET", elem.data, true);
        xmlhttp.send();    
        elem.hidden = "hidden";
        window.setTimeout("refreshtmp_216195087402002832_err_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_216195087402002832_err_html_obj()", 300); 
</script>
<div id="tmp_216195087402002832_err_html_obj1"></div>
<div><object id="tmp_216195087402002832_err_html_obj" data="tmp/216195087402002832.err.html", hidden="hidden"></object></div>




**RUN LOG** (interactive, `echo "Hello" > my_fifo `)




<script type=text/javascript>
function refreshtmp_216195087402002832_fin_html_obj()
{
    var elem = document.getElementById("tmp_216195087402002832_fin_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_216195087402002832_fin_html_obj1");
            if (xmlhttp.readyState==4 && xmlhttp.status==200)
            {
                elem2.innerText = xmlhttp.responseText;
                // console.log(xmlhttp.responseText);
                return xmlhttp.responseText;
            }
        }
        xmlhttp.open("GET", elem.data, true);
        xmlhttp.send();    
        elem.hidden = "hidden";
        window.setTimeout("refreshtmp_216195087402002832_fin_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_216195087402002832_fin_html_obj()", 300); 
</script>
<div id="tmp_216195087402002832_fin_html_obj1"></div>
<div><object id="tmp_216195087402002832_fin_html_obj" data="tmp/216195087402002832.fin.html", hidden="hidden"></object></div>




```python
!ps aux | grep Hello
```

    pechatn+  1642  0.0  0.0  19584   912 ?        S    Dec03   0:00 bash -c echo "Hello" > my_fifo && echo "Printed!"
    pechatn+ 11376  0.0  0.0  19588  1056 ?        S    10:39   0:00 bash -c echo 'Process started!' > tmp/216195087402002832.fin.html; echo "Hello" > my_fifo  ; echo "Process finished! code=$?" >> tmp/216195087402002832.fin.html
    pechatn+ 11377  0.0  0.0   4504   792 pts/25   Ss+  10:39   0:00 /bin/sh -c ps aux | grep Hello
    pechatn+ 11379  0.0  0.0  21292   940 pts/25   S+   10:39   0:00 grep Hello



```python
!cat /proc/11444/status
```

    Name:	write_fifo.exe
    Umask:	0002
    State:	S (sleeping)
    Tgid:	11444
    Ngid:	0
    Pid:	11444
    PPid:	11443
    TracerPid:	0
    Uid:	1000	1000	1000	1000
    Gid:	1000	1000	1000	1000
    FDSize:	64
    Groups:	4 24 27 30 46 113 128 130 999 1000 
    NStgid:	11444
    NSpid:	11444
    NSpgid:	4234
    NSsid:	4234
    VmPeak:	    4356 kB
    VmSize:	    4220 kB
    VmLck:	       0 kB
    VmPin:	       0 kB
    VmHWM:	     628 kB
    VmRSS:	     628 kB
    RssAnon:	      68 kB
    RssFile:	     560 kB
    RssShmem:	       0 kB
    VmData:	      48 kB
    VmStk:	     132 kB
    VmExe:	       4 kB
    VmLib:	    1952 kB
    VmPTE:	      48 kB
    VmSwap:	       0 kB
    HugetlbPages:	       0 kB
    CoreDumping:	0
    Threads:	1
    SigQ:	19/7738
    SigPnd:	0000000000000000
    ShdPnd:	0000000000000000
    SigBlk:	0000000000000000
    SigIgn:	0000000000000000
    SigCgt:	0000000000000000
    CapInh:	0000000000000000
    CapPrm:	0000000000000000
    CapEff:	0000000000000000
    CapBnd:	0000003fffffffff
    CapAmb:	0000000000000000
    NoNewPrivs:	0
    Seccomp:	0
    Speculation_Store_Bypass:	vulnerable
    Cpus_allowed:	1
    Cpus_allowed_list:	0
    Mems_allowed:	00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000001
    Mems_allowed_list:	0
    voluntary_ctxt_switches:	1
    nonvoluntary_ctxt_switches:	0



```python
!cat my_fifo
```

    Hello



```python
!ps aux | grep write_fifo 
```

    pechatn+ 11443  0.0  0.1  19596  3036 ?        S    10:43   0:00 bash -c echo 'Process started!' > tmp/141103302578553425.fin.html; ./write_fifo.exe ; echo "Process finished! code=$?" >> tmp/141103302578553425.fin.html
    pechatn+ 11444  0.0  0.0   4220   628 ?        S    10:43   0:00 ./write_fifo.exe
    pechatn+ 11448  0.0  0.0   4504   784 pts/25   Ss+  10:43   0:00 /bin/sh -c ps aux | grep write_fifo 
    pechatn+ 11450  0.0  0.0  21292  1016 pts/25   S+   10:43   0:00 grep write_fifo



```python

```

# Пример применения на моей практике
Только на семинаре


```python

```
