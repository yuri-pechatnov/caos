```python
from IPython.core.magic import register_cell_magic

@register_cell_magic
def save_cell_as_string(string_name, cell):
    cell = "# " + string_name + " #SET_UP_MAGIC_BEGIN" + "\n" + cell + "\n #SET_UP_MAGIC_END \n"
    globals()[string_name] = cell
    # c = compile(cell, "<cell>", "exec")
    get_ipython().run_cell(cell)
```


```python
%%save_cell_as_string one_liner_str

get_ipython().run_cell_magic('javascript', '', '// setup cpp code highlighting\nIPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\'reg\':[/^%%cpp/]} ;')

# creating magics
from IPython.core.magic import register_cell_magic, register_line_magic
from IPython.display import display, Markdown, HTML
import argparse
from subprocess import Popen, PIPE
import random
import sys
import os
import re
import signal
import shutil
import shlex
import glob

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
            line_to_write = (line if not args.ejudge_style else line.rstrip()) + "\n"
            if line.startswith("%"):
                run_prefix = "%run "
                if line.startswith(run_prefix):
                    cmds.append(line[len(run_prefix):].strip())
                    f.write(line_comment_start + " " + line_to_write)
                    continue
                run_prefix = "%# "
                if line.startswith(run_prefix):
                    f.write(line_comment_start + " " + line_to_write)
                    continue
                raise Exception("Unknown %%save_file subcommand: '%s'" % line)
            else:
                f.write(line_to_write)
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
        
def show_file(file, clear_at_begin=True, return_html_string=False):
    if clear_at_begin:
        get_ipython().system("truncate --size 0 " + file)
    obj = file.replace('.', '_').replace('/', '_') + "_obj"
    html_string = '''
        <!--MD_BEGIN_FILTER-->
        <script type=text/javascript>
        var entrance___OBJ__ = 0;
        var errors___OBJ__ = 0;
        function refresh__OBJ__()
        {
            entrance___OBJ__ -= 1;
            var elem = document.getElementById("__OBJ__");
            if (elem) {
                var xmlhttp=new XMLHttpRequest();
                xmlhttp.onreadystatechange=function()
                {
                    var elem = document.getElementById("__OBJ__");
                    console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___OBJ__);
                    if (elem && xmlhttp.readyState==4) {
                        if (xmlhttp.status==200)
                        {
                            errors___OBJ__ = 0;
                            if (!entrance___OBJ__) {
                                elem.innerText = xmlhttp.responseText;
                                entrance___OBJ__ += 1;
                                console.log("req");
                                window.setTimeout("refresh__OBJ__()", 300); 
                            }
                            return xmlhttp.responseText;
                        } else {
                            errors___OBJ__ += 1;
                            if (errors___OBJ__ < 10 && !entrance___OBJ__) {
                                entrance___OBJ__ += 1;
                                console.log("req");
                                window.setTimeout("refresh__OBJ__()", 300); 
                            }
                        }
                    }
                }
                xmlhttp.open("GET", "__FILE__", true);
                xmlhttp.setRequestHeader("Cache-Control", "no-cache");
                xmlhttp.send();     
            }
        }
        
        if (!entrance___OBJ__) {
            entrance___OBJ__ += 1;
            refresh__OBJ__(); 
        }
        </script>
        
        <font color="white"> <tt>
        <p id="__OBJ__" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>
        </tt> </font>
        <!--MD_END_FILTER-->
        <!--MD_FROM_FILE __FILE__ -->
        '''.replace("__OBJ__", obj).replace("__FILE__", file)
    if return_html_string:
        return html_string
    display(HTML(html_string))
    
BASH_POPEN_TMP_DIR = "./bash_popen_tmp"
    
def bash_popen_terminate_all():
    for p in globals().get("bash_popen_list", []):
        print("Terminate pid=" + str(p.pid), file=sys.stderr)
        p.terminate()
    globals()["bash_popen_list"] = []
    if os.path.exists(BASH_POPEN_TMP_DIR):
        shutil.rmtree(BASH_POPEN_TMP_DIR)

bash_popen_terminate_all()  

def bash_popen(cmd):
    if not os.path.exists(BASH_POPEN_TMP_DIR):
        os.mkdir(BASH_POPEN_TMP_DIR)
    h = os.path.join(BASH_POPEN_TMP_DIR, str(random.randint(0, 1e18)))
    stdout_file = h + ".out.html"
    stderr_file = h + ".err.html"
    run_log_file = h + ".fin.html"
    
    stdout = open(stdout_file, "wb")
    stdout = open(stderr_file, "wb")
    
    html = """
    <table width="100%">
    <colgroup>
       <col span="1" style="width: 70px;">
       <col span="1">
    </colgroup>    
    <tbody>
      <tr> <td><b>STDOUT</b></td> <td> {stdout} </td> </tr>
      <tr> <td><b>STDERR</b></td> <td> {stderr} </td> </tr>
      <tr> <td><b>RUN LOG</b></td> <td> {run_log} </td> </tr>
    </tbody>
    </table>
    """.format(
        stdout=show_file(stdout_file, return_html_string=True),
        stderr=show_file(stderr_file, return_html_string=True),
        run_log=show_file(run_log_file, return_html_string=True),
    )
    
    cmd = """
        bash -c {cmd} &
        pid=$!
        echo "Process started! pid=${{pid}}" > {run_log_file}
        wait ${{pid}}
        echo "Process finished! exit_code=$?" >> {run_log_file}
    """.format(cmd=shlex.quote(cmd), run_log_file=run_log_file)
    # print(cmd)
    display(HTML(html))
    
    p = Popen(["bash", "-c", cmd], stdin=PIPE, stdout=stdout, stderr=stdout)
    
    bash_popen_list.append(p)
    return p


@register_line_magic
def bash_async(line):
    bash_popen(line)
    
    
def show_log_file(file, return_html_string=False):
    obj = file.replace('.', '_').replace('/', '_') + "_obj"
    html_string = '''
        <!--MD_BEGIN_FILTER-->
        <script type=text/javascript>
        var entrance___OBJ__ = 0;
        var errors___OBJ__ = 0;
        function halt__OBJ__(elem, color)
        {
            elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    
        }
        function refresh__OBJ__()
        {
            entrance___OBJ__ -= 1;
            if (entrance___OBJ__ < 0) {
                entrance___OBJ__ = 0;
            }
            var elem = document.getElementById("__OBJ__");
            if (elem) {
                var xmlhttp=new XMLHttpRequest();
                xmlhttp.onreadystatechange=function()
                {
                    var elem = document.getElementById("__OBJ__");
                    console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___OBJ__);
                    if (elem && xmlhttp.readyState==4) {
                        if (xmlhttp.status==200)
                        {
                            errors___OBJ__ = 0;
                            if (!entrance___OBJ__) {
                                if (elem.innerHTML != xmlhttp.responseText) {
                                    elem.innerHTML = xmlhttp.responseText;
                                }
                                if (elem.innerHTML.includes("Process finished.")) {
                                    halt__OBJ__(elem, "#333333");
                                } else {
                                    entrance___OBJ__ += 1;
                                    console.log("req");
                                    window.setTimeout("refresh__OBJ__()", 300); 
                                }
                            }
                            return xmlhttp.responseText;
                        } else {
                            errors___OBJ__ += 1;
                            if (!entrance___OBJ__) {
                                if (errors___OBJ__ < 6) {
                                    entrance___OBJ__ += 1;
                                    console.log("req");
                                    window.setTimeout("refresh__OBJ__()", 300); 
                                } else {
                                    halt__OBJ__(elem, "#994444");
                                }
                            }
                        }
                    }
                }
                xmlhttp.open("GET", "__FILE__", true);
                xmlhttp.setRequestHeader("Cache-Control", "no-cache");
                xmlhttp.send();     
            }
        }
        
        if (!entrance___OBJ__) {
            entrance___OBJ__ += 1;
            refresh__OBJ__(); 
        }
        </script>

        <p id="__OBJ__" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">
        </p>
        
        </font>
        <!--MD_END_FILTER-->
        <!--MD_FROM_FILE __FILE__.md -->
        '''.replace("__OBJ__", obj).replace("__FILE__", file)
    if return_html_string:
        return html_string
    display(HTML(html_string))

    
class TInteractiveLauncher:
    tmp_path = "./interactive_launcher_tmp"
    def __init__(self, cmd):
        try:
            os.mkdir(TInteractiveLauncher.tmp_path)
        except:
            pass
        name = str(random.randint(0, 1e18))
        self.inq_path = os.path.join(TInteractiveLauncher.tmp_path, name + ".inq")
        self.log_path = os.path.join(TInteractiveLauncher.tmp_path, name + ".log")
        
        os.mkfifo(self.inq_path)
        open(self.log_path, 'w').close()
        open(self.log_path + ".md", 'w').close()

        self.pid = os.fork()
        if self.pid == -1:
            print("Error")
        if self.pid == 0:
            exe_cands = glob.glob("../tools/launcher.py") + glob.glob("../../tools/launcher.py")
            assert(len(exe_cands) == 1)
            assert(os.execvp("python3", ["python3", exe_cands[0], "-l", self.log_path, "-i", self.inq_path, "-c", cmd]) == 0)
        self.inq_f = open(self.inq_path, "w")
        interactive_launcher_opened_set.add(self.pid)
        show_log_file(self.log_path)

    def write(self, s):
        s = s.encode()
        assert len(s) == os.write(self.inq_f.fileno(), s)
        
    def get_pid(self):
        n = 100
        for i in range(n):
            try:
                return int(re.findall(r"PID = (\d+)", open(self.log_path).readline())[0])
            except:
                if i + 1 == n:
                    raise
                time.sleep(0.1)
        
    def input_queue_path(self):
        return self.inq_path
        
    def close(self):
        self.inq_f.close()
        os.waitpid(self.pid, 0)
        os.remove(self.inq_path)
        # os.remove(self.log_path)
        self.inq_path = None
        self.log_path = None 
        interactive_launcher_opened_set.remove(self.pid)
        self.pid = None
        
    @staticmethod
    def terminate_all():
        if "interactive_launcher_opened_set" not in globals():
            globals()["interactive_launcher_opened_set"] = set()
        global interactive_launcher_opened_set
        for pid in interactive_launcher_opened_set:
            print("Terminate pid=" + str(pid), file=sys.stderr)
            os.kill(pid, signal.SIGKILL)
            os.waitpid(pid, 0)
        interactive_launcher_opened_set = set()
        if os.path.exists(TInteractiveLauncher.tmp_path):
            shutil.rmtree(TInteractiveLauncher.tmp_path)
    
TInteractiveLauncher.terminate_all()
   
yandex_metrica_allowed = bool(globals().get("yandex_metrica_allowed", False))
if yandex_metrica_allowed:
    display(HTML('''<!-- Yandex.Metrika counter -->
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
    <!-- /Yandex.Metrika counter -->'''))

def make_oneliner():
    return ''.join([
        '# look at tools/set_up_magics.ipynb\n',
        'yandex_metrica_allowed = True ; get_ipython().run_cell(%s);' % repr(one_liner_str),
        'display(HTML("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований. Если вы не хотите, чтобы по вам собиралась статистика, исправьте: yandex_metrica_allowed = False" if yandex_metrica_allowed else ""))' 
    ]) 
        
        
```


    <IPython.core.display.Javascript object>



