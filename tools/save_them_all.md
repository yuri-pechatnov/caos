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
    #["../tools"], 
    #sorted(glob.glob("../sem26*")),
    sorted(glob.glob("../sem17*")),
], [])

print("Highlevel dirs:", highlevel_dirs)
```


      File "<ipython-input-12-f9dbabd7c183>", line 7
        №sorted(glob.glob("../sem26*")),
              ^
    SyntaxError: invalid character in identifier




```python
tmp_dir = "./tmp_dir"
get_ipython().system('rm -r {tmp_dir} ; mkdir {tmp_dir} 2>&1 | grep -v "File exists"'.format(tmp_dir=tmp_dir))
```

### Генерируем все .md-шки стандартными средствами
\+ Делаем .md-шки очищенные для вывода. По этим .md-шкам можно будет смотреть реальную историю изменений. И дифф при пулреквестах.


```python
from multiprocessing import Pool

tasks = []

def convert_tasks(n, d):
    no_output_file = d + "_no_output"
    src_copy = str(abs(hash(n))) + '_' + os.path.basename(n)
    path = os.path.dirname(n)
    return [
        "jupyter nbconvert {} --to markdown --output {}".format(n, d),
        " && ".join([
            "cp {src} {tmp_dir}/{src_copy}",
            "jupyter nbconvert {tmp_dir}/{src_copy} --ClearOutputPreprocessor.enabled=True --inplace",
            "jupyter nbconvert {tmp_dir}/{src_copy} --to markdown --output {src_copy}",
            "cp {tmp_dir}/{src_copy}.md {path}/{no_output_file}.md",
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

print("\n".join(tasks))

def execute_all_in_parallel(tasks):
    ps = []
    for t in tasks:
        ps.append(subprocess.Popen(["bash", "-c", t], stdout=subprocess.PIPE, stderr=subprocess.PIPE))
    for p in ps:
        out, err = p.communicate()
        print(out.decode(), err.decode())

execute_all_in_parallel(tasks)
```

    ../sem26-fs-fuse ['../sem26-fs-fuse/fs_fuse.ipynb']
    ../sem27-python-bindings ['../sem27-python-bindings/bindings.ipynb']
    jupyter nbconvert ../sem26-fs-fuse/fs_fuse.ipynb --to markdown --output README
    cp ../sem26-fs-fuse/fs_fuse.ipynb ./tmp_dir/1677796532700934744_fs_fuse.ipynb && jupyter nbconvert ./tmp_dir/1677796532700934744_fs_fuse.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/1677796532700934744_fs_fuse.ipynb --to markdown --output 1677796532700934744_fs_fuse.ipynb && cp ./tmp_dir/1677796532700934744_fs_fuse.ipynb.md ../sem26-fs-fuse/README_no_output.md
    jupyter nbconvert ../sem27-python-bindings/bindings.ipynb --to markdown --output README
    cp ../sem27-python-bindings/bindings.ipynb ./tmp_dir/1633925075381849959_bindings.ipynb && jupyter nbconvert ./tmp_dir/1633925075381849959_bindings.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/1633925075381849959_bindings.ipynb --to markdown --output 1633925075381849959_bindings.ipynb && cp ./tmp_dir/1633925075381849959_bindings.ipynb.md ../sem27-python-bindings/README_no_output.md
     [NbConvertApp] Converting notebook ../sem26-fs-fuse/fs_fuse.ipynb to markdown
    [NbConvertApp] Writing 43992 bytes to ../sem26-fs-fuse/README.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/1677796532700934744_fs_fuse.ipynb to notebook
    [NbConvertApp] Writing 39042 bytes to ./tmp_dir/1677796532700934744_fs_fuse.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/1677796532700934744_fs_fuse.ipynb to markdown
    [NbConvertApp] Writing 28445 bytes to ./tmp_dir/1677796532700934744_fs_fuse.ipynb.md
    
     [NbConvertApp] Converting notebook ../sem27-python-bindings/bindings.ipynb to markdown
    [NbConvertApp] Writing 38278 bytes to ../sem27-python-bindings/README.md
    
     [NbConvertApp] Converting notebook ./tmp_dir/1633925075381849959_bindings.ipynb to notebook
    [NbConvertApp] Writing 42981 bytes to ./tmp_dir/1633925075381849959_bindings.ipynb
    [NbConvertApp] Converting notebook ./tmp_dir/1633925075381849959_bindings.ipynb to markdown
    [NbConvertApp] Writing 31699 bytes to ./tmp_dir/1633925075381849959_bindings.ipynb.md
    


### Магические исправления

Стандартная конвертилка не учитывает некоторых особенностей маркдауна на гитхабе и некоторых особенностей моих ноутбуков. Поэтому результат надо допилить напильником.


```python
import re


def basic_improve(fname):
    with open(fname, "r") as f:
        r = f.read()
    for b in ["\x00", "\x1B", "\x08"]:
        r = r.replace(b, "")
    with open(fname, "w") as f:
        f.write(r)
    return "dos2unix {}".format(fname)

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
    if fname.endswith(".md"):
        improve_md(fname)

```


```python
tasks = []
shell_tasks = []


for sfx in [".ipynb", ".md"]:
    for hdir in highlevel_dirs:
        for fname in glob.glob("./{}/*".format(hdir) + sfx):
            shell_tasks.append(basic_improve(fname))
            tasks.append(lambda fname=fname: improve_file(fname))
            
execute_all_in_parallel(shell_tasks)
for t in tasks:
    t()
```

     dos2unix: converting file ./../sem26-fs-fuse/fs_fuse.ipynb to Unix format...
    
     dos2unix: converting file ./../sem27-python-bindings/bindings.ipynb to Unix format...
    
     dos2unix: converting file ./../sem26-fs-fuse/README.md to Unix format...
    
     dos2unix: converting file ./../sem26-fs-fuse/README_no_output.md to Unix format...
    
     dos2unix: converting file ./../sem27-python-bindings/README.md to Unix format...
    
     dos2unix: converting file ./../sem27-python-bindings/README_no_output.md to Unix format...
    


### Смотрим изменения


```python
for subdir in highlevel_dirs:
    get_ipython().system("git diff {}/*_no_output*".format(subdir))
```

    [1mdiff --git a/sem26-fs-fuse/README_no_output.md b/sem26-fs-fuse/README_no_output.md[m
    [1mindex eb07774..f7d11e0 100644[m
    [1m--- a/sem26-fs-fuse/README_no_output.md[m
    [1m+++ b/sem26-fs-fuse/README_no_output.md[m
    [36m@@ -192,10 +192,13 @@[m [mint main(int argc, char** argv) {[m
     [m
     ```[m
     [m
    [32m+[m[32m# FUSE[m
     [m
    [31m-```python[m
    [32m+[m[32mВажные опции[m
    [32m+[m[32m* `-f` - запуск в синхронном режиме (без этой опции будет создан демон, а сама программа почти сразу завершится)[m
    [32m+[m[32m* `-s` - запуск в однопоточном режиме.[m
     [m
    [31m-```[m
    [32m+[m[32mВ этом месте что-нибудь про демонизацию стоит расскзать, наверное.[m
     [m
     ## <a name="fusepy"></a> Python + fusepy[m
     [m
    [1mdiff --git a/sem27-python-bindings/README_no_output.md b/sem27-python-bindings/README_no_output.md[m
    [1mindex 7cd90ce..34ef1b8 100644[m
    [1m--- a/sem27-python-bindings/README_no_output.md[m
    [1m+++ b/sem27-python-bindings/README_no_output.md[m
    [36m@@ -207,7 +207,7 @@[m [mstatic PyObject* print_dict(PyObject* self, PyObject* args, PyObject* kwargs) {[m
         printf("\n");[m
         fflush(stdout);[m
     [m
    [31m-    Py_RETURN_NONE;[m
    [32m+[m[32m    Py_RETURN_NONE; // Инкрементит счетчик ссылок None и возвращает его[m
     }[m
     [m
     // Список функций модуля[m


### <a name="github"></a> Коммитим на github


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

    > git add --ignore-errors  ../sem26-fs-fuse/*.ipynb
    > git add --ignore-errors  ../sem26-fs-fuse/*.md
    > git add --ignore-errors  ../sem26-fs-fuse/*.c
    > git add --ignore-errors  ../sem26-fs-fuse/*.cpp
    > git add --ignore-errors -f  -f ../sem26-fs-fuse/bash_popen_tmp/*.html
    warning: could not open directory 'sem26-fs-fuse/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem26-fs-fuse/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem26-fs-fuse/interactive_launcher_tmp/*.log
    > git add -u ../sem26-fs-fuse
    > git add --ignore-errors  ../sem27-python-bindings/*.ipynb
    > git add --ignore-errors  ../sem27-python-bindings/*.md
    > git add --ignore-errors  ../sem27-python-bindings/*.c
    > git add --ignore-errors  ../sem27-python-bindings/*.cpp
    > git add --ignore-errors -f  -f ../sem27-python-bindings/bash_popen_tmp/*.html
    warning: could not open directory 'sem27-python-bindings/bash_popen_tmp/': No such file or directory
    fatal: pathspec '../sem27-python-bindings/bash_popen_tmp/*.html' did not match any files
    > git add --ignore-errors -f  -f ../sem27-python-bindings/interactive_launcher_tmp/*.log
    warning: could not open directory 'sem27-python-bindings/interactive_launcher_tmp/': No such file or directory
    fatal: pathspec '../sem27-python-bindings/interactive_launcher_tmp/*.log' did not match any files
    > git add -u ../sem27-python-bindings
    > git commit -m 'yet another update'
    On branch master
    Your branch is up to date with 'origin/master'.
    
    Changes not staged for commit:
      (use "git add <file>..." to update what will be committed)
      (use "git restore <file>..." to discard changes in working directory)
    	[31mmodified:   ../sem17-sockets-tcp-udp/sockets-tcp-udp.ipynb[m
    	[31mmodified:   save_them_all.ipynb[m
    
    Untracked files:
      (use "git add <file>..." to include in what will be committed)
    	[31m../sem14-fifo-proc/interactive_launcher_tmp/127766679693617464.log.md[m
    	[31m../sem14-fifo-proc/interactive_launcher_tmp/130144623552729609.log.md[m
    	[31m../sem14-fifo-proc/interactive_launcher_tmp/131874295726709881.log.md[m
    	[31m../sem14-fifo-proc/interactive_launcher_tmp/354080172890454133.log.md[m
    	[31m../sem14-fifo-proc/interactive_launcher_tmp/401914075815381515.log.md[m
    	[31m../sem14-fifo-proc/interactive_launcher_tmp/52428862673654368.log.md[m
    	[31m../sem24-http-libcurl-cmake/curl_cmake_example/[m
    	[31m../sem24-http-libcurl-cmake/interactive_launcher_tmp/278908528095121695.log.md[m
    	[31m../sem24-http-libcurl-cmake/interactive_launcher_tmp/279871983378398179.log.md[m
    	[31m../sem24-http-libcurl-cmake/interactive_launcher_tmp/625914360777685168.log.md[m
    	[31m../sem24-http-libcurl-cmake/interactive_launcher_tmp/825717250500169114.log.md[m
    	[31m../sem24-http-libcurl-cmake/simple_cmake_example/CMakeLists.txt[m
    	[31m../sem24-http-libcurl-cmake/webdav_dir/[m
    	[31m../sem26-fs-fuse/err.txt[m
    	[31m../sem26-fs-fuse/fuse_c_example/CMake/[m
    	[31m../sem26-fs-fuse/fuse_c_example/CMakeLists.txt[m
    	[31m../sem26-fs-fuse/fuse_c_example/build/CMakeCache.txt[m
    	[31m../sem26-fs-fuse/fuse_c_example/build/CMakeFiles/3.16.3/CMakeCCompiler.cmake[m
    	[31m../sem26-fs-fuse/fuse_c_example/build/CMakeFiles/3.16.3/CMakeCXXCompiler.cmake[m
    	[31m../sem26-fs-fuse/fuse_c_example/build/CMakeFiles/3.16.3/CMakeDetermineCompilerABI_C.bin[m
    	[31m../sem26-fs-fuse/fuse_c_example/build/CMakeFiles/3.16.3/CMakeDetermineCompilerABI_CXX.bin[m
    	[31m../sem26-fs-fuse/fuse_c_example/build/CMakeFiles/3.16.3/CMakeSystem.cmake[m
    	[31m../sem26-fs-fuse/fuse_c_example/build/CMakeFiles/3.16.3/CompilerIdC/[m
    	[31m../sem26-fs-fuse/fuse_c_example/build/CMakeFiles/3.16.3/CompilerIdCXX/a.out[m
    	[31m../sem26-fs-fuse/fuse_c_example/build/CMakeFiles/CMakeDirectoryInformation.cmake[m
    	[31m../sem26-fs-fuse/fuse_c_example/build/CMakeFiles/CMakeOutput.log[m
    	[31m../sem26-fs-fuse/fuse_c_example/build/CMakeFiles/Makefile.cmake[m
    	[31m../sem26-fs-fuse/fuse_c_example/build/CMakeFiles/Makefile2[m
    	[31m../sem26-fs-fuse/fuse_c_example/build/CMakeFiles/TargetDirectories.txt[m
    	[31m../sem26-fs-fuse/fuse_c_example/build/CMakeFiles/cmake.check_cache[m
    	[31m../sem26-fs-fuse/fuse_c_example/build/CMakeFiles/fuse-example.dir/[m
    	[31m../sem26-fs-fuse/fuse_c_example/build/CMakeFiles/progress.marks[m
    	[31m../sem26-fs-fuse/fuse_c_example/build/Makefile[m
    	[31m../sem26-fs-fuse/fuse_c_example/build/cmake_install.cmake[m
    	[31m../sem26-fs-fuse/fuse_c_example/build/fuse-example[m
    	[31m../sem26-fs-fuse/fuse_json.py[m
    	[31m../sem26-fs-fuse/interactive_launcher_tmp/286997408202756564.log.md[m
    	[31m../sem26-fs-fuse/interactive_launcher_tmp/470231275623214705.log.md[m
    	[31m../sem26-fs-fuse/interactive_launcher_tmp/576155038309430211.log.md[m
    	[31m../sem26-fs-fuse/interactive_launcher_tmp/634529996390274655.log.md[m
    	[31m../sem26-fs-fuse/interactive_launcher_tmp/74592816055267592.log.md[m
    	[31m../sem26-fs-fuse/interactive_launcher_tmp/758593138878100872.log.md[m
    	[31m../sem26-fs-fuse/interactive_launcher_tmp/861048178809047391.log.md[m
    	[31m../sem26-fs-fuse/new_line[m
    	[31m../sem26-fs-fuse/with_fuse_1.cmake[m
    	[31m../sem26-fs-fuse/with_fuse_2.cmake[m
    	[31m../sem27-python-bindings/api_module_example.py[m
    	[31m../sem27-python-bindings/c_api_module_2_example.py[m
    	[31m../sem27-python-bindings/count_1e8_cython.py[m
    	[31m../sem27-python-bindings/count_1e8_native.py[m
    	[31m../sem27-python-bindings/ctypes_example.py[m
    	[31m../sem27-python-bindings/cython_setup.py[m
    	[31m../sem27-python-bindings/pairs.h[m
    	[31m../sem27-python-bindings/pairs.pxd[m
    	[31m../sem27-python-bindings/pairs.pyx[m
    	[31m../sem27-python-bindings/pybind_setup.py[m
    	[31m../sem27-python-bindings/test_pairs.py[m
    	[31m../sem27-python-bindings/test_pybind_pairs.py[m
    	[31ma.py[m
    	[31minteractive_launcher_tmp/341853987367548759.log.md[m
    	[31minteractive_launcher_tmp/937518918408435731.log.md[m
    	[31minteractive_launcher_tmp/978838368375159155.log.md[m
    	[31minteractive_launcher_tmp/981384515921563800.log.md[m
    	[31mlauncher.py[m
    	[31mtmp_dir/[m
    
    no changes added to commit (use "git add" and/or "git commit -a")
    > git push origin master
    Everything up-to-date



```python

```


```python

```


```python

```
