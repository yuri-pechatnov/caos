```python
# look at tools/set_up_magics.ipynb
yandex_metrica_allowed = True ; get_ipython().run_cell('# one_liner_str\n\nget_ipython().run_cell_magic(\'javascript\', \'\', \n    \'// setup cpp code highlighting\\n\'\n    \'IPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\\\'reg\\\':[/^%%cpp/]} ;\'\n    \'IPython.CodeCell.options_default.highlight_modes["text/x-cmake"] = {\\\'reg\\\':[/^%%cmake/]} ;\'\n)\n\n# creating magics\nfrom IPython.core.magic import register_cell_magic, register_line_magic\nfrom IPython.display import display, Markdown, HTML\nimport argparse\nfrom subprocess import Popen, PIPE\nimport random\nimport sys\nimport os\nimport re\nimport signal\nimport shutil\nimport shlex\nimport glob\n\n@register_cell_magic\ndef save_file(args_str, cell, line_comment_start="#"):\n    parser = argparse.ArgumentParser()\n    parser.add_argument("fname")\n    parser.add_argument("--ejudge-style", action="store_true")\n    args = parser.parse_args(args_str.split())\n    \n    cell = cell if cell[-1] == \'\\n\' or args.no_eof_newline else cell + "\\n"\n    cmds = []\n    with open(args.fname, "w") as f:\n        f.write(line_comment_start + " %%cpp " + args_str + "\\n")\n        for line in cell.split("\\n"):\n            line_to_write = (line if not args.ejudge_style else line.rstrip()) + "\\n"\n            if line.startswith("%"):\n                run_prefix = "%run "\n                if line.startswith(run_prefix):\n                    cmds.append(line[len(run_prefix):].strip())\n                    f.write(line_comment_start + " " + line_to_write)\n                    continue\n                run_prefix = "%# "\n                if line.startswith(run_prefix):\n                    f.write(line_comment_start + " " + line_to_write)\n                    continue\n                raise Exception("Unknown %%save_file subcommand: \'%s\'" % line)\n            else:\n                f.write(line_to_write)\n        f.write("" if not args.ejudge_style else line_comment_start + r" line without \\n")\n    for cmd in cmds:\n        display(Markdown("Run: `%s`" % cmd))\n        get_ipython().system(cmd)\n\n@register_cell_magic\ndef cpp(fname, cell):\n    save_file(fname, cell, "//")\n    \n@register_cell_magic\ndef cmake(fname, cell):\n    save_file(fname, cell, "#")\n\n@register_cell_magic\ndef asm(fname, cell):\n    save_file(fname, cell, "//")\n    \n@register_cell_magic\ndef makefile(fname, cell):\n    assert not fname\n    save_file("makefile", cell.replace(" " * 4, "\\t"))\n        \n@register_line_magic\ndef p(line):\n    line = line.strip() \n    if line[0] == \'#\':\n        display(Markdown(line[1:].strip()))\n    else:\n        try:\n            expr, comment = line.split(" #")\n            display(Markdown("`{} = {}`  # {}".format(expr.strip(), eval(expr), comment.strip())))\n        except:\n            display(Markdown("{} = {}".format(line, eval(line))))\n        \ndef show_file(file, clear_at_begin=True, return_html_string=False):\n    if clear_at_begin:\n        get_ipython().system("truncate --size 0 " + file)\n    obj = file.replace(\'.\', \'_\').replace(\'/\', \'_\') + "_obj"\n    html_string = \'\'\'\n        <!--MD_BEGIN_FILTER-->\n        <script type=text/javascript>\n        var entrance___OBJ__ = 0;\n        var errors___OBJ__ = 0;\n        function refresh__OBJ__()\n        {\n            entrance___OBJ__ -= 1;\n            var elem = document.getElementById("__OBJ__");\n            if (elem) {\n                var xmlhttp=new XMLHttpRequest();\n                xmlhttp.onreadystatechange=function()\n                {\n                    var elem = document.getElementById("__OBJ__");\n                    console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___OBJ__);\n                    if (elem && xmlhttp.readyState==4) {\n                        if (xmlhttp.status==200)\n                        {\n                            errors___OBJ__ = 0;\n                            if (!entrance___OBJ__) {\n                                elem.innerText = xmlhttp.responseText;\n                                entrance___OBJ__ += 1;\n                                console.log("req");\n                                window.setTimeout("refresh__OBJ__()", 300); \n                            }\n                            return xmlhttp.responseText;\n                        } else {\n                            errors___OBJ__ += 1;\n                            if (errors___OBJ__ < 10 && !entrance___OBJ__) {\n                                entrance___OBJ__ += 1;\n                                console.log("req");\n                                window.setTimeout("refresh__OBJ__()", 300); \n                            }\n                        }\n                    }\n                }\n                xmlhttp.open("GET", "__FILE__", true);\n                xmlhttp.setRequestHeader("Cache-Control", "no-cache");\n                xmlhttp.send();     \n            }\n        }\n        \n        if (!entrance___OBJ__) {\n            entrance___OBJ__ += 1;\n            refresh__OBJ__(); \n        }\n        </script>\n        \n        <font color="white"> <tt>\n        <p id="__OBJ__" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>\n        </tt> </font>\n        <!--MD_END_FILTER-->\n        <!--MD_FROM_FILE __FILE__ -->\n        \'\'\'.replace("__OBJ__", obj).replace("__FILE__", file)\n    if return_html_string:\n        return html_string\n    display(HTML(html_string))\n    \nBASH_POPEN_TMP_DIR = "./bash_popen_tmp"\n    \ndef bash_popen_terminate_all():\n    for p in globals().get("bash_popen_list", []):\n        print("Terminate pid=" + str(p.pid), file=sys.stderr)\n        p.terminate()\n    globals()["bash_popen_list"] = []\n    if os.path.exists(BASH_POPEN_TMP_DIR):\n        shutil.rmtree(BASH_POPEN_TMP_DIR)\n\nbash_popen_terminate_all()  \n\ndef bash_popen(cmd):\n    if not os.path.exists(BASH_POPEN_TMP_DIR):\n        os.mkdir(BASH_POPEN_TMP_DIR)\n    h = os.path.join(BASH_POPEN_TMP_DIR, str(random.randint(0, 1e18)))\n    stdout_file = h + ".out.html"\n    stderr_file = h + ".err.html"\n    run_log_file = h + ".fin.html"\n    \n    stdout = open(stdout_file, "wb")\n    stdout = open(stderr_file, "wb")\n    \n    html = """\n    <table width="100%">\n    <colgroup>\n       <col span="1" style="width: 70px;">\n       <col span="1">\n    </colgroup>    \n    <tbody>\n      <tr> <td><b>STDOUT</b></td> <td> {stdout} </td> </tr>\n      <tr> <td><b>STDERR</b></td> <td> {stderr} </td> </tr>\n      <tr> <td><b>RUN LOG</b></td> <td> {run_log} </td> </tr>\n    </tbody>\n    </table>\n    """.format(\n        stdout=show_file(stdout_file, return_html_string=True),\n        stderr=show_file(stderr_file, return_html_string=True),\n        run_log=show_file(run_log_file, return_html_string=True),\n    )\n    \n    cmd = """\n        bash -c {cmd} &\n        pid=$!\n        echo "Process started! pid=${{pid}}" > {run_log_file}\n        wait ${{pid}}\n        echo "Process finished! exit_code=$?" >> {run_log_file}\n    """.format(cmd=shlex.quote(cmd), run_log_file=run_log_file)\n    # print(cmd)\n    display(HTML(html))\n    \n    p = Popen(["bash", "-c", cmd], stdin=PIPE, stdout=stdout, stderr=stdout)\n    \n    bash_popen_list.append(p)\n    return p\n\n\n@register_line_magic\ndef bash_async(line):\n    bash_popen(line)\n    \n    \ndef show_log_file(file, return_html_string=False):\n    obj = file.replace(\'.\', \'_\').replace(\'/\', \'_\') + "_obj"\n    html_string = \'\'\'\n        <!--MD_BEGIN_FILTER-->\n        <script type=text/javascript>\n        var entrance___OBJ__ = 0;\n        var errors___OBJ__ = 0;\n        function halt__OBJ__(elem, color)\n        {\n            elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    \n        }\n        function refresh__OBJ__()\n        {\n            entrance___OBJ__ -= 1;\n            if (entrance___OBJ__ < 0) {\n                entrance___OBJ__ = 0;\n            }\n            var elem = document.getElementById("__OBJ__");\n            if (elem) {\n                var xmlhttp=new XMLHttpRequest();\n                xmlhttp.onreadystatechange=function()\n                {\n                    var elem = document.getElementById("__OBJ__");\n                    console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___OBJ__);\n                    if (elem && xmlhttp.readyState==4) {\n                        if (xmlhttp.status==200)\n                        {\n                            errors___OBJ__ = 0;\n                            if (!entrance___OBJ__) {\n                                if (elem.innerHTML != xmlhttp.responseText) {\n                                    elem.innerHTML = xmlhttp.responseText;\n                                }\n                                if (elem.innerHTML.includes("Process finished.")) {\n                                    halt__OBJ__(elem, "#333333");\n                                } else {\n                                    entrance___OBJ__ += 1;\n                                    console.log("req");\n                                    window.setTimeout("refresh__OBJ__()", 300); \n                                }\n                            }\n                            return xmlhttp.responseText;\n                        } else {\n                            errors___OBJ__ += 1;\n                            if (!entrance___OBJ__) {\n                                if (errors___OBJ__ < 6) {\n                                    entrance___OBJ__ += 1;\n                                    console.log("req");\n                                    window.setTimeout("refresh__OBJ__()", 300); \n                                } else {\n                                    halt__OBJ__(elem, "#994444");\n                                }\n                            }\n                        }\n                    }\n                }\n                xmlhttp.open("GET", "__FILE__", true);\n                xmlhttp.setRequestHeader("Cache-Control", "no-cache");\n                xmlhttp.send();     \n            }\n        }\n        \n        if (!entrance___OBJ__) {\n            entrance___OBJ__ += 1;\n            refresh__OBJ__(); \n        }\n        </script>\n\n        <p id="__OBJ__" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">\n        </p>\n        \n        </font>\n        <!--MD_END_FILTER-->\n        <!--MD_FROM_FILE __FILE__.md -->\n        \'\'\'.replace("__OBJ__", obj).replace("__FILE__", file)\n    if return_html_string:\n        return html_string\n    display(HTML(html_string))\n\n    \nclass TInteractiveLauncher:\n    tmp_path = "./interactive_launcher_tmp"\n    def __init__(self, cmd):\n        try:\n            os.mkdir(TInteractiveLauncher.tmp_path)\n        except:\n            pass\n        name = str(random.randint(0, 1e18))\n        self.inq_path = os.path.join(TInteractiveLauncher.tmp_path, name + ".inq")\n        self.log_path = os.path.join(TInteractiveLauncher.tmp_path, name + ".log")\n        \n        os.mkfifo(self.inq_path)\n        open(self.log_path, \'w\').close()\n        open(self.log_path + ".md", \'w\').close()\n\n        self.pid = os.fork()\n        if self.pid == -1:\n            print("Error")\n        if self.pid == 0:\n            exe_cands = glob.glob("../tools/launcher.py") + glob.glob("../../tools/launcher.py")\n            assert(len(exe_cands) == 1)\n            assert(os.execvp("python3", ["python3", exe_cands[0], "-l", self.log_path, "-i", self.inq_path, "-c", cmd]) == 0)\n        self.inq_f = open(self.inq_path, "w")\n        interactive_launcher_opened_set.add(self.pid)\n        show_log_file(self.log_path)\n\n    def write(self, s):\n        s = s.encode()\n        assert len(s) == os.write(self.inq_f.fileno(), s)\n        \n    def get_pid(self):\n        n = 100\n        for i in range(n):\n            try:\n                return int(re.findall(r"PID = (\\d+)", open(self.log_path).readline())[0])\n            except:\n                if i + 1 == n:\n                    raise\n                time.sleep(0.1)\n        \n    def input_queue_path(self):\n        return self.inq_path\n        \n    def close(self):\n        self.inq_f.close()\n        os.waitpid(self.pid, 0)\n        os.remove(self.inq_path)\n        # os.remove(self.log_path)\n        self.inq_path = None\n        self.log_path = None \n        interactive_launcher_opened_set.remove(self.pid)\n        self.pid = None\n        \n    @staticmethod\n    def terminate_all():\n        if "interactive_launcher_opened_set" not in globals():\n            globals()["interactive_launcher_opened_set"] = set()\n        global interactive_launcher_opened_set\n        for pid in interactive_launcher_opened_set:\n            print("Terminate pid=" + str(pid), file=sys.stderr)\n            os.kill(pid, signal.SIGKILL)\n            os.waitpid(pid, 0)\n        interactive_launcher_opened_set = set()\n        if os.path.exists(TInteractiveLauncher.tmp_path):\n            shutil.rmtree(TInteractiveLauncher.tmp_path)\n    \nTInteractiveLauncher.terminate_all()\n   \nyandex_metrica_allowed = bool(globals().get("yandex_metrica_allowed", False))\nif yandex_metrica_allowed:\n    display(HTML(\'\'\'<!-- YANDEX_METRICA_BEGIN -->\n    <script type="text/javascript" >\n       (function(m,e,t,r,i,k,a){m[i]=m[i]||function(){(m[i].a=m[i].a||[]).push(arguments)};\n       m[i].l=1*new Date();k=e.createElement(t),a=e.getElementsByTagName(t)[0],k.async=1,k.src=r,a.parentNode.insertBefore(k,a)})\n       (window, document, "script", "https://mc.yandex.ru/metrika/tag.js", "ym");\n\n       ym(59260609, "init", {\n            clickmap:true,\n            trackLinks:true,\n            accurateTrackBounce:true\n       });\n    </script>\n    <noscript><div><img src="https://mc.yandex.ru/watch/59260609" style="position:absolute; left:-9999px;" alt="" /></div></noscript>\n    <!-- YANDEX_METRICA_END -->\'\'\'))\n\ndef make_oneliner():\n    html_text = \'("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований. Если вы не хотите, чтобы по вам собиралась статистика, исправьте: yandex_metrica_allowed = False" if yandex_metrica_allowed else "")\'\n    html_text += \' + "<""!-- MAGICS_SETUP_PRINTING_END -->"\'\n    return \'\'.join([\n        \'# look at tools/set_up_magics.ipynb\\n\',\n        \'yandex_metrica_allowed = True ; get_ipython().run_cell(%s);\' % repr(one_liner_str),\n        \'display(HTML(%s))\' % html_text,\n        \' #\'\'MAGICS_SETUP_END\'\n    ])\n       \n\n');display(HTML(("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований. Если вы не хотите, чтобы по вам собиралась статистика, исправьте: yandex_metrica_allowed = False" if yandex_metrica_allowed else "") + "<""!-- MAGICS_SETUP_PRINTING_END -->")) #MAGICS_SETUP_END
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


# Опрос для всех, кто зашел на эту страницу

Он не страшный, там всего два обязательных вопроса на выбор одного варианта из трёх. Извиняюсь за размер, но к сожалению студенты склонны игнорировать опросы :| 

Пытаюсь компенсировать :)

<a href="https://docs.google.com/forms/d/e/1FAIpQLSdUnBAae8nwdSduZieZv7uatWPOMv9jujCM4meBZcHlTikeXg/viewform?usp=sf_link"><img src="poll.png" width="100%"  align="left" alt="Опрос"></a>

# Криптография, openssl

<table width=100%  > <tr>
    <th width=15%> <b>Видео с семинара &rarr; </b> </th>
    <th>
    <a href="https://www.youtube.com/watch?v=MYAos8P0rfw&list=PLjzMm8llUm4CL-_HgDrmoSTZBCdUk5HQL&index=4"><img src="video.jpg" width="320" 
   height="160" align="left" alt="Видео с семинара"></a>
    </th>
    <th> </th>
</tr> </table>

Сегодня в программе:
* <a href="#hash" style="color:#856024"> Хеши </a>
* <a href="#salt" style="color:#856024"> Соль </a>
* <a href="#symmetric" style="color:#856024"> Симметричное шифрование </a>
* <a href="#asymmetric" style="color:#856024"> Асимметричное шифрование </a>
* <a href="#libcrypto" style="color:#856024"> libcrypto </a>
    

[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/openssl)
  
  
<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>



##  <a name="hash"></a>  Хеши

Цель хеш-функции - конвертировать произвольную последовательность бит (строку) в последовательность бит фиксированной длины (хеш-значение). При этом делать это таким образом, чтобы восстановить по хеш-значению исходную строку было крайне сложно.

(Это очень упрощенно, более детально можно посмотреть [на википедии](https://ru.wikipedia.org/wiki/Криптографическая_хеш-функция#Требования))

Самое очевидное применение хешей - хранение паролей пользователей на диске


```bash
%%bash

