```python
# look at tools/set_up_magics.ipynb
yandex_metrica_allowed = True ; get_ipython().run_cell('# one_liner_str\n\nget_ipython().run_cell_magic(\'javascript\', \'\', \'// setup cpp code highlighting\\nIPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\\\'reg\\\':[/^%%cpp/]} ;\')\n\n# creating magics\nfrom IPython.core.magic import register_cell_magic, register_line_magic\nfrom IPython.display import display, Markdown, HTML\nimport argparse\nfrom subprocess import Popen, PIPE\nimport random\nimport sys\nimport os\nimport re\nimport signal\nimport shutil\nimport shlex\nimport glob\n\n@register_cell_magic\ndef save_file(args_str, cell, line_comment_start="#"):\n    parser = argparse.ArgumentParser()\n    parser.add_argument("fname")\n    parser.add_argument("--ejudge-style", action="store_true")\n    args = parser.parse_args(args_str.split())\n    \n    cell = cell if cell[-1] == \'\\n\' or args.no_eof_newline else cell + "\\n"\n    cmds = []\n    with open(args.fname, "w") as f:\n        f.write(line_comment_start + " %%cpp " + args_str + "\\n")\n        for line in cell.split("\\n"):\n            line_to_write = (line if not args.ejudge_style else line.rstrip()) + "\\n"\n            if line.startswith("%"):\n                run_prefix = "%run "\n                if line.startswith(run_prefix):\n                    cmds.append(line[len(run_prefix):].strip())\n                    f.write(line_comment_start + " " + line_to_write)\n                    continue\n                run_prefix = "%# "\n                if line.startswith(run_prefix):\n                    f.write(line_comment_start + " " + line_to_write)\n                    continue\n                raise Exception("Unknown %%save_file subcommand: \'%s\'" % line)\n            else:\n                f.write(line_to_write)\n        f.write("" if not args.ejudge_style else line_comment_start + r" line without \\n")\n    for cmd in cmds:\n        display(Markdown("Run: `%s`" % cmd))\n        get_ipython().system(cmd)\n\n@register_cell_magic\ndef cpp(fname, cell):\n    save_file(fname, cell, "//")\n\n@register_cell_magic\ndef asm(fname, cell):\n    save_file(fname, cell, "//")\n    \n@register_cell_magic\ndef makefile(fname, cell):\n    assert not fname\n    save_file("makefile", cell.replace(" " * 4, "\\t"))\n        \n@register_line_magic\ndef p(line):\n    try:\n        expr, comment = line.split(" #")\n        display(Markdown("`{} = {}`  # {}".format(expr.strip(), eval(expr), comment.strip())))\n    except:\n        display(Markdown("{} = {}".format(line, eval(line))))\n        \ndef show_file(file, clear_at_begin=True, return_html_string=False):\n    if clear_at_begin:\n        get_ipython().system("truncate --size 0 " + file)\n    obj = file.replace(\'.\', \'_\').replace(\'/\', \'_\') + "_obj"\n    html_string = \'\'\'\n        <!--MD_BEGIN_FILTER-->\n        <script type=text/javascript>\n        var entrance___OBJ__ = 0;\n        var errors___OBJ__ = 0;\n        function refresh__OBJ__()\n        {\n            entrance___OBJ__ -= 1;\n            var elem = document.getElementById("__OBJ__");\n            if (elem) {\n                var xmlhttp=new XMLHttpRequest();\n                xmlhttp.onreadystatechange=function()\n                {\n                    var elem = document.getElementById("__OBJ__");\n                    console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___OBJ__);\n                    if (elem && xmlhttp.readyState==4) {\n                        if (xmlhttp.status==200)\n                        {\n                            errors___OBJ__ = 0;\n                            if (!entrance___OBJ__) {\n                                elem.innerText = xmlhttp.responseText;\n                                entrance___OBJ__ += 1;\n                                console.log("req");\n                                window.setTimeout("refresh__OBJ__()", 300); \n                            }\n                            return xmlhttp.responseText;\n                        } else {\n                            errors___OBJ__ += 1;\n                            if (errors___OBJ__ < 10 && !entrance___OBJ__) {\n                                entrance___OBJ__ += 1;\n                                console.log("req");\n                                window.setTimeout("refresh__OBJ__()", 300); \n                            }\n                        }\n                    }\n                }\n                xmlhttp.open("GET", "__FILE__", true);\n                xmlhttp.setRequestHeader("Cache-Control", "no-cache");\n                xmlhttp.send();     \n            }\n        }\n        \n        if (!entrance___OBJ__) {\n            entrance___OBJ__ += 1;\n            refresh__OBJ__(); \n        }\n        </script>\n        \n        <font color="white"> <tt>\n        <p id="__OBJ__" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>\n        </tt> </font>\n        <!--MD_END_FILTER-->\n        <!--MD_FROM_FILE __FILE__ -->\n        \'\'\'.replace("__OBJ__", obj).replace("__FILE__", file)\n    if return_html_string:\n        return html_string\n    display(HTML(html_string))\n    \nBASH_POPEN_TMP_DIR = "./bash_popen_tmp"\n    \ndef bash_popen_terminate_all():\n    for p in globals().get("bash_popen_list", []):\n        print("Terminate pid=" + str(p.pid), file=sys.stderr)\n        p.terminate()\n    globals()["bash_popen_list"] = []\n    if os.path.exists(BASH_POPEN_TMP_DIR):\n        shutil.rmtree(BASH_POPEN_TMP_DIR)\n\nbash_popen_terminate_all()  \n\ndef bash_popen(cmd):\n    if not os.path.exists(BASH_POPEN_TMP_DIR):\n        os.mkdir(BASH_POPEN_TMP_DIR)\n    h = os.path.join(BASH_POPEN_TMP_DIR, str(random.randint(0, 1e18)))\n    stdout_file = h + ".out.html"\n    stderr_file = h + ".err.html"\n    run_log_file = h + ".fin.html"\n    \n    stdout = open(stdout_file, "wb")\n    stdout = open(stderr_file, "wb")\n    \n    html = """\n    <table width="100%">\n    <colgroup>\n       <col span="1" style="width: 70px;">\n       <col span="1">\n    </colgroup>    \n    <tbody>\n      <tr> <td><b>STDOUT</b></td> <td> {stdout} </td> </tr>\n      <tr> <td><b>STDERR</b></td> <td> {stderr} </td> </tr>\n      <tr> <td><b>RUN LOG</b></td> <td> {run_log} </td> </tr>\n    </tbody>\n    </table>\n    """.format(\n        stdout=show_file(stdout_file, return_html_string=True),\n        stderr=show_file(stderr_file, return_html_string=True),\n        run_log=show_file(run_log_file, return_html_string=True),\n    )\n    \n    cmd = """\n        bash -c {cmd} &\n        pid=$!\n        echo "Process started! pid=${{pid}}" > {run_log_file}\n        wait ${{pid}}\n        echo "Process finished! exit_code=$?" >> {run_log_file}\n    """.format(cmd=shlex.quote(cmd), run_log_file=run_log_file)\n    # print(cmd)\n    display(HTML(html))\n    \n    p = Popen(["bash", "-c", cmd], stdin=PIPE, stdout=stdout, stderr=stdout)\n    \n    bash_popen_list.append(p)\n    return p\n\n\n@register_line_magic\ndef bash_async(line):\n    bash_popen(line)\n    \n    \ndef show_log_file(file, return_html_string=False):\n    obj = file.replace(\'.\', \'_\').replace(\'/\', \'_\') + "_obj"\n    html_string = \'\'\'\n        <!--MD_BEGIN_FILTER-->\n        <script type=text/javascript>\n        var entrance___OBJ__ = 0;\n        var errors___OBJ__ = 0;\n        function halt__OBJ__(elem, color)\n        {\n            elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    \n        }\n        function refresh__OBJ__()\n        {\n            entrance___OBJ__ -= 1;\n            if (entrance___OBJ__ < 0) {\n                entrance___OBJ__ = 0;\n            }\n            var elem = document.getElementById("__OBJ__");\n            if (elem) {\n                var xmlhttp=new XMLHttpRequest();\n                xmlhttp.onreadystatechange=function()\n                {\n                    var elem = document.getElementById("__OBJ__");\n                    console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___OBJ__);\n                    if (elem && xmlhttp.readyState==4) {\n                        if (xmlhttp.status==200)\n                        {\n                            errors___OBJ__ = 0;\n                            if (!entrance___OBJ__) {\n                                if (elem.innerHTML != xmlhttp.responseText) {\n                                    elem.innerHTML = xmlhttp.responseText;\n                                }\n                                if (elem.innerHTML.includes("Process finished.")) {\n                                    halt__OBJ__(elem, "#333333");\n                                } else {\n                                    entrance___OBJ__ += 1;\n                                    console.log("req");\n                                    window.setTimeout("refresh__OBJ__()", 300); \n                                }\n                            }\n                            return xmlhttp.responseText;\n                        } else {\n                            errors___OBJ__ += 1;\n                            if (!entrance___OBJ__) {\n                                if (errors___OBJ__ < 6) {\n                                    entrance___OBJ__ += 1;\n                                    console.log("req");\n                                    window.setTimeout("refresh__OBJ__()", 300); \n                                } else {\n                                    halt__OBJ__(elem, "#994444");\n                                }\n                            }\n                        }\n                    }\n                }\n                xmlhttp.open("GET", "__FILE__", true);\n                xmlhttp.setRequestHeader("Cache-Control", "no-cache");\n                xmlhttp.send();     \n            }\n        }\n        \n        if (!entrance___OBJ__) {\n            entrance___OBJ__ += 1;\n            refresh__OBJ__(); \n        }\n        </script>\n\n        <p id="__OBJ__" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">\n        </p>\n        \n        </font>\n        <!--MD_END_FILTER-->\n        <!--MD_FROM_FILE __FILE__.md -->\n        \'\'\'.replace("__OBJ__", obj).replace("__FILE__", file)\n    if return_html_string:\n        return html_string\n    display(HTML(html_string))\n\n    \nclass TInteractiveLauncher:\n    tmp_path = "./interactive_launcher_tmp"\n    def __init__(self, cmd):\n        try:\n            os.mkdir(TInteractiveLauncher.tmp_path)\n        except:\n            pass\n        name = str(random.randint(0, 1e18))\n        self.inq_path = os.path.join(TInteractiveLauncher.tmp_path, name + ".inq")\n        self.log_path = os.path.join(TInteractiveLauncher.tmp_path, name + ".log")\n        \n        os.mkfifo(self.inq_path)\n        open(self.log_path, \'w\').close()\n        open(self.log_path + ".md", \'w\').close()\n\n        self.pid = os.fork()\n        if self.pid == -1:\n            print("Error")\n        if self.pid == 0:\n            exe_cands = glob.glob("../tools/launcher.py") + glob.glob("../../tools/launcher.py")\n            assert(len(exe_cands) == 1)\n            assert(os.execvp("python3", ["python3", exe_cands[0], "-l", self.log_path, "-i", self.inq_path, "-c", cmd]) == 0)\n        self.inq_f = open(self.inq_path, "w")\n        interactive_launcher_opened_set.add(self.pid)\n        show_log_file(self.log_path)\n\n    def write(self, s):\n        s = s.encode()\n        assert len(s) == os.write(self.inq_f.fileno(), s)\n        \n    def get_pid(self):\n        n = 100\n        for i in range(n):\n            try:\n                return int(re.findall(r"PID = (\\d+)", open(self.log_path).readline())[0])\n            except:\n                if i + 1 == n:\n                    raise\n                time.sleep(0.1)\n        \n    def input_queue_path(self):\n        return self.inq_path\n        \n    def close(self):\n        self.inq_f.close()\n        os.waitpid(self.pid, 0)\n        os.remove(self.inq_path)\n        # os.remove(self.log_path)\n        self.inq_path = None\n        self.log_path = None \n        interactive_launcher_opened_set.remove(self.pid)\n        self.pid = None\n        \n    @staticmethod\n    def terminate_all():\n        if "interactive_launcher_opened_set" not in globals():\n            globals()["interactive_launcher_opened_set"] = set()\n        global interactive_launcher_opened_set\n        for pid in interactive_launcher_opened_set:\n            print("Terminate pid=" + str(pid), file=sys.stderr)\n            os.kill(pid, signal.SIGKILL)\n            os.waitpid(pid, 0)\n        interactive_launcher_opened_set = set()\n        if os.path.exists(TInteractiveLauncher.tmp_path):\n            shutil.rmtree(TInteractiveLauncher.tmp_path)\n    \nTInteractiveLauncher.terminate_all()\n   \nyandex_metrica_allowed = bool(globals().get("yandex_metrica_allowed", False))\nif yandex_metrica_allowed:\n    display(HTML(\'\'\'<!-- YANDEX_METRICA_BEGIN -->\n    <script type="text/javascript" >\n       (function(m,e,t,r,i,k,a){m[i]=m[i]||function(){(m[i].a=m[i].a||[]).push(arguments)};\n       m[i].l=1*new Date();k=e.createElement(t),a=e.getElementsByTagName(t)[0],k.async=1,k.src=r,a.parentNode.insertBefore(k,a)})\n       (window, document, "script", "https://mc.yandex.ru/metrika/tag.js", "ym");\n\n       ym(59260609, "init", {\n            clickmap:true,\n            trackLinks:true,\n            accurateTrackBounce:true\n       });\n    </script>\n    <noscript><div><img src="https://mc.yandex.ru/watch/59260609" style="position:absolute; left:-9999px;" alt="" /></div></noscript>\n    <!-- YANDEX_METRICA_END -->\'\'\'))\n\ndef make_oneliner():\n    html_text = \'("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований. Если вы не хотите, чтобы по вам собиралась статистика, исправьте: yandex_metrica_allowed = False" if yandex_metrica_allowed else "")\'\n    html_text += \' + "<""!-- MAGICS_SETUP_PRINTING_END -->"\'\n    return \'\'.join([\n        \'# look at tools/set_up_magics.ipynb\\n\',\n        \'yandex_metrica_allowed = True ; get_ipython().run_cell(%s);\' % repr(one_liner_str),\n        \'display(HTML(%s))\' % html_text,\n        \' #\'\'MAGICS_SETUP_END\'\n    ])\n       \n\n');display(HTML(("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований. Если вы не хотите, чтобы по вам собиралась статистика, исправьте: yandex_metrica_allowed = False" if yandex_metrica_allowed else "") + "<""!-- MAGICS_SETUP_PRINTING_END -->")) #MAGICS_SETUP_END
```


    <IPython.core.display.Javascript object>