```python
print(make_oneliner())

```

    # look at tools/set_up_magics.ipynb
    yandex_metrica_allowed = True ; get_ipython().run_cell('# one_liner_str #SET_UP_MAGIC_BEGIN\n\nget_ipython().run_cell_magic(\'javascript\', \'\', \'// setup cpp code highlighting\\nIPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\\\'reg\\\':[/^%%cpp/]} ;\')\n\n# creating magics\nfrom IPython.core.magic import register_cell_magic, register_line_magic\nfrom IPython.display import display, Markdown, HTML\nimport argparse\nfrom subprocess import Popen, PIPE\nimport random\nimport sys\nimport os\nimport re\nimport signal\nimport shutil\nimport shlex\nimport glob\n\n@register_cell_magic\ndef save_file(args_str, cell, line_comment_start="#"):\n    parser = argparse.ArgumentParser()\n    parser.add_argument("fname")\n    parser.add_argument("--ejudge-style", action="store_true")\n    args = parser.parse_args(args_str.split())\n    \n    cell = cell if cell[-1] == \'\\n\' or args.no_eof_newline else cell + "\\n"\n    cmds = []\n    with open(args.fname, "w") as f:\n        f.write(line_comment_start + " %%cpp " + args_str + "\\n")\n        for line in cell.split("\\n"):\n            line_to_write = (line if not args.ejudge_style else line.rstrip()) + "\\n"\n            if line.startswith("%"):\n                run_prefix = "%run "\n                if line.startswith(run_prefix):\n                    cmds.append(line[len(run_prefix):].strip())\n                    f.write(line_comment_start + " " + line_to_write)\n                    continue\n                run_prefix = "%# "\n                if line.startswith(run_prefix):\n                    f.write(line_comment_start + " " + line_to_write)\n                    continue\n                raise Exception("Unknown %%save_file subcommand: \'%s\'" % line)\n            else:\n                f.write(line_to_write)\n        f.write("" if not args.ejudge_style else line_comment_start + r" line without \\n")\n    for cmd in cmds:\n        display(Markdown("Run: `%s`" % cmd))\n        get_ipython().system(cmd)\n\n@register_cell_magic\ndef cpp(fname, cell):\n    save_file(fname, cell, "//")\n\n@register_cell_magic\ndef asm(fname, cell):\n    save_file(fname, cell, "//")\n    \n@register_cell_magic\ndef makefile(fname, cell):\n    assert not fname\n    save_file("makefile", cell.replace(" " * 4, "\\t"))\n        \n@register_line_magic\ndef p(line):\n    try:\n        expr, comment = line.split(" #")\n        display(Markdown("`{} = {}`  # {}".format(expr.strip(), eval(expr), comment.strip())))\n    except:\n        display(Markdown("{} = {}".format(line, eval(line))))\n        \ndef show_file(file, clear_at_begin=True, return_html_string=False):\n    if clear_at_begin:\n        get_ipython().system("truncate --size 0 " + file)\n    obj = file.replace(\'.\', \'_\').replace(\'/\', \'_\') + "_obj"\n    html_string = \'\'\'\n        <!--MD_BEGIN_FILTER-->\n        <script type=text/javascript>\n        var entrance___OBJ__ = 0;\n        var errors___OBJ__ = 0;\n        function refresh__OBJ__()\n        {\n            entrance___OBJ__ -= 1;\n            var elem = document.getElementById("__OBJ__");\n            if (elem) {\n                var xmlhttp=new XMLHttpRequest();\n                xmlhttp.onreadystatechange=function()\n                {\n                    var elem = document.getElementById("__OBJ__");\n                    console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___OBJ__);\n                    if (elem && xmlhttp.readyState==4) {\n                        if (xmlhttp.status==200)\n                        {\n                            errors___OBJ__ = 0;\n                            if (!entrance___OBJ__) {\n                                elem.innerText = xmlhttp.responseText;\n                                entrance___OBJ__ += 1;\n                                console.log("req");\n                                window.setTimeout("refresh__OBJ__()", 300); \n                            }\n                            return xmlhttp.responseText;\n                        } else {\n                            errors___OBJ__ += 1;\n                            if (errors___OBJ__ < 10 && !entrance___OBJ__) {\n                                entrance___OBJ__ += 1;\n                                console.log("req");\n                                window.setTimeout("refresh__OBJ__()", 300); \n                            }\n                        }\n                    }\n                }\n                xmlhttp.open("GET", "__FILE__", true);\n                xmlhttp.setRequestHeader("Cache-Control", "no-cache");\n                xmlhttp.send();     \n            }\n        }\n        \n        if (!entrance___OBJ__) {\n            entrance___OBJ__ += 1;\n            refresh__OBJ__(); \n        }\n        </script>\n        \n        <font color="white"> <tt>\n        <p id="__OBJ__" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>\n        </tt> </font>\n        <!--MD_END_FILTER-->\n        <!--MD_FROM_FILE __FILE__ -->\n        \'\'\'.replace("__OBJ__", obj).replace("__FILE__", file)\n    if return_html_string:\n        return html_string\n    display(HTML(html_string))\n    \nBASH_POPEN_TMP_DIR = "./bash_popen_tmp"\n    \ndef bash_popen_terminate_all():\n    for p in globals().get("bash_popen_list", []):\n        print("Terminate pid=" + str(p.pid), file=sys.stderr)\n        p.terminate()\n    globals()["bash_popen_list"] = []\n    if os.path.exists(BASH_POPEN_TMP_DIR):\n        shutil.rmtree(BASH_POPEN_TMP_DIR)\n\nbash_popen_terminate_all()  \n\ndef bash_popen(cmd):\n    if not os.path.exists(BASH_POPEN_TMP_DIR):\n        os.mkdir(BASH_POPEN_TMP_DIR)\n    h = os.path.join(BASH_POPEN_TMP_DIR, str(random.randint(0, 1e18)))\n    stdout_file = h + ".out.html"\n    stderr_file = h + ".err.html"\n    run_log_file = h + ".fin.html"\n    \n    stdout = open(stdout_file, "wb")\n    stdout = open(stderr_file, "wb")\n    \n    html = """\n    <table width="100%">\n    <colgroup>\n       <col span="1" style="width: 70px;">\n       <col span="1">\n    </colgroup>    \n    <tbody>\n      <tr> <td><b>STDOUT</b></td> <td> {stdout} </td> </tr>\n      <tr> <td><b>STDERR</b></td> <td> {stderr} </td> </tr>\n      <tr> <td><b>RUN LOG</b></td> <td> {run_log} </td> </tr>\n    </tbody>\n    </table>\n    """.format(\n        stdout=show_file(stdout_file, return_html_string=True),\n        stderr=show_file(stderr_file, return_html_string=True),\n        run_log=show_file(run_log_file, return_html_string=True),\n    )\n    \n    cmd = """\n        bash -c {cmd} &\n        pid=$!\n        echo "Process started! pid=${{pid}}" > {run_log_file}\n        wait ${{pid}}\n        echo "Process finished! exit_code=$?" >> {run_log_file}\n    """.format(cmd=shlex.quote(cmd), run_log_file=run_log_file)\n    # print(cmd)\n    display(HTML(html))\n    \n    p = Popen(["bash", "-c", cmd], stdin=PIPE, stdout=stdout, stderr=stdout)\n    \n    bash_popen_list.append(p)\n    return p\n\n\n@register_line_magic\ndef bash_async(line):\n    bash_popen(line)\n    \n    \ndef show_log_file(file, return_html_string=False):\n    obj = file.replace(\'.\', \'_\').replace(\'/\', \'_\') + "_obj"\n    html_string = \'\'\'\n        <!--MD_BEGIN_FILTER-->\n        <script type=text/javascript>\n        var entrance___OBJ__ = 0;\n        var errors___OBJ__ = 0;\n        function halt__OBJ__(elem, color)\n        {\n            elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    \n        }\n        function refresh__OBJ__()\n        {\n            entrance___OBJ__ -= 1;\n            if (entrance___OBJ__ < 0) {\n                entrance___OBJ__ = 0;\n            }\n            var elem = document.getElementById("__OBJ__");\n            if (elem) {\n                var xmlhttp=new XMLHttpRequest();\n                xmlhttp.onreadystatechange=function()\n                {\n                    var elem = document.getElementById("__OBJ__");\n                    console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___OBJ__);\n                    if (elem && xmlhttp.readyState==4) {\n                        if (xmlhttp.status==200)\n                        {\n                            errors___OBJ__ = 0;\n                            if (!entrance___OBJ__) {\n                                if (elem.innerHTML != xmlhttp.responseText) {\n                                    elem.innerHTML = xmlhttp.responseText;\n                                }\n                                if (elem.innerHTML.includes("Process finished.")) {\n                                    halt__OBJ__(elem, "#333333");\n                                } else {\n                                    entrance___OBJ__ += 1;\n                                    console.log("req");\n                                    window.setTimeout("refresh__OBJ__()", 300); \n                                }\n                            }\n                            return xmlhttp.responseText;\n                        } else {\n                            errors___OBJ__ += 1;\n                            if (!entrance___OBJ__) {\n                                if (errors___OBJ__ < 6) {\n                                    entrance___OBJ__ += 1;\n                                    console.log("req");\n                                    window.setTimeout("refresh__OBJ__()", 300); \n                                } else {\n                                    halt__OBJ__(elem, "#994444");\n                                }\n                            }\n                        }\n                    }\n                }\n                xmlhttp.open("GET", "__FILE__", true);\n                xmlhttp.setRequestHeader("Cache-Control", "no-cache");\n                xmlhttp.send();     \n            }\n        }\n        \n        if (!entrance___OBJ__) {\n            entrance___OBJ__ += 1;\n            refresh__OBJ__(); \n        }\n        </script>\n\n        <p id="__OBJ__" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">\n        </p>\n        \n        </font>\n        <!--MD_END_FILTER-->\n        <!--MD_FROM_FILE __FILE__.md -->\n        \'\'\'.replace("__OBJ__", obj).replace("__FILE__", file)\n    if return_html_string:\n        return html_string\n    display(HTML(html_string))\n\n    \nclass TInteractiveLauncher:\n    tmp_path = "./interactive_launcher_tmp"\n    def __init__(self, cmd):\n        try:\n            os.mkdir(TInteractiveLauncher.tmp_path)\n        except:\n            pass\n        name = str(random.randint(0, 1e18))\n        self.inq_path = os.path.join(TInteractiveLauncher.tmp_path, name + ".inq")\n        self.log_path = os.path.join(TInteractiveLauncher.tmp_path, name + ".log")\n        \n        os.mkfifo(self.inq_path)\n        open(self.log_path, \'w\').close()\n        open(self.log_path + ".md", \'w\').close()\n\n        self.pid = os.fork()\n        if self.pid == -1:\n            print("Error")\n        if self.pid == 0:\n            exe_cands = glob.glob("../tools/launcher.py") + glob.glob("../../tools/launcher.py")\n            assert(len(exe_cands) == 1)\n            assert(os.execvp("python3", ["python3", exe_cands[0], "-l", self.log_path, "-i", self.inq_path, "-c", cmd]) == 0)\n        self.inq_f = open(self.inq_path, "w")\n        interactive_launcher_opened_set.add(self.pid)\n        show_log_file(self.log_path)\n\n    def write(self, s):\n        s = s.encode()\n        assert len(s) == os.write(self.inq_f.fileno(), s)\n        \n    def get_pid(self):\n        n = 100\n        for i in range(n):\n            try:\n                return int(re.findall(r"PID = (\\d+)", open(self.log_path).readline())[0])\n            except:\n                if i + 1 == n:\n                    raise\n                time.sleep(0.1)\n        \n    def input_queue_path(self):\n        return self.inq_path\n        \n    def close(self):\n        self.inq_f.close()\n        os.waitpid(self.pid, 0)\n        os.remove(self.inq_path)\n        # os.remove(self.log_path)\n        self.inq_path = None\n        self.log_path = None \n        interactive_launcher_opened_set.remove(self.pid)\n        self.pid = None\n        \n    @staticmethod\n    def terminate_all():\n        if "interactive_launcher_opened_set" not in globals():\n            globals()["interactive_launcher_opened_set"] = set()\n        global interactive_launcher_opened_set\n        for pid in interactive_launcher_opened_set:\n            print("Terminate pid=" + str(pid), file=sys.stderr)\n            os.kill(pid, signal.SIGKILL)\n            os.waitpid(pid, 0)\n        interactive_launcher_opened_set = set()\n        if os.path.exists(TInteractiveLauncher.tmp_path):\n            shutil.rmtree(TInteractiveLauncher.tmp_path)\n    \nTInteractiveLauncher.terminate_all()\n   \nyandex_metrica_allowed = bool(globals().get("yandex_metrica_allowed", False))\nif yandex_metrica_allowed:\n    display(HTML(\'\'\'<!-- Yandex.Metrika counter -->\n    <script type="text/javascript" >\n       (function(m,e,t,r,i,k,a){m[i]=m[i]||function(){(m[i].a=m[i].a||[]).push(arguments)};\n       m[i].l=1*new Date();k=e.createElement(t),a=e.getElementsByTagName(t)[0],k.async=1,k.src=r,a.parentNode.insertBefore(k,a)})\n       (window, document, "script", "https://mc.yandex.ru/metrika/tag.js", "ym");\n\n       ym(59260609, "init", {\n            clickmap:true,\n            trackLinks:true,\n            accurateTrackBounce:true\n       });\n    </script>\n    <noscript><div><img src="https://mc.yandex.ru/watch/59260609" style="position:absolute; left:-9999px;" alt="" /></div></noscript>\n    <!-- /Yandex.Metrika counter -->\'\'\'))\n\ndef make_oneliner():\n    return \'\'.join([\n        \'# look at tools/set_up_magics.ipynb\\n\',\n        \'yandex_metrica_allowed = True ; get_ipython().run_cell(%s);\' % repr(one_liner_str),\n        \'display(HTML("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований. Если вы не хотите, чтобы по вам собиралась статистика, исправьте: yandex_metrica_allowed = False" if yandex_metrica_allowed else ""))\' \n    ]) \n        \n        \n\n #SET_UP_MAGIC_END \n');display(HTML("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований. Если вы не хотите, чтобы по вам собиралась статистика, исправьте: yandex_metrica_allowed = False" if yandex_metrica_allowed else ""))



