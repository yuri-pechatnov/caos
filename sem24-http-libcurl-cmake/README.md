```python
# look at tools/set_up_magics.ipynb
yandex_metrica_allowed = True ; get_ipython().run_cell('# one_liner_str\n\nget_ipython().run_cell_magic(\'javascript\', \'\', \n    \'// setup cpp code highlighting\\n\'\n    \'IPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\\\'reg\\\':[/^%%cpp/]} ;\'\n    \'IPython.CodeCell.options_default.highlight_modes["text/x-cmake"] = {\\\'reg\\\':[/^%%cmake/]} ;\'\n)\n\n# creating magics\nfrom IPython.core.magic import register_cell_magic, register_line_magic\nfrom IPython.display import display, Markdown, HTML\nimport argparse\nfrom subprocess import Popen, PIPE\nimport random\nimport sys\nimport os\nimport re\nimport signal\nimport shutil\nimport shlex\nimport glob\n\n@register_cell_magic\ndef save_file(args_str, cell, line_comment_start="#"):\n    parser = argparse.ArgumentParser()\n    parser.add_argument("fname")\n    parser.add_argument("--ejudge-style", action="store_true")\n    args = parser.parse_args(args_str.split())\n    \n    cell = cell if cell[-1] == \'\\n\' or args.no_eof_newline else cell + "\\n"\n    cmds = []\n    with open(args.fname, "w") as f:\n        f.write(line_comment_start + " %%cpp " + args_str + "\\n")\n        for line in cell.split("\\n"):\n            line_to_write = (line if not args.ejudge_style else line.rstrip()) + "\\n"\n            if line.startswith("%"):\n                run_prefix = "%run "\n                if line.startswith(run_prefix):\n                    cmds.append(line[len(run_prefix):].strip())\n                    f.write(line_comment_start + " " + line_to_write)\n                    continue\n                run_prefix = "%# "\n                if line.startswith(run_prefix):\n                    f.write(line_comment_start + " " + line_to_write)\n                    continue\n                raise Exception("Unknown %%save_file subcommand: \'%s\'" % line)\n            else:\n                f.write(line_to_write)\n        f.write("" if not args.ejudge_style else line_comment_start + r" line without \\n")\n    for cmd in cmds:\n        display(Markdown("Run: `%s`" % cmd))\n        get_ipython().system(cmd)\n\n@register_cell_magic\ndef cpp(fname, cell):\n    save_file(fname, cell, "//")\n    \n@register_cell_magic\ndef cmake(fname, cell):\n    save_file(fname, cell, "#")\n\n@register_cell_magic\ndef asm(fname, cell):\n    save_file(fname, cell, "//")\n    \n@register_cell_magic\ndef makefile(fname, cell):\n    assert not fname\n    save_file("makefile", cell.replace(" " * 4, "\\t"))\n        \n@register_line_magic\ndef p(line):\n    try:\n        expr, comment = line.split(" #")\n        display(Markdown("`{} = {}`  # {}".format(expr.strip(), eval(expr), comment.strip())))\n    except:\n        display(Markdown("{} = {}".format(line, eval(line))))\n        \ndef show_file(file, clear_at_begin=True, return_html_string=False):\n    if clear_at_begin:\n        get_ipython().system("truncate --size 0 " + file)\n    obj = file.replace(\'.\', \'_\').replace(\'/\', \'_\') + "_obj"\n    html_string = \'\'\'\n        <!--MD_BEGIN_FILTER-->\n        <script type=text/javascript>\n        var entrance___OBJ__ = 0;\n        var errors___OBJ__ = 0;\n        function refresh__OBJ__()\n        {\n            entrance___OBJ__ -= 1;\n            var elem = document.getElementById("__OBJ__");\n            if (elem) {\n                var xmlhttp=new XMLHttpRequest();\n                xmlhttp.onreadystatechange=function()\n                {\n                    var elem = document.getElementById("__OBJ__");\n                    console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___OBJ__);\n                    if (elem && xmlhttp.readyState==4) {\n                        if (xmlhttp.status==200)\n                        {\n                            errors___OBJ__ = 0;\n                            if (!entrance___OBJ__) {\n                                elem.innerText = xmlhttp.responseText;\n                                entrance___OBJ__ += 1;\n                                console.log("req");\n                                window.setTimeout("refresh__OBJ__()", 300); \n                            }\n                            return xmlhttp.responseText;\n                        } else {\n                            errors___OBJ__ += 1;\n                            if (errors___OBJ__ < 10 && !entrance___OBJ__) {\n                                entrance___OBJ__ += 1;\n                                console.log("req");\n                                window.setTimeout("refresh__OBJ__()", 300); \n                            }\n                        }\n                    }\n                }\n                xmlhttp.open("GET", "__FILE__", true);\n                xmlhttp.setRequestHeader("Cache-Control", "no-cache");\n                xmlhttp.send();     \n            }\n        }\n        \n        if (!entrance___OBJ__) {\n            entrance___OBJ__ += 1;\n            refresh__OBJ__(); \n        }\n        </script>\n        \n        <font color="white"> <tt>\n        <p id="__OBJ__" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>\n        </tt> </font>\n        <!--MD_END_FILTER-->\n        <!--MD_FROM_FILE __FILE__ -->\n        \'\'\'.replace("__OBJ__", obj).replace("__FILE__", file)\n    if return_html_string:\n        return html_string\n    display(HTML(html_string))\n    \nBASH_POPEN_TMP_DIR = "./bash_popen_tmp"\n    \ndef bash_popen_terminate_all():\n    for p in globals().get("bash_popen_list", []):\n        print("Terminate pid=" + str(p.pid), file=sys.stderr)\n        p.terminate()\n    globals()["bash_popen_list"] = []\n    if os.path.exists(BASH_POPEN_TMP_DIR):\n        shutil.rmtree(BASH_POPEN_TMP_DIR)\n\nbash_popen_terminate_all()  \n\ndef bash_popen(cmd):\n    if not os.path.exists(BASH_POPEN_TMP_DIR):\n        os.mkdir(BASH_POPEN_TMP_DIR)\n    h = os.path.join(BASH_POPEN_TMP_DIR, str(random.randint(0, 1e18)))\n    stdout_file = h + ".out.html"\n    stderr_file = h + ".err.html"\n    run_log_file = h + ".fin.html"\n    \n    stdout = open(stdout_file, "wb")\n    stdout = open(stderr_file, "wb")\n    \n    html = """\n    <table width="100%">\n    <colgroup>\n       <col span="1" style="width: 70px;">\n       <col span="1">\n    </colgroup>    \n    <tbody>\n      <tr> <td><b>STDOUT</b></td> <td> {stdout} </td> </tr>\n      <tr> <td><b>STDERR</b></td> <td> {stderr} </td> </tr>\n      <tr> <td><b>RUN LOG</b></td> <td> {run_log} </td> </tr>\n    </tbody>\n    </table>\n    """.format(\n        stdout=show_file(stdout_file, return_html_string=True),\n        stderr=show_file(stderr_file, return_html_string=True),\n        run_log=show_file(run_log_file, return_html_string=True),\n    )\n    \n    cmd = """\n        bash -c {cmd} &\n        pid=$!\n        echo "Process started! pid=${{pid}}" > {run_log_file}\n        wait ${{pid}}\n        echo "Process finished! exit_code=$?" >> {run_log_file}\n    """.format(cmd=shlex.quote(cmd), run_log_file=run_log_file)\n    # print(cmd)\n    display(HTML(html))\n    \n    p = Popen(["bash", "-c", cmd], stdin=PIPE, stdout=stdout, stderr=stdout)\n    \n    bash_popen_list.append(p)\n    return p\n\n\n@register_line_magic\ndef bash_async(line):\n    bash_popen(line)\n    \n    \ndef show_log_file(file, return_html_string=False):\n    obj = file.replace(\'.\', \'_\').replace(\'/\', \'_\') + "_obj"\n    html_string = \'\'\'\n        <!--MD_BEGIN_FILTER-->\n        <script type=text/javascript>\n        var entrance___OBJ__ = 0;\n        var errors___OBJ__ = 0;\n        function halt__OBJ__(elem, color)\n        {\n            elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    \n        }\n        function refresh__OBJ__()\n        {\n            entrance___OBJ__ -= 1;\n            if (entrance___OBJ__ < 0) {\n                entrance___OBJ__ = 0;\n            }\n            var elem = document.getElementById("__OBJ__");\n            if (elem) {\n                var xmlhttp=new XMLHttpRequest();\n                xmlhttp.onreadystatechange=function()\n                {\n                    var elem = document.getElementById("__OBJ__");\n                    console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___OBJ__);\n                    if (elem && xmlhttp.readyState==4) {\n                        if (xmlhttp.status==200)\n                        {\n                            errors___OBJ__ = 0;\n                            if (!entrance___OBJ__) {\n                                if (elem.innerHTML != xmlhttp.responseText) {\n                                    elem.innerHTML = xmlhttp.responseText;\n                                }\n                                if (elem.innerHTML.includes("Process finished.")) {\n                                    halt__OBJ__(elem, "#333333");\n                                } else {\n                                    entrance___OBJ__ += 1;\n                                    console.log("req");\n                                    window.setTimeout("refresh__OBJ__()", 300); \n                                }\n                            }\n                            return xmlhttp.responseText;\n                        } else {\n                            errors___OBJ__ += 1;\n                            if (!entrance___OBJ__) {\n                                if (errors___OBJ__ < 6) {\n                                    entrance___OBJ__ += 1;\n                                    console.log("req");\n                                    window.setTimeout("refresh__OBJ__()", 300); \n                                } else {\n                                    halt__OBJ__(elem, "#994444");\n                                }\n                            }\n                        }\n                    }\n                }\n                xmlhttp.open("GET", "__FILE__", true);\n                xmlhttp.setRequestHeader("Cache-Control", "no-cache");\n                xmlhttp.send();     \n            }\n        }\n        \n        if (!entrance___OBJ__) {\n            entrance___OBJ__ += 1;\n            refresh__OBJ__(); \n        }\n        </script>\n\n        <p id="__OBJ__" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">\n        </p>\n        \n        </font>\n        <!--MD_END_FILTER-->\n        <!--MD_FROM_FILE __FILE__.md -->\n        \'\'\'.replace("__OBJ__", obj).replace("__FILE__", file)\n    if return_html_string:\n        return html_string\n    display(HTML(html_string))\n\n    \nclass TInteractiveLauncher:\n    tmp_path = "./interactive_launcher_tmp"\n    def __init__(self, cmd):\n        try:\n            os.mkdir(TInteractiveLauncher.tmp_path)\n        except:\n            pass\n        name = str(random.randint(0, 1e18))\n        self.inq_path = os.path.join(TInteractiveLauncher.tmp_path, name + ".inq")\n        self.log_path = os.path.join(TInteractiveLauncher.tmp_path, name + ".log")\n        \n        os.mkfifo(self.inq_path)\n        open(self.log_path, \'w\').close()\n        open(self.log_path + ".md", \'w\').close()\n\n        self.pid = os.fork()\n        if self.pid == -1:\n            print("Error")\n        if self.pid == 0:\n            exe_cands = glob.glob("../tools/launcher.py") + glob.glob("../../tools/launcher.py")\n            assert(len(exe_cands) == 1)\n            assert(os.execvp("python3", ["python3", exe_cands[0], "-l", self.log_path, "-i", self.inq_path, "-c", cmd]) == 0)\n        self.inq_f = open(self.inq_path, "w")\n        interactive_launcher_opened_set.add(self.pid)\n        show_log_file(self.log_path)\n\n    def write(self, s):\n        s = s.encode()\n        assert len(s) == os.write(self.inq_f.fileno(), s)\n        \n    def get_pid(self):\n        n = 100\n        for i in range(n):\n            try:\n                return int(re.findall(r"PID = (\\d+)", open(self.log_path).readline())[0])\n            except:\n                if i + 1 == n:\n                    raise\n                time.sleep(0.1)\n        \n    def input_queue_path(self):\n        return self.inq_path\n        \n    def close(self):\n        self.inq_f.close()\n        os.waitpid(self.pid, 0)\n        os.remove(self.inq_path)\n        # os.remove(self.log_path)\n        self.inq_path = None\n        self.log_path = None \n        interactive_launcher_opened_set.remove(self.pid)\n        self.pid = None\n        \n    @staticmethod\n    def terminate_all():\n        if "interactive_launcher_opened_set" not in globals():\n            globals()["interactive_launcher_opened_set"] = set()\n        global interactive_launcher_opened_set\n        for pid in interactive_launcher_opened_set:\n            print("Terminate pid=" + str(pid), file=sys.stderr)\n            os.kill(pid, signal.SIGKILL)\n            os.waitpid(pid, 0)\n        interactive_launcher_opened_set = set()\n        if os.path.exists(TInteractiveLauncher.tmp_path):\n            shutil.rmtree(TInteractiveLauncher.tmp_path)\n    \nTInteractiveLauncher.terminate_all()\n   \nyandex_metrica_allowed = bool(globals().get("yandex_metrica_allowed", False))\nif yandex_metrica_allowed:\n    display(HTML(\'\'\'<!-- YANDEX_METRICA_BEGIN -->\n    <script type="text/javascript" >\n       (function(m,e,t,r,i,k,a){m[i]=m[i]||function(){(m[i].a=m[i].a||[]).push(arguments)};\n       m[i].l=1*new Date();k=e.createElement(t),a=e.getElementsByTagName(t)[0],k.async=1,k.src=r,a.parentNode.insertBefore(k,a)})\n       (window, document, "script", "https://mc.yandex.ru/metrika/tag.js", "ym");\n\n       ym(59260609, "init", {\n            clickmap:true,\n            trackLinks:true,\n            accurateTrackBounce:true\n       });\n    </script>\n    <noscript><div><img src="https://mc.yandex.ru/watch/59260609" style="position:absolute; left:-9999px;" alt="" /></div></noscript>\n    <!-- YANDEX_METRICA_END -->\'\'\'))\n\ndef make_oneliner():\n    html_text = \'("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований. Если вы не хотите, чтобы по вам собиралась статистика, исправьте: yandex_metrica_allowed = False" if yandex_metrica_allowed else "")\'\n    html_text += \' + "<""!-- MAGICS_SETUP_PRINTING_END -->"\'\n    return \'\'.join([\n        \'# look at tools/set_up_magics.ipynb\\n\',\n        \'yandex_metrica_allowed = True ; get_ipython().run_cell(%s);\' % repr(one_liner_str),\n        \'display(HTML(%s))\' % html_text,\n        \' #\'\'MAGICS_SETUP_END\'\n    ])\n       \n\n');display(HTML(("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований. Если вы не хотите, чтобы по вам собиралась статистика, исправьте: yandex_metrica_allowed = False" if yandex_metrica_allowed else "") + "<""!-- MAGICS_SETUP_PRINTING_END -->")) #MAGICS_SETUP_END
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


# HTTP, libcurl, cmake

<table width=100%  > <tr>
    <th width=15%> <b>Видео с семинара &rarr; </b> </th>
    <th>
    <a href="https://youtu.be/THn5AmDlwu4"><img src="video.jpg" width="320" 
   height="160" align="left" alt="Видео с семинара"></a>
    </th>
    <th> </th>
</tr> </table>


## HTTP

[HTTP (HyperText Transfer Protocol)](https://ru.wikipedia.org/wiki/HTTP) — протокол прикладного/транспортного уровня передачи данных. 
Изначально был создан как протокол прикладного уровня для передачи документов в html формате (теги и все вот это). Но позже был распробован и сейчас может используется для передачи произвольных данных, что характерно для транспортного уровня.

Отправка HTTP запроса:
* <a href="#get_term" style="color:#856024"> Из терминала </a>
  * <a href="#netcat" style="color:#856024"> С помощью netcat, telnet </a> на уровне TCP, самостоятельно формируя HTTP запрос.
  * <a href="#curl" style="color:#856024"> С помощью curl </a> на уровне HTTP
* <a href="#get_python" style="color:#856024"> Из python </a> на уровне HTTP
* <a href="#get_c" style="color:#856024"> Из программы на C </a> на уровне HTTP

* <a href="#touch_http" style="color:#856024"> Более разнообразное использование HTTP </a> 

#### HTTP 1.1 и HTTP/2

На семинаре будем рассматривать HTTP 1.1, но стоит знать, что текущая версия протокола существенно более эффективна.

[Как HTTP/2 сделает веб быстрее / Хабр](https://habr.com/ru/company/nix/blog/304518/)

| HTTP 1.1 | HTTP/2 |
|----------|--------|
| одно соединение - один запрос, <br> как следствие вынужденная конкатенация, встраивание и спрайтинг (spriting) данных, | несколько запросов на соединение |
| все нужные заголовки каждый раз отправляются полсносью | сжатие заголовков, позволяет не отправлять каждый раз одни и те же заголовки |
| | возможность отправки данных по инициативе сервера |
| текстовый протокол | двоичный протокол |
| | приоритезация потоков - клиент может сообщать, что ему более важно| 
 
[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/http-curl)

## libcurl

Библиотека умеющая все то же, что и утилита curl.


## cmake

Решает задачу кроссплатформенной сборки

* Фронтенд для систем непосредственно занимающихся сборкой
* cmake хорошо интегрирован с многими IDE 
* CMakeLists.txt в корне дерева исходников - главный конфигурационный файл и главный индикатор того, что проект собирается с помощью cmake

Примеры:
* <a href="#сmake_simple" style="color:#856024"> Простой пример </a>
* <a href="#сmake_curl" style="color:#856024"> Пример с libcurl </a>



[Введение в CMake / Хабр](https://habr.com/ru/post/155467/)


[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/blob/master/practice/linux_basics/cmake.md)

[Документация для libCURL](https://curl.haxx.se/libcurl/c/)  
  
<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>



## <a name="get_term"></a> HTTP из терминала

#### <a name="netcat"></a> На уровне TCP


```bash
%%bash
# make request string
VAR=$(cat <<HEREDOC_END
GET / HTTP/1.1
Host: ejudge.atp-fivt.org
HEREDOC_END
)

