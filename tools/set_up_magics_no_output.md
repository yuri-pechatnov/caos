```python
from IPython.core.magic import register_cell_magic

@register_cell_magic
def save_cell_as_string(string_name, cell):
    cell = "# " + string_name + "\n" + cell + "\n"
    globals()[string_name] = cell
    get_ipython().run_cell(cell)
```


```python
%%save_cell_as_string one_liner_str

get_ipython().run_cell_magic('javascript', '', 
    '// setup cpp code highlighting\n'
    'IPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\'reg\':[/^%%cpp/]} ;'
    'IPython.CodeCell.options_default.highlight_modes["text/x-cmake"] = {\'reg\':[/^%%cmake/]} ;'
)

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
import time

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
                if line.startswith("%" + line_comment_start + " "):
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
def cmake(fname, cell):
    save_file(fname, cell, "#")

@register_cell_magic
def asm(fname, cell):
    save_file(fname, cell, "//")
    
@register_cell_magic
def makefile(fname, cell):
    fname = fname or "makefile"
    assert fname.endswith("makefile")
    save_file(fname, cell.replace(" " * 4, "\t"))
        
@register_line_magic
def p(line):
    line = line.strip() 
    if line[0] == '#':
        display(Markdown(line[1:].strip()))
    else:
        try:
            expr, comment = line.split(" #")
            display(Markdown("`{} = {}`  # {}".format(expr.strip(), eval(expr), comment.strip())))
        except:
            display(Markdown("{} = {}".format(line, eval(line))))
    
    
def show_log_file(file, return_html_string=False):
    obj = file.replace('.', '_').replace('/', '_') + "_obj"
    html_string = '''
        
        
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
        
    def wait_stop(self, timeout):
        for i in range(int(timeout * 10)):
            wpid, status = os.waitpid(self.pid, os.WNOHANG)
            if wpid != 0:
                return True
            time.sleep(0.1)
        return False
        
    def close(self, timeout=3):
        self.inq_f.close()
        if not self.wait_stop(timeout):
            os.kill(self.get_pid(), signal.SIGKILL)
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
    display(HTML('''<!-- YANDEX_METRICA_BEGIN -->
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
    <!-- YANDEX_METRICA_END -->'''))

def make_oneliner():
    html_text = '("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований. Если вы не хотите, чтобы по вам собиралась статистика, исправьте: yandex_metrica_allowed = False" if yandex_metrica_allowed else "")'
    html_text += ' + "<""!-- MAGICS_SETUP_PRINTING_END -->"'
    return ''.join([
        '# look at tools/set_up_magics.ipynb\n',
        'yandex_metrica_allowed = True ; get_ipython().run_cell(%s);' % repr(one_liner_str),
        'display(HTML(%s))' % html_text,
        ' #''MAGICS_SETUP_END'
    ])
       
```


```python
print(make_oneliner())

```


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


```python
a = TInteractiveLauncher("echo 1 ; echo 2 1>&2 ; read XX ; echo \"A${XX}B\" ")
```


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


```python
a = TInteractiveLauncher("cat")
a.write("hoho!\n")

```


```python
a.write("aoha!\n")
```


```python
a.close()
```


```python

(glob.glob("../tools/launcher.py") + glob.glob("../../tools/launcher.py"))[0]
```


```python

```


```python
a = TInteractiveLauncher("sleep 100")
a.close()
```


```python

```


```cpp
%%cpp a.cpp
%// comment
%run echo 11
```


```python
%%save_file a.py
%# comment
%run cat a.py
```


```python

```


```python

```


```python

```


```python

```


```python

```
