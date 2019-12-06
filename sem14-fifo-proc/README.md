```python
# look at tools/set_up_magics.ipynb
get_ipython().run_cell('# one_liner_str\n\nget_ipython().run_cell_magic(\'javascript\', \'\', \'// setup cpp code highlighting\\nIPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\\\'reg\\\':[/^%%cpp/]} ;\')\n\n# creating magics\nfrom IPython.core.magic import register_cell_magic, register_line_magic\nfrom IPython.display import display, Markdown, HTML\nimport argparse\nfrom subprocess import Popen, PIPE\nimport random\nimport sys\nimport os\nimport shutil\nimport shlex\n\n@register_cell_magic\ndef save_file(args_str, cell, line_comment_start="#"):\n    parser = argparse.ArgumentParser()\n    parser.add_argument("fname")\n    parser.add_argument("--ejudge-style", action="store_true")\n    args = parser.parse_args(args_str.split())\n    \n    cell = cell if cell[-1] == \'\\n\' or args.no_eof_newline else cell + "\\n"\n    cmds = []\n    with open(args.fname, "w") as f:\n        f.write(line_comment_start + " %%cpp " + args_str + "\\n")\n        for line in cell.split("\\n"):\n            line_to_write = (line if not args.ejudge_style else line.rstrip()) + "\\n"\n            if line.startswith("%"):\n                run_prefix = "%run "\n                assert line.startswith(run_prefix)\n                cmds.append(line[len(run_prefix):].strip())\n                f.write(line_comment_start + " " + line_to_write)\n            else:\n                f.write(line_to_write)\n        f.write("" if not args.ejudge_style else line_comment_start + r" line without \\n")\n    for cmd in cmds:\n        display(Markdown("Run: `%s`" % cmd))\n        get_ipython().system(cmd)\n\n@register_cell_magic\ndef cpp(fname, cell):\n    save_file(fname, cell, "//")\n\n@register_cell_magic\ndef asm(fname, cell):\n    save_file(fname, cell, "//")\n    \n@register_cell_magic\ndef makefile(fname, cell):\n    assert not fname\n    save_file("makefile", cell.replace(" " * 4, "\\t"))\n        \n@register_line_magic\ndef p(line):\n    try:\n        expr, comment = line.split(" #")\n        display(Markdown("`{} = {}`  # {}".format(expr.strip(), eval(expr), comment.strip())))\n    except:\n        display(Markdown("{} = {}".format(line, eval(line))))\n        \ndef show_file(file, clear_at_begin=True, return_html_string=False):\n    if clear_at_begin:\n        get_ipython().system("truncate --size 0 " + file)\n    obj = file.replace(\'.\', \'_\').replace(\'/\', \'_\') + "_obj"\n    html_string = \'\'\'\n        <script type=text/javascript>\n        var entrance___OBJ__ = 0;\n        function refresh__OBJ__()\n        {\n            entrance___OBJ__ -= 1;\n            var elem = document.getElementById("__OBJ__");\n            if (elem) {\n                var xmlhttp=new XMLHttpRequest();\n                xmlhttp.onreadystatechange=function()\n                {\n                    var elem = document.getElementById("__OBJ__");\n                    //console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___OBJ__);\n                    if (elem && xmlhttp.readyState==4 && xmlhttp.status==200)\n                    {\n                        elem.innerText = xmlhttp.responseText;\n                        if (!entrance___OBJ__) {\n                            entrance___OBJ__ += 1;\n                            // console.log("req");\n                            window.setTimeout("refresh__OBJ__()", 300); \n                        }\n                        return xmlhttp.responseText;\n                    } \n                }\n                xmlhttp.open("GET", "__FILE__", true);\n                xmlhttp.send();     \n            }\n        }\n        \n        if (!entrance___OBJ__) {\n            entrance___OBJ__ += 1;\n            refresh__OBJ__(); \n        }\n        \n        </script>\n        \n        <font color="white"> <tt>\n        <p id="__OBJ__" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>\n        </tt> </font>\n        \'\'\'.replace("__OBJ__", obj).replace("__FILE__", file)\n    if return_html_string:\n        return html_string\n    display(HTML(html_string))\n    \nBASH_POPEN_TMP_DIR = "./bash_popen_tmp"\n    \ndef bash_popen_terminate_all():\n    for p in globals().get("bash_popen_list", []):\n        print("Terminate pid=" + str(p.pid), file=sys.stderr)\n        p.terminate()\n    globals()["bash_popen_list"] = []\n    if os.path.exists(BASH_POPEN_TMP_DIR):\n        shutil.rmtree(BASH_POPEN_TMP_DIR)\n\nbash_popen_terminate_all()  \n\ndef bash_popen(cmd):\n    if not os.path.exists(BASH_POPEN_TMP_DIR):\n        os.mkdir(BASH_POPEN_TMP_DIR)\n    h = os.path.join(BASH_POPEN_TMP_DIR, str(random.randint(0, 1e18)))\n    stdout_file = h + ".out.html"\n    stderr_file = h + ".err.html"\n    run_log_file = h + ".fin.html"\n    \n    stdout = open(stdout_file, "wb")\n    stdout = open(stderr_file, "wb")\n    \n    html = """\n    <table width="100%">\n    <colgroup>\n       <col span="1" style="width: 70px;">\n       <col span="1">\n    </colgroup>    \n    <tbody>\n      <tr> <td><b>STDOUT</b> <td> {stdout}  \n      <tr> <td><b>STDERR</b> <td> {stderr}  \n      <tr> <td><b>RUN LOG</b> <td> {run_log}  \n    </tbody>\n    </table>\n    """.format(\n        stdout=show_file(stdout_file, return_html_string=True),\n        stderr=show_file(stderr_file, return_html_string=True),\n        run_log=show_file(run_log_file, return_html_string=True),\n    )\n    \n    cmd = """\n        bash -c {cmd} &\n        pid=$!\n        echo "Process started! pid=${{pid}}" > {run_log_file}\n        wait ${{pid}}\n        echo "Process finished! exit_code=$?" >> {run_log_file}\n    """.format(cmd=shlex.quote(cmd), run_log_file=run_log_file)\n    # print(cmd)\n    display(HTML(html))\n    \n    p = Popen(["bash", "-c", cmd], stdin=PIPE, stdout=stdout, stderr=stdout)\n    \n    bash_popen_list.append(p)\n    return p\n\n\n@register_line_magic\ndef bash_async(line):\n    bash_popen(line)\n    \ndef make_oneliner():\n    return \'# look at tools/set_up_magics.ipynb\\nget_ipython().run_cell(%s)\\nNone\' % repr(one_liner_str)\n')
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



<table width="100%">
<colgroup>
   <col span="1" style="width: 70px;">
   <col span="1">
</colgroup>    
<tbody>
  <tr> <td><b>STDOUT</b> <td> 
    <script type=text/javascript>
    var entrance___bash_popen_tmp_841531200595707846_out_html_obj = 0;
    function refresh__bash_popen_tmp_841531200595707846_out_html_obj()
    {
        entrance___bash_popen_tmp_841531200595707846_out_html_obj -= 1;
        var elem = document.getElementById("__bash_popen_tmp_841531200595707846_out_html_obj");
        if (elem) {
            var xmlhttp=new XMLHttpRequest();
            xmlhttp.onreadystatechange=function()
            {
                var elem = document.getElementById("__bash_popen_tmp_841531200595707846_out_html_obj");
                //console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___bash_popen_tmp_841531200595707846_out_html_obj);
                if (elem && xmlhttp.readyState==4 && xmlhttp.status==200)
                {
                    elem.innerText = xmlhttp.responseText;
                    if (!entrance___bash_popen_tmp_841531200595707846_out_html_obj) {
                        entrance___bash_popen_tmp_841531200595707846_out_html_obj += 1;
                        // console.log("req");
                        window.setTimeout("refresh__bash_popen_tmp_841531200595707846_out_html_obj()", 300); 
                    }
                    return xmlhttp.responseText;
                } 
            }
            xmlhttp.open("GET", "./bash_popen_tmp/841531200595707846.out.html", true);
            xmlhttp.send();     
        }
    }

    if (!entrance___bash_popen_tmp_841531200595707846_out_html_obj) {
        entrance___bash_popen_tmp_841531200595707846_out_html_obj += 1;
        refresh__bash_popen_tmp_841531200595707846_out_html_obj(); 
    }

    </script>

    <font color="white"> <tt>
    <p id="__bash_popen_tmp_841531200595707846_out_html_obj" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>
    </tt> </font>
      
  <tr> <td><b>STDERR</b> <td> 
    <script type=text/javascript>
    var entrance___bash_popen_tmp_841531200595707846_err_html_obj = 0;
    function refresh__bash_popen_tmp_841531200595707846_err_html_obj()
    {
        entrance___bash_popen_tmp_841531200595707846_err_html_obj -= 1;
        var elem = document.getElementById("__bash_popen_tmp_841531200595707846_err_html_obj");
        if (elem) {
            var xmlhttp=new XMLHttpRequest();
            xmlhttp.onreadystatechange=function()
            {
                var elem = document.getElementById("__bash_popen_tmp_841531200595707846_err_html_obj");
                //console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___bash_popen_tmp_841531200595707846_err_html_obj);
                if (elem && xmlhttp.readyState==4 && xmlhttp.status==200)
                {
                    elem.innerText = xmlhttp.responseText;
                    if (!entrance___bash_popen_tmp_841531200595707846_err_html_obj) {
                        entrance___bash_popen_tmp_841531200595707846_err_html_obj += 1;
                        // console.log("req");
                        window.setTimeout("refresh__bash_popen_tmp_841531200595707846_err_html_obj()", 300); 
                    }
                    return xmlhttp.responseText;
                } 
            }
            xmlhttp.open("GET", "./bash_popen_tmp/841531200595707846.err.html", true);
            xmlhttp.send();     
        }
    }

    if (!entrance___bash_popen_tmp_841531200595707846_err_html_obj) {
        entrance___bash_popen_tmp_841531200595707846_err_html_obj += 1;
        refresh__bash_popen_tmp_841531200595707846_err_html_obj(); 
    }

    </script>

    <font color="white"> <tt>
    <p id="__bash_popen_tmp_841531200595707846_err_html_obj" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>
    </tt> </font>
      
  <tr> <td><b>RUN LOG</b> <td> 
    <script type=text/javascript>
    var entrance___bash_popen_tmp_841531200595707846_fin_html_obj = 0;
    function refresh__bash_popen_tmp_841531200595707846_fin_html_obj()
    {
        entrance___bash_popen_tmp_841531200595707846_fin_html_obj -= 1;
        var elem = document.getElementById("__bash_popen_tmp_841531200595707846_fin_html_obj");
        if (elem) {
            var xmlhttp=new XMLHttpRequest();
            xmlhttp.onreadystatechange=function()
            {
                var elem = document.getElementById("__bash_popen_tmp_841531200595707846_fin_html_obj");
                //console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___bash_popen_tmp_841531200595707846_fin_html_obj);
                if (elem && xmlhttp.readyState==4 && xmlhttp.status==200)
                {
                    elem.innerText = xmlhttp.responseText;
                    if (!entrance___bash_popen_tmp_841531200595707846_fin_html_obj) {
                        entrance___bash_popen_tmp_841531200595707846_fin_html_obj += 1;
                        // console.log("req");
                        window.setTimeout("refresh__bash_popen_tmp_841531200595707846_fin_html_obj()", 300); 
                    }
                    return xmlhttp.responseText;
                } 
            }
            xmlhttp.open("GET", "./bash_popen_tmp/841531200595707846.fin.html", true);
            xmlhttp.send();     
        }
    }

    if (!entrance___bash_popen_tmp_841531200595707846_fin_html_obj) {
        entrance___bash_popen_tmp_841531200595707846_fin_html_obj += 1;
        refresh__bash_popen_tmp_841531200595707846_fin_html_obj(); 
    }

    </script>

    <font color="white"> <tt>
    <p id="__bash_popen_tmp_841531200595707846_fin_html_obj" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>
    </tt> </font>
      
</tbody>
</table>




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



<table width="100%">
<colgroup>
   <col span="1" style="width: 70px;">
   <col span="1">
</colgroup>    
<tbody>
  <tr> <td><b>STDOUT</b> <td> 
    <script type=text/javascript>
    var entrance___bash_popen_tmp_82959761933755356_out_html_obj = 0;
    function refresh__bash_popen_tmp_82959761933755356_out_html_obj()
    {
        entrance___bash_popen_tmp_82959761933755356_out_html_obj -= 1;
        var elem = document.getElementById("__bash_popen_tmp_82959761933755356_out_html_obj");
        if (elem) {
            var xmlhttp=new XMLHttpRequest();
            xmlhttp.onreadystatechange=function()
            {
                var elem = document.getElementById("__bash_popen_tmp_82959761933755356_out_html_obj");
                //console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___bash_popen_tmp_82959761933755356_out_html_obj);
                if (elem && xmlhttp.readyState==4 && xmlhttp.status==200)
                {
                    elem.innerText = xmlhttp.responseText;
                    if (!entrance___bash_popen_tmp_82959761933755356_out_html_obj) {
                        entrance___bash_popen_tmp_82959761933755356_out_html_obj += 1;
                        // console.log("req");
                        window.setTimeout("refresh__bash_popen_tmp_82959761933755356_out_html_obj()", 300); 
                    }
                    return xmlhttp.responseText;
                } 
            }
            xmlhttp.open("GET", "./bash_popen_tmp/82959761933755356.out.html", true);
            xmlhttp.send();     
        }
    }

    if (!entrance___bash_popen_tmp_82959761933755356_out_html_obj) {
        entrance___bash_popen_tmp_82959761933755356_out_html_obj += 1;
        refresh__bash_popen_tmp_82959761933755356_out_html_obj(); 
    }

    </script>

    <font color="white"> <tt>
    <p id="__bash_popen_tmp_82959761933755356_out_html_obj" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>
    </tt> </font>
      
  <tr> <td><b>STDERR</b> <td> 
    <script type=text/javascript>
    var entrance___bash_popen_tmp_82959761933755356_err_html_obj = 0;
    function refresh__bash_popen_tmp_82959761933755356_err_html_obj()
    {
        entrance___bash_popen_tmp_82959761933755356_err_html_obj -= 1;
        var elem = document.getElementById("__bash_popen_tmp_82959761933755356_err_html_obj");
        if (elem) {
            var xmlhttp=new XMLHttpRequest();
            xmlhttp.onreadystatechange=function()
            {
                var elem = document.getElementById("__bash_popen_tmp_82959761933755356_err_html_obj");
                //console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___bash_popen_tmp_82959761933755356_err_html_obj);
                if (elem && xmlhttp.readyState==4 && xmlhttp.status==200)
                {
                    elem.innerText = xmlhttp.responseText;
                    if (!entrance___bash_popen_tmp_82959761933755356_err_html_obj) {
                        entrance___bash_popen_tmp_82959761933755356_err_html_obj += 1;
                        // console.log("req");
                        window.setTimeout("refresh__bash_popen_tmp_82959761933755356_err_html_obj()", 300); 
                    }
                    return xmlhttp.responseText;
                } 
            }
            xmlhttp.open("GET", "./bash_popen_tmp/82959761933755356.err.html", true);
            xmlhttp.send();     
        }
    }

    if (!entrance___bash_popen_tmp_82959761933755356_err_html_obj) {
        entrance___bash_popen_tmp_82959761933755356_err_html_obj += 1;
        refresh__bash_popen_tmp_82959761933755356_err_html_obj(); 
    }

    </script>

    <font color="white"> <tt>
    <p id="__bash_popen_tmp_82959761933755356_err_html_obj" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>
    </tt> </font>
      
  <tr> <td><b>RUN LOG</b> <td> 
    <script type=text/javascript>
    var entrance___bash_popen_tmp_82959761933755356_fin_html_obj = 0;
    function refresh__bash_popen_tmp_82959761933755356_fin_html_obj()
    {
        entrance___bash_popen_tmp_82959761933755356_fin_html_obj -= 1;
        var elem = document.getElementById("__bash_popen_tmp_82959761933755356_fin_html_obj");
        if (elem) {
            var xmlhttp=new XMLHttpRequest();
            xmlhttp.onreadystatechange=function()
            {
                var elem = document.getElementById("__bash_popen_tmp_82959761933755356_fin_html_obj");
                //console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___bash_popen_tmp_82959761933755356_fin_html_obj);
                if (elem && xmlhttp.readyState==4 && xmlhttp.status==200)
                {
                    elem.innerText = xmlhttp.responseText;
                    if (!entrance___bash_popen_tmp_82959761933755356_fin_html_obj) {
                        entrance___bash_popen_tmp_82959761933755356_fin_html_obj += 1;
                        // console.log("req");
                        window.setTimeout("refresh__bash_popen_tmp_82959761933755356_fin_html_obj()", 300); 
                    }
                    return xmlhttp.responseText;
                } 
            }
            xmlhttp.open("GET", "./bash_popen_tmp/82959761933755356.fin.html", true);
            xmlhttp.send();     
        }
    }

    if (!entrance___bash_popen_tmp_82959761933755356_fin_html_obj) {
        entrance___bash_popen_tmp_82959761933755356_fin_html_obj += 1;
        refresh__bash_popen_tmp_82959761933755356_fin_html_obj(); 
    }

    </script>

    <font color="white"> <tt>
    <p id="__bash_popen_tmp_82959761933755356_fin_html_obj" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>
    </tt> </font>
      
</tbody>
</table>




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



<table width="100%">
<colgroup>
   <col span="1" style="width: 70px;">
   <col span="1">
</colgroup>    
<tbody>
  <tr> <td><b>STDOUT</b> <td> 
    <script type=text/javascript>
    var entrance___bash_popen_tmp_572005684422340190_out_html_obj = 0;
    function refresh__bash_popen_tmp_572005684422340190_out_html_obj()
    {
        entrance___bash_popen_tmp_572005684422340190_out_html_obj -= 1;
        var elem = document.getElementById("__bash_popen_tmp_572005684422340190_out_html_obj");
        if (elem) {
            var xmlhttp=new XMLHttpRequest();
            xmlhttp.onreadystatechange=function()
            {
                var elem = document.getElementById("__bash_popen_tmp_572005684422340190_out_html_obj");
                //console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___bash_popen_tmp_572005684422340190_out_html_obj);
                if (elem && xmlhttp.readyState==4 && xmlhttp.status==200)
                {
                    elem.innerText = xmlhttp.responseText;
                    if (!entrance___bash_popen_tmp_572005684422340190_out_html_obj) {
                        entrance___bash_popen_tmp_572005684422340190_out_html_obj += 1;
                        // console.log("req");
                        window.setTimeout("refresh__bash_popen_tmp_572005684422340190_out_html_obj()", 300); 
                    }
                    return xmlhttp.responseText;
                } 
            }
            xmlhttp.open("GET", "./bash_popen_tmp/572005684422340190.out.html", true);
            xmlhttp.send();     
        }
    }

    if (!entrance___bash_popen_tmp_572005684422340190_out_html_obj) {
        entrance___bash_popen_tmp_572005684422340190_out_html_obj += 1;
        refresh__bash_popen_tmp_572005684422340190_out_html_obj(); 
    }

    </script>

    <font color="white"> <tt>
    <p id="__bash_popen_tmp_572005684422340190_out_html_obj" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>
    </tt> </font>
      
  <tr> <td><b>STDERR</b> <td> 
    <script type=text/javascript>
    var entrance___bash_popen_tmp_572005684422340190_err_html_obj = 0;
    function refresh__bash_popen_tmp_572005684422340190_err_html_obj()
    {
        entrance___bash_popen_tmp_572005684422340190_err_html_obj -= 1;
        var elem = document.getElementById("__bash_popen_tmp_572005684422340190_err_html_obj");
        if (elem) {
            var xmlhttp=new XMLHttpRequest();
            xmlhttp.onreadystatechange=function()
            {
                var elem = document.getElementById("__bash_popen_tmp_572005684422340190_err_html_obj");
                //console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___bash_popen_tmp_572005684422340190_err_html_obj);
                if (elem && xmlhttp.readyState==4 && xmlhttp.status==200)
                {
                    elem.innerText = xmlhttp.responseText;
                    if (!entrance___bash_popen_tmp_572005684422340190_err_html_obj) {
                        entrance___bash_popen_tmp_572005684422340190_err_html_obj += 1;
                        // console.log("req");
                        window.setTimeout("refresh__bash_popen_tmp_572005684422340190_err_html_obj()", 300); 
                    }
                    return xmlhttp.responseText;
                } 
            }
            xmlhttp.open("GET", "./bash_popen_tmp/572005684422340190.err.html", true);
            xmlhttp.send();     
        }
    }

    if (!entrance___bash_popen_tmp_572005684422340190_err_html_obj) {
        entrance___bash_popen_tmp_572005684422340190_err_html_obj += 1;
        refresh__bash_popen_tmp_572005684422340190_err_html_obj(); 
    }

    </script>

    <font color="white"> <tt>
    <p id="__bash_popen_tmp_572005684422340190_err_html_obj" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>
    </tt> </font>
      
  <tr> <td><b>RUN LOG</b> <td> 
    <script type=text/javascript>
    var entrance___bash_popen_tmp_572005684422340190_fin_html_obj = 0;
    function refresh__bash_popen_tmp_572005684422340190_fin_html_obj()
    {
        entrance___bash_popen_tmp_572005684422340190_fin_html_obj -= 1;
        var elem = document.getElementById("__bash_popen_tmp_572005684422340190_fin_html_obj");
        if (elem) {
            var xmlhttp=new XMLHttpRequest();
            xmlhttp.onreadystatechange=function()
            {
                var elem = document.getElementById("__bash_popen_tmp_572005684422340190_fin_html_obj");
                //console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___bash_popen_tmp_572005684422340190_fin_html_obj);
                if (elem && xmlhttp.readyState==4 && xmlhttp.status==200)
                {
                    elem.innerText = xmlhttp.responseText;
                    if (!entrance___bash_popen_tmp_572005684422340190_fin_html_obj) {
                        entrance___bash_popen_tmp_572005684422340190_fin_html_obj += 1;
                        // console.log("req");
                        window.setTimeout("refresh__bash_popen_tmp_572005684422340190_fin_html_obj()", 300); 
                    }
                    return xmlhttp.responseText;
                } 
            }
            xmlhttp.open("GET", "./bash_popen_tmp/572005684422340190.fin.html", true);
            xmlhttp.send();     
        }
    }

    if (!entrance___bash_popen_tmp_572005684422340190_fin_html_obj) {
        entrance___bash_popen_tmp_572005684422340190_fin_html_obj += 1;
        refresh__bash_popen_tmp_572005684422340190_fin_html_obj(); 
    }

    </script>

    <font color="white"> <tt>
    <p id="__bash_popen_tmp_572005684422340190_fin_html_obj" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>
    </tt> </font>
      
</tbody>
</table>




```python
!ps aux | grep Hello
```

    pechatn+  2518  0.0  0.1  19600  3120 ?        S    21:17   0:00 bash -c echo 'Process started!' > tmp/515226009711462029.fin.html; echo "Hello" > my_fifo ; echo "After writing to my_fifo" ; echo "Process finished! code=$?" >> tmp/515226009711462029.fin.html
    pechatn+  2726  0.0  0.1  19596  3224 ?        S    21:32   0:00 bash -c echo "Hello" > my_fifo ; echo "After writing to my_fifo"
    pechatn+  2799  0.0  0.1  19596  3200 ?        S    21:37   0:00 bash -c echo "Hello" > my_fifo ; echo "After writing to my_fifo"
    pechatn+  2836  0.0  0.1  19596  3236 ?        S    21:40   0:00 bash -c echo "Hello" > my_fifo ; echo "After writing to my_fifo"
    pechatn+  2927  0.0  0.1  19596  3164 ?        S    21:43   0:00 bash -c echo "Hello" > my_fifo ; echo "After writing to my_fifo"
    pechatn+  2998  0.0  0.1  19596  3156 ?        S    21:45   0:00 bash -c echo "Hello" > my_fifo ; echo "After writing to my_fifo"
    pechatn+  3012  0.0  0.1  19596  3204 ?        S    21:45   0:00 bash -c echo "Hello" > my_fifo ; echo "After writing to my_fifo"
    pechatn+  3028  0.0  0.1  19596  3120 ?        S    21:46   0:00 bash -c echo "Hello" > my_fifo ; echo "After writing to my_fifo"
    pechatn+  3122  0.0  0.1  19596  3148 ?        S    21:49   0:00 bash -c echo "Hello" > my_fifo ; echo "After writing to my_fifo"
    pechatn+  3285  0.0  0.1  19596  3236 ?        S    21:56   0:00 bash -c echo "Hello" > my_fifo ; echo "After writing to my_fifo"
    pechatn+  3318  0.0  0.1  19596  3196 ?        S    22:00   0:00 bash -c echo "Hello" > my_fifo ; echo "After writing to my_fifo"
    pechatn+  3377  0.0  0.1  19596  3204 ?        S    22:01   0:00 bash -c echo "Hello" > my_fifo ; echo "After wddfsffsfsdfdssdriting to my_fifo"
    pechatn+  3409  0.0  0.1  19596  3236 ?        S    22:02   0:00 bash -c echo "Hello" > my_fifo ; echo "After wddfsffsfsdfdssdriting to my_fifo"
    pechatn+  3572  0.0  0.1  19596  3132 ?        S    22:11   0:00 bash -c echo "Hello $USER" > my_fifo ; echo After writing to my_fifo
    pechatn+  3591  0.0  0.1  19596  3148 ?        S    22:12   0:00 bash -c echo "Hello $USER" > my_fifo ; echo After writing to my_fifo
    pechatn+  3619  0.0  0.1  19596  3160 ?        S    22:14   0:00 bash -c echo "Hello $USER" > my_fifo ; echo After writing to my_fifo
    pechatn+  3631  0.0  0.1  19596  3196 ?        S    22:14   0:00 bash -c echo "Hello $USER" > my_fifo ; echo After writing to my_fifo
    pechatn+  3649  0.0  0.1  19596  3224 ?        S    22:15   0:00 bash -c echo "Hello $USER" > my_fifo ; echo After writing to my_fifo
    pechatn+  3663  0.0  0.1  19596  3156 ?        S    22:15   0:00 bash -c echo "Hello $USER" > my_fifo ; echo After writing to my_fifo
    pechatn+  3677  0.0  0.1  19596  3160 ?        S    22:16   0:00 bash -c echo "Hello $USER" > my_fifo ; echo After writing to my_fifo
    pechatn+  3689  0.0  0.1  19596  3160 ?        S    22:16   0:00 bash -c echo "Hello $USER" > my_fifo ; echo After writing to my_fifo
    pechatn+  3869  0.0  0.1  19596  3160 ?        S    22:26   0:00 bash -c echo "Hello $USER" > my_fifo ; echo \After writing to my_fifo\
    pechatn+  3981  0.0  0.1  19596  3120 ?        S    22:31   0:00 bash -c echo "Hello $USER" > my_fifo ; echo 'After writing to my_fifo'
    pechatn+  3994  0.0  0.1  19596  3164 ?        S    22:31   0:00 bash -c echo "Hello $USER" > my_fifo ; echo 'After writing to my_fifo'
    pechatn+  4006  0.0  0.1  19596  3200 ?        S    22:32   0:00 bash -c echo "Hello $USER" > my_fifo ; echo 'After writing to my_fifo'
    pechatn+  4020  0.0  0.1  19596  3164 ?        S    22:32   0:00 bash -c echo "Hello $USER" > my_fifo ; echo 'After writing to my_fifo'
    pechatn+  4032  0.0  0.1  19596  3260 ?        S    22:33   0:00 bash -c echo "Hello $USER" > my_fifo ; echo 'After writing to my_fifo'
    pechatn+  4048  0.0  0.1  19596  3164 ?        S    22:34   0:00 bash -c echo "Hello $USER" > my_fifo ; echo 'After writing to my_fifo'
    pechatn+  4144  0.0  0.1  19596  3164 ?        S    22:37   0:00 bash -c          bash -c 'echo "Hello" > my_fifo ' &         pid=$!         echo "Process started! pid=${pid}" > ./bash_popen_tmp/572005684422340190.fin.html         wait ${pid}         echo "Process finished! exit_code=$?" >> ./bash_popen_tmp/572005684422340190.fin.html     
    pechatn+  4145  0.0  0.1  19596  3224 ?        S    22:37   0:00 bash -c echo "Hello" > my_fifo 
    pechatn+  4146  0.0  0.0   4504   796 pts/17   Ss+  22:37   0:00 /bin/sh -c ps aux | grep Hello
    pechatn+  4148  0.0  0.0  21292  1020 pts/17   S+   22:37   0:00 grep Hello



```python
!cat /proc/4145/status
```

    Name:	bash
    Umask:	0002
    State:	S (sleeping)
    Tgid:	4145
    Ngid:	0
    Pid:	4145
    PPid:	4144
    TracerPid:	0
    Uid:	1000	1000	1000	1000
    Gid:	1000	1000	1000	1000
    FDSize:	64
    Groups:	4 24 27 30 46 113 128 130 999 1000 
    NStgid:	4145
    NSpid:	4145
    NSpgid:	4079
    NSsid:	4079
    VmPeak:	   19596 kB
    VmSize:	   19596 kB
    VmLck:	       0 kB
    VmPin:	       0 kB
    VmHWM:	    3224 kB
    VmRSS:	    3224 kB
    RssAnon:	     200 kB
    RssFile:	    3024 kB
    RssShmem:	       0 kB
    VmData:	     176 kB
    VmStk:	     132 kB
    VmExe:	     976 kB
    VmLib:	    2112 kB
    VmPTE:	      72 kB
    VmSwap:	       0 kB
    HugetlbPages:	       0 kB
    CoreDumping:	0
    Threads:	1
    SigQ:	3/7735
    SigPnd:	0000000000000000
    ShdPnd:	0000000000000000
    SigBlk:	0000000000000000
    SigIgn:	0000000000000006
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
    nonvoluntary_ctxt_switches:	1



```python
!cat my_fifo
```

    Hello



```python
!ps aux | grep write_fifo 
```

    pechatn+  4157  0.0  0.0   4504   840 pts/17   Ss+  22:37   0:00 /bin/sh -c ps aux | grep write_fifo 
    pechatn+  4159  0.0  0.0  21292   924 pts/17   S+   22:37   0:00 grep write_fifo



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



<table width="100%">
<colgroup>
   <col span="1" style="width: 70px;">
   <col span="1">
</colgroup>    
<tbody>
  <tr> <td><b>STDOUT</b> <td> 
    <script type=text/javascript>
    var entrance___bash_popen_tmp_799402355760673100_out_html_obj = 0;
    function refresh__bash_popen_tmp_799402355760673100_out_html_obj()
    {
        entrance___bash_popen_tmp_799402355760673100_out_html_obj -= 1;
        var elem = document.getElementById("__bash_popen_tmp_799402355760673100_out_html_obj");
        if (elem) {
            var xmlhttp=new XMLHttpRequest();
            xmlhttp.onreadystatechange=function()
            {
                var elem = document.getElementById("__bash_popen_tmp_799402355760673100_out_html_obj");
                //console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___bash_popen_tmp_799402355760673100_out_html_obj);
                if (elem && xmlhttp.readyState==4 && xmlhttp.status==200)
                {
                    elem.innerText = xmlhttp.responseText;
                    if (!entrance___bash_popen_tmp_799402355760673100_out_html_obj) {
                        entrance___bash_popen_tmp_799402355760673100_out_html_obj += 1;
                        // console.log("req");
                        window.setTimeout("refresh__bash_popen_tmp_799402355760673100_out_html_obj()", 300); 
                    }
                    return xmlhttp.responseText;
                } 
            }
            xmlhttp.open("GET", "./bash_popen_tmp/799402355760673100.out.html", true);
            xmlhttp.send();     
        }
    }

    if (!entrance___bash_popen_tmp_799402355760673100_out_html_obj) {
        entrance___bash_popen_tmp_799402355760673100_out_html_obj += 1;
        refresh__bash_popen_tmp_799402355760673100_out_html_obj(); 
    }

    </script>

    <font color="white"> <tt>
    <p id="__bash_popen_tmp_799402355760673100_out_html_obj" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>
    </tt> </font>
      
  <tr> <td><b>STDERR</b> <td> 
    <script type=text/javascript>
    var entrance___bash_popen_tmp_799402355760673100_err_html_obj = 0;
    function refresh__bash_popen_tmp_799402355760673100_err_html_obj()
    {
        entrance___bash_popen_tmp_799402355760673100_err_html_obj -= 1;
        var elem = document.getElementById("__bash_popen_tmp_799402355760673100_err_html_obj");
        if (elem) {
            var xmlhttp=new XMLHttpRequest();
            xmlhttp.onreadystatechange=function()
            {
                var elem = document.getElementById("__bash_popen_tmp_799402355760673100_err_html_obj");
                //console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___bash_popen_tmp_799402355760673100_err_html_obj);
                if (elem && xmlhttp.readyState==4 && xmlhttp.status==200)
                {
                    elem.innerText = xmlhttp.responseText;
                    if (!entrance___bash_popen_tmp_799402355760673100_err_html_obj) {
                        entrance___bash_popen_tmp_799402355760673100_err_html_obj += 1;
                        // console.log("req");
                        window.setTimeout("refresh__bash_popen_tmp_799402355760673100_err_html_obj()", 300); 
                    }
                    return xmlhttp.responseText;
                } 
            }
            xmlhttp.open("GET", "./bash_popen_tmp/799402355760673100.err.html", true);
            xmlhttp.send();     
        }
    }

    if (!entrance___bash_popen_tmp_799402355760673100_err_html_obj) {
        entrance___bash_popen_tmp_799402355760673100_err_html_obj += 1;
        refresh__bash_popen_tmp_799402355760673100_err_html_obj(); 
    }

    </script>

    <font color="white"> <tt>
    <p id="__bash_popen_tmp_799402355760673100_err_html_obj" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>
    </tt> </font>
      
  <tr> <td><b>RUN LOG</b> <td> 
    <script type=text/javascript>
    var entrance___bash_popen_tmp_799402355760673100_fin_html_obj = 0;
    function refresh__bash_popen_tmp_799402355760673100_fin_html_obj()
    {
        entrance___bash_popen_tmp_799402355760673100_fin_html_obj -= 1;
        var elem = document.getElementById("__bash_popen_tmp_799402355760673100_fin_html_obj");
        if (elem) {
            var xmlhttp=new XMLHttpRequest();
            xmlhttp.onreadystatechange=function()
            {
                var elem = document.getElementById("__bash_popen_tmp_799402355760673100_fin_html_obj");
                //console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___bash_popen_tmp_799402355760673100_fin_html_obj);
                if (elem && xmlhttp.readyState==4 && xmlhttp.status==200)
                {
                    elem.innerText = xmlhttp.responseText;
                    if (!entrance___bash_popen_tmp_799402355760673100_fin_html_obj) {
                        entrance___bash_popen_tmp_799402355760673100_fin_html_obj += 1;
                        // console.log("req");
                        window.setTimeout("refresh__bash_popen_tmp_799402355760673100_fin_html_obj()", 300); 
                    }
                    return xmlhttp.responseText;
                } 
            }
            xmlhttp.open("GET", "./bash_popen_tmp/799402355760673100.fin.html", true);
            xmlhttp.send();     
        }
    }

    if (!entrance___bash_popen_tmp_799402355760673100_fin_html_obj) {
        entrance___bash_popen_tmp_799402355760673100_fin_html_obj += 1;
        refresh__bash_popen_tmp_799402355760673100_fin_html_obj(); 
    }

    </script>

    <font color="white"> <tt>
    <p id="__bash_popen_tmp_799402355760673100_fin_html_obj" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>
    </tt> </font>
      
</tbody>
</table>




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



<table width="100%">
<colgroup>
   <col span="1" style="width: 70px;">
   <col span="1">
</colgroup>    
<tbody>
  <tr> <td><b>STDOUT</b> <td> 
    <script type=text/javascript>
    var entrance___bash_popen_tmp_804562509990530205_out_html_obj = 0;
    function refresh__bash_popen_tmp_804562509990530205_out_html_obj()
    {
        entrance___bash_popen_tmp_804562509990530205_out_html_obj -= 1;
        var elem = document.getElementById("__bash_popen_tmp_804562509990530205_out_html_obj");
        if (elem) {
            var xmlhttp=new XMLHttpRequest();
            xmlhttp.onreadystatechange=function()
            {
                var elem = document.getElementById("__bash_popen_tmp_804562509990530205_out_html_obj");
                //console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___bash_popen_tmp_804562509990530205_out_html_obj);
                if (elem && xmlhttp.readyState==4 && xmlhttp.status==200)
                {
                    elem.innerText = xmlhttp.responseText;
                    if (!entrance___bash_popen_tmp_804562509990530205_out_html_obj) {
                        entrance___bash_popen_tmp_804562509990530205_out_html_obj += 1;
                        // console.log("req");
                        window.setTimeout("refresh__bash_popen_tmp_804562509990530205_out_html_obj()", 300); 
                    }
                    return xmlhttp.responseText;
                } 
            }
            xmlhttp.open("GET", "./bash_popen_tmp/804562509990530205.out.html", true);
            xmlhttp.send();     
        }
    }

    if (!entrance___bash_popen_tmp_804562509990530205_out_html_obj) {
        entrance___bash_popen_tmp_804562509990530205_out_html_obj += 1;
        refresh__bash_popen_tmp_804562509990530205_out_html_obj(); 
    }

    </script>

    <font color="white"> <tt>
    <p id="__bash_popen_tmp_804562509990530205_out_html_obj" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>
    </tt> </font>
      
  <tr> <td><b>STDERR</b> <td> 
    <script type=text/javascript>
    var entrance___bash_popen_tmp_804562509990530205_err_html_obj = 0;
    function refresh__bash_popen_tmp_804562509990530205_err_html_obj()
    {
        entrance___bash_popen_tmp_804562509990530205_err_html_obj -= 1;
        var elem = document.getElementById("__bash_popen_tmp_804562509990530205_err_html_obj");
        if (elem) {
            var xmlhttp=new XMLHttpRequest();
            xmlhttp.onreadystatechange=function()
            {
                var elem = document.getElementById("__bash_popen_tmp_804562509990530205_err_html_obj");
                //console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___bash_popen_tmp_804562509990530205_err_html_obj);
                if (elem && xmlhttp.readyState==4 && xmlhttp.status==200)
                {
                    elem.innerText = xmlhttp.responseText;
                    if (!entrance___bash_popen_tmp_804562509990530205_err_html_obj) {
                        entrance___bash_popen_tmp_804562509990530205_err_html_obj += 1;
                        // console.log("req");
                        window.setTimeout("refresh__bash_popen_tmp_804562509990530205_err_html_obj()", 300); 
                    }
                    return xmlhttp.responseText;
                } 
            }
            xmlhttp.open("GET", "./bash_popen_tmp/804562509990530205.err.html", true);
            xmlhttp.send();     
        }
    }

    if (!entrance___bash_popen_tmp_804562509990530205_err_html_obj) {
        entrance___bash_popen_tmp_804562509990530205_err_html_obj += 1;
        refresh__bash_popen_tmp_804562509990530205_err_html_obj(); 
    }

    </script>

    <font color="white"> <tt>
    <p id="__bash_popen_tmp_804562509990530205_err_html_obj" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>
    </tt> </font>
      
  <tr> <td><b>RUN LOG</b> <td> 
    <script type=text/javascript>
    var entrance___bash_popen_tmp_804562509990530205_fin_html_obj = 0;
    function refresh__bash_popen_tmp_804562509990530205_fin_html_obj()
    {
        entrance___bash_popen_tmp_804562509990530205_fin_html_obj -= 1;
        var elem = document.getElementById("__bash_popen_tmp_804562509990530205_fin_html_obj");
        if (elem) {
            var xmlhttp=new XMLHttpRequest();
            xmlhttp.onreadystatechange=function()
            {
                var elem = document.getElementById("__bash_popen_tmp_804562509990530205_fin_html_obj");
                //console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___bash_popen_tmp_804562509990530205_fin_html_obj);
                if (elem && xmlhttp.readyState==4 && xmlhttp.status==200)
                {
                    elem.innerText = xmlhttp.responseText;
                    if (!entrance___bash_popen_tmp_804562509990530205_fin_html_obj) {
                        entrance___bash_popen_tmp_804562509990530205_fin_html_obj += 1;
                        // console.log("req");
                        window.setTimeout("refresh__bash_popen_tmp_804562509990530205_fin_html_obj()", 300); 
                    }
                    return xmlhttp.responseText;
                } 
            }
            xmlhttp.open("GET", "./bash_popen_tmp/804562509990530205.fin.html", true);
            xmlhttp.send();     
        }
    }

    if (!entrance___bash_popen_tmp_804562509990530205_fin_html_obj) {
        entrance___bash_popen_tmp_804562509990530205_fin_html_obj += 1;
        refresh__bash_popen_tmp_804562509990530205_fin_html_obj(); 
    }

    </script>

    <font color="white"> <tt>
    <p id="__bash_popen_tmp_804562509990530205_fin_html_obj" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>
    </tt> </font>
      
</tbody>
</table>




```python
!echo "Hello 1" > my_fifo
```


```python
!echo "Hello 2" > my_fifo # то все зависнет тут
```

    ^C
    /bin/sh: 1: cannot create my_fifo: Interrupted system call



```python

```