# Если работаем в терминале, то просто пишем "nc ejudge.atp-fivt.org 80" и вводим запрос
# ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ - имитация ввода в stdin. "-q1" - чтобы netcat не закрылся сразу после закрытия stdin 
echo -e "$VAR\n" | nc -q1 ejudge.atp-fivt.org 80 | head -n 14
#                                                ↑↑↑↑↑↑↑↑↑↑↑↑ - обрезаем только начало вывода, чтобы не затопило выводом
```

    HTTP/1.1 200 OK
    Server: nginx/1.14.0 (Ubuntu)
    Date: Wed, 08 Apr 2020 21:29:29 GMT
    Content-Type: text/html; charset=UTF-8
    Content-Length: 4502
    Connection: keep-alive
    Last-Modified: Wed, 15 May 2019 07:01:47 GMT
    ETag: "1196-588e7b90e0fc5"
    Accept-Ranges: bytes
    
    <html>
      <head>
        <meta charset="utf-8"/>
        <title>АКОС ФИВТ МФТИ</title>



```python
# Можно еще исползовать telnet: "telnet ejudge.atp-fivt.org 80"
import time
a = TInteractiveLauncher("telnet ejudge.atp-fivt.org 80 | head -n 10")
a.write("""\
GET / HTTP/1.1
Host: ejudge.atp-fivt.org

""")
time.sleep(1)
a.close()
```



<!--MD_BEGIN_FILTER-->
<script type=text/javascript>
var entrance___interactive_launcher_tmp_539818863820297977_log_obj = 0;
var errors___interactive_launcher_tmp_539818863820297977_log_obj = 0;
function halt__interactive_launcher_tmp_539818863820297977_log_obj(elem, color)
{
    elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    
}
function refresh__interactive_launcher_tmp_539818863820297977_log_obj()
{
    entrance___interactive_launcher_tmp_539818863820297977_log_obj -= 1;
    if (entrance___interactive_launcher_tmp_539818863820297977_log_obj < 0) {
        entrance___interactive_launcher_tmp_539818863820297977_log_obj = 0;
    }
    var elem = document.getElementById("__interactive_launcher_tmp_539818863820297977_log_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem = document.getElementById("__interactive_launcher_tmp_539818863820297977_log_obj");
            console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___interactive_launcher_tmp_539818863820297977_log_obj);
            if (elem && xmlhttp.readyState==4) {
                if (xmlhttp.status==200)
                {
                    errors___interactive_launcher_tmp_539818863820297977_log_obj = 0;
                    if (!entrance___interactive_launcher_tmp_539818863820297977_log_obj) {
                        if (elem.innerHTML != xmlhttp.responseText) {
                            elem.innerHTML = xmlhttp.responseText;
                        }
                        if (elem.innerHTML.includes("Process finished.")) {
                            halt__interactive_launcher_tmp_539818863820297977_log_obj(elem, "#333333");
                        } else {
                            entrance___interactive_launcher_tmp_539818863820297977_log_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__interactive_launcher_tmp_539818863820297977_log_obj()", 300); 
                        }
                    }
                    return xmlhttp.responseText;
                } else {
                    errors___interactive_launcher_tmp_539818863820297977_log_obj += 1;
                    if (!entrance___interactive_launcher_tmp_539818863820297977_log_obj) {
                        if (errors___interactive_launcher_tmp_539818863820297977_log_obj < 6) {
                            entrance___interactive_launcher_tmp_539818863820297977_log_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__interactive_launcher_tmp_539818863820297977_log_obj()", 300); 
                        } else {
                            halt__interactive_launcher_tmp_539818863820297977_log_obj(elem, "#994444");
                        }
                    }
                }
            }
        }
        xmlhttp.open("GET", "./interactive_launcher_tmp/539818863820297977.log", true);
        xmlhttp.setRequestHeader("Cache-Control", "no-cache");
        xmlhttp.send();     
    }
}

if (!entrance___interactive_launcher_tmp_539818863820297977_log_obj) {
    entrance___interactive_launcher_tmp_539818863820297977_log_obj += 1;
    refresh__interactive_launcher_tmp_539818863820297977_log_obj(); 
}
</script>

<p id="__interactive_launcher_tmp_539818863820297977_log_obj" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">
</p>

</font>
<!--MD_END_FILTER-->
<!--MD_FROM_FILE ./interactive_launcher_tmp/539818863820297977.log.md -->




```bash
%%bash
VAR=$(cat <<HEREDOC_END
USER pechatnov@yandex.ru
HEREDOC_END
)