```python
%%save_file launcher.py

import argparse
parser = argparse.ArgumentParser()
parser.add_argument("-i", dest="queue_in")
parser.add_argument("-c", dest="command")
parser.add_argument("-l", dest="log_file")
args = parser.parse_args()

def put_line(file, t, line):
    color = {"O": "white", "E": "red", "I": "yellow", "L": "orange"}[t]
    fmt_md = '{t} | {line}'
    if t == 'L':
        fmt_s = '<font style="color: {color};"> {line} </font> </br>\n'
    else:
        fmt_s = '<tt style="color: {color};"><b>{t}</b></tt>&nbsp;<font style="font-size: 1.6em; line-height: 0.0em; ">|</font> {line} </br>\n'
    with open(file, "a") as f:
        f.write(fmt_s.format(color=color, t=t, line=line))
    with open(file + ".md", "a") as f:
        f.write(fmt_md.format(t=t, line=line.rstrip() + "\n"))

try:
    import os
    import sys
    from subprocess import Popen, PIPE
    import time
    import fcntl
    import errno

    def make_nonblock(fd):
        fl = fcntl.fcntl(fd, fcntl.F_GETFL)
        fcntl.fcntl(fd, fcntl.F_SETFL, fl | os.O_NONBLOCK)
        return fd
    
    os.truncate(args.log_file, 0)

    p = Popen(["bash", "-c", args.command], stdin=PIPE, stdout=PIPE, stderr=PIPE)
    stdin_fd = make_nonblock(p.stdin.fileno())
    stdout_fd = make_nonblock(p.stdout.fileno())
    stderr_fd = make_nonblock(p.stderr.fileno())
    
    put_line(args.log_file, "L", "Process started. PID = {}".format(p.pid))
    
    queue_in = open(args.queue_in, "r")
    queue_in_fd = make_nonblock(queue_in.fileno())
    queue_in_line = ""
    
    while True:
#         put_line(args.log_file, "L", "iteration")
        wpid, status = os.waitpid(p.pid, os.WNOHANG)
        
        active_output = True
        while active_output:
            active_output = False
            try:
                line = p.stdout.readline()
                if len(line) > 0:
                    put_line(args.log_file, "O", line[:-1].decode("utf-8"))
                    active_output = True
            except:
                pass
            try:
                line = p.stderr.readline()
                if len(line) > 0:
                    put_line(args.log_file, "E", line[:-1].decode("utf-8"))
                    active_output = True
            except:
                pass
        if queue_in_fd >= 0:
            try:
                s = os.read(queue_in_fd, 10000).decode("utf-8")
                if len(s) == 0:
                    queue_in_fd = -1
                else:
                    queue_in_line += s
            except IOError as e:
                if e.errno != errno.EAGAIN:
                    put_line(args.log_file, "L", "FError in reading: " + str(e))
                pass
        if queue_in_line.find('\n') != -1 or (len(queue_in_line) > 0 and queue_in_fd < 0):
            try:
                if queue_in_line.find('\n') != -1:
                    line, queue_in_line = queue_in_line.split('\n', 1)
                    line += '\n'
                else:
                    line, queue_in_line = queue_in_line, ""
                put_line(args.log_file, "I", line)
                l = os.write(stdin_fd, line.encode())
                assert(l == len(line))
            except Exception as e:
                put_line(args.log_file, "L", str(e))
                pass
        
        if queue_in_fd < 0 and len(queue_in_line) == 0 and stdin_fd >= 0:
            os.close(stdin_fd)
            stdin_fd = -1
        
        if wpid != 0:
            report = "???"
            if os.WIFEXITED(status):
                report = "Process finished. Exit code {}".format(os.WEXITSTATUS(status)) 
            if os.WIFSIGNALED(status):
                report = "Process finished. Got signal {}".format(os.WTERMSIG(status)) 
            break
        time.sleep(0.1)
    put_line(args.log_file, "L", report)
except Exception as e:
    put_line(args.log_file, "L", str(e))
```