echo "user=Vasya password_hash=$(echo -n 12345 | openssl sha256 -r)"
echo "user=Petya password_hash=$(echo -n asfjdjdvsdf | openssl sha256 -r)"
echo "user=admin password_hash=$(echo -n qwerty | openssl sha256 -r)"
```

    user=Vasya password_hash=5994471abb01112afcc18159f6cc74b4f511b99806da59b3caf5a9c173cacfc5 *stdin
    user=Petya password_hash=9513963c366d0baccdbcd507bd1d78fa9c1a21aa102c30af3bb20f167fde8f2e *stdin
    user=admin password_hash=65e84be33532fb784c48129675f9eff3a682b27168c0ea744b2cf58ee02337c5 *stdin


Тогда потенциальному злоумышленнику, чтобы получить пароль (или эквивалент пароля) нужно по хешу восстановить прообраз хеш-функции, что сложно, если пароль сложный, а хеш-функция хорошая.

Однако в приведенной схеме есть дыра. Какая?

<details> <summary> (большой блок из пустых строк) </summary>
  <p> <br><br><br><br><br><br><br><br><br><br><br><br><br><br> </p>
</details>

##  <a name="salt"></a>  Соль


```bash
%%bash
echo "user=Vasya password_hash=$(echo -n 3.1415-2.718182 | openssl sha256 -r)"
echo "user=Petya password_hash=$(echo -n sfkjvdjkth | openssl sha256 -r)"
echo "user=admin password_hash=$(echo -n 3.1415-2.718182 | openssl sha256 -r)"
```

    user=Vasya password_hash=a6f62b5131e63fac2e6f1be3e443a12e58e2c5fea002df0924f58eeefb7e81a9 *stdin
    user=Petya password_hash=eea9d8bec1b74e88807bf93f3a0e095df6543b83d46550456d7f8d2139c0db5c *stdin
    user=admin password_hash=a6f62b5131e63fac2e6f1be3e443a12e58e2c5fea002df0924f58eeefb7e81a9 *stdin


Пусть здесь получить пароли по хешам все еще сложно, 
но можно получить другую информацию: что у `Vasya` и `admin` совпадают пароли.
    
А если злоумышленник мыслит широко, он может просто подкараулить Васю в темном подъезде. И получить пароль админа.

Чтобы злоумышленник не мог получить информацию о совпадении паролей, можно использовать соль.


```bash
%%bash
echo "user=Vasya salt=saltAHFG password_hash=$(echo -n saltAHFG%3.1415-2.718182 | openssl sha256 -r)"
echo "user=Petya salt=saltMSIG password_hash=$(echo -n saltMSIG%sfkjvdjkth | openssl sha256 -r)"
echo "user=admin salt=saltPQNY password_hash=$(echo -n saltPQNY%3.1415-2.718182 | openssl sha256 -r)"
```

    user=Vasya salt=saltAHFG password_hash=0c9ce37e04e94dc13f16304a93b21e7f2c44ca32d6c26fbea3375ea85263aaa0 *stdin
    user=Petya salt=saltMSIG password_hash=df9c27cc066b36be6dc73a39f03e73ec4996b378bfff562421e53bf85f3a99c5 *stdin
    user=admin salt=saltPQNY password_hash=8af9bf88fbe91b010c37d5065c90935c5bb51f5e2898bd92a7235581bd0ccb36 *stdin


Теперь информации о совпадении паролей у злоумышленника так же нет.

##  <a name="symmetric"></a>  Симметричное шифрование

Позволяет шифровать большие объемы текста. Для шифрования и расшифровки используется общий секрет.

### Шифроблокноты

Это самый надежный из симметричных шифров: генерируется случайная последовательность большой длины и становится ключом.

https://ru.wikipedia.org/wiki/Шифр_Вернама

https://habr.com/ru/post/347216/


```python
import random
import base64