# попытка загрузить почту по POP3 протоколу (не получится, там надо с шифрованием заморочиться)
echo -e "$VAR\n" | nc -q1 pop.yandex.ru 110 
```

    +OK POP Ya! na@2-9ce8cb8ac11f b1FWeUmdOSw1
    -ERR [AUTH] Working without SSL/TLS encryption is not allowed. Please visit https://yandex.ru/support/mail-new/mail-clients/ssl.html  sc=b1FWeUmdOSw1_101301_2-9ce8cb8ac11f


#### <a name="curl"></a> Сразу на уровне HTTP

curl - возволяет делать произвольные HTTP запросы

wget - в первую очередь предназначен для скачивания файлов. Например, умеет выкачивать страницу рекурсивно


```bash
%%bash
curl ejudge.atp-fivt.org | head -n 10
```

    <html>
      <head>
        <meta charset="utf-8"/>
        <title>АКОС ФИВТ МФТИ</title>
      </head>
      <body>
        <h1>Ejudge для АКОС на ФИВТ МФТИ</h1>
        <h2>Весенний семестр</h2>
        <h3>Группы ПМФ</h3>
        <p><b>!!!!!!!!!!</b> <a href="/client?contest_id=19">Контрольная 15 мая 2019</a><b>!!!!!!!!!</b></p>


      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100  4502  100  4502    0     0  12158      0 --:--:-- --:--:-- --:--:-- 12167
    (23) Failed writing body



```bash
%%bash
wget ejudge.atp-fivt.org -O - | head -n 10
```

    <html>
      <head>
        <meta charset="utf-8"/>
        <title>АКОС ФИВТ МФТИ</title>
      </head>
      <body>
        <h1>Ejudge для АКОС на ФИВТ МФТИ</h1>
        <h2>Весенний семестр</h2>
        <h3>Группы ПМФ</h3>
        <p><b>!!!!!!!!!!</b> <a href="/client?contest_id=19">Контрольная 15 мая 2019</a><b>!!!!!!!!!</b></p>


    --2020-04-10 16:12:55--  http://ejudge.atp-fivt.org/
    Resolving ejudge.atp-fivt.org (ejudge.atp-fivt.org)... 87.251.82.74
    Connecting to ejudge.atp-fivt.org (ejudge.atp-fivt.org)|87.251.82.74|:80... connected.
    HTTP request sent, awaiting response... 200 OK
    Length: 4502 (4,4K) [text/html]
    Saving to: ‘STDOUT’
    
         0K ..                                                     58% 1,01M=0,002s
    
    
    Cannot write to ‘-’ (Success).


## <a name="get_python"></a> HTTP из python


```python
import requests
data = requests.get("http://ejudge.atp-fivt.org").content.decode()
print(data[:200])
```

    <html>
      <head>
        <meta charset="utf-8"/>
        <title>АКОС ФИВТ МФТИ</title>
      </head>
      <body>
        <h1>Ejudge для АКОС на ФИВТ МФТИ</h1>
        <h2>Весенний семестр</h2>
        <h3>Группы ПМФ</h3>
        <p>


## <a name="get_c"></a> HTTP из C

Пример от Яковлева


```python
%%cpp curl_easy.c
%run gcc -Wall curl_easy.c -lcurl -o curl_easy.exe
%run ./curl_easy.exe | head -n 5

