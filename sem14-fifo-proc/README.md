```python
# look at tools/set_up_magics.ipynb
get_ipython().run_cell('# one_liner_str\n\nget_ipython().run_cell_magic(\'javascript\', \'\', \'// setup cpp code highlighting\\nIPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\\\'reg\\\':[/^%%cpp/]} ;\')\n\n# creating magics\nfrom IPython.core.magic import register_cell_magic, register_line_magic\nfrom IPython.display import display, Markdown, HTML\nimport argparse\nfrom subprocess import Popen, PIPE\nimport random\nimport sys\n\n@register_cell_magic\ndef save_file(args_str, cell, line_comment_start="#"):\n    parser = argparse.ArgumentParser()\n    parser.add_argument("fname")\n    parser.add_argument("--ejudge-style", action="store_true")\n    args = parser.parse_args(args_str.split())\n    \n    cell = cell if cell[-1] == \'\\n\' or args.no_eof_newline else cell + "\\n"\n    cmds = []\n    with open(args.fname, "w") as f:\n        f.write(line_comment_start + " %%cpp " + args_str + "\\n")\n        for line in cell.split("\\n"):\n            line_to_write = (line if not args.ejudge_style else line.rstrip()) + "\\n"\n            if line.startswith("%"):\n                run_prefix = "%run "\n                assert line.startswith(run_prefix)\n                cmds.append(line[len(run_prefix):].strip())\n                f.write(line_comment_start + " " + line_to_write)\n            else:\n                f.write(line_to_write)\n        f.write("" if not args.ejudge_style else line_comment_start + r" line without \\n")\n    for cmd in cmds:\n        display(Markdown("Run: `%s`" % cmd))\n        get_ipython().system(cmd)\n\n@register_cell_magic\ndef cpp(fname, cell):\n    save_file(fname, cell, "//")\n\n@register_cell_magic\ndef asm(fname, cell):\n    save_file(fname, cell, "//")\n    \n@register_cell_magic\ndef makefile(fname, cell):\n    assert not fname\n    save_file("makefile", cell.replace(" " * 4, "\\t"))\n        \n@register_line_magic\ndef p(line):\n    try:\n        expr, comment = line.split(" #")\n        display(Markdown("`{} = {}`  # {}".format(expr.strip(), eval(expr), comment.strip())))\n    except:\n        display(Markdown("{} = {}".format(line, eval(line))))\n        \ndef show_file(file, clear_at_begin=False):\n    if clear_at_begin:\n        get_ipython().system("truncate --size 0 " + file)\n    obj = file.replace(\'.\', \'_\').replace(\'/\', \'_\') + "_obj"\n    display(HTML(\'\'\'\n        <script type=text/javascript>\n        function refresh__OBJ__()\n        {\n            var elem = document.getElementById("__OBJ__");\n            if (elem) {\n                var xmlhttp=new XMLHttpRequest();\n                xmlhttp.onreadystatechange=function()\n                {\n                    var elem2 = document.getElementById("__OBJ__1");\n                    if (xmlhttp.readyState==4 && xmlhttp.status==200)\n                    {\n                        elem2.innerText = xmlhttp.responseText;\n                        // console.log(xmlhttp.responseText);\n                        return xmlhttp.responseText;\n                    }\n                }\n                xmlhttp.open("GET", elem.data, true);\n                xmlhttp.send();    \n                elem.hidden = "hidden";\n                window.setTimeout("refresh__OBJ__()", 300); \n            }\n        }\n        window.setTimeout("refresh__OBJ__()", 300); \n        </script>\n        <div id="__OBJ__1"></div>\n        <div><object id="__OBJ__" data="__FILE__", hidden="hidden"></object></div>\n        \'\'\'.replace("__OBJ__", obj)\n           .replace("__FILE__", file)))\n    \ndef bash_popen_terminate_all():\n    for p in globals().get("bash_popen_list", []):\n        print("Terminate pid=" + str(p.pid), file=sys.stderr)\n        p.terminate()\n        globals()["bash_popen_list"] = []\n\nbash_popen_terminate_all()  \n\ndef bash_popen(cmd):\n    original_cmd = cmd\n    h = "tmp/" + str(random.randint(0, 1e18))\n    \n    stdout = open(h + ".out.html", "wb")\n    display(Markdown("**STDOUT** (interactive)"))\n    show_file(h + ".out.html", clear_at_begin=True)\n    \n    stdout = open(h + ".err.html", "wb")\n    display(Markdown("**STDERR** (interactive)"))\n    show_file(h + ".err.html", clear_at_begin=True)\n    \n    fin_file = h + ".fin.html"\n    cmd = "echo \'Process started!\' > " + fin_file + "; " + cmd + " ; echo \\"Process finished! code=$?\\" >> " + fin_file\n    display(Markdown("**RUN LOG** (interactive, `" + original_cmd + "`)"))\n    show_file(h + ".fin.html", clear_at_begin=True)\n    \n    p = Popen(["bash", "-c", cmd], stdin=PIPE, stdout=stdout, stderr=stdout)\n    bash_popen_list.append(p)\n    return p\n\n\n@register_line_magic\ndef bash_async(line):\n    bash_popen(line)\n    \ndef make_oneliner():\n    return \'# look at tools/set_up_magics.ipynb\\nget_ipython().run_cell(%s)\\nNone\' % repr(one_liner_str)\n')
None
```


    <IPython.core.display.Javascript object>


