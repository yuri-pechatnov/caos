# <a name="how"></a> Как сделать пулл реквест?

0. Выбираем, где хотим провести изменения, в форке репозитория (более предпочтительно, но не принципиально) или в самом репозитории (в этом случае нужно запросить у меня доступ).
1. Нужно произвести все желаемые изменения в семинарском ноутбуке. И убедиться, что эти изменения сохранены (юпитер у меня иногда тупит, поэтому жму трижды `ctrl-s` с интервалом около секунды).
  <br> Постарайтесь ограничиться минимальными изменениями. Так же убедитесь, что у вас актуальная версия репозитория. (Я сам в недавние ноутбуки могу теоретически каждый день коммитить, а мои изменения затирать не надо :) ).
2. Далее в этом ноутбуке (он умеет правильно генерировать `.md` файлы):
  <br>A.  <a href="#what" style="color:#856024"> Здесь </a> выбираем семинар(ы), к которому сделали правку. `../tools` выбирать не надо.
  <br>B.  <a href="#github" style="color:#856024"> Здесь </a> можно написать свой commit message, если есть желание. Можно оставить как есть. В этом репозитории нет культуры хороших сообщений к коммитам :)
  <br>C.  Запускаем этот ноутбук, он сгенерит `.md`-шки и закоммитит изменения на гитхаб.
3. Если изменение было в форке, то делаем пулл реквест.

### <a name="what"></a> Выбираем что коммитить


```python
import glob
import os
import subprocess

highlevel_dirs = sum([
    ["../tools"], 
    #sorted(glob.glob("../sem24*")),
    sorted(glob.glob("../sem28*")),
], [])

print("Highlevel dirs:", highlevel_dirs)
```

    Highlevel dirs: ['../tools', '../sem28-unix-time']



```python
tmp_dir = "./tmp_dir"
get_ipython().system('rm -r {tmp_dir} ; mkdir {tmp_dir} 2>&1 | grep -v "File exists"'.format(tmp_dir=tmp_dir))
```


```python
!rm ../sem28-unix-time/*_no_output* || true
!rm ../tools/*_no_output* || true
```

    rm: cannot remove '../sem28-unix-time/*_no_output*': No such file or directory
    rm: cannot remove '../tools/*_no_output*': No such file or directory



```python
os.path.dirname("../tools/set_up_magics_dev_no_output.ipynb")
```




    '../tools'




```python
!cp ../tools/save_them_all.ipynb ./tmp_dir/save_them_allAAA.ipynb
!jupyter nbconvert ./tmp_dir/save_them_allAAA.ipynb --clear-output --inplace
!jupyter nbconvert ./tmp_dir/save_them_allAAA.ipynb --to markdown --output README_no_output
!cp ./tmp_dir/README_no_output.md ../tools
```

    [NbConvertApp] Converting notebook ./tmp_dir/save_them_allAAA.ipynb to notebook
    [NbConvertApp] Writing 15896 bytes to ./tmp_dir/save_them_allAAA.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/save_them_allAAA.ipynb to markdown
    [NbConvertApp] Writing 11238 bytes to ./tmp_dir/README_no_output.md



```python

```