<!-- YANDEX_METRICA_BEGIN -->
    <script type="text/javascript" >
       (function(m,e,t,r,i,k,a){m[i]=m[i]||function(){(m[i].a=m[i].a||[]).push(arguments)};
       m[i].l=1*new Date();k=e.createElement(t),a=e.getElementsByTagName(t)[0],k.async=1,k.src=r,a.parentNode.insertBefore(k,a)})
       (window, document, "script", "https://mc.yandex.ru/metrika/tag.js", "ym");

       ym(59260609, "init", {
            clickmap:true,
            trackLinks:true,
            accurateTrackBounce:true
       });
    </script>
    <noscript><div><img src="https://mc.yandex.ru/watch/59260609" style="position:absolute; left:-9999px;" alt="" /></div></noscript>
    <!-- YANDEX_METRICA_END -->



В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований. Если вы не хотите, чтобы по вам собиралась статистика, исправьте: yandex_metrica_allowed = False<!-- MAGICS_SETUP_PRINTING_END -->


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
a = TInteractiveLauncher(
    'echo "Hello" > my_fifo ; echo "After writing to my_fifo"'
)
```



<!--MD_BEGIN_FILTER-->
<script type=text/javascript>
var entrance___interactive_launcher_tmp_36680464884053384_log_obj = 0;
var errors___interactive_launcher_tmp_36680464884053384_log_obj = 0;
function halt__interactive_launcher_tmp_36680464884053384_log_obj(elem, color)
{
    elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    
}
function refresh__interactive_launcher_tmp_36680464884053384_log_obj()
{
    entrance___interactive_launcher_tmp_36680464884053384_log_obj -= 1;
    if (entrance___interactive_launcher_tmp_36680464884053384_log_obj < 0) {
        entrance___interactive_launcher_tmp_36680464884053384_log_obj = 0;
    }
    var elem = document.getElementById("__interactive_launcher_tmp_36680464884053384_log_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem = document.getElementById("__interactive_launcher_tmp_36680464884053384_log_obj");
            console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___interactive_launcher_tmp_36680464884053384_log_obj);
            if (elem && xmlhttp.readyState==4) {
                if (xmlhttp.status==200)
                {
                    errors___interactive_launcher_tmp_36680464884053384_log_obj = 0;
                    if (!entrance___interactive_launcher_tmp_36680464884053384_log_obj) {
                        if (elem.innerHTML != xmlhttp.responseText) {
                            elem.innerHTML = xmlhttp.responseText;
                        }
                        if (elem.innerHTML.includes("Process finished.")) {
                            halt__interactive_launcher_tmp_36680464884053384_log_obj(elem, "#333333");
                        } else {
                            entrance___interactive_launcher_tmp_36680464884053384_log_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__interactive_launcher_tmp_36680464884053384_log_obj()", 300); 
                        }
                    }
                    return xmlhttp.responseText;
                } else {
                    errors___interactive_launcher_tmp_36680464884053384_log_obj += 1;
                    if (!entrance___interactive_launcher_tmp_36680464884053384_log_obj) {
                        if (errors___interactive_launcher_tmp_36680464884053384_log_obj < 6) {
                            entrance___interactive_launcher_tmp_36680464884053384_log_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__interactive_launcher_tmp_36680464884053384_log_obj()", 300); 
                        } else {
                            halt__interactive_launcher_tmp_36680464884053384_log_obj(elem, "#994444");
                        }
                    }
                }
            }
        }
        xmlhttp.open("GET", "./interactive_launcher_tmp/36680464884053384.log", true);
        xmlhttp.setRequestHeader("Cache-Control", "no-cache");
        xmlhttp.send();     
    }
}

if (!entrance___interactive_launcher_tmp_36680464884053384_log_obj) {
    entrance___interactive_launcher_tmp_36680464884053384_log_obj += 1;
    refresh__interactive_launcher_tmp_36680464884053384_log_obj(); 
}
</script>

<p id="__interactive_launcher_tmp_36680464884053384_log_obj" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">
</p>

</font>
<!--MD_END_FILTER-->
<!--MD_FROM_FILE ./interactive_launcher_tmp/36680464884053384.log.md -->




```python
!cat my_fifo
```

    Hello



```python
a.close()
```

# Теперь на С
Обратите внимание, что fifo не может открыться на запись, пока ее не начнут читать.


```python
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
a = TInteractiveLauncher('./write_fifo.exe')
```



<!--MD_BEGIN_FILTER-->
<script type=text/javascript>
var entrance___interactive_launcher_tmp_408174933795177832_log_obj = 0;
var errors___interactive_launcher_tmp_408174933795177832_log_obj = 0;
function halt__interactive_launcher_tmp_408174933795177832_log_obj(elem, color)
{
    elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    
}
function refresh__interactive_launcher_tmp_408174933795177832_log_obj()
{
    entrance___interactive_launcher_tmp_408174933795177832_log_obj -= 1;
    if (entrance___interactive_launcher_tmp_408174933795177832_log_obj < 0) {
        entrance___interactive_launcher_tmp_408174933795177832_log_obj = 0;
    }
    var elem = document.getElementById("__interactive_launcher_tmp_408174933795177832_log_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem = document.getElementById("__interactive_launcher_tmp_408174933795177832_log_obj");
            console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___interactive_launcher_tmp_408174933795177832_log_obj);
            if (elem && xmlhttp.readyState==4) {
                if (xmlhttp.status==200)
                {
                    errors___interactive_launcher_tmp_408174933795177832_log_obj = 0;
                    if (!entrance___interactive_launcher_tmp_408174933795177832_log_obj) {
                        if (elem.innerHTML != xmlhttp.responseText) {
                            elem.innerHTML = xmlhttp.responseText;
                        }
                        if (elem.innerHTML.includes("Process finished.")) {
                            halt__interactive_launcher_tmp_408174933795177832_log_obj(elem, "#333333");
                        } else {
                            entrance___interactive_launcher_tmp_408174933795177832_log_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__interactive_launcher_tmp_408174933795177832_log_obj()", 300); 
                        }
                    }
                    return xmlhttp.responseText;
                } else {
                    errors___interactive_launcher_tmp_408174933795177832_log_obj += 1;
                    if (!entrance___interactive_launcher_tmp_408174933795177832_log_obj) {
                        if (errors___interactive_launcher_tmp_408174933795177832_log_obj < 6) {
                            entrance___interactive_launcher_tmp_408174933795177832_log_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__interactive_launcher_tmp_408174933795177832_log_obj()", 300); 
                        } else {
                            halt__interactive_launcher_tmp_408174933795177832_log_obj(elem, "#994444");
                        }
                    }
                }
            }
        }
        xmlhttp.open("GET", "./interactive_launcher_tmp/408174933795177832.log", true);
        xmlhttp.setRequestHeader("Cache-Control", "no-cache");
        xmlhttp.send();     
    }
}

if (!entrance___interactive_launcher_tmp_408174933795177832_log_obj) {
    entrance___interactive_launcher_tmp_408174933795177832_log_obj += 1;
    refresh__interactive_launcher_tmp_408174933795177832_log_obj(); 
}
</script>

<p id="__interactive_launcher_tmp_408174933795177832_log_obj" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">
</p>

</font>
<!--MD_END_FILTER-->
<!--MD_FROM_FILE ./interactive_launcher_tmp/408174933795177832.log.md -->




```python
!cat my_fifo
```

    Hello from C!
    


```python
a.close()
```

# Директория /proc/<pid\>/*

Интересная штука директория `/proc` это виртулаьная файловая система в которой можно получать сведения о процессах, читая из из файлов. (Это не обычные файлы на диске, а скорее некоторое view на сведения о процессах из ядра системы).

Что есть в proc: http://man7.org/linux/man-pages/man5/proc.5.html

Имеющая отношение к делу статья на хабре: https://habr.com/ru/post/209446/

Посмотрим, что можно узнать о запущенном процессе:


```python
# запустим процесс в фоне
a = TInteractiveLauncher('echo "Hello" > my_fifo')
```



<!--MD_BEGIN_FILTER-->
<script type=text/javascript>
var entrance___interactive_launcher_tmp_983043216781878980_log_obj = 0;
var errors___interactive_launcher_tmp_983043216781878980_log_obj = 0;
function halt__interactive_launcher_tmp_983043216781878980_log_obj(elem, color)
{
    elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    
}
function refresh__interactive_launcher_tmp_983043216781878980_log_obj()
{
    entrance___interactive_launcher_tmp_983043216781878980_log_obj -= 1;
    if (entrance___interactive_launcher_tmp_983043216781878980_log_obj < 0) {
        entrance___interactive_launcher_tmp_983043216781878980_log_obj = 0;
    }
    var elem = document.getElementById("__interactive_launcher_tmp_983043216781878980_log_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem = document.getElementById("__interactive_launcher_tmp_983043216781878980_log_obj");
            console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___interactive_launcher_tmp_983043216781878980_log_obj);
            if (elem && xmlhttp.readyState==4) {
                if (xmlhttp.status==200)
                {
                    errors___interactive_launcher_tmp_983043216781878980_log_obj = 0;
                    if (!entrance___interactive_launcher_tmp_983043216781878980_log_obj) {
                        if (elem.innerHTML != xmlhttp.responseText) {
                            elem.innerHTML = xmlhttp.responseText;
                        }
                        if (elem.innerHTML.includes("Process finished.")) {
                            halt__interactive_launcher_tmp_983043216781878980_log_obj(elem, "#333333");
                        } else {
                            entrance___interactive_launcher_tmp_983043216781878980_log_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__interactive_launcher_tmp_983043216781878980_log_obj()", 300); 
                        }
                    }
                    return xmlhttp.responseText;
                } else {
                    errors___interactive_launcher_tmp_983043216781878980_log_obj += 1;
                    if (!entrance___interactive_launcher_tmp_983043216781878980_log_obj) {
                        if (errors___interactive_launcher_tmp_983043216781878980_log_obj < 6) {
                            entrance___interactive_launcher_tmp_983043216781878980_log_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__interactive_launcher_tmp_983043216781878980_log_obj()", 300); 
                        } else {
                            halt__interactive_launcher_tmp_983043216781878980_log_obj(elem, "#994444");
                        }
                    }
                }
            }
        }
        xmlhttp.open("GET", "./interactive_launcher_tmp/983043216781878980.log", true);
        xmlhttp.setRequestHeader("Cache-Control", "no-cache");
        xmlhttp.send();     
    }
}

if (!entrance___interactive_launcher_tmp_983043216781878980_log_obj) {
    entrance___interactive_launcher_tmp_983043216781878980_log_obj += 1;
    refresh__interactive_launcher_tmp_983043216781878980_log_obj(); 
}
</script>

<p id="__interactive_launcher_tmp_983043216781878980_log_obj" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">
</p>

</font>
<!--MD_END_FILTER-->
<!--MD_FROM_FILE ./interactive_launcher_tmp/983043216781878980.log.md -->




```python
!cat /proc/3599/status
```

    Name:	bash
    Umask:	0002
    State:	S (sleeping)
    Tgid:	3599
    Ngid:	0
    Pid:	3599
    PPid:	3598
    TracerPid:	0
    Uid:	1000	1000	1000	1000
    Gid:	1000	1000	1000	1000
    FDSize:	64
    Groups:	4 24 27 30 46 113 128 130 999 1000 
    NStgid:	3599
    NSpid:	3599
    NSpgid:	17335
    NSsid:	17335
    VmPeak:	   19584 kB
    VmSize:	   19584 kB
    VmLck:	       0 kB
    VmPin:	       0 kB
    VmHWM:	     884 kB
    VmRSS:	     884 kB
    RssAnon:	     104 kB
    RssFile:	     780 kB
    RssShmem:	       0 kB
    VmData:	     164 kB
    VmStk:	     132 kB
    VmExe:	     976 kB
    VmLib:	    2112 kB
    VmPTE:	      64 kB
    VmSwap:	       0 kB
    HugetlbPages:	       0 kB
    CoreDumping:	0
    Threads:	1
    SigQ:	4/7735
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

    pechatn+  3604  0.0  0.0   4504   700 pts/26   Ss+  20:34   0:00 /bin/sh -c ps aux | grep write_fifo 
    pechatn+  3606  0.0  0.0  21292   968 pts/26   S+   20:34   0:00 grep write_fifo



```python
a.close()
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
a = TInteractiveLauncher('cat my_fifo')
```



<!--MD_BEGIN_FILTER-->
<script type=text/javascript>
var entrance___interactive_launcher_tmp_741034825313819161_log_obj = 0;
var errors___interactive_launcher_tmp_741034825313819161_log_obj = 0;
function halt__interactive_launcher_tmp_741034825313819161_log_obj(elem, color)
{
    elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    
}
function refresh__interactive_launcher_tmp_741034825313819161_log_obj()
{
    entrance___interactive_launcher_tmp_741034825313819161_log_obj -= 1;
    if (entrance___interactive_launcher_tmp_741034825313819161_log_obj < 0) {
        entrance___interactive_launcher_tmp_741034825313819161_log_obj = 0;
    }
    var elem = document.getElementById("__interactive_launcher_tmp_741034825313819161_log_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem = document.getElementById("__interactive_launcher_tmp_741034825313819161_log_obj");
            console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___interactive_launcher_tmp_741034825313819161_log_obj);
            if (elem && xmlhttp.readyState==4) {
                if (xmlhttp.status==200)
                {
                    errors___interactive_launcher_tmp_741034825313819161_log_obj = 0;
                    if (!entrance___interactive_launcher_tmp_741034825313819161_log_obj) {
                        if (elem.innerHTML != xmlhttp.responseText) {
                            elem.innerHTML = xmlhttp.responseText;
                        }
                        if (elem.innerHTML.includes("Process finished.")) {
                            halt__interactive_launcher_tmp_741034825313819161_log_obj(elem, "#333333");
                        } else {
                            entrance___interactive_launcher_tmp_741034825313819161_log_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__interactive_launcher_tmp_741034825313819161_log_obj()", 300); 
                        }
                    }
                    return xmlhttp.responseText;
                } else {
                    errors___interactive_launcher_tmp_741034825313819161_log_obj += 1;
                    if (!entrance___interactive_launcher_tmp_741034825313819161_log_obj) {
                        if (errors___interactive_launcher_tmp_741034825313819161_log_obj < 6) {
                            entrance___interactive_launcher_tmp_741034825313819161_log_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__interactive_launcher_tmp_741034825313819161_log_obj()", 300); 
                        } else {
                            halt__interactive_launcher_tmp_741034825313819161_log_obj(elem, "#994444");
                        }
                    }
                }
            }
        }
        xmlhttp.open("GET", "./interactive_launcher_tmp/741034825313819161.log", true);
        xmlhttp.setRequestHeader("Cache-Control", "no-cache");
        xmlhttp.send();     
    }
}

if (!entrance___interactive_launcher_tmp_741034825313819161_log_obj) {
    entrance___interactive_launcher_tmp_741034825313819161_log_obj += 1;
    refresh__interactive_launcher_tmp_741034825313819161_log_obj(); 
}
</script>

<p id="__interactive_launcher_tmp_741034825313819161_log_obj" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">
</p>

</font>
<!--MD_END_FILTER-->
<!--MD_FROM_FILE ./interactive_launcher_tmp/741034825313819161.log.md -->




```python
!echo "Hello 1" > my_fifo
!echo "Hello 2" > my_fifo
!echo "Hello 3" > my_fifo
```


```python
os.close(fd) # Только после закрытия дескриптора процесс 'cat my_fifo' завершится. Так как закроется fifo
```


```python
a.close()
```

### Если же ненужного чтения не создавать:


```python
a = TInteractiveLauncher('cat my_fifo')
```



<!--MD_BEGIN_FILTER-->
<script type=text/javascript>
var entrance___interactive_launcher_tmp_855988039801024707_log_obj = 0;
var errors___interactive_launcher_tmp_855988039801024707_log_obj = 0;
function halt__interactive_launcher_tmp_855988039801024707_log_obj(elem, color)
{
    elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    
}
function refresh__interactive_launcher_tmp_855988039801024707_log_obj()
{
    entrance___interactive_launcher_tmp_855988039801024707_log_obj -= 1;
    if (entrance___interactive_launcher_tmp_855988039801024707_log_obj < 0) {
        entrance___interactive_launcher_tmp_855988039801024707_log_obj = 0;
    }
    var elem = document.getElementById("__interactive_launcher_tmp_855988039801024707_log_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem = document.getElementById("__interactive_launcher_tmp_855988039801024707_log_obj");
            console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___interactive_launcher_tmp_855988039801024707_log_obj);
            if (elem && xmlhttp.readyState==4) {
                if (xmlhttp.status==200)
                {
                    errors___interactive_launcher_tmp_855988039801024707_log_obj = 0;
                    if (!entrance___interactive_launcher_tmp_855988039801024707_log_obj) {
                        if (elem.innerHTML != xmlhttp.responseText) {
                            elem.innerHTML = xmlhttp.responseText;
                        }
                        if (elem.innerHTML.includes("Process finished.")) {
                            halt__interactive_launcher_tmp_855988039801024707_log_obj(elem, "#333333");
                        } else {
                            entrance___interactive_launcher_tmp_855988039801024707_log_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__interactive_launcher_tmp_855988039801024707_log_obj()", 300); 
                        }
                    }
                    return xmlhttp.responseText;
                } else {
                    errors___interactive_launcher_tmp_855988039801024707_log_obj += 1;
                    if (!entrance___interactive_launcher_tmp_855988039801024707_log_obj) {
                        if (errors___interactive_launcher_tmp_855988039801024707_log_obj < 6) {
                            entrance___interactive_launcher_tmp_855988039801024707_log_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__interactive_launcher_tmp_855988039801024707_log_obj()", 300); 
                        } else {
                            halt__interactive_launcher_tmp_855988039801024707_log_obj(elem, "#994444");
                        }
                    }
                }
            }
        }
        xmlhttp.open("GET", "./interactive_launcher_tmp/855988039801024707.log", true);
        xmlhttp.setRequestHeader("Cache-Control", "no-cache");
        xmlhttp.send();     
    }
}

if (!entrance___interactive_launcher_tmp_855988039801024707_log_obj) {
    entrance___interactive_launcher_tmp_855988039801024707_log_obj += 1;
    refresh__interactive_launcher_tmp_855988039801024707_log_obj(); 
}
</script>

<p id="__interactive_launcher_tmp_855988039801024707_log_obj" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">
</p>

</font>
<!--MD_END_FILTER-->
<!--MD_FROM_FILE ./interactive_launcher_tmp/855988039801024707.log.md -->




```python
!echo "Hello 1" > my_fifo
```


```python
b = TInteractiveLauncher(
    'echo "Hello 2" > my_fifo # то все зависнет тут'
)
```



<!--MD_BEGIN_FILTER-->
<script type=text/javascript>
var entrance___interactive_launcher_tmp_168715572125305438_log_obj = 0;
var errors___interactive_launcher_tmp_168715572125305438_log_obj = 0;
function halt__interactive_launcher_tmp_168715572125305438_log_obj(elem, color)
{
    elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    
}
function refresh__interactive_launcher_tmp_168715572125305438_log_obj()
{
    entrance___interactive_launcher_tmp_168715572125305438_log_obj -= 1;
    if (entrance___interactive_launcher_tmp_168715572125305438_log_obj < 0) {
        entrance___interactive_launcher_tmp_168715572125305438_log_obj = 0;
    }
    var elem = document.getElementById("__interactive_launcher_tmp_168715572125305438_log_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem = document.getElementById("__interactive_launcher_tmp_168715572125305438_log_obj");
            console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___interactive_launcher_tmp_168715572125305438_log_obj);
            if (elem && xmlhttp.readyState==4) {
                if (xmlhttp.status==200)
                {
                    errors___interactive_launcher_tmp_168715572125305438_log_obj = 0;
                    if (!entrance___interactive_launcher_tmp_168715572125305438_log_obj) {
                        if (elem.innerHTML != xmlhttp.responseText) {
                            elem.innerHTML = xmlhttp.responseText;
                        }
                        if (elem.innerHTML.includes("Process finished.")) {
                            halt__interactive_launcher_tmp_168715572125305438_log_obj(elem, "#333333");
                        } else {
                            entrance___interactive_launcher_tmp_168715572125305438_log_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__interactive_launcher_tmp_168715572125305438_log_obj()", 300); 
                        }
                    }
                    return xmlhttp.responseText;
                } else {
                    errors___interactive_launcher_tmp_168715572125305438_log_obj += 1;
                    if (!entrance___interactive_launcher_tmp_168715572125305438_log_obj) {
                        if (errors___interactive_launcher_tmp_168715572125305438_log_obj < 6) {
                            entrance___interactive_launcher_tmp_168715572125305438_log_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__interactive_launcher_tmp_168715572125305438_log_obj()", 300); 
                        } else {
                            halt__interactive_launcher_tmp_168715572125305438_log_obj(elem, "#994444");
                        }
                    }
                }
            }
        }
        xmlhttp.open("GET", "./interactive_launcher_tmp/168715572125305438.log", true);
        xmlhttp.setRequestHeader("Cache-Control", "no-cache");
        xmlhttp.send();     
    }
}

if (!entrance___interactive_launcher_tmp_168715572125305438_log_obj) {
    entrance___interactive_launcher_tmp_168715572125305438_log_obj += 1;
    refresh__interactive_launcher_tmp_168715572125305438_log_obj(); 
}
</script>

<p id="__interactive_launcher_tmp_168715572125305438_log_obj" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">
</p>

</font>
<!--MD_END_FILTER-->
<!--MD_FROM_FILE ./interactive_launcher_tmp/168715572125305438.log.md -->




```python
os.kill(b.get_pid(), 9)
b.close()
```


```python
a.close()
```


```python

```