def xor(x, y):
    return bytes(a ^ b for a, b in zip(x, y))

%p # both Alice and Bob
common_secret = bytes(random.randint(0, 255) for i in range(35))  # на самом деле тут стоило бы исплользовать более надежный генератор случайных чисел 
%p base64.b64encode(common_secret)  # Содержимое шифроблокнота

%p # Alice → 
plain_text = b"there are several spy secrets here"
%p plain_text  # Текст, который хотим зашифровать (Алиса хочет отправить его Бобу)
cipher_text = xor(plain_text, common_secret)
%p base64.b64encode(cipher_text)  # Шифротекст

%p #  → Bob
recovered_plain_text = xor(cipher_text, common_secret)
%p recovered_plain_text  # Текст, который получил Боб

```


both Alice and Bob



`base64.b64encode(common_secret) = b't3DnjBipwdICZrbWIvgW3q4m7LnfOsLw0zjHji+nuIL7yRg='`  # Содержимое шифроблокнота



Alice →



`plain_text = b'there are several spy secrets here'`  # Текст, который хотим зашифровать (Алиса хочет отправить его Бобу)



`base64.b64encode(cipher_text) = b'wxiC/n2JoKBnRsWzVJ1kv8IGn8mmGrGVsEqi+lyH0OeJrA=='`  # Шифротекст



→ Bob



`recovered_plain_text = b'there are several spy secrets here'`  # Текст, который получил Боб


Но есть очевидный минус - общий секрет должет быть размера не меньшего, чем весь объем отправляемых данных.

## Блочное шифррование

По сути пара функций: `output_block = E(input_block, secret)` и обратная к ней. `output_block`, `input_block` и `secret`- строки фиксированной длины. Обычно число фигурирующее в названии блочного шифра (aes-256) - это длина ключа в битах.

https://ru.wikipedia.org/wiki/Блочный_шифр

https://ru.wikipedia.org/wiki/Блочный_шифр#Определение

https://ru.wikipedia.org/wiki/Режим_шифрования#Counter_mode_(CTR)

Казалось бы теперь можно просто зашифровать текст, просто применив функцию блочного шифра поблочно к тексту (называется режимом шифрования ECB), но нет! Иначе есть шанс получить что-то такое :)

<table> 
<tr>
    <th> Исходное изображение </th> <th> Изображение зашифрованное в режиме ECB </th> 
</tr>
<tr>
    <th> 
        <img src="https://upload.wikimedia.org/wikipedia/commons/thumb/3/35/Tux.svg/300px-Tux.svg.png" width="200" height="200" align="left" alt="Видео с семинара">
    </th>
    <th>
        <img src="https://upload.wikimedia.org/wikipedia/commons/f/f0/Tux_ecb.jpg" width="200" height="200" align="left" alt="Видео с семинара">
    </th>
</tr> 
</table>



Один из режимов шифрования (способов использования функции блочного шифра) - режим CTR. С ним нет такой проблемы как с ECB. И он очень простой по сути.

Идея в том, чтобы как бы генерировать шифроблокнот на ходу, используя функцию блочного шифра.

Примерно так: `E(nonce*1e9 + 0, secret), E(nonce*1e9 + 1, secret), E(nonce*1e9 + 2, secret), ...`.

`nonce` - Number used ONCE - однократно используемое число. Чтобы функция блочного шифра с одним ключом никогда не применялась для шифрования одного и того же входного блока. `nonce` обычно передается в незашифрованном виде.


```bash
%%bash
export MY_PASSWORD=MY_SECRET_PASSWORD

