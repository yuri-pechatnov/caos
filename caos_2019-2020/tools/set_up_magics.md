python
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





```
L | Process started. PID = 11336
O | 1
E | 2
I | hoho!
O | Ahoho!B
L | Process finished. Exit code 0

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





```
L | Process started. PID = 11338
O | 1
E | 2
L | Process finished. Got signal 9

```





```python
a = TInteractiveLauncher("cat")
a.write("hoho!\n")

```





```
L | Process started. PID = 11340
I | hoho!
O | hoho!
I | aoha!
O | aoha!
L | Process finished. Exit code 0

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




    '../tools/launcher.py'




```python

```


```python
a = TInteractiveLauncher("sleep 100")
a.close()
```





```
L | Process started. PID = 11342
L | Process finished. Got signal 9

```





```python

```


```cpp
%%cpp a.cpp
%// comment
%run echo 11
```


Run: `echo 11`


    11



```python
%%save_file a.py
%# comment
%run cat a.py
```


Run: `cat a.py`


    # %%cpp a.py
    # %# comment
    # %run cat a.py
    



```python

```


```python

```