#include <curl/curl.h>
#include <assert.h>

int main() {
    CURL *curl = curl_easy_init();
    assert(curl);
    
    CURLcode res;
    curl_easy_setopt(curl, CURLOPT_URL, "http://ejudge.atp-fivt.org");
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    assert(res == 0);
    return 0;
}
```


Run: `gcc -Wall curl_easy.c -lcurl -o curl_easy.exe`



Run: `./curl_easy.exe | head -n 5`


    <html>
      <head>
        <meta charset="utf-8"/>
        <title>АКОС ФИВТ МФТИ</title>
      </head>


#### <a name="touch_http"></a> Потрогаем HTTP  более разнообразно



Установка: 
<br>https://install.advancedrestclient.com/ - программка для удобной отправки разнообразных http запросов
<br>`pip3 install wsgidav cheroot` - webdav сервер


```python
!mkdir webdav_dir || true
!echo "Hello!" > webdav_dir/file.txt

a = TInteractiveLauncher("wsgidav --port=9024 --root=./webdav_dir --auth=anonymous --host=0.0.0.0")

```

    mkdir: cannot create directory ‘webdav_dir’: File exists




<!--MD_BEGIN_FILTER-->
<script type=text/javascript>
var entrance___interactive_launcher_tmp_186616182816427350_log_obj = 0;
var errors___interactive_launcher_tmp_186616182816427350_log_obj = 0;
function halt__interactive_launcher_tmp_186616182816427350_log_obj(elem, color)
{
    elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    
}
function refresh__interactive_launcher_tmp_186616182816427350_log_obj()
{
    entrance___interactive_launcher_tmp_186616182816427350_log_obj -= 1;
    if (entrance___interactive_launcher_tmp_186616182816427350_log_obj < 0) {
        entrance___interactive_launcher_tmp_186616182816427350_log_obj = 0;
    }
    var elem = document.getElementById("__interactive_launcher_tmp_186616182816427350_log_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem = document.getElementById("__interactive_launcher_tmp_186616182816427350_log_obj");
            console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___interactive_launcher_tmp_186616182816427350_log_obj);
            if (elem && xmlhttp.readyState==4) {
                if (xmlhttp.status==200)
                {
                    errors___interactive_launcher_tmp_186616182816427350_log_obj = 0;
                    if (!entrance___interactive_launcher_tmp_186616182816427350_log_obj) {
                        if (elem.innerHTML != xmlhttp.responseText) {
                            elem.innerHTML = xmlhttp.responseText;
                        }
                        if (elem.innerHTML.includes("Process finished.")) {
                            halt__interactive_launcher_tmp_186616182816427350_log_obj(elem, "#333333");
                        } else {
                            entrance___interactive_launcher_tmp_186616182816427350_log_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__interactive_launcher_tmp_186616182816427350_log_obj()", 300); 
                        }
                    }
                    return xmlhttp.responseText;
                } else {
                    errors___interactive_launcher_tmp_186616182816427350_log_obj += 1;
                    if (!entrance___interactive_launcher_tmp_186616182816427350_log_obj) {
                        if (errors___interactive_launcher_tmp_186616182816427350_log_obj < 6) {
                            entrance___interactive_launcher_tmp_186616182816427350_log_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__interactive_launcher_tmp_186616182816427350_log_obj()", 300); 
                        } else {
                            halt__interactive_launcher_tmp_186616182816427350_log_obj(elem, "#994444");
                        }
                    }
                }
            }
        }
        xmlhttp.open("GET", "./interactive_launcher_tmp/186616182816427350.log", true);
        xmlhttp.setRequestHeader("Cache-Control", "no-cache");
        xmlhttp.send();     
    }
}

if (!entrance___interactive_launcher_tmp_186616182816427350_log_obj) {
    entrance___interactive_launcher_tmp_186616182816427350_log_obj += 1;
    refresh__interactive_launcher_tmp_186616182816427350_log_obj(); 
}
</script>

<p id="__interactive_launcher_tmp_186616182816427350_log_obj" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">
</p>

</font>
<!--MD_END_FILTER-->
<!--MD_FROM_FILE ./interactive_launcher_tmp/186616182816427350.log.md -->




```python
!curl localhost:9024 | head -n 4
```

      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100  1831  100  1831    0     0   200k      0 --:--:-- --:--:-- --:--:--  223k
    <!DOCTYPE html>
    <html>
    <head>
      <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">



```python
!curl -X "PUT" localhost:9024/curl_added_file.txt --data-binary @curl_easy.c
```

    <!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.01//EN' 'http://www.w3.org/TR/html4/strict.dtd'>
    <html><head>
      <meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>
      <title>201 Created</title>
    </head><body>
      <h1>201 Created</h1>
      <p>201 Created</p>
    <hr/>
    <a href='https://github.com/mar10/wsgidav/'>WsgiDAV/3.0.3</a> - 2020-04-10 17:40:56.725260
    </body></html>


```python
!ls webdav_dir
!cat webdav_dir/curl_added_file.txt | grep main -C 2
```

    curl_added_file.txt  file.txt  hello_2.txt
    #include <assert.h>
    
    int main() {
        CURL *curl = curl_easy_init();
        assert(curl);



```python
!curl -X "DELETE" localhost:9024/curl_added_file.txt 
```


```python
!ls webdav_dir
```

    file.txt  hello_2.txt



```python
os.kill(a.get_pid(), signal.SIGINT)
```


```python