echo "Alice → "
echo -n "Some secret message" > plain_text.txt
echo "  Plain text: '$(cat plain_text.txt)'"
SALT=$(openssl rand -hex 8)
echo "  Salt is: $SALT"
openssl enc -aes-256-ctr -e -S $SALT -in plain_text.txt -out cipher_text.txt -pass env:MY_PASSWORD
echo "  Ciphertext base64: '$(base64 cipher_text.txt)'"

echo "→  Bob"
echo "  Ciphertext base64: '$(base64 cipher_text.txt)'"
openssl enc -aes-256-ctr -d -in cipher_text.txt -out recovered_plain_text.txt -pass env:MY_PASSWORD
echo "  Recovered plaintext: '$(cat recovered_plain_text.txt)'"
```

    Alice → 
      Plain text: 'Some secret message'
      Salt is: 220b054eafaafa61
      Ciphertext base64: 'U2FsdGVkX18iCwVOr6r6YU5US9agAA8WWuDCi5HSNuoPSaY='
    →  Bob
      Ciphertext base64: 'U2FsdGVkX18iCwVOr6r6YU5US9agAA8WWuDCi5HSNuoPSaY='
      Recovered plaintext: 'Some secret message'


Можно еще глянуть на структуру зашифрованного с помощью утилиты сообщения:


```bash
%%bash
export MY_PASSWORD=MY_SECRET_PASSWORD
echo -n "Some secret message!" > plain_text.txt
SALT='66AA1122060A0102'