```python
import errno
print(errno.EAGAIN)
```

    11



```python
a = TInteractiveLauncher("echo 1 ; echo 2 1>&2 ; read XX ; echo \"A${XX}B\" ")
```



<!--MD_BEGIN_FILTER-->
<script type=text/javascript>
var entrance___interactive_launcher_tmp_704228343092166969_log_obj = 0;
var errors___interactive_launcher_tmp_704228343092166969_log_obj = 0;
function halt__interactive_launcher_tmp_704228343092166969_log_obj(elem, color)
{
    elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    
}
function refresh__interactive_launcher_tmp_704228343092166969_log_obj()
{
    entrance___interactive_launcher_tmp_704228343092166969_log_obj -= 1;
    if (entrance___interactive_launcher_tmp_704228343092166969_log_obj < 0) {
        entrance___interactive_launcher_tmp_704228343092166969_log_obj = 0;
    }
    var elem = document.getElementById("__interactive_launcher_tmp_704228343092166969_log_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem = document.getElementById("__interactive_launcher_tmp_704228343092166969_log_obj");
            console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___interactive_launcher_tmp_704228343092166969_log_obj);
            if (elem && xmlhttp.readyState==4) {
                if (xmlhttp.status==200)
                {
                    errors___interactive_launcher_tmp_704228343092166969_log_obj = 0;
                    if (!entrance___interactive_launcher_tmp_704228343092166969_log_obj) {
                        if (elem.innerHTML != xmlhttp.responseText) {
                            elem.innerHTML = xmlhttp.responseText;
                        }
                        if (elem.innerHTML.includes("Process finished.")) {
                            halt__interactive_launcher_tmp_704228343092166969_log_obj(elem, "#333333");
                        } else {
                            entrance___interactive_launcher_tmp_704228343092166969_log_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__interactive_launcher_tmp_704228343092166969_log_obj()", 300); 
                        }
                    }
                    return xmlhttp.responseText;
                } else {
                    errors___interactive_launcher_tmp_704228343092166969_log_obj += 1;
                    if (!entrance___interactive_launcher_tmp_704228343092166969_log_obj) {
                        if (errors___interactive_launcher_tmp_704228343092166969_log_obj < 6) {
                            entrance___interactive_launcher_tmp_704228343092166969_log_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__interactive_launcher_tmp_704228343092166969_log_obj()", 300); 
                        } else {
                            halt__interactive_launcher_tmp_704228343092166969_log_obj(elem, "#994444");
                        }
                    }
                }
            }
        }
        xmlhttp.open("GET", "./interactive_launcher_tmp/704228343092166969.log", true);
        xmlhttp.setRequestHeader("Cache-Control", "no-cache");
        xmlhttp.send();     
    }
}

