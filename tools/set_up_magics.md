```python
from IPython.core.magic import register_cell_magic

@register_cell_magic
def save_cell_as_string(string_name, cell):
    cell = "# " + string_name + " <too much code> \n"
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
                assert line.startswith(run_prefix)
                cmds.append(line[len(run_prefix):].strip())
                f.write(line_comment_start + " " + line_to_write)
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
      <tr> <td><b>STDOUT</b> <td> {stdout}  
      <tr> <td><b>STDERR</b> <td> {stderr}  
      <tr> <td><b>RUN LOG</b> <td> {run_log}  
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
            assert(os.execvp("python3", ["python3", "launcher.py", "-l", self.log_path, "-i", self.inq_path, "-c", cmd]) == 0)
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
    
    
def make_oneliner():
    return '# look at tools/set_up_magics.ipynb\nget_ipython().run_cell(%s)\nNone' % repr(one_liner_str)
```


    <IPython.core.display.Javascript object>



```python

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
        f.write(fmt_md.format(t=t, line=line))

try:
    import os
    import sys
    from subprocess import Popen, PIPE
    import time
    import fcntl

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
    make_nonblock(queue_in.fileno())
    queue_in_line = ""
    
    while True:
#         put_line(args.log_file, "L", "iteration")
        wpid, status = os.waitpid(p.pid, os.WNOHANG)
        
        if not queue_in_line:
            try:
                queue_in_line = queue_in.readline()
                if queue_in_line:
                    put_line(args.log_file, "I", queue_in_line[:-1])
            except:
                pass
        if queue_in_line:
            try:
                l = os.write(stdin_fd, queue_in_line.encode())
#                 put_line(args.log_file, "L", "successful write: " + queue_in_line + " len=%d" % l)
                queue_in_line = queue_in_line[l:]
            except Exception as e:
                put_line(args.log_file, "L", str(e))
                pass
        
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

```


```python
a = TInteractiveLauncher("echo 1 ; echo 2 1>&2 ; read XX ; echo \"A${XX}B\" ")
```





```
L | Process started. PID = 32194O | 1E | 2I | hoho!O | Ahoho!BL | Process finished. Exit code 0
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
```





```
L | Process started. PID = 32196O | 1E | 2L | Process finished. Got signal 9
```





```python
os.kill(a.get_pid(), 9)
a.close()
```


```python

```


```python

```


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
  <tr> <td><b>STDOUT</b> <td> 
    
    
```

```

      
  <tr> <td><b>STDERR</b> <td> 
    
    
```
After writing to my_fifo

```

      
  <tr> <td><b>RUN LOG</b> <td> 
    
    
```
Process started! pid=32208
Process finished! exit_code=0

```

      
</tbody>
</table>




```python
!cat my_fifo
```

    Hello pechatnov



```python
!ls bash_popen_tmp/
```

    806329575918927099.err.html  806329575918927099.out.html
    806329575918927099.fin.html



```python

```


```python
print(make_oneliner())
```

    # look at tools/set_up_magics.ipynb
    get_ipython().run_cell('# one_liner_str <too much code> \n')
    None



```python

```


```python

```


```python

```