echo "Case 1. Use pass phrase:"
echo "Plain text: '$(cat plain_text.txt)' ($(cat plain_text.txt | wc -c) bytes)"                                             | sed -e 's/^/  /'
# sed -e 's/^/  /' -- просто добавляет отступ в два пробела к каждой выведенной строке
# -p -- опция, чтобы выводить соль, ключ, стартовый вектор
openssl enc -aes-256-ctr -S $SALT -in plain_text.txt -out cipher_text.txt -pass env:MY_PASSWORD -p                           | sed -e 's/^/  /'
echo -e "Ciphertexthexdump: '''\n$(hexdump cipher_text.txt -C)\n''' ($(cat cipher_text.txt | wc -c) bytes)"                     | sed -e 's/^/  /'
openssl enc -aes-256-ctr -d -in cipher_text.txt -out recovered_plain_text.txt -pass env:MY_PASSWORD 
echo "Recovered plaintext: '$(cat recovered_plain_text.txt)'"                                                                | sed -e 's/^/  /'


IV='E4DEC57ADC9A771DC72A77775A1CF4FF'
KEY='BBC5929AA59B56851391DD723922C2E0F31A2FC873D52D3FBA3FD5391CAD471E'
echo "Case 2. Use explicit key and IV:"
echo "Plain text: '$(cat plain_text.txt)' ($(cat plain_text.txt | wc -c) bytes)"                                             | sed -e 's/^/  /'
openssl enc -aes-256-ctr -in plain_text.txt -out cipher_text.txt -iv $IV -K $KEY -p                                          | sed -e 's/^/  /'
echo -e "Ciphertexthexdump: '''\n$(hexdump cipher_text.txt -C)\n''' ($(cat cipher_text.txt | wc -c) bytes)"                     | sed -e 's/^/  /'
openssl enc -aes-256-ctr -d -in cipher_text.txt -out recovered_plain_text.txt -iv $IV -K $KEY
echo "Recovered plaintext: '$(cat recovered_plain_text.txt)'"                                                                | sed -e 's/^/  /'