if (!entrance___interactive_launcher_tmp_704228343092166969_log_obj) {
    entrance___interactive_launcher_tmp_704228343092166969_log_obj += 1;
    refresh__interactive_launcher_tmp_704228343092166969_log_obj(); 
}
</script>

<p id="__interactive_launcher_tmp_704228343092166969_log_obj" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">
</p>

</font>
<!--MD_END_FILTER-->
<!--MD_FROM_FILE ./interactive_launcher_tmp/704228343092166969.log.md -->




```python
a.write("hoho!\n")
a.close()
```


```python
#display(Markdown(open("./interactive_launcher_tmp/159341120905617190.log.md").read()))
```


```python
a = TInteractiveLauncher("echo 1 ; echo 2 1>&2 ; read XX ; echo \"A${XX}B\" ")
os.kill(a.get_pid(), 9)
a.close()
```



<!--MD_BEGIN_FILTER-->
<script type=text/javascript>
var entrance___interactive_launcher_tmp_773937186095382150_log_obj = 0;
var errors___interactive_launcher_tmp_773937186095382150_log_obj = 0;
function halt__interactive_launcher_tmp_773937186095382150_log_obj(elem, color)
{
    elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    
}
function refresh__interactive_launcher_tmp_773937186095382150_log_obj()
{
    entrance___interactive_launcher_tmp_773937186095382150_log_obj -= 1;
    if (entrance___interactive_launcher_tmp_773937186095382150_log_obj < 0) {
        entrance___interactive_launcher_tmp_773937186095382150_log_obj = 0;
    }
    var elem = document.getElementById("__interactive_launcher_tmp_773937186095382150_log_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem = document.getElementById("__interactive_launcher_tmp_773937186095382150_log_obj");
            console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___interactive_launcher_tmp_773937186095382150_log_obj);
            if (elem && xmlhttp.readyState==4) {
                if (xmlhttp.status==200)
                {
                    errors___interactive_launcher_tmp_773937186095382150_log_obj = 0;
                    if (!entrance___interactive_launcher_tmp_773937186095382150_log_obj) {
                        if (elem.innerHTML != xmlhttp.responseText) {
                            elem.innerHTML = xmlhttp.responseText;
                        }
                        if (elem.innerHTML.includes("Process finished.")) {
                            halt__interactive_launcher_tmp_773937186095382150_log_obj(elem, "#333333");
                        } else {
                            entrance___interactive_launcher_tmp_773937186095382150_log_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__interactive_launcher_tmp_773937186095382150_log_obj()", 300); 
                        }
                    }
                    return xmlhttp.responseText;
                } else {
                    errors___interactive_launcher_tmp_773937186095382150_log_obj += 1;
                    if (!entrance___interactive_launcher_tmp_773937186095382150_log_obj) {
                        if (errors___interactive_launcher_tmp_773937186095382150_log_obj < 6) {
                            entrance___interactive_launcher_tmp_773937186095382150_log_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__interactive_launcher_tmp_773937186095382150_log_obj()", 300); 
                        } else {
                            halt__interactive_launcher_tmp_773937186095382150_log_obj(elem, "#994444");
                        }
                    }
                }
            }
        }
        xmlhttp.open("GET", "./interactive_launcher_tmp/773937186095382150.log", true);
        xmlhttp.setRequestHeader("Cache-Control", "no-cache");
        xmlhttp.send();     
    }
}

if (!entrance___interactive_launcher_tmp_773937186095382150_log_obj) {
    entrance___interactive_launcher_tmp_773937186095382150_log_obj += 1;
    refresh__interactive_launcher_tmp_773937186095382150_log_obj(); 
}
</script>

<p id="__interactive_launcher_tmp_773937186095382150_log_obj" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">
</p>

</font>
<!--MD_END_FILTER-->
<!--MD_FROM_FILE ./interactive_launcher_tmp/773937186095382150.log.md -->




```python
a = TInteractiveLauncher("cat")
a.write("hoho!\n")

```



<!--MD_BEGIN_FILTER-->
<script type=text/javascript>
var entrance___interactive_launcher_tmp_26672992216455199_log_obj = 0;
var errors___interactive_launcher_tmp_26672992216455199_log_obj = 0;
function halt__interactive_launcher_tmp_26672992216455199_log_obj(elem, color)
{
    elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    
}
function refresh__interactive_launcher_tmp_26672992216455199_log_obj()
{
    entrance___interactive_launcher_tmp_26672992216455199_log_obj -= 1;
    if (entrance___interactive_launcher_tmp_26672992216455199_log_obj < 0) {
        entrance___interactive_launcher_tmp_26672992216455199_log_obj = 0;
    }
    var elem = document.getElementById("__interactive_launcher_tmp_26672992216455199_log_obj");
    if (elem) {
        var xmlhttp=new XMLHttpRequest();
        xmlhttp.onreadystatechange=function()
        {
            var elem = document.getElementById("__interactive_launcher_tmp_26672992216455199_log_obj");
            console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___interactive_launcher_tmp_26672992216455199_log_obj);
            if (elem && xmlhttp.readyState==4) {
                if (xmlhttp.status==200)
                {
                    errors___interactive_launcher_tmp_26672992216455199_log_obj = 0;
                    if (!entrance___interactive_launcher_tmp_26672992216455199_log_obj) {
                        if (elem.innerHTML != xmlhttp.responseText) {
                            elem.innerHTML = xmlhttp.responseText;
                        }
                        if (elem.innerHTML.includes("Process finished.")) {
                            halt__interactive_launcher_tmp_26672992216455199_log_obj(elem, "#333333");
                        } else {
                            entrance___interactive_launcher_tmp_26672992216455199_log_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__interactive_launcher_tmp_26672992216455199_log_obj()", 300); 
                        }
                    }
                    return xmlhttp.responseText;
                } else {
                    errors___interactive_launcher_tmp_26672992216455199_log_obj += 1;
                    if (!entrance___interactive_launcher_tmp_26672992216455199_log_obj) {
                        if (errors___interactive_launcher_tmp_26672992216455199_log_obj < 6) {
                            entrance___interactive_launcher_tmp_26672992216455199_log_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__interactive_launcher_tmp_26672992216455199_log_obj()", 300); 
                        } else {
                            halt__interactive_launcher_tmp_26672992216455199_log_obj(elem, "#994444");
                        }
                    }
                }
            }
        }
        xmlhttp.open("GET", "./interactive_launcher_tmp/26672992216455199.log", true);
        xmlhttp.setRequestHeader("Cache-Control", "no-cache");
        xmlhttp.send();     
    }
}