```

## libcurl

Установка: `sudo apt-get install libcurl4-openssl-dev` (Но это не точно! Воспоминания годичной давности. Напишите мне пожалуйста получится или не получится)

Документация: https://curl.haxx.se/libcurl/c/CURLOPT_WRITEFUNCTION.html  
Интересный факт: размер chunk'a всегда равен 1.

Модифицирпованный пример от Яковлева


```python
%%cpp curl_medium.c
%run gcc -Wall curl_medium.c -lcurl -o curl_medium.exe
%run ./curl_medium.exe "http://ejudge.atp-fivt.org" | head -n 5

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <curl/curl.h>

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
} buffer_t;

static size_t callback_function(
    char *ptr, // буфер с прочитанными данными
    size_t chunk_size, // размер фрагмента данных; всегда равен 1 
    size_t nmemb, // количество фрагментов данных
    void *user_data // произвольные данные пользователя
) {
    buffer_t *buffer = user_data;
    size_t total_size = chunk_size * nmemb;
    size_t required_capacity = buffer->length + total_size;
    if (required_capacity > buffer->capacity) {
        required_capacity *= 2;
        buffer->data = realloc(buffer->data, required_capacity);
        assert(buffer->data);
        buffer->capacity = required_capacity;
    }
    memcpy(buffer->data + buffer->length, ptr, total_size);
    buffer->length += total_size;
    return total_size;
}            