```python
from multiprocessing import Pool

    
tasks = []

def convert_tasks(n, d):
    no_output_file = d + "_no_output"
    src_copy = os.path.basename(n) + '_' + str(abs(hash(n)))
    path = os.path.dirname(n)
    return [
        "jupyter nbconvert {} --to markdown --output {}".format(n, d),
        " && ".join([
            "cp {src} {tmp_dir}/{src_copy}",
            "jupyter nbconvert {tmp_dir}/{src_copy} --clear-output --inplace",
            "jupyter nbconvert {tmp_dir}/{src_copy} --to markdown --output {no_output_file}",
            "cp {tmp_dir}/{no_output_file}.md {path}",
        ]).format(src=n, no_output_file=no_output_file, dst=d, tmp_dir=tmp_dir, src_copy=src_copy, path=path),
    ]
    
for subdir in highlevel_dirs:
    notebooks = glob.glob(subdir + "/*.ipynb")
    print(subdir, notebooks)
    for m in glob.glob(subdir + "/*.md"):
        os.remove(m)
    if len(notebooks) == 1:
        tasks.extend(convert_tasks(notebooks[0], "README"))
    else:
        for n in notebooks:
            tasks.extend(convert_tasks(n, os.path.basename(n.replace(".ipynb", ""))))
        nmds = [os.path.basename(n).replace(".ipynb", ".md") for n in notebooks]
        with open(os.path.join(subdir, "README.md"), "w") as f:
            f.write('\n'.join(
                ['# Ноутбуки семинара'] + 
                ['* [{nmd}]({nmd})'.format(nmd=nmd) for nmd in nmds] + 
                ['']
            ))

print(tasks)


p = Pool(8)
def execute_task(t):
    get_ipython().system(t)
p.map(execute_task, tasks)
```

    ../tools ['../tools/set_up_magics.ipynb', '../tools/save_them_all.ipynb', '../tools/stand.ipynb', '../tools/set_up_magics_dev.ipynb']
    ../sem28-unix-time ['../sem28-unix-time/time.ipynb']
    ['jupyter nbconvert ../tools/set_up_magics.ipynb --to markdown --output set_up_magics', 'cp ../tools/set_up_magics.ipynb ./tmp_dir/set_up_magics.ipynb_1809760105274635650 && jupyter nbconvert ./tmp_dir/set_up_magics.ipynb_1809760105274635650 --clear-output --inplace && jupyter nbconvert ./tmp_dir/set_up_magics.ipynb_1809760105274635650 --to markdown --output set_up_magics_no_output && cp ./tmp_dir/set_up_magics_no_output.md ../tools', 'jupyter nbconvert ../tools/save_them_all.ipynb --to markdown --output save_them_all', 'cp ../tools/save_them_all.ipynb ./tmp_dir/save_them_all.ipynb_5752926541494098657 && jupyter nbconvert ./tmp_dir/save_them_all.ipynb_5752926541494098657 --clear-output --inplace && jupyter nbconvert ./tmp_dir/save_them_all.ipynb_5752926541494098657 --to markdown --output save_them_all_no_output && cp ./tmp_dir/save_them_all_no_output.md ../tools', 'jupyter nbconvert ../tools/stand.ipynb --to markdown --output stand', 'cp ../tools/stand.ipynb ./tmp_dir/stand.ipynb_7120860308082601853 && jupyter nbconvert ./tmp_dir/stand.ipynb_7120860308082601853 --clear-output --inplace && jupyter nbconvert ./tmp_dir/stand.ipynb_7120860308082601853 --to markdown --output stand_no_output && cp ./tmp_dir/stand_no_output.md ../tools', 'jupyter nbconvert ../tools/set_up_magics_dev.ipynb --to markdown --output set_up_magics_dev', 'cp ../tools/set_up_magics_dev.ipynb ./tmp_dir/set_up_magics_dev.ipynb_2653528037997578719 && jupyter nbconvert ./tmp_dir/set_up_magics_dev.ipynb_2653528037997578719 --clear-output --inplace && jupyter nbconvert ./tmp_dir/set_up_magics_dev.ipynb_2653528037997578719 --to markdown --output set_up_magics_dev_no_output && cp ./tmp_dir/set_up_magics_dev_no_output.md ../tools', 'jupyter nbconvert ../sem28-unix-time/time.ipynb --to markdown --output README', 'cp ../sem28-unix-time/time.ipynb ./tmp_dir/time.ipynb_3597715960520351318 && jupyter nbconvert ./tmp_dir/time.ipynb_3597715960520351318 --clear-output --inplace && jupyter nbconvert ./tmp_dir/time.ipynb_3597715960520351318 --to markdown --output README_no_output && cp ./tmp_dir/README_no_output.md ../sem28-unix-time']


    Process ForkPoolWorker-1:
    Process ForkPoolWorker-4:
    Process ForkPoolWorker-2:
    Process ForkPoolWorker-3:
    Process ForkPoolWorker-8:
    Process ForkPoolWorker-7:
    Process ForkPoolWorker-6:
    Traceback (most recent call last):
    Process ForkPoolWorker-5:
    Traceback (most recent call last):
      File "/usr/lib/python3.8/multiprocessing/process.py", line 315, in _bootstrap
        self.run()
    Traceback (most recent call last):
      File "/usr/lib/python3.8/multiprocessing/process.py", line 108, in run
        self._target(*self._args, **self._kwargs)
    Traceback (most recent call last):
      File "/usr/lib/python3.8/multiprocessing/process.py", line 315, in _bootstrap
        self.run()
      File "/usr/lib/python3.8/multiprocessing/process.py", line 315, in _bootstrap
        self.run()
    Traceback (most recent call last):
    Traceback (most recent call last):
      File "/usr/lib/python3.8/multiprocessing/process.py", line 315, in _bootstrap
        self.run()
      File "/usr/lib/python3.8/multiprocessing/pool.py", line 114, in worker
        task = get()
      File "/usr/lib/python3.8/multiprocessing/process.py", line 315, in _bootstrap
        self.run()
    Traceback (most recent call last):
      File "/usr/lib/python3.8/multiprocessing/process.py", line 108, in run
        self._target(*self._args, **self._kwargs)
    Traceback (most recent call last):
      File "/usr/lib/python3.8/multiprocessing/process.py", line 108, in run
        self._target(*self._args, **self._kwargs)
      File "/usr/lib/python3.8/multiprocessing/process.py", line 108, in run
        self._target(*self._args, **self._kwargs)
      File "/usr/lib/python3.8/multiprocessing/queues.py", line 358, in get
        return _ForkingPickler.loads(res)
      File "/usr/lib/python3.8/multiprocessing/process.py", line 315, in _bootstrap
        self.run()
      File "/usr/lib/python3.8/multiprocessing/process.py", line 315, in _bootstrap
        self.run()
      File "/usr/lib/python3.8/multiprocessing/process.py", line 108, in run
        self._target(*self._args, **self._kwargs)
      File "/usr/lib/python3.8/multiprocessing/process.py", line 315, in _bootstrap
        self.run()
      File "/usr/lib/python3.8/multiprocessing/pool.py", line 114, in worker
        task = get()
      File "/usr/lib/python3.8/multiprocessing/process.py", line 108, in run
        self._target(*self._args, **self._kwargs)
    AttributeError: Can't get attribute 'execute_task' on <module '__main__'>
      File "/usr/lib/python3.8/multiprocessing/pool.py", line 114, in worker
        task = get()
      File "/usr/lib/python3.8/multiprocessing/process.py", line 108, in run
        self._target(*self._args, **self._kwargs)
      File "/usr/lib/python3.8/multiprocessing/pool.py", line 114, in worker
        task = get()
      File "/usr/lib/python3.8/multiprocessing/process.py", line 108, in run
        self._target(*self._args, **self._kwargs)
      File "/usr/lib/python3.8/multiprocessing/queues.py", line 358, in get
        return _ForkingPickler.loads(res)
      File "/usr/lib/python3.8/multiprocessing/pool.py", line 114, in worker
        task = get()
      File "/usr/lib/python3.8/multiprocessing/pool.py", line 114, in worker
        task = get()
      File "/usr/lib/python3.8/multiprocessing/queues.py", line 358, in get
        return _ForkingPickler.loads(res)
      File "/usr/lib/python3.8/multiprocessing/queues.py", line 358, in get
        return _ForkingPickler.loads(res)
      File "/usr/lib/python3.8/multiprocessing/pool.py", line 114, in worker
        task = get()
    AttributeError: Can't get attribute 'execute_task' on <module '__main__'>
      File "/usr/lib/python3.8/multiprocessing/pool.py", line 114, in worker
        task = get()
      File "/usr/lib/python3.8/multiprocessing/queues.py", line 358, in get
        return _ForkingPickler.loads(res)
    AttributeError: Can't get attribute 'execute_task' on <module '__main__'>
    AttributeError: Can't get attribute 'execute_task' on <module '__main__'>
      File "/usr/lib/python3.8/multiprocessing/queues.py", line 358, in get
        return _ForkingPickler.loads(res)
      File "/usr/lib/python3.8/multiprocessing/queues.py", line 358, in get
        return _ForkingPickler.loads(res)
      File "/usr/lib/python3.8/multiprocessing/queues.py", line 358, in get
        return _ForkingPickler.loads(res)
    AttributeError: Can't get attribute 'execute_task' on <module '__main__'>
    AttributeError: Can't get attribute 'execute_task' on <module '__main__'>
    AttributeError: Can't get attribute 'execute_task' on <module '__main__'>
    AttributeError: Can't get attribute 'execute_task' on <module '__main__'>


    [NbConvertApp] Converting notebook ../sem28-unix-time/time.ipynb to markdown
    [NbConvertApp] Converting notebook ./tmp_dir/time.ipynb_3597715960520351318 to notebook
    [NbConvertApp] Writing 42054 bytes to ./tmp_dir/time.ipynb
    [NbConvertApp] Writing 32626 bytes to ../sem28-unix-time/README.md
    [NbConvertApp] Converting notebook ./tmp_dir/time.ipynb_3597715960520351318 to markdown
    [NbConvertApp] Writing 32626 bytes to ./tmp_dir/README_no_output.md


    Process ForkPoolWorker-9:
    Process ForkPoolWorker-15:
    Process ForkPoolWorker-12:
    Process ForkPoolWorker-10:
    Process ForkPoolWorker-16:
    Process ForkPoolWorker-11:
    Process ForkPoolWorker-13:
    Process ForkPoolWorker-14:
    Traceback (most recent call last):
    Traceback (most recent call last):
    Traceback (most recent call last):
    Traceback (most recent call last):
    Traceback (most recent call last):
    Traceback (most recent call last):
      File "/usr/lib/python3.8/multiprocessing/process.py", line 315, in _bootstrap
        self.run()
      File "/usr/lib/python3.8/multiprocessing/process.py", line 315, in _bootstrap
        self.run()
      File "/usr/lib/python3.8/multiprocessing/process.py", line 315, in _bootstrap
        self.run()
      File "/usr/lib/python3.8/multiprocessing/process.py", line 108, in run
        self._target(*self._args, **self._kwargs)
      File "/usr/lib/python3.8/multiprocessing/process.py", line 315, in _bootstrap
        self.run()
      File "/usr/lib/python3.8/multiprocessing/process.py", line 315, in _bootstrap
        self.run()
      File "/usr/lib/python3.8/multiprocessing/process.py", line 315, in _bootstrap
        self.run()
      File "/usr/lib/python3.8/multiprocessing/pool.py", line 114, in worker
        task = get()
      File "/usr/lib/python3.8/multiprocessing/process.py", line 108, in run
        self._target(*self._args, **self._kwargs)
      File "/usr/lib/python3.8/multiprocessing/queues.py", line 355, in get
        with self._rlock:
      File "/usr/lib/python3.8/multiprocessing/process.py", line 108, in run
        self._target(*self._args, **self._kwargs)
      File "/usr/lib/python3.8/multiprocessing/synchronize.py", line 95, in __enter__
        return self._semlock.__enter__()
    Traceback (most recent call last):
      File "/usr/lib/python3.8/multiprocessing/process.py", line 108, in run
        self._target(*self._args, **self._kwargs)
      File "/usr/lib/python3.8/multiprocessing/pool.py", line 114, in worker
        task = get()
      File "/usr/lib/python3.8/multiprocessing/process.py", line 108, in run
        self._target(*self._args, **self._kwargs)
      File "/usr/lib/python3.8/multiprocessing/pool.py", line 114, in worker
        task = get()
      File "/usr/lib/python3.8/multiprocessing/process.py", line 108, in run
        self._target(*self._args, **self._kwargs)
      File "/usr/lib/python3.8/multiprocessing/pool.py", line 114, in worker
        task = get()
    KeyboardInterrupt
      File "/usr/lib/python3.8/multiprocessing/queues.py", line 355, in get
        with self._rlock:
    Traceback (most recent call last):
      File "/usr/lib/python3.8/multiprocessing/queues.py", line 355, in get
        with self._rlock:
      File "/usr/lib/python3.8/multiprocessing/pool.py", line 114, in worker
        task = get()
      File "/usr/lib/python3.8/multiprocessing/process.py", line 315, in _bootstrap
        self.run()
      File "/usr/lib/python3.8/multiprocessing/synchronize.py", line 95, in __enter__
        return self._semlock.__enter__()
      File "/usr/lib/python3.8/multiprocessing/queues.py", line 356, in get
        res = self._reader.recv_bytes()
      File "/usr/lib/python3.8/multiprocessing/synchronize.py", line 95, in __enter__
        return self._semlock.__enter__()
      File "/usr/lib/python3.8/multiprocessing/process.py", line 315, in _bootstrap
        self.run()
      File "/usr/lib/python3.8/multiprocessing/pool.py", line 114, in worker
        task = get()
      File "/usr/lib/python3.8/multiprocessing/queues.py", line 355, in get
        with self._rlock:
    KeyboardInterrupt
      File "/usr/lib/python3.8/multiprocessing/connection.py", line 216, in recv_bytes
        buf = self._recv_bytes(maxlength)
      File "/usr/lib/python3.8/multiprocessing/process.py", line 108, in run
        self._target(*self._args, **self._kwargs)
      File "/usr/lib/python3.8/multiprocessing/process.py", line 108, in run
        self._target(*self._args, **self._kwargs)
      File "/usr/lib/python3.8/multiprocessing/queues.py", line 355, in get
        with self._rlock:
    KeyboardInterrupt
      File "/usr/lib/python3.8/multiprocessing/synchronize.py", line 95, in __enter__
        return self._semlock.__enter__()
      File "/usr/lib/python3.8/multiprocessing/pool.py", line 114, in worker
        task = get()
      File "/usr/lib/python3.8/multiprocessing/connection.py", line 414, in _recv_bytes
        buf = self._recv(4)
      File "/usr/lib/python3.8/multiprocessing/pool.py", line 114, in worker
        task = get()
      File "/usr/lib/python3.8/multiprocessing/synchronize.py", line 95, in __enter__
        return self._semlock.__enter__()
      File "/usr/lib/python3.8/multiprocessing/queues.py", line 355, in get
        with self._rlock:
      File "/usr/lib/python3.8/multiprocessing/synchronize.py", line 95, in __enter__
        return self._semlock.__enter__()
      File "/usr/lib/python3.8/multiprocessing/connection.py", line 379, in _recv
        chunk = read(handle, remaining)
      File "/usr/lib/python3.8/multiprocessing/queues.py", line 355, in get
        with self._rlock:
    KeyboardInterrupt
    KeyboardInterrupt
    KeyboardInterrupt
    KeyboardInterrupt
      File "/usr/lib/python3.8/multiprocessing/synchronize.py", line 95, in __enter__
        return self._semlock.__enter__()
    KeyboardInterrupt



    ---------------------------------------------------------------------------

    KeyboardInterrupt                         Traceback (most recent call last)

    <ipython-input-8-346ab7ee0494> in <module>
         42 def execute_task(t):
         43     get_ipython().system(t)
    ---> 44 p.map(execute_task, tasks)
    

    /usr/lib/python3.8/multiprocessing/pool.py in map(self, func, iterable, chunksize)
        362         in a list that is returned.
        363         '''
    --> 364         return self._map_async(func, iterable, mapstar, chunksize).get()
        365 
        366     def starmap(self, func, iterable, chunksize=None):


    /usr/lib/python3.8/multiprocessing/pool.py in get(self, timeout)
        760 
        761     def get(self, timeout=None):
    --> 762         self.wait(timeout)
        763         if not self.ready():
        764             raise TimeoutError


    /usr/lib/python3.8/multiprocessing/pool.py in wait(self, timeout)
        757 
        758     def wait(self, timeout=None):
    --> 759         self._event.wait(timeout)
        760 
        761     def get(self, timeout=None):


    /usr/lib/python3.8/threading.py in wait(self, timeout)
        556             signaled = self._flag
        557             if not signaled:
    --> 558                 signaled = self._cond.wait(timeout)
        559             return signaled
        560 


    /usr/lib/python3.8/threading.py in wait(self, timeout)
        300         try:    # restore state no matter what (e.g., KeyboardInterrupt)
        301             if timeout is None:
    --> 302                 waiter.acquire()
        303                 gotit = True
        304             else:


    KeyboardInterrupt: 



```python

```


```python
import re

def basic_improve(fname):
    with open(fname, "r") as f:
        r = f.read()
    for b in ["\x00", "\x1B", "\x08"]:
        r = r.replace(b, "")
    with open(fname, "w") as f:
        f.write(r)
    get_ipython().system("dos2unix {}".format(fname))

def improve_md(fname):
    with open(fname, "r") as f:
        r = f.read()
    r = r.replace("```python\n%%cpp", "```cpp\n%%cpp")
    r = r.replace("```python\n%%cmake", "```cmake\n%%cmake")
    r = r.replace('\n', "SUPER_SLASH" + "_N_REPLACER")
    
    r = re.sub(r'\<\!--MD_BEGIN_FILTER--\>.*?\<\!--MD_END_FILTER--\>', "", r)
    #r = re.sub(r'(\#SET_UP_MAGIC_BEGIN.*?\#SET_UP_MAGIC_END)', "<too much code>", r)
    r = re.sub(r'\<\!\-\-\ YANDEX_METRICA_BEGIN\ \-\-\>.*\<\!\-\-\ YANDEX_METRICA_END\ \-\-\>', '', r)
    
    r = r.replace("", '')
    r = r.replace("SUPER_SLASH" + "_N_REPLACER", '\n')
    
    template = "#""MAGICS_SETUP_END"
    bpos = r.rfind(template)
    if bpos != -1:
        r = r[bpos + len(template):]
        template = "```"
        bpos = r.find(template)
        assert bpos >= 0
        r = r[bpos + len(template):]
    
    
    template = "<""!-- MAGICS_SETUP_PRINTING_END -->"
    bpos = r.rfind(template)
    if bpos != -1:
        r = r[bpos + len(template):]
    
    def file_repl(matchobj, path=os.path.dirname(fname)):
        fname = os.path.join(path, matchobj.group(1))
        if fname.find("__FILE__") == -1:
            with open(fname, "r") as f:
                return "\n```\n" + f.read() + "\n```\n"
    
    r = r.replace("", "")
    r = r.replace("", "")
    
    r = re.sub(r'\<\!--MD_FROM_FILE (.*?) --\>', file_repl, r)
    with open(fname, "w") as f:
        f.write(r)
        
def improve_file(fname):
    basic_improve(fname)
    if fname.endswith(".md"):
        improve_md(fname)

```


```python
for sfx in [".ipynb", ".md"]:
    for hdir in highlevel_dirs:
        for fname in glob.glob("./{}/*".format(hdir) + sfx):
            improve_file(fname)
```

    dos2unix: converting file ./../tools/save_them_all_no_output.ipynb to Unix format...
    dos2unix: converting file ./../tools/set_up_magics.ipynb to Unix format...
    dos2unix: converting file ./../tools/set_up_magics_no_output.ipynb to Unix format...
    dos2unix: converting file ./../tools/save_them_all.ipynb to Unix format...
    dos2unix: converting file ./../tools/stand_no_output.ipynb to Unix format...
    dos2unix: converting file ./../tools/stand.ipynb to Unix format...
    dos2unix: converting file ./../tools/set_up_magics_dev_no_output.ipynb to Unix format...
    dos2unix: converting file ./../tools/set_up_magics_dev.ipynb to Unix format...
    dos2unix: converting file ./../sem28-unix-time/README_no_output.ipynb to Unix format...
    dos2unix: converting file ./../sem28-unix-time/time.ipynb to Unix format...
    dos2unix: converting file ./../tools/save_them_all.md to Unix format...
    dos2unix: converting file ./../tools/README.md to Unix format...
    dos2unix: converting file ./../tools/set_up_magics_dev.md to Unix format...
    dos2unix: converting file ./../tools/stand.md to Unix format...
    dos2unix: converting file ./../tools/set_up_magics.md to Unix format...
    dos2unix: converting file ./../sem28-unix-time/README.md to Unix format...


### <a name="github"></a> Коммитим на github


```python

```


```python
cmds = []
add_cmd = "git add --ignore-errors "
add_cmd_f = "git add --ignore-errors -f "
for subdir in highlevel_dirs:
    for sfx in [".ipynb", ".md", ".c", ".cpp"]:
        cmds.append(add_cmd + " {}/*{}".format(subdir, sfx))
    cmds.append(add_cmd_f + " -f {}/bash_popen_tmp/*.html".format(subdir))
    cmds.append(add_cmd_f + " -f {}/interactive_launcher_tmp/*.log".format(subdir))
    cmds.append("git add -u {}".format(subdir))
    
def execute_cmd(cmd):
    print(">", cmd)
    get_ipython().system(cmd)
    
for cmd in cmds:
    execute_cmd(cmd)
# execute_cmd("git add -u")
execute_cmd("git commit -m 'yet another update'")
execute_cmd("git push origin master")
```

    > git add --ignore-errors  ../tools/*.ipynb
    > git add --ignore-errors  ../tools/*.md
    > git add --ignore-errors  ../tools/*.c
    fatal: pathspec '../tools/*.c' did not match any files
    > git add --ignore-errors  ../tools/*.cpp
    > git add --ignore-errors -f  -f ../tools/bash_popen_tmp/*.html
    > git add --ignore-errors -f  -f ../tools/interactive_launcher_tmp/*.log
    > git add -u ../tools
    > git add --ignore-errors  ../sem28-unix-time/*.ipynb
    > git add --ignore-errors  ../sem28-unix-time/*.md
    > git add --ignore-errors  ../sem28-unix-time/*.c
    > git add --ignore-errors  ../sem28-unix-time/*.cpp
    > git add --ignore-errors -f  -f ../sem28-unix-time/bash_popen_tmp/*.html
    warning: could not open directory 'sem28-unix-time/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem28-unix-time/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem28-unix-time/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem28-unix-time/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem28-unix-time/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem28-unix-time
    > git commit -m 'yet another update'
    [master 6f20b17] yet another update
     7 files changed, 2244 insertions(+), 1 deletion(-)
     create mode 100644 sem28-unix-time/README_no_output.ipynb
     create mode 100644 tools/save_them_all_no_output.ipynb
     create mode 100644 tools/set_up_magics_dev_no_output.ipynb
     create mode 100644 tools/set_up_magics_no_output.ipynb
     create mode 100644 tools/stand_no_output.ipynb
    > git push origin master
    Enumerating objects: 11, done.
    Counting objects: 100% (11/11), done.
    Delta compression using up to 4 threads
    Compressing objects: 100% (6/6), done.
    Writing objects: 100% (6/6), 1.02 KiB | 1.02 MiB/s, done.
    Total 6 (delta 5), reused 0 (delta 0)
    remote: Resolving deltas: 100% (5/5), completed with 5 local objects.[K
    To github.com:yuri-pechatnov/caos_2019-2020.git
       a29f3dc..6f20b17  master -> master



```python

```


```python

```


```python

```