if (!entrance___interactive_launcher_tmp_26672992216455199_log_obj) {
    entrance___interactive_launcher_tmp_26672992216455199_log_obj += 1;
    refresh__interactive_launcher_tmp_26672992216455199_log_obj(); 
}
</script>

<p id="__interactive_launcher_tmp_26672992216455199_log_obj" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">
</p>

</font>
<!--MD_END_FILTER-->
<!--MD_FROM_FILE ./interactive_launcher_tmp/26672992216455199.log.md -->




```python
a.write("aoha!\n")
```


```python
a.close()
```


```python

(glob.glob("../tools/launcher.py") + glob.glob("../../tools/launcher.py"))[0]
```




    '../tools/launcher.py'




```python
!rm -f my_fifo
!mkfifo my_fifo
```


```python
%bash_async echo "Hello $USER" > my_fifo ; echo 'After writing to my_fifo'
```



<table width="100%">
<colgroup>
   <col span="1" style="width: 70px;">
   <col span="1">
</colgroup>    
<tbody>
  <tr> <td><b>STDOUT</b></td> <td> 
    <!--MD_BEGIN_FILTER-->
    <script type=text/javascript>
    var entrance___bash_popen_tmp_666572132904306691_out_html_obj = 0;
    var errors___bash_popen_tmp_666572132904306691_out_html_obj = 0;
    function refresh__bash_popen_tmp_666572132904306691_out_html_obj()
    {
        entrance___bash_popen_tmp_666572132904306691_out_html_obj -= 1;
        var elem = document.getElementById("__bash_popen_tmp_666572132904306691_out_html_obj");
        if (elem) {
            var xmlhttp=new XMLHttpRequest();
            xmlhttp.onreadystatechange=function()
            {
                var elem = document.getElementById("__bash_popen_tmp_666572132904306691_out_html_obj");
                console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___bash_popen_tmp_666572132904306691_out_html_obj);
                if (elem && xmlhttp.readyState==4) {
                    if (xmlhttp.status==200)
                    {
                        errors___bash_popen_tmp_666572132904306691_out_html_obj = 0;
                        if (!entrance___bash_popen_tmp_666572132904306691_out_html_obj) {
                            elem.innerText = xmlhttp.responseText;
                            entrance___bash_popen_tmp_666572132904306691_out_html_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__bash_popen_tmp_666572132904306691_out_html_obj()", 300); 
                        }
                        return xmlhttp.responseText;
                    } else {
                        errors___bash_popen_tmp_666572132904306691_out_html_obj += 1;
                        if (errors___bash_popen_tmp_666572132904306691_out_html_obj < 10 && !entrance___bash_popen_tmp_666572132904306691_out_html_obj) {
                            entrance___bash_popen_tmp_666572132904306691_out_html_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__bash_popen_tmp_666572132904306691_out_html_obj()", 300); 
                        }
                    }
                }
            }
            xmlhttp.open("GET", "./bash_popen_tmp/666572132904306691.out.html", true);
            xmlhttp.setRequestHeader("Cache-Control", "no-cache");
            xmlhttp.send();     
        }
    }

    if (!entrance___bash_popen_tmp_666572132904306691_out_html_obj) {
        entrance___bash_popen_tmp_666572132904306691_out_html_obj += 1;
        refresh__bash_popen_tmp_666572132904306691_out_html_obj(); 
    }
    </script>

    <font color="white"> <tt>
    <p id="__bash_popen_tmp_666572132904306691_out_html_obj" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>
    </tt> </font>
    <!--MD_END_FILTER-->
    <!--MD_FROM_FILE ./bash_popen_tmp/666572132904306691.out.html -->
     </td> </tr>
  <tr> <td><b>STDERR</b></td> <td> 
    <!--MD_BEGIN_FILTER-->
    <script type=text/javascript>
    var entrance___bash_popen_tmp_666572132904306691_err_html_obj = 0;
    var errors___bash_popen_tmp_666572132904306691_err_html_obj = 0;
    function refresh__bash_popen_tmp_666572132904306691_err_html_obj()
    {
        entrance___bash_popen_tmp_666572132904306691_err_html_obj -= 1;
        var elem = document.getElementById("__bash_popen_tmp_666572132904306691_err_html_obj");
        if (elem) {
            var xmlhttp=new XMLHttpRequest();
            xmlhttp.onreadystatechange=function()
            {
                var elem = document.getElementById("__bash_popen_tmp_666572132904306691_err_html_obj");
                console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___bash_popen_tmp_666572132904306691_err_html_obj);
                if (elem && xmlhttp.readyState==4) {
                    if (xmlhttp.status==200)
                    {
                        errors___bash_popen_tmp_666572132904306691_err_html_obj = 0;
                        if (!entrance___bash_popen_tmp_666572132904306691_err_html_obj) {
                            elem.innerText = xmlhttp.responseText;
                            entrance___bash_popen_tmp_666572132904306691_err_html_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__bash_popen_tmp_666572132904306691_err_html_obj()", 300); 
                        }
                        return xmlhttp.responseText;
                    } else {
                        errors___bash_popen_tmp_666572132904306691_err_html_obj += 1;
                        if (errors___bash_popen_tmp_666572132904306691_err_html_obj < 10 && !entrance___bash_popen_tmp_666572132904306691_err_html_obj) {
                            entrance___bash_popen_tmp_666572132904306691_err_html_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__bash_popen_tmp_666572132904306691_err_html_obj()", 300); 
                        }
                    }
                }
            }
            xmlhttp.open("GET", "./bash_popen_tmp/666572132904306691.err.html", true);
            xmlhttp.setRequestHeader("Cache-Control", "no-cache");
            xmlhttp.send();     
        }
    }

    if (!entrance___bash_popen_tmp_666572132904306691_err_html_obj) {
        entrance___bash_popen_tmp_666572132904306691_err_html_obj += 1;
        refresh__bash_popen_tmp_666572132904306691_err_html_obj(); 
    }
    </script>

    <font color="white"> <tt>
    <p id="__bash_popen_tmp_666572132904306691_err_html_obj" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>
    </tt> </font>
    <!--MD_END_FILTER-->
    <!--MD_FROM_FILE ./bash_popen_tmp/666572132904306691.err.html -->
     </td> </tr>
  <tr> <td><b>RUN LOG</b></td> <td> 
    <!--MD_BEGIN_FILTER-->
    <script type=text/javascript>
    var entrance___bash_popen_tmp_666572132904306691_fin_html_obj = 0;
    var errors___bash_popen_tmp_666572132904306691_fin_html_obj = 0;
    function refresh__bash_popen_tmp_666572132904306691_fin_html_obj()
    {
        entrance___bash_popen_tmp_666572132904306691_fin_html_obj -= 1;
        var elem = document.getElementById("__bash_popen_tmp_666572132904306691_fin_html_obj");
        if (elem) {
            var xmlhttp=new XMLHttpRequest();
            xmlhttp.onreadystatechange=function()
            {
                var elem = document.getElementById("__bash_popen_tmp_666572132904306691_fin_html_obj");
                console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___bash_popen_tmp_666572132904306691_fin_html_obj);
                if (elem && xmlhttp.readyState==4) {
                    if (xmlhttp.status==200)
                    {
                        errors___bash_popen_tmp_666572132904306691_fin_html_obj = 0;
                        if (!entrance___bash_popen_tmp_666572132904306691_fin_html_obj) {
                            elem.innerText = xmlhttp.responseText;
                            entrance___bash_popen_tmp_666572132904306691_fin_html_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__bash_popen_tmp_666572132904306691_fin_html_obj()", 300); 
                        }
                        return xmlhttp.responseText;
                    } else {
                        errors___bash_popen_tmp_666572132904306691_fin_html_obj += 1;
                        if (errors___bash_popen_tmp_666572132904306691_fin_html_obj < 10 && !entrance___bash_popen_tmp_666572132904306691_fin_html_obj) {
                            entrance___bash_popen_tmp_666572132904306691_fin_html_obj += 1;
                            console.log("req");
                            window.setTimeout("refresh__bash_popen_tmp_666572132904306691_fin_html_obj()", 300); 
                        }
                    }
                }
            }
            xmlhttp.open("GET", "./bash_popen_tmp/666572132904306691.fin.html", true);
            xmlhttp.setRequestHeader("Cache-Control", "no-cache");
            xmlhttp.send();     
        }
    }

    if (!entrance___bash_popen_tmp_666572132904306691_fin_html_obj) {
        entrance___bash_popen_tmp_666572132904306691_fin_html_obj += 1;
        refresh__bash_popen_tmp_666572132904306691_fin_html_obj(); 
    }
    </script>

    <font color="white"> <tt>
    <p id="__bash_popen_tmp_666572132904306691_fin_html_obj" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>
    </tt> </font>
    <!--MD_END_FILTER-->
    <!--MD_FROM_FILE ./bash_popen_tmp/666572132904306691.fin.html -->
     </td> </tr>