int main(int argc, char *argv[]) {
    assert(argc == 2);
    const char* url = argv[1];
    CURL *curl = curl_easy_init();
    assert(curl);
    
    CURLcode res;

    // регистрация callback-функции записи
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback_function);

    // указатель &buffer будет передан в callback-функцию
    // параметром void *user_data
    buffer_t buffer = {.data = NULL, .length = 0, .capacity = 0};
    
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    res = curl_easy_perform(curl);
    assert(res == 0);
    
    write(STDOUT_FILENO, buffer.data, buffer.length);
    
    free(buffer.data);
    curl_easy_cleanup(curl);
}
```


Run: `gcc -Wall curl_medium.c -lcurl -o curl_medium.exe`



Run: `./curl_medium.exe "http://ejudge.atp-fivt.org" | head -n 5`


    <html>
      <head>
        <meta charset="utf-8"/>
        <title>АКОС ФИВТ МФТИ</title>
      </head>



```python

```

## cmake
Установка: `apt-get install cmake cmake-extras`

#### <a name="cmake_simple"></a> Простой пример

Источник: [Введение в CMake / Хабр](https://habr.com/ru/post/155467/). Там же можно найти множество более интересных примеров.


```python
!mkdir simple_cmake_example || true
```

    mkdir: cannot create directory ‘simple_cmake_example’: File exists



```python
%%cmake simple_cmake_example/CMakeLists.txt
cmake_minimum_required(VERSION 2.8) # Проверка версии CMake.
                                    # Если версия установленой программы
                                    # старее указаной, произайдёт аварийный выход.

