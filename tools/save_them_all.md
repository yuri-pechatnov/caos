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
    #sorted(glob.glob("../sem17*")),
    sorted(glob.glob("../sem19*")),
    #sorted(glob.glob("../extra*")),
], [])

print("Highlevel dirs:", highlevel_dirs)
```

    Highlevel dirs: ['../tools', '../sem19-multiplexing']



```python
tmp_dir = "./tmp_dir"
get_ipython().system('rm -r {tmp_dir} ; mkdir {tmp_dir} 2>&1 | grep -v "File exists"'.format(tmp_dir=tmp_dir))
```

### Генерируем все .md-шки стандартными средствами
\+ Делаем .md-шки очищенные для вывода. По этим .md-шкам можно будет смотреть реальную историю изменений. И дифф при пулреквестах.


```python
def execute_all_in_parallel(tasks):
    ps = []
    for t in tasks:
        ps.append(subprocess.Popen(["bash", "-c", t], stdout=subprocess.PIPE, stderr=subprocess.PIPE))
    for p in ps:
        out, err = p.communicate()
        print(out.decode(), err.decode())
```


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

execute_all_in_parallel(tasks)
```

    ../tools ['../tools/set_up_magics.ipynb', '../tools/save_them_all.ipynb', '../tools/stand.ipynb', '../tools/set_up_magics_dev.ipynb']
    ../sem19-multiplexing ['../sem19-multiplexing/epoll-poll-select-linuxaio.ipynb']
    jupyter nbconvert ../tools/set_up_magics.ipynb --to markdown --output set_up_magics
    cp ../tools/set_up_magics.ipynb ./tmp_dir/8183460404960798655_set_up_magics.ipynb && jupyter nbconvert ./tmp_dir/8183460404960798655_set_up_magics.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/8183460404960798655_set_up_magics.ipynb --to markdown --output 8183460404960798655_set_up_magics.ipynb && cp ./tmp_dir/8183460404960798655_set_up_magics.ipynb.md ../tools/set_up_magics_no_output.md
    jupyter nbconvert ../tools/save_them_all.ipynb --to markdown --output save_them_all
    cp ../tools/save_them_all.ipynb ./tmp_dir/4218786211731848798_save_them_all.ipynb && jupyter nbconvert ./tmp_dir/4218786211731848798_save_them_all.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/4218786211731848798_save_them_all.ipynb --to markdown --output 4218786211731848798_save_them_all.ipynb && cp ./tmp_dir/4218786211731848798_save_them_all.ipynb.md ../tools/save_them_all_no_output.md
    jupyter nbconvert ../tools/stand.ipynb --to markdown --output stand
    cp ../tools/stand.ipynb ./tmp_dir/8679878107905176962_stand.ipynb && jupyter nbconvert ./tmp_dir/8679878107905176962_stand.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/8679878107905176962_stand.ipynb --to markdown --output 8679878107905176962_stand.ipynb && cp ./tmp_dir/8679878107905176962_stand.ipynb.md ../tools/stand_no_output.md
    jupyter nbconvert ../tools/set_up_magics_dev.ipynb --to markdown --output set_up_magics_dev
    cp ../tools/set_up_magics_dev.ipynb ./tmp_dir/2718278040961790765_set_up_magics_dev.ipynb && jupyter nbconvert ./tmp_dir/2718278040961790765_set_up_magics_dev.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/2718278040961790765_set_up_magics_dev.ipynb --to markdown --output 2718278040961790765_set_up_magics_dev.ipynb && cp ./tmp_dir/2718278040961790765_set_up_magics_dev.ipynb.md ../tools/set_up_magics_dev_no_output.md
    jupyter nbconvert ../sem19-multiplexing/epoll-poll-select-linuxaio.ipynb --to markdown --output README
    cp ../sem19-multiplexing/epoll-poll-select-linuxaio.ipynb ./tmp_dir/5170477303705024251_epoll-poll-select-linuxaio.ipynb && jupyter nbconvert ./tmp_dir/5170477303705024251_epoll-poll-select-linuxaio.ipynb --ClearOutputPreprocessor.enabled=True --inplace && jupyter nbconvert ./tmp_dir/5170477303705024251_epoll-poll-select-linuxaio.ipynb --to markdown --output 5170477303705024251_epoll-poll-select-linuxaio.ipynb && cp ./tmp_dir/5170477303705024251_epoll-poll-select-linuxaio.ipynb.md ../sem19-multiplexing/README_no_output.md


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
                return "\n<pre>\n" + f.read() + "\n</pre>\n"
    
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