echo "Case 3. Encode with EBC mode and decode with CTR mode (IV=0):"
IV='00000000000000000000000000000000'
KEY='BBC5929AA59B56851391DD723922C2E0F31A2FC873D52D3FBA3FD5391CAD471E'
echo -n -e "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" > plain_text.txt
echo -e "Plain text: '''$(cat plain_text.txt | hexdump -v -e '/1 "%02X "')''' ($(cat plain_text.txt | wc -c) bytes)"                             | sed -e 's/^/  /'
openssl enc -aes-256-ecb -in plain_text.txt -out cipher_text.txt -K $KEY -p                                          | sed -e 's/^/  /'
echo -e "Ciphertexthexdump: '''\n$(hexdump cipher_text.txt -C)\n''' ($(cat cipher_text.txt | wc -c) bytes)"                     | sed -e 's/^/  /'
openssl enc -aes-256-ctr -d -in cipher_text.txt -out recovered_plain_text.txt -iv $IV -K $KEY
echo -e "Recovered plaintext: '''\n$(hexdump recovered_plain_text.txt)\n''' ($(cat recovered_plain_text.txt | wc -c) bytes)" | sed -e 's/^/  /'

```

    Case 1. Use pass phrase:
      Plain text: 'Some secret message!' (20 bytes)
      salt=66AA1122060A0102
      key=BBC5929AA59B56851391DD723922C2E0F31A2FC873D52D3FBA3FD5391CAD471E
      iv =E4DEC57ADC9A771DC72A77775A1CF4FF
      Ciphertexthexdump: '''
      00000000  53 61 6c 74 65 64 5f 5f  66 aa 11 22 06 0a 01 02  |Salted__f.."....|
      00000010  ca 12 3b 51 34 0e 2d 52  3c 38 36 66 6f 74 4f 57  |..;Q4.-R<86fotOW|
      00000020  bc 8b d6 e0                                       |....|
      00000024
      ''' (36 bytes)
      Recovered plaintext: 'Some secret message!'
    Case 2. Use explicit key and IV:
      Plain text: 'Some secret message!' (20 bytes)
      salt=0000000000000000
      key=BBC5929AA59B56851391DD723922C2E0F31A2FC873D52D3FBA3FD5391CAD471E
      iv =E4DEC57ADC9A771DC72A77775A1CF4FF
      Ciphertexthexdump: '''
      00000000  ca 12 3b 51 34 0e 2d 52  3c 38 36 66 6f 74 4f 57  |..;Q4.-R<86fotOW|
      00000010  bc 8b d6 e0                                       |....|
      00000014
      ''' (20 bytes)
      Recovered plaintext: 'Some secret message!'
    Case 3. Encode with EBC mode and decode with CTR mode (IV=0):
      Plain text: '''00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ''' (20 bytes)
      salt=0000000000000000
      key=BBC5929AA59B56851391DD723922C2E0F31A2FC873D52D3FBA3FD5391CAD471E
      Ciphertexthexdump: '''
      00000000  3b 0f 19 a6 fe c8 60 68  14 6a a6 8f 49 4b 03 bb  |;.....`h.j..IK..|
      00000010  76 92 ad 2e d1 02 aa 75  08 0e 27 92 a4 0e 87 30  |v......u..'....0|
      00000020
      ''' (32 bytes)
      Recovered plaintext: '''
      0000000 0000 0000 0000 0000 0000 0000 0000 0000
      0000010 6fb5 ede7 1427 2c9b b25b c5d6 f4be a0be
      0000020
      ''' (32 bytes)


Несложно догадаться, что в Case 1 добавляется 16 байт метаинформации. И в этих байтах легко узнается наша соль и слово `Salted__`.

А в Case 2 ничего не добавляется (длина не увеличивается по сравнению с plaintext). Так что судя по всему там просто xor со сгенерированным шифроблокнотом

Case 3 просто извращенный пример: первый блок текста $P_0$ шифруется в режиме ECB, получается $E_k(P_0)$. А потом декодируется в режиме CTR (с IV=0), получается $E_k(P_0)$ ^ $E_k(0)$. А так как $P_0$ в примере сам равен 0, то получается, что $E_k(P_0)$ ^ $E_k(0) = E_k(0)$ ^ $E_k(0) = 0$. То есть удачненько так расшифрованное совпало с исходным текстом :) Это вообще не то, что может пригодиться на практике, просто забавный примерчик.


```python
!openssl rand -base64 30
```

## Имитовставка

Шифроблокноты хорошо защищают текст, от того, чтобы злоумышленник смог этот текст узнать. Но что если злоумышленник и так знает текст (документ с размерами зарплат), и его цель подменить там одно число? Тогда ему не нужно расшифровывать документ, он может его перехватить, инвертировать один бит в нужном месте и отправить дальше.

Бороться с этим можно хемсуммой. При этом не простой (чтобы злоумышленник не мог ее пересчитать), а параметризованной ключом шифрования. Такая хешсумма называется имитовставкой.

##  <a name="asymmetric"></a>  Acимметричное шифрование

В симметричном шифровании у отправителя и получателя должен быть общий секрет. А что делать если его нет? Использовать асимметричное шифрование!

Обычно применяется для обмена некоторой метаинформацией и получения общего секрета.


### Протокол Диффи-Хеллмана

Допустим два агента хотят пообщаться, но у них нет общего ключа и их могу прослушивать. Что делать?

Использовать труднорешаемую задачу :)

Например, это может быть задача дискретного логарифмирования (взятия логарифма в кольце по модулю).

Тогда агенты A и B могут сообща выбрать основание $x$ (через незащищенный канал), потом раздельно выбрать случайные числа $a$, $b$. Возвести $x$ в эти степени и обменяться полученными $x^a$, $x^b$ через незащищенный канал.

Фокус в том, что сейчас люди не умеют по $x$ и $x^a$ находить $a$. Так что $x^a$ передавать безопасно.

А дальше второй фокус: агент A может сделать $(x^b)^a = x^{(a \cdot b)}$, а агент B - $(x^a)^b = x^{(a \cdot b)}$. И получается, что у A и B есть общий секрет. А злоумышленник имея только $x, x^b, x^a$ не может получить $x^{(a \cdot b)}$.

https://ru.wikipedia.org/wiki/Протокол_Диффи_—_Хеллмана

### RSA 

(Rivest, Shamir и Adleman)

https://ru.wikipedia.org/wiki/RSA#Алгоритм_создания_открытого_и_секретного_ключей


```bash
%%bash

echo "+++ Alice generate key"
openssl genrsa -out alice_private_key 2048 2>&1
openssl rsa -in alice_private_key -out alice_public_key -pubout 2>&1

echo "Bob → "
echo -n "Bob's secret message" > bobs_plaintext
echo "  Bob ciphers message: '$(cat bobs_plaintext)'"
openssl rsautl -encrypt -pubin -inkey alice_public_key -in bobs_plaintext -out bobs_ciphertext
echo "  Encrypted message: $(base64 bobs_ciphertext)"

echo "→ Alice"
openssl rsautl -decrypt -inkey alice_private_key -in bobs_ciphertext -out recovered_bobs_plaintext
echo "  Decrypted message: '$(cat recovered_bobs_plaintext)'"
```

    +++ Alice generate key
    Generating RSA private key, 2048 bit long modulus
    .................................................................................................................................................................................................+++
    ..........................................+++
    e is 65537 (0x10001)
    writing RSA key
    Bob → 
      Bob ciphers message: 'Bob's secret message'
      Encrypted message: ZqHFa3tETajZoyUx+yxmi4utzVQaeRTBywtm7sohctBUtI6OYIRF1h5lmjf3KWuUrEM+rEpHeeHu
    wXEl1aHf+G2kAsxtHl2zwxBhHxF8Y1PFAjEDLZaOFrS9BmeH4Zlz2MtOIlDUeZjR/ejUM4Jvq0ig
    9LMwed9QLzsZZz9H1pX4CIY1SDMyW/R4FgkvS9lA1RErybIYLUq9wzU04Z0EBXjDm18dkpiU3I/5
    JJimI767Yxgh8QxYhjK7Xo06fgj+dZ+pIEOAB+QOgPrEQucr6zGmQopOCT5uwnEKjBjDpZzvTf40
    sPxjqHJpeOXmTS1P/65MpsSx4tmpXIVbmAS4ig==
    → Alice
      Decrypted message: 'Bob's secret message'


В этом примере RSA использовался не по назначению, так как нельзя зашифровать текст, который длиннее ключа. При передаче большого текста, стоило через RSA договориться об общем секрете, а потом передавать большой текст используя блочное шифрование.

##  <a name="libcrypto"></a>  libcrypto

Ссылочки:

https://wiki.openssl.org/index.php/Libcrypto_API

https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption - отсюда взят пример

https://github.com/openssl/openssl

https://www.openssl.org/docs/man1.1.1/

И пример с шифрованием-дешифрованием с блочным шифром AES-256 в режиме CTR.


```python
!mkdir libcrypto_example || true
```


```python
%%cmake libcrypto_example/CMakeLists.txt

cmake_minimum_required(VERSION 2.8) 

set(CMAKE_CXX_FLAGS "-std=c++17")

find_package(OpenSSL COMPONENTS crypto REQUIRED)

add_executable(main main.cpp)
# set_property(TARGET main PROPERTY CXX_STANDARD 17)
target_include_directories(main PUBLIC ${OPENSSL_INCLUDE_DIR}) 
target_link_libraries(main ${OPENSSL_CRYPTO_LIBRARY})            
```


```python
%%cpp libcrypto_example/main.cpp
%run mkdir libcrypto_example/build 
%run cd libcrypto_example/build && cmake .. 2>&1 > /dev/null && make
%run libcrypto_example/build/main 
%run rm -r libcrypto_example/build

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <iostream>
#include <array>

#define EVP_ASSERT(stmt) do { if (!(stmt)) { \
    fprintf(stderr, "Statement failed: %s\n", #stmt); \
    ERR_print_errors_fp(stderr); \
    abort(); \
} } while (false)

struct TByteString: std::vector<unsigned char> {
    using std::vector<unsigned char>::vector;
    int ssize() { return static_cast<int>(size()); }
    char* SignedData() { reinterpret_cast<const char*>(data()); };
};

TByteString operator "" _b(const char* data, std::size_t len) {
    auto start = reinterpret_cast<const unsigned char*>(data);
    return {start, start + len};
}

template <char ...chars> TByteString operator "" _b() {
    char hex[] = {chars...};
    assert(strncmp(hex, "0x", 2) == 0 && sizeof(hex) % 2 == 0);
    TByteString result;
    for (const char* ch = hex + 2; ch < hex + sizeof(hex); ch += 2) { 
        result.push_back(std::strtol(std::array<char, 3>{ch[0], ch[1], 0}.data(), nullptr, 16));
    }
    return result;
}


TByteString Encrypt(const TByteString& plaintext, const TByteString& key, const TByteString& iv) {
    TByteString ciphertext(plaintext.size(), 0); // Верно для режима CTR, для остальных может быть не так
     
    auto* ctx = EVP_CIPHER_CTX_new();
    EVP_ASSERT(ctx);

    assert(key.size() * 8 == 256); // check key size for aes_256
    assert(iv.size() * 8 == 128); // check iv size for cipher with block size of 128 bits
    EVP_ASSERT(1 == EVP_EncryptInit_ex(ctx, EVP_aes_256_ctr(), NULL, key.data(), iv.data()));

    int len;
    // В эту функцию можно передавать исходный текст по частям, выход так же пишется по частям
    EVP_ASSERT(1 == EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size()));
    // В конце что-то могло остаться в буфере ctx и это нужно дописать
    EVP_ASSERT(1 == EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len));
    
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext;
}

TByteString Decrypt(const TByteString& ciphertext, const TByteString& key, const TByteString& iv) {
    TByteString plaintext(ciphertext.size(), 0);
    
    auto* ctx = EVP_CIPHER_CTX_new();
    EVP_ASSERT(ctx);
    
    assert(key.size() * 8 == 256); // check key size for aes_256
    assert(iv.size() * 8 == 128); // check iv size for cipher with block size of 128 bits
    EVP_ASSERT(1 == EVP_DecryptInit_ex(ctx, EVP_aes_256_ctr(), NULL, key.data(), iv.data()));

    int len;
    EVP_ASSERT(1 == EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()));
    EVP_ASSERT(1 == EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len));
    
    EVP_CIPHER_CTX_free(ctx);
    return plaintext;
}

int main () {
    TByteString key = 0x0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF_b; // A 256 bit key (common secret)
    TByteString iv = 0xFEDCBA9876543210FEDCBA9876543210_b; // A 128 bit IV (initialization vector, can be public)
    
    printf("Alice →\n");
    TByteString plaintext = "The quick brown fox jumps over the lazy dog"_b; // Message to be encrypted
    printf("  Message to be encrypted: '%.*s'\n", plaintext.ssize(), plaintext.SignedData());
    TByteString ciphertext = Encrypt(plaintext, key, iv); // Encrypt the plaintext
    printf("  Ciphertext is:\n");
    BIO_dump_fp(stdout, ciphertext.SignedData(), ciphertext.size()); // Just pretty output
    
    printf("→ Bob\n");
    TByteString decryptedText = Decrypt(ciphertext, key, iv); // Decrypt the ciphertext

    printf("  Decrypted text is: '%.*s'\n", decryptedText.ssize(), decryptedText.SignedData());
    return 0;
}
```


Run: `mkdir libcrypto_example/build`



Run: `cd libcrypto_example/build && cmake .. 2>&1 > /dev/null && make`


    [35m[1mScanning dependencies of target main[0m
    [ 50%] [32mBuilding CXX object CMakeFiles/main.dir/main.cpp.o[0m
    [100%] [32m[1mLinking CXX executable main[0m
    [100%] Built target main



Run: `libcrypto_example/build/main`


    Alice →
      Message to be encrypted: 'The quick brown fox jumps over the lazy dog'
      Ciphertext is:
    0000 - e9 08 8b 57 6d 25 04 33-e6 56 b0 1d 35 aa 1e 19   ...Wm%.3.V..5...
    0010 - f4 ce 66 b4 b3 b8 4f a2-3c 0b 8c 18 ea 72 49 45   ..f...O.<....rIE
    0020 - 95 1d 44 b3 72 9d b9 5a-20 d0 e2                  ..D.r..Z ..
    → Bob
      Decrypted text is: 'The quick brown fox jumps over the lazy dog'



Run: `rm -r libcrypto_example/build`


Воспроизведем тот же шифротекст с помощью консольной тулзы. (hex-представление key и iv скопировано из предыдущего примера)


```bash
%%bash
KEY=0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF
IV=FEDCBA9876543210FEDCBA9876543210

echo -n 'The quick brown fox jumps over the lazy dog' | openssl enc -e -aes-256-ctr -K $KEY -iv $IV | hexdump -C
```

    00000000  e9 08 8b 57 6d 25 04 33  e6 56 b0 1d 35 aa 1e 19  |...Wm%.3.V..5...|
    00000010  f4 ce 66 b4 b3 b8 4f a2  3c 0b 8c 18 ea 72 49 45  |..f...O.<....rIE|
    00000020  95 1d 44 b3 72 9d b9 5a  20 d0 e2                 |..D.r..Z ..|
    0000002b



```python

```


```python

```

# <a name="hw"></a> Комментарии к ДЗ

* Не путайте режимы шифрования
* Откуда взять соль? Зашифруйте что-нибудь с помощью тулзы, явно указав соль, и откройте зашифрованный файлик hexdump'ом с ascii колонкой. Ответ станет очевидным
* Не выводите бинарные данные в терминал (результаты шифрования), а то можете удивиться. Лучше использовать hexdump: `echo 'suppose_it_is_binary_data' | hexdump -C`
* Частая ошибка: использование опции `-a` для генерации шифротекста.
* 
```cpp
EVP_CIPHER_key_length(...)
EVP_CIPHER_iv_length(...)
```


```python

```


```python

```