# named FIFO

Ранее мы познакомились с пайпами (анонимными fifo (далее буду называть просто pipe'ами)). Теперь же посмотрим на именованые.
Отличие в том, что именоваванные fifo (дальше буду называть просто fifo) являются файлами в файловой системе linux. Соответственно они могут существовать, не будучи открытыми какой-либо программой. Как и файл их можно удалить.

Как создать из консоли - `man mkfifo`, как создать из кода на C - `man 3 mkfifo`. Чтение и запись в fifo происходит так же как и с обычным файлом.

**Важно:** fifo, это файл читаемый двумя процессами и важно, кто открыл процесс на запись, кто на чтение. Например, fifo не может быть открыта на запись, пока кто-нибудь не открыл ее на чтение.


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
function refreshtmp_326396582312714894_out_html_obj()
{
    var elem = document.getElementById("tmp_326396582312714894_out_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_326396582312714894_out_html_obj1");
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
        window.setTimeout("refreshtmp_326396582312714894_out_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_326396582312714894_out_html_obj()", 300); 
</script>
<div id="tmp_326396582312714894_out_html_obj1"></div>
<div><object id="tmp_326396582312714894_out_html_obj" data="tmp/326396582312714894.out.html", hidden="hidden"></object></div>




**STDERR** (interactive)




<script type=text/javascript>
function refreshtmp_326396582312714894_err_html_obj()
{
    var elem = document.getElementById("tmp_326396582312714894_err_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_326396582312714894_err_html_obj1");
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
        window.setTimeout("refreshtmp_326396582312714894_err_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_326396582312714894_err_html_obj()", 300); 
</script>
<div id="tmp_326396582312714894_err_html_obj1"></div>
<div><object id="tmp_326396582312714894_err_html_obj" data="tmp/326396582312714894.err.html", hidden="hidden"></object></div>




**RUN LOG** (interactive, `echo "Hello" > my_fifo ; echo "After writing to my_fifo"`)




<script type=text/javascript>
function refreshtmp_326396582312714894_fin_html_obj()
{
    var elem = document.getElementById("tmp_326396582312714894_fin_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_326396582312714894_fin_html_obj1");
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
        window.setTimeout("refreshtmp_326396582312714894_fin_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_326396582312714894_fin_html_obj()", 300); 
</script>
<div id="tmp_326396582312714894_fin_html_obj1"></div>
<div><object id="tmp_326396582312714894_fin_html_obj" data="tmp/326396582312714894.fin.html", hidden="hidden"></object></div>




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
function refreshtmp_100686501059335292_out_html_obj()
{
    var elem = document.getElementById("tmp_100686501059335292_out_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_100686501059335292_out_html_obj1");
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
        window.setTimeout("refreshtmp_100686501059335292_out_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_100686501059335292_out_html_obj()", 300); 
</script>
<div id="tmp_100686501059335292_out_html_obj1"></div>
<div><object id="tmp_100686501059335292_out_html_obj" data="tmp/100686501059335292.out.html", hidden="hidden"></object></div>




**STDERR** (interactive)




<script type=text/javascript>
function refreshtmp_100686501059335292_err_html_obj()
{
    var elem = document.getElementById("tmp_100686501059335292_err_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_100686501059335292_err_html_obj1");
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
        window.setTimeout("refreshtmp_100686501059335292_err_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_100686501059335292_err_html_obj()", 300); 
</script>
<div id="tmp_100686501059335292_err_html_obj1"></div>
<div><object id="tmp_100686501059335292_err_html_obj" data="tmp/100686501059335292.err.html", hidden="hidden"></object></div>




**RUN LOG** (interactive, `./write_fifo.exe`)




<script type=text/javascript>
function refreshtmp_100686501059335292_fin_html_obj()
{
    var elem = document.getElementById("tmp_100686501059335292_fin_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_100686501059335292_fin_html_obj1");
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
        window.setTimeout("refreshtmp_100686501059335292_fin_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_100686501059335292_fin_html_obj()", 300); 
</script>
<div id="tmp_100686501059335292_fin_html_obj1"></div>
<div><object id="tmp_100686501059335292_fin_html_obj" data="tmp/100686501059335292.fin.html", hidden="hidden"></object></div>




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
function refreshtmp_635857202682272018_out_html_obj()
{
    var elem = document.getElementById("tmp_635857202682272018_out_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_635857202682272018_out_html_obj1");
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
        window.setTimeout("refreshtmp_635857202682272018_out_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_635857202682272018_out_html_obj()", 300); 
</script>
<div id="tmp_635857202682272018_out_html_obj1"></div>
<div><object id="tmp_635857202682272018_out_html_obj" data="tmp/635857202682272018.out.html", hidden="hidden"></object></div>




**STDERR** (interactive)




<script type=text/javascript>
function refreshtmp_635857202682272018_err_html_obj()
{
    var elem = document.getElementById("tmp_635857202682272018_err_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_635857202682272018_err_html_obj1");
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
        window.setTimeout("refreshtmp_635857202682272018_err_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_635857202682272018_err_html_obj()", 300); 
</script>
<div id="tmp_635857202682272018_err_html_obj1"></div>
<div><object id="tmp_635857202682272018_err_html_obj" data="tmp/635857202682272018.err.html", hidden="hidden"></object></div>




**RUN LOG** (interactive, `echo "Hello" > my_fifo `)




<script type=text/javascript>
function refreshtmp_635857202682272018_fin_html_obj()
{
    var elem = document.getElementById("tmp_635857202682272018_fin_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_635857202682272018_fin_html_obj1");
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
        window.setTimeout("refreshtmp_635857202682272018_fin_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_635857202682272018_fin_html_obj()", 300); 
</script>
<div id="tmp_635857202682272018_fin_html_obj1"></div>
<div><object id="tmp_635857202682272018_fin_html_obj" data="tmp/635857202682272018.fin.html", hidden="hidden"></object></div>




```python
!ps aux | grep Hello
```

    pechatn+  1642  0.0  0.0  19584   912 ?        S    Dec05   0:00 bash -c echo "Hello" > my_fifo && echo "Printed!"
    pechatn+ 24470  0.0  0.0  19588   908 ?        S    20:14   0:00 bash -c echo 'Process started!' > tmp/681703421791017912.fin.html; echo "Hello" > my_fifo ; echo "After writing to my_fifo" ; echo "Process finished! code=$?" >> tmp/681703421791017912.fin.html
    pechatn+ 24524  0.0  0.0  19588   900 ?        S    20:15   0:00 bash -c echo 'Process started!' > tmp/635857202682272018.fin.html; echo "Hello" > my_fifo  ; echo "Process finished! code=$?" >> tmp/635857202682272018.fin.html
    pechatn+ 24525  0.0  0.0   4504   844 pts/19   Ss+  20:15   0:00 /bin/sh -c ps aux | grep Hello
    pechatn+ 24527  0.0  0.0  21292   992 pts/19   S+   20:15   0:00 grep Hello



```python
!cat /proc/24524/status
```

    Name:	bash
    Umask:	0002
    State:	S (sleeping)
    Tgid:	24524
    Ngid:	0
    Pid:	24524
    PPid:	4234
    TracerPid:	0
    Uid:	1000	1000	1000	1000
    Gid:	1000	1000	1000	1000
    FDSize:	128
    Groups:	4 24 27 30 46 113 128 130 999 1000 
    NStgid:	24524
    NSpid:	24524
    NSpgid:	4234
    NSsid:	4234
    VmPeak:	   19588 kB
    VmSize:	   19588 kB
    VmLck:	       0 kB
    VmPin:	       0 kB
    VmHWM:	     900 kB
    VmRSS:	     900 kB
    RssAnon:	     104 kB
    RssFile:	     796 kB
    RssShmem:	       0 kB
    VmData:	     168 kB
    VmStk:	     132 kB
    VmExe:	     976 kB
    VmLib:	    2112 kB
    VmPTE:	      64 kB
    VmSwap:	       0 kB
    HugetlbPages:	       0 kB
    CoreDumping:	0
    Threads:	1
    SigQ:	22/7738
    SigPnd:	0000000000000000
    ShdPnd:	0000000000000000
    SigBlk:	0000000000000000
    SigIgn:	0000000000000004
    SigCgt:	0000000000010000
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
    nonvoluntary_ctxt_switches:	2



```python
!cat my_fifo
```

    Hello



```python
!ps aux | grep write_fifo 
```

    pechatn+ 24536  0.0  0.0   4504   788 pts/19   Ss+  20:16   0:00 /bin/sh -c ps aux | grep write_fifo 
    pechatn+ 24538  0.0  0.0  21292   988 pts/19   S+   20:16   0:00 grep write_fifo



```python

```

# Пример применения на моей практике
Только на семинаре


```python

```


```python

```

# Пример, почему важно правильно открывать fifo


```python
import os
```


```python
fd = os.open("my_fifo", os.O_RDWR) # создаем ненужное открытие файла на запись
```


```python
%bash_async cat my_fifo
```


**STDOUT** (interactive)




<script type=text/javascript>
function refreshtmp_584204787735188028_out_html_obj()
{
    var elem = document.getElementById("tmp_584204787735188028_out_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_584204787735188028_out_html_obj1");
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
        window.setTimeout("refreshtmp_584204787735188028_out_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_584204787735188028_out_html_obj()", 300); 
</script>
<div id="tmp_584204787735188028_out_html_obj1"></div>
<div><object id="tmp_584204787735188028_out_html_obj" data="tmp/584204787735188028.out.html", hidden="hidden"></object></div>




**STDERR** (interactive)




<script type=text/javascript>
function refreshtmp_584204787735188028_err_html_obj()
{
    var elem = document.getElementById("tmp_584204787735188028_err_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_584204787735188028_err_html_obj1");
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
        window.setTimeout("refreshtmp_584204787735188028_err_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_584204787735188028_err_html_obj()", 300); 
</script>
<div id="tmp_584204787735188028_err_html_obj1"></div>
<div><object id="tmp_584204787735188028_err_html_obj" data="tmp/584204787735188028.err.html", hidden="hidden"></object></div>




**RUN LOG** (interactive, `cat my_fifo`)




<script type=text/javascript>
function refreshtmp_584204787735188028_fin_html_obj()
{
    var elem = document.getElementById("tmp_584204787735188028_fin_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_584204787735188028_fin_html_obj1");
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
        window.setTimeout("refreshtmp_584204787735188028_fin_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_584204787735188028_fin_html_obj()", 300); 
</script>
<div id="tmp_584204787735188028_fin_html_obj1"></div>
<div><object id="tmp_584204787735188028_fin_html_obj" data="tmp/584204787735188028.fin.html", hidden="hidden"></object></div>




```python
!echo "Hello 1" > my_fifo
!echo "Hello 2" > my_fifo
!echo "Hello 3" > my_fifo
```


```python
os.close(fd) # Только после закрытия дескриптора процесс 'cat my_fifo' завершится. Так как закроется fifo
```

### Если же ненужного чтения не создавать:


```python
%bash_async cat my_fifo
```


**STDOUT** (interactive)




<script type=text/javascript>
function refreshtmp_186072983456055787_out_html_obj()
{
    var elem = document.getElementById("tmp_186072983456055787_out_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_186072983456055787_out_html_obj1");
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
        window.setTimeout("refreshtmp_186072983456055787_out_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_186072983456055787_out_html_obj()", 300); 
</script>
<div id="tmp_186072983456055787_out_html_obj1"></div>
<div><object id="tmp_186072983456055787_out_html_obj" data="tmp/186072983456055787.out.html", hidden="hidden"></object></div>




**STDERR** (interactive)




<script type=text/javascript>
function refreshtmp_186072983456055787_err_html_obj()
{
    var elem = document.getElementById("tmp_186072983456055787_err_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_186072983456055787_err_html_obj1");
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
        window.setTimeout("refreshtmp_186072983456055787_err_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_186072983456055787_err_html_obj()", 300); 
</script>
<div id="tmp_186072983456055787_err_html_obj1"></div>
<div><object id="tmp_186072983456055787_err_html_obj" data="tmp/186072983456055787.err.html", hidden="hidden"></object></div>




**RUN LOG** (interactive, `cat my_fifo`)




<script type=text/javascript>
function refreshtmp_186072983456055787_fin_html_obj()
{
    var elem = document.getElementById("tmp_186072983456055787_fin_html_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem2 = document.getElementById("tmp_186072983456055787_fin_html_obj1");
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
        window.setTimeout("refreshtmp_186072983456055787_fin_html_obj()", 300); 
    }
}
window.setTimeout("refreshtmp_186072983456055787_fin_html_obj()", 300); 
</script>
<div id="tmp_186072983456055787_fin_html_obj1"></div>
<div><object id="tmp_186072983456055787_fin_html_obj" data="tmp/186072983456055787.fin.html", hidden="hidden"></object></div>




```python
!echo "Hello 1" > my_fifo
```


```python
!echo "Hello 2" > my_fifo # то все зависнет тут
```

    ^C
    /bin/sh: 1: cannot create my_fifo: Interrupted system call



```python
!echo "Hello 3" > my_fifo
```