</tbody>
</table>




```python
!cat my_fifo
```

    Hello pechatnov



```python
!ls bash_popen_tmp/
```

    666572132904306691.err.html  666572132904306691.out.html
    666572132904306691.fin.html



```python

```


```python

```

    # look at tools/set_up_magics.ipynb
    get_ipython().run_cell('# one_liner_str #SET_UP_MAGIC_BEGIN\n\nget_ipython().run_cell_magic(\'javascript\', \'\', \'// setup cpp code highlighting\\nIPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\\\'reg\\\':[/^%%cpp/]} ;\')\n\n# creating magics\nfrom IPython.core.magic import register_cell_magic, register_line_magic\nfrom IPython.display import display, Markdown, HTML\nimport argparse\nfrom subprocess import Popen, PIPE\nimport random\nimport sys\nimport os\nimport re\nimport signal\nimport shutil\nimport shlex\nimport glob\n\n@register_cell_magic\ndef save_file(args_str, cell, line_comment_start="#"):\n    parser = argparse.ArgumentParser()\n    parser.add_argument("fname")\n    parser.add_argument("--ejudge-style", action="store_true")\n    args = parser.parse_args(args_str.split())\n    \n    cell = cell if cell[-1] == \'\\n\' or args.no_eof_newline else cell + "\\n"\n    cmds = []\n    with open(args.fname, "w") as f:\n        f.write(line_comment_start + " %%cpp " + args_str + "\\n")\n        for line in cell.split("\\n"):\n            line_to_write = (line if not args.ejudge_style else line.rstrip()) + "\\n"\n            if line.startswith("%"):\n                run_prefix = "%run "\n                assert line.startswith(run_prefix)\n                cmds.append(line[len(run_prefix):].strip())\n                f.write(line_comment_start + " " + line_to_write)\n            else:\n                f.write(line_to_write)\n        f.write("" if not args.ejudge_style else line_comment_start + r" line without \\n")\n    for cmd in cmds:\n        display(Markdown("Run: `%s`" % cmd))\n        get_ipython().system(cmd)\n\n@register_cell_magic\ndef cpp(fname, cell):\n    save_file(fname, cell, "//")\n\n@register_cell_magic\ndef asm(fname, cell):\n    save_file(fname, cell, "//")\n    \n@register_cell_magic\ndef makefile(fname, cell):\n    assert not fname\n    save_file("makefile", cell.replace(" " * 4, "\\t"))\n        \n@register_line_magic\ndef p(line):\n    try:\n        expr, comment = line.split(" #")\n        display(Markdown("`{} = {}`  # {}".format(expr.strip(), eval(expr), comment.strip())))\n    except:\n        display(Markdown("{} = {}".format(line, eval(line))))\n        \ndef show_file(file, clear_at_begin=True, return_html_string=False):\n    if clear_at_begin:\n        get_ipython().system("truncate --size 0 " + file)\n    obj = file.replace(\'.\', \'_\').replace(\'/\', \'_\') + "_obj"\n    html_string = \'\'\'\n        <!--MD_BEGIN_FILTER-->\n        <script type=text/javascript>\n        var entrance___OBJ__ = 0;\n        var errors___OBJ__ = 0;\n        function refresh__OBJ__()\n        {\n            entrance___OBJ__ -= 1;\n            var elem = document.getElementById("__OBJ__");\n            if (elem) {\n                var xmlhttp=new XMLHttpRequest();\n                xmlhttp.onreadystatechange=function()\n                {\n                    var elem = document.getElementById("__OBJ__");\n                    console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___OBJ__);\n                    if (elem && xmlhttp.readyState==4) {\n                        if (xmlhttp.status==200)\n                        {\n                            errors___OBJ__ = 0;\n                            if (!entrance___OBJ__) {\n                                elem.innerText = xmlhttp.responseText;\n                                entrance___OBJ__ += 1;\n                                console.log("req");\n                                window.setTimeout("refresh__OBJ__()", 300); \n                            }\n                            return xmlhttp.responseText;\n                        } else {\n                            errors___OBJ__ += 1;\n                            if (errors___OBJ__ < 10 && !entrance___OBJ__) {\n                                entrance___OBJ__ += 1;\n                                console.log("req");\n                                window.setTimeout("refresh__OBJ__()", 300); \n                            }\n                        }\n                    }\n                }\n                xmlhttp.open("GET", "__FILE__", true);\n                xmlhttp.setRequestHeader("Cache-Control", "no-cache");\n                xmlhttp.send();     \n            }\n        }\n        \n        if (!entrance___OBJ__) {\n            entrance___OBJ__ += 1;\n            refresh__OBJ__(); \n        }\n        </script>\n        \n        <font color="white"> <tt>\n        <p id="__OBJ__" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>\n        </tt> </font>\n        <!--MD_END_FILTER-->\n        <!--MD_FROM_FILE __FILE__ -->\n        \'\'\'.replace("__OBJ__", obj).replace("__FILE__", file)\n    if return_html_string:\n        return html_string\n    display(HTML(html_string))\n    \nBASH_POPEN_TMP_DIR = "./bash_popen_tmp"\n    \ndef bash_popen_terminate_all():\n    for p in globals().get("bash_popen_list", []):\n        print("Terminate pid=" + str(p.pid), file=sys.stderr)\n        p.terminate()\n    globals()["bash_popen_list"] = []\n    if os.path.exists(BASH_POPEN_TMP_DIR):\n        shutil.rmtree(BASH_POPEN_TMP_DIR)\n\nbash_popen_terminate_all()  \n\ndef bash_popen(cmd):\n    if not os.path.exists(BASH_POPEN_TMP_DIR):\n        os.mkdir(BASH_POPEN_TMP_DIR)\n    h = os.path.join(BASH_POPEN_TMP_DIR, str(random.randint(0, 1e18)))\n    stdout_file = h + ".out.html"\n    stderr_file = h + ".err.html"\n    run_log_file = h + ".fin.html"\n    \n    stdout = open(stdout_file, "wb")\n    stdout = open(stderr_file, "wb")\n    \n    html = """\n    <table width="100%">\n    <colgroup>\n       <col span="1" style="width: 70px;">\n       <col span="1">\n    </colgroup>    \n    <tbody>\n      <tr> <td><b>STDOUT</b></td> <td> {stdout} </td> </tr>\n      <tr> <td><b>STDERR</b></td> <td> {stderr} </td> </tr>\n      <tr> <td><b>RUN LOG</b></td> <td> {run_log} </td> </tr>\n    </tbody>\n    </table>\n    """.format(\n        stdout=show_file(stdout_file, return_html_string=True),\n        stderr=show_file(stderr_file, return_html_string=True),\n        run_log=show_file(run_log_file, return_html_string=True),\n    )\n    \n    cmd = """\n        bash -c {cmd} &\n        pid=$!\n        echo "Process started! pid=${{pid}}" > {run_log_file}\n        wait ${{pid}}\n        echo "Process finished! exit_code=$?" >> {run_log_file}\n    """.format(cmd=shlex.quote(cmd), run_log_file=run_log_file)\n    # print(cmd)\n    display(HTML(html))\n    \n    p = Popen(["bash", "-c", cmd], stdin=PIPE, stdout=stdout, stderr=stdout)\n    \n    bash_popen_list.append(p)\n    return p\n\n\n@register_line_magic\ndef bash_async(line):\n    bash_popen(line)\n    \n    \ndef show_log_file(file, return_html_string=False):\n    obj = file.replace(\'.\', \'_\').replace(\'/\', \'_\') + "_obj"\n    html_string = \'\'\'\n        <!--MD_BEGIN_FILTER-->\n        <script type=text/javascript>\n        var entrance___OBJ__ = 0;\n        var errors___OBJ__ = 0;\n        function halt__OBJ__(elem, color)\n        {\n            elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    \n        }\n        function refresh__OBJ__()\n        {\n            entrance___OBJ__ -= 1;\n            if (entrance___OBJ__ < 0) {\n                entrance___OBJ__ = 0;\n            }\n            var elem = document.getElementById("__OBJ__");\n            if (elem) {\n                var xmlhttp=new XMLHttpRequest();\n                xmlhttp.onreadystatechange=function()\n                {\n                    var elem = document.getElementById("__OBJ__");\n                    console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___OBJ__);\n                    if (elem && xmlhttp.readyState==4) {\n                        if (xmlhttp.status==200)\n                        {\n                            errors___OBJ__ = 0;\n                            if (!entrance___OBJ__) {\n                                if (elem.innerHTML != xmlhttp.responseText) {\n                                    elem.innerHTML = xmlhttp.responseText;\n                                }\n                                if (elem.innerHTML.includes("Process finished.")) {\n                                    halt__OBJ__(elem, "#333333");\n                                } else {\n                                    entrance___OBJ__ += 1;\n                                    console.log("req");\n                                    window.setTimeout("refresh__OBJ__()", 300); \n                                }\n                            }\n                            return xmlhttp.responseText;\n                        } else {\n                            errors___OBJ__ += 1;\n                            if (!entrance___OBJ__) {\n                                if (errors___OBJ__ < 6) {\n                                    entrance___OBJ__ += 1;\n                                    console.log("req");\n                                    window.setTimeout("refresh__OBJ__()", 300); \n                                } else {\n                                    halt__OBJ__(elem, "#994444");\n                                }\n                            }\n                        }\n                    }\n                }\n                xmlhttp.open("GET", "__FILE__", true);\n                xmlhttp.setRequestHeader("Cache-Control", "no-cache");\n                xmlhttp.send();     \n            }\n        }\n        \n        if (!entrance___OBJ__) {\n            entrance___OBJ__ += 1;\n            refresh__OBJ__(); \n        }\n        </script>\n\n        <p id="__OBJ__" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">\n        </p>\n        \n        </font>\n        <!--MD_END_FILTER-->\n        <!--MD_FROM_FILE __FILE__.md -->\n        \'\'\'.replace("__OBJ__", obj).replace("__FILE__", file)\n    if return_html_string:\n        return html_string\n    display(HTML(html_string))\n\n    \nclass TInteractiveLauncher:\n    tmp_path = "./interactive_launcher_tmp"\n    def __init__(self, cmd):\n        try:\n            os.mkdir(TInteractiveLauncher.tmp_path)\n        except:\n            pass\n        name = str(random.randint(0, 1e18))\n        self.inq_path = os.path.join(TInteractiveLauncher.tmp_path, name + ".inq")\n        self.log_path = os.path.join(TInteractiveLauncher.tmp_path, name + ".log")\n        \n        os.mkfifo(self.inq_path)\n        open(self.log_path, \'w\').close()\n        open(self.log_path + ".md", \'w\').close()\n\n        self.pid = os.fork()\n        if self.pid == -1:\n            print("Error")\n        if self.pid == 0:\n            exe_cands = glob.glob("../tools/launcher.py") + glob.glob("../../tools/launcher.py")\n            assert(len(exe_cands) == 1)\n            assert(os.execvp("python3", ["python3", exe_cands[0], "-l", self.log_path, "-i", self.inq_path, "-c", cmd]) == 0)\n        self.inq_f = open(self.inq_path, "w")\n        interactive_launcher_opened_set.add(self.pid)\n        show_log_file(self.log_path)\n\n    def write(self, s):\n        s = s.encode()\n        assert len(s) == os.write(self.inq_f.fileno(), s)\n        \n    def get_pid(self):\n        n = 100\n        for i in range(n):\n            try:\n                return int(re.findall(r"PID = (\\d+)", open(self.log_path).readline())[0])\n            except:\n                if i + 1 == n:\n                    raise\n                time.sleep(0.1)\n        \n    def input_queue_path(self):\n        return self.inq_path\n        \n    def close(self):\n        self.inq_f.close()\n        os.waitpid(self.pid, 0)\n        os.remove(self.inq_path)\n        # os.remove(self.log_path)\n        self.inq_path = None\n        self.log_path = None \n        interactive_launcher_opened_set.remove(self.pid)\n        self.pid = None\n        \n    @staticmethod\n    def terminate_all():\n        if "interactive_launcher_opened_set" not in globals():\n            globals()["interactive_launcher_opened_set"] = set()\n        global interactive_launcher_opened_set\n        for pid in interactive_launcher_opened_set:\n            print("Terminate pid=" + str(pid), file=sys.stderr)\n            os.kill(pid, signal.SIGKILL)\n            os.waitpid(pid, 0)\n        interactive_launcher_opened_set = set()\n        if os.path.exists(TInteractiveLauncher.tmp_path):\n            shutil.rmtree(TInteractiveLauncher.tmp_path)\n    \nTInteractiveLauncher.terminate_all()\n    \ndisplay(HTML(\'\'\'<!-- Yandex.Metrika counter -->\n<script type="text/javascript" >\n   (function(m,e,t,r,i,k,a){m[i]=m[i]||function(){(m[i].a=m[i].a||[]).push(arguments)};\n   m[i].l=1*new Date();k=e.createElement(t),a=e.getElementsByTagName(t)[0],k.async=1,k.src=r,a.parentNode.insertBefore(k,a)})\n   (window, document, "script", "https://mc.yandex.ru/metrika/tag.js", "ym");\n\n   ym(59260609, "init", {\n        clickmap:true,\n        trackLinks:true,\n        accurateTrackBounce:true\n   });\n</script>\n<noscript><div><img src="https://mc.yandex.ru/watch/59260609" style="position:absolute; left:-9999px;" alt="" /></div></noscript>\n<!-- /Yandex.Metrika counter -->\'\'\'))\n\ndef make_oneliner():\n    return \'\'.join([\n        \'# look at tools/set_up_magics.ipynb\\n\',\n        \'get_ipython().run_cell(%s);\' % repr(one_liner_str),\n        \'display(HTML("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований"))\' \n    ]) \n        \n        \n\n #SET_UP_MAGIC_END \n');display(HTML("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований"))



```python

```


```python

```


```python

```