add_executable(main main.cpp)       # Создает исполняемый файл с именем main
                                    # из исходника main.cpp
```


```python
%%cpp simple_cmake_example/main.cpp
%run mkdir simple_cmake_example/build #// cоздаем директорию для файлов сборки
%# // переходим в нее, вызываем cmake, чтобы он создал правильный Makefile
%# // а затем make, который по Makefile правильно все соберет
%run cd simple_cmake_example/build && cmake .. && make  
%run simple_cmake_example/build/main #// запускаем собранный бинарь
%run ls -la simple_cmake_example #// смотрим, а что же теперь есть в основной директории 
%run ls -la simple_cmake_example/build #// ... и в директории сборки
%run rm -r simple_cmake_example/build #// удаляем директорию с файлами сборки

#include <iostream>
int main(int argc, char** argv)
{
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
```


Run: `mkdir simple_cmake_example/build #// cоздаем директорию для файлов сборки`



Run: `cd simple_cmake_example/build && cmake .. && make`


    -- The C compiler identification is GNU 5.5.0
    -- The CXX compiler identification is GNU 5.5.0
    -- Check for working C compiler: /usr/bin/cc
    -- Check for working C compiler: /usr/bin/cc -- works
    -- Detecting C compiler ABI info
    -- Detecting C compiler ABI info - done
    -- Detecting C compile features
    -- Detecting C compile features - done
    -- Check for working CXX compiler: /usr/bin/c++
    -- Check for working CXX compiler: /usr/bin/c++ -- works
    -- Detecting CXX compiler ABI info
    -- Detecting CXX compiler ABI info - done
    -- Detecting CXX compile features
    -- Detecting CXX compile features - done
    -- Configuring done
    -- Generating done
    -- Build files have been written to: /home/pechatnov/vbox/caos_2019-2020/sem24-http-libcurl-cmake/simple_cmake_example/build
    [35m[1mScanning dependencies of target main[0m
    [ 50%] [32mBuilding CXX object CMakeFiles/main.dir/main.cpp.o[0m
    [100%] [32m[1mLinking CXX executable main[0m
    [100%] Built target main



Run: `simple_cmake_example/build/main #// запускаем собранный бинарь`


    Hello, World!



Run: `ls -la simple_cmake_example #// смотрим, а что же теперь есть в основной директории`


    total 20
    drwxrwxr-x 3 pechatnov pechatnov 4096 апр 10 17:59 .
    drwxrwxr-x 8 pechatnov pechatnov 4096 апр 10 17:56 ..
    drwxrwxr-x 3 pechatnov pechatnov 4096 апр 10 17:59 build
    -rw-rw-r-- 1 pechatnov pechatnov  523 апр 10 14:08 CMakeLists.txt
    -rw-rw-r-- 1 pechatnov pechatnov  984 апр 10 17:59 main.cpp



Run: `ls -la simple_cmake_example/build #// ... и в директории сборки`


    total 48
    drwxrwxr-x 3 pechatnov pechatnov  4096 апр 10 17:59 .
    drwxrwxr-x 3 pechatnov pechatnov  4096 апр 10 17:59 ..
    -rw-rw-r-- 1 pechatnov pechatnov 11809 апр 10 17:59 CMakeCache.txt
    drwxrwxr-x 5 pechatnov pechatnov  4096 апр 10 18:00 CMakeFiles
    -rw-rw-r-- 1 pechatnov pechatnov  1479 апр 10 17:59 cmake_install.cmake
    -rwxrwxr-x 1 pechatnov pechatnov  9216 апр 10 18:00 main
    -rw-rw-r-- 1 pechatnov pechatnov  4986 апр 10 17:59 Makefile



Run: `rm -r simple_cmake_example/build #// удаляем директорию с файлами сборки`


#### <a name="cmake_curl"></a> Пример с libcurl


```python
!mkdir curl_cmake_example || true
!cp curl_medium.c curl_cmake_example/main.c
```


```python
%%cmake curl_cmake_example/CMakeLists.txt
%run mkdir curl_cmake_example/build 
%run cd curl_cmake_example/build && cmake .. && make  
%run curl_cmake_example/build/main "http://ejudge.atp-fivt.org" | head -n 5 #// запускаем собранный бинарь
%run rm -r curl_cmake_example/build


cmake_minimum_required(VERSION 2.8) 

set(CMAKE_C_FLAGS "-std=gnu11") # дополнительные опции компилятора Си

# найти библиотеку CURL; опция REQUIRED означает,
# что библиотека является обязательной для сборки проекта,
# и если необходимые файлы не будут найдены, cmake
# завершит работу с ошибкой
find_package(CURL REQUIRED)

# это библиотека в проекте не нужна, просто пример, как написать обработку случаев, когда библиотека не найдена
find_package(SDL)
if(NOT SDL_FOUND)
    message(">>>>> Failed to find SDL (not a problem)")
else()
    message(">>>>> Managed to find SDL, can add include directories, add target libraries")
endif()

# это библиотека в проекте не нужна, просто пример, как подключить модуль интеграции с pkg-config
find_package(PkgConfig REQUIRED)
# и ненужный в этом проекте FUSE через pkg-config
pkg_check_modules(
  FUSE         # имя префикса для названий выходных переменных
  # REQUIRED # опционально можно писать, чтобы было required
  fuse3        # имя библиотеки, должен существовать файл fuse3.pc
)
if(NOT FUSE_FOUND)
    message(">>>>> Failed to find FUSE (not a problem)")
else()
    message(">>>>> Managed to find FUSE, can add include directories, add target libraries")
endif()

# добавляем цель собрать исполняемый файл из перечисленных исходнико
add_executable(main main.c)
            
# добавляет в список каталогов для цели main, 
# которые превратятся в опции -I компилятора для всех 
# каталогов, которые перечислены в переменной CURL_INCLUDE_DIRECTORIES
target_include_directories(main PUBLIC ${CURL_INCLUDE_DIRECTORIES}) 
# include_directories(${CURL_INCLUDE_DIRECTORIES}) # можно вот так

# для цели my_cool_program указываем библиотеки, с которыми
# программа будет слинкована (в результате станет опциями -l и -L)
target_link_libraries(main ${CURL_LIBRARIES})
            
```


Run: `mkdir curl_cmake_example/build`



Run: `cd curl_cmake_example/build && cmake .. && make`


    -- The C compiler identification is GNU 5.5.0
    -- The CXX compiler identification is GNU 5.5.0
    -- Check for working C compiler: /usr/bin/cc
    -- Check for working C compiler: /usr/bin/cc -- works
    -- Detecting C compiler ABI info
    -- Detecting C compiler ABI info - done
    -- Detecting C compile features
    -- Detecting C compile features - done
    -- Check for working CXX compiler: /usr/bin/c++
    -- Check for working CXX compiler: /usr/bin/c++ -- works
    -- Detecting CXX compiler ABI info
    -- Detecting CXX compiler ABI info - done
    -- Detecting CXX compile features
    -- Detecting CXX compile features - done
    -- Found CURL: /usr/lib/x86_64-linux-gnu/libcurl.so (found version "7.47.0") 
    -- Looking for pthread.h
    -- Looking for pthread.h - found
    -- Looking for pthread_create
    -- Looking for pthread_create - not found
    -- Looking for pthread_create in pthreads
    -- Looking for pthread_create in pthreads - not found
    -- Looking for pthread_create in pthread
    -- Looking for pthread_create in pthread - found
    -- Found Threads: TRUE  
    -- Could NOT find SDL (missing:  SDL_LIBRARY SDL_INCLUDE_DIR) 
    >>>>> Failed to find SDL (not a problem)
    -- Found PkgConfig: /usr/bin/pkg-config (found version "0.29.1") 
    -- Checking for module 'fuse3'
    --   No package 'fuse3' found
    >>>>> Failed to find FUSE (not a problem)
    -- Configuring done
    -- Generating done
    -- Build files have been written to: /home/pechatnov/vbox/caos_2019-2020/sem24-http-libcurl-cmake/curl_cmake_example/build
    [35m[1mScanning dependencies of target main[0m
    [ 50%] [32mBuilding C object CMakeFiles/main.dir/main.c.o[0m
    [100%] [32m[1mLinking C executable main[0m
    [100%] Built target main



Run: `curl_cmake_example/build/main "http://ejudge.atp-fivt.org" | head -n 5 #// запускаем собранный бинарь`


    <html>
      <head>
        <meta charset="utf-8"/>
        <title>АКОС ФИВТ МФТИ</title>
      </head>



Run: `rm -r curl_cmake_example/build`



```python

```




```python

```

# <a name="hw"></a> Комментарии к ДЗ

* `Connection: close`
* Комментарий от [Михаила Циона](https://github.com/MVCionOld):
<br> От себя хочу добавить про использование `сURL`'a. Одним из хэдеров в  `http`-запросе есть `User-Agent`, которые сигнализирует сайту про, что "вы" это то браузер, поисковый бот/скраппер, мобильный телефоны или холодильник. Некоторые сайты нормально открываются в браузере, но при попытке получить исходный `HTML` код с помощью `cURL` эти запросы могут отклоняться. Могут возвращаться коды ответов, например, 403, то есть доступ запрещён.
<br> Зачастую боты не несут никакой пользы, но в то же время создают нагрузку на сервис и/или ведут другую вредоносную активность. Насколько мне известно, есть два способа бороться с такими негодяями: проверять `User-Agent` и использование `JavaScript`. Во втором случае это инъекции на куки, асинхронная генерация страницы и тд. Что касается агента - банально денаить конкретные паттерны. У `сURL`'a есть своя строка для агента, в основном меняется только версия, например `curl/7.37.0`.
<br> Возможно, кто-то сталкивался с тем, что при написании скраппера основанного на `сURL`'e вы получали `BadRequest` (например, при тестировании задачи **inf21-2**), хотя сайт прекрасно открывался. Это как раз первый случай.
<br> Однако, можно менять агента, например, из терминала: 
<br> `curl -H "User-Agent: Mozilla/5.0" url`
<br> при использовании `libcurl`:
<br> `curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");`


```python

```


```python

```