### Смотрим изменения


```python
for subdir in highlevel_dirs:
    get_ipython().system("git diff {}/*_no_output*".format(subdir))
```

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


```python

```


```python
!git add ../README.md
!git commit -m "Update readme"
!git push origin master
```

    On branch master
    Your branch is up to date with 'origin/master'.
    
    Changes not staged for commit:
      (use "git add/rm <file>..." to update what will be committed)
      (use "git restore <file>..." to discard changes in working directory)
    	[31mmodified:   ../caos_2019-2020/sem04-asm-arm/arm.ipynb[m
    	[31mmodified:   ../caos_2019-2020/sem05-asm-arm-addressing/adressing.ipynb[m
    	[31mmodified:   ../caos_2019-2020/sem09-low-level-io/low-level-io.ipynb[m
    	[31mmodified:   ../caos_2019-2020/sem12-fork-exec-pipe/fork-pipe-exec.ipynb[m
    	[31mmodified:   ../caos_2019-2020/sem13-signal/signal.ipynb[m
    	[31mmodified:   ../caos_2019-2020/sem14-fifo-proc/fifo-proc.ipynb[m
    	[31mmodified:   ../caos_2019-2020/sem16-fcntl-dup-pipe/fcntl-dup-pipe.ipynb[m
    	[31mmodified:   ../sem01-intro-linux/intro_linux.ipynb[m
    	[31mmodified:   ../sem05-arm-asm/arm_asm.ipynb[m
    	[31mmodified:   ../sem05-arm-asm/test_call.c[m
    	[31mmodified:   ../sem07-x86-asm/asm_x86-64.ipynb[m
    	[31mmodified:   ../sem08-x86-fpmath-sse/example.c[m
    	[31mmodified:   ../sem08-x86-fpmath-sse/fpmath_sse.ipynb[m
    	[31mmodified:   ../sem09-x86-asm-nostdlib/nostdlib.ipynb[m
    	[31mmodified:   ../sem13-fork-exec-pipe/fork-pipe-exec.ipynb[m
    	[31mdeleted:    interactive_launcher_tmp/161300355223537194.log[m
    	[31mdeleted:    interactive_launcher_tmp/371757449732314597.log[m
    	[31mdeleted:    interactive_launcher_tmp/519997700123128471.log[m
    	[31mdeleted:    interactive_launcher_tmp/688225086340752142.log[m
    	[31mmodified:   save_them_all.ipynb[m
    	[31mmodified:   set_up_magics.ipynb[m
    
    Untracked files:
      (use "git add <file>..." to include in what will be committed)
    	[31m../extra-c-basics/001.expected[m
    	[31m../extra-c-basics/001.in[m
    	[31m../extra-c-basics/001.out[m
    	[31m../extra-c-basics/common.h[m
    	[31m../extra-c-basics/lib.h[m
    	[31m../extra-c-basics/out[m
    	[31m../extra-c-basics/test.h[m
    	[31m../extra-c-basics/test.sh[m
    	[31m../sem01-intro-linux/2[m
    	[31m../sem01-intro-linux/a.sh[m
    	[31m../sem01-intro-linux/a.txt[m
    	[31m../sem01-intro-linux/b.txt[m
    	[31m../sem01-intro-linux/err[m
    	[31m../sem01-intro-linux/err.txt[m
    	[31m../sem01-intro-linux/file.txt[m
    	[31m../sem01-intro-linux/file2.txt[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/104342371564797143.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/134856167677051543.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/159458045408584490.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/160806476576838908.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/237887356681257319.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/272097196292346552.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/291478633046804378.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/299779856288802977.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/308063480967833729.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/33203167529660997.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/357027856339287279.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/357412744100799582.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/363736236628022803.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/368536380590515236.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/38653968711948138.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/390229348646733600.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/44638202710957050.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/488615332501153522.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/547003026116871195.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/551941962051262166.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/577835561503179454.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/580865104873188457.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/58318340108386158.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/60572178042054264.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/659941140088179386.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/718444346696566167.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/731534507899320722.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/761344427069245362.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/777467580737092300.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/799265701962172890.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/887167192337308461.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/906648630641962511.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/928497432930519081.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/93290423175998573.log.md[m
    	[31m../sem01-intro-linux/interactive_launcher_tmp/965132557118189549.log.md[m
    	[31m../sem01-intro-linux/out[m
    	[31m../sem01-intro-linux/out.txt[m
    	[31m../sem02-instruments-compilation/a.out[m
    	[31m../sem02-instruments-compilation/a.txt[m
    	[31m../sem02-instruments-compilation/g.h[m
    	[31m../sem02-instruments-compilation/lib.[m
    	[31m../sem02-instruments-compilation/lib.a[m
    	[31m../sem02-instruments-compilation/out[m
    	[31m../sem02-instruments-compilation/preprocessing_max.h[m
    	[31m../sem03-cmake-python-bindings/._poll.png[m
    	[31m../sem03-cmake-python-bindings/api_module_example.py[m
    	[31m../sem03-cmake-python-bindings/c_api_module_2_example.py[m
    	[31m../sem03-cmake-python-bindings/c_api_own_type_module_example.py[m
    	[31m../sem03-cmake-python-bindings/count_1e8_cython.py[m
    	[31m../sem03-cmake-python-bindings/count_1e8_native.py[m
    	[31m../sem03-cmake-python-bindings/ctypes_example.py[m
    	[31m../sem03-cmake-python-bindings/cython_setup.py[m
    	[31m../sem03-cmake-python-bindings/make_example/[m
    	[31m../sem03-cmake-python-bindings/pairs.h[m
    	[31m../sem03-cmake-python-bindings/pairs.pxd[m
    	[31m../sem03-cmake-python-bindings/pairs.pyx[m
    	[31m../sem03-cmake-python-bindings/pybind_setup.py[m
    	[31m../sem03-cmake-python-bindings/python_cmake_example/[m
    	[31m../sem03-cmake-python-bindings/simple_cmake_example/[m
    	[31m../sem03-cmake-python-bindings/test_pairs.py[m
    	[31m../sem03-cmake-python-bindings/test_pybind_pairs.py[m
    	[31m../sem04-int-float/code_sample[m
    	[31m../sem04-int-float/run_ub.py[m
    	[31m../sem04-int-float/stand.h[m
    	[31m../sem04-int-float/text[m
    	[31m../sem05-arm-asm/code_sample[m
    	[31m../sem05-arm-asm/run_ub.py[m
    	[31m../sem05-arm-asm/stand.h[m
    	[31m../sem06-arm-asm-addressing/structs_in_memory_common.h[m
    	[31m../sem07-x86-asm/asm_filter_useless[m
    	[31m../sem08-x86-fpmath-sse/asm_filter_useless[m
    	[31m../sem09-x86-asm-nostdlib/example4.s[m
    	[31m../sem09-x86-asm-nostdlib/strace.out[m
    	[31m../sem10-low-level-io/a.txt[m
    	[31m../sem10-low-level-io/b.txt[m
    	[31m../sem10-low-level-io/linux_example_input_000.txt[m
    	[31m../sem10-low-level-io/linux_example_input_001.txt[m
    	[31m../sem10-low-level-io/linux_example_input_002.txt[m
    	[31m../sem10-low-level-io/linux_example_read.txt[m
    	[31m../sem10-low-level-io/linux_file_hello_world.out[m
    	[31m../sem10-low-level-io/strange_example.out[m
    	[31m../sem10-low-level-io/winapi_example_input_001.txt[m
    	[31m../sem11-file-attributes/a.txt[m
    	[31m../sem11-file-attributes/c.txt[m
    	[31m../sem11-file-attributes/fcntl_open_flags.1[m
    	[31m../sem11-file-attributes/fcntl_open_flags.2[m
    	[31m../sem11-file-attributes/tmp/[m
    	[31m../sem11-file-attributes/tmp2/[m
    	[31m../sem11-file-attributes/x.txt[m
    	[31m../sem11-file-attributes/x_hard.txt[m
    	[31m../sem11-file-attributes/x_ordinary.txt[m
    	[31m../sem11-file-attributes/x_sym.txt[m
    	[31m../sem11-file-attributes/y.txt[m
    	[31m../sem12-mmap-instrumentation/buf.txt[m
    	[31m../sem12-mmap-instrumentation/core[m
    	[31m../sem12-mmap-instrumentation/err.txt[m
    	[31m../sem12-mmap-instrumentation/input.txt[m
    	[31m../sem12-mmap-instrumentation/out.txt[m
    	[31m../sem12-mmap-instrumentation/perf.data[m
    	[31m../sem12-mmap-instrumentation/perf.data.old[m
    	[31m../sem12-mmap-instrumentation/perf.log[m
    	[31m../sem12-mmap-instrumentation/ub.s[m
    	[31m../sem13-fork-exec-pipe/interactive_launcher_tmp/120099524664157959.log.md[m
    	[31m../sem13-fork-exec-pipe/interactive_launcher_tmp/163140227730853841.log.md[m
    	[31m../sem13-fork-exec-pipe/interactive_launcher_tmp/163593432921738763.log.md[m
    	[31m../sem13-fork-exec-pipe/interactive_launcher_tmp/209145350284758468.log.md[m
    	[31m../sem13-fork-exec-pipe/interactive_launcher_tmp/233840872790699678.log.md[m
    	[31m../sem13-fork-exec-pipe/interactive_launcher_tmp/262589068332370635.log.md[m
    	[31m../sem13-fork-exec-pipe/interactive_launcher_tmp/42444701050405087.log.md[m
    	[31m../sem13-fork-exec-pipe/interactive_launcher_tmp/471762647218276060.log.md[m
    	[31m../sem13-fork-exec-pipe/interactive_launcher_tmp/568443493602728579.log.md[m
    	[31m../sem13-fork-exec-pipe/interactive_launcher_tmp/587663143183065472.log.md[m
    	[31m../sem13-fork-exec-pipe/interactive_launcher_tmp/661271384168580616.log.md[m
    	[31m../sem13-fork-exec-pipe/interactive_launcher_tmp/800322025668676291.log.md[m
    	[31m../sem13-fork-exec-pipe/interactive_launcher_tmp/828907652833599586.log.md[m
    	[31m../sem13-fork-exec-pipe/interactive_launcher_tmp/872255679658661305.log.md[m
    	[31m../sem13-fork-exec-pipe/interactive_launcher_tmp/910194392546339665.log.md[m
    	[31m../sem13-fork-exec-pipe/interactive_launcher_tmp/989029077056429449.log.md[m
    	[31m../sem13-fork-exec-pipe/log.txt[m
    	[31m../sem15-signal/core[m
    	[31m../sem15-signal/interactive_launcher_tmp/579243924070969653.log.md[m
    	[31m../sem16-fcntl-dup-pipe/fcntl_open_flags.1[m
    	[31m../sem16-fcntl-dup-pipe/fcntl_open_flags.2[m
    	[31m../sem16-fcntl-dup-pipe/out.txt[m
    	[31m../sem17-longjmp-catch-segfault/ex2[m
    	[31m../sem17-longjmp-catch-segfault/ex3[m
    	[31m../sem17-longjmp-catch-segfault/ex5[m
    	[31ma.py[m
    	[31minteractive_launcher_tmp/151915220983990284.log[m
    	[31minteractive_launcher_tmp/151915220983990284.log.md[m
    	[31minteractive_launcher_tmp/440803670352846463.log[m
    	[31minteractive_launcher_tmp/440803670352846463.log.md[m
    	[31minteractive_launcher_tmp/563095797938734025.log[m
    	[31minteractive_launcher_tmp/563095797938734025.log.md[m
    	[31minteractive_launcher_tmp/716527050313963328.log[m
    	[31minteractive_launcher_tmp/716527050313963328.log.md[m
    	[31mlauncher.py[m
    	[31mtmp_dir/[m
    
    no changes added to commit (use "git add" and/or "git commit -a")
    Everything up-to-date



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
