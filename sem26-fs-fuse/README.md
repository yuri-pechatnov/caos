

```python
# look at tools/set_up_magics.ipynb
yandex_metrica_allowed = True ; get_ipython().run_cell('# one_liner_str\n\nget_ipython().run_cell_magic(\'javascript\', \'\', \n    \'// setup cpp code highlighting\\n\'\n    \'IPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\\\'reg\\\':[/^%%cpp/]} ;\'\n    \'IPython.CodeCell.options_default.highlight_modes["text/x-cmake"] = {\\\'reg\\\':[/^%%cmake/]} ;\'\n)\n\n# creating magics\nfrom IPython.core.magic import register_cell_magic, register_line_magic\nfrom IPython.display import display, Markdown, HTML\nimport argparse\nfrom subprocess import Popen, PIPE\nimport random\nimport sys\nimport os\nimport re\nimport signal\nimport shutil\nimport shlex\nimport glob\nimport time\n\n@register_cell_magic\ndef save_file(args_str, cell, line_comment_start="#"):\n    parser = argparse.ArgumentParser()\n    parser.add_argument("fname")\n    parser.add_argument("--ejudge-style", action="store_true")\n    args = parser.parse_args(args_str.split())\n    \n    cell = cell if cell[-1] == \'\\n\' or args.no_eof_newline else cell + "\\n"\n    cmds = []\n    with open(args.fname, "w") as f:\n        f.write(line_comment_start + " %%cpp " + args_str + "\\n")\n        for line in cell.split("\\n"):\n            line_to_write = (line if not args.ejudge_style else line.rstrip()) + "\\n"\n            if line.startswith("%"):\n                run_prefix = "%run "\n                if line.startswith(run_prefix):\n                    cmds.append(line[len(run_prefix):].strip())\n                    f.write(line_comment_start + " " + line_to_write)\n                    continue\n                run_prefix = "%# "\n                if line.startswith(run_prefix):\n                    f.write(line_comment_start + " " + line_to_write)\n                    continue\n                raise Exception("Unknown %%save_file subcommand: \'%s\'" % line)\n            else:\n                f.write(line_to_write)\n        f.write("" if not args.ejudge_style else line_comment_start + r" line without \\n")\n    for cmd in cmds:\n        display(Markdown("Run: `%s`" % cmd))\n        get_ipython().system(cmd)\n\n@register_cell_magic\ndef cpp(fname, cell):\n    save_file(fname, cell, "//")\n    \n@register_cell_magic\ndef cmake(fname, cell):\n    save_file(fname, cell, "#")\n\n@register_cell_magic\ndef asm(fname, cell):\n    save_file(fname, cell, "//")\n    \n@register_cell_magic\ndef makefile(fname, cell):\n    assert not fname\n    save_file("makefile", cell.replace(" " * 4, "\\t"))\n        \n@register_line_magic\ndef p(line):\n    line = line.strip() \n    if line[0] == \'#\':\n        display(Markdown(line[1:].strip()))\n    else:\n        try:\n            expr, comment = line.split(" #")\n            display(Markdown("`{} = {}`  # {}".format(expr.strip(), eval(expr), comment.strip())))\n        except:\n            display(Markdown("{} = {}".format(line, eval(line))))\n    \n    \ndef show_log_file(file, return_html_string=False):\n    obj = file.replace(\'.\', \'_\').replace(\'/\', \'_\') + "_obj"\n    html_string = \'\'\'\n        <!--MD_BEGIN_FILTER-->\n        <script type=text/javascript>\n        var entrance___OBJ__ = 0;\n        var errors___OBJ__ = 0;\n        function halt__OBJ__(elem, color)\n        {\n            elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    \n        }\n        function refresh__OBJ__()\n        {\n            entrance___OBJ__ -= 1;\n            if (entrance___OBJ__ < 0) {\n                entrance___OBJ__ = 0;\n            }\n            var elem = document.getElementById("__OBJ__");\n            if (elem) {\n                var xmlhttp=new XMLHttpRequest();\n                xmlhttp.onreadystatechange=function()\n                {\n                    var elem = document.getElementById("__OBJ__");\n                    console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___OBJ__);\n                    if (elem && xmlhttp.readyState==4) {\n                        if (xmlhttp.status==200)\n                        {\n                            errors___OBJ__ = 0;\n                            if (!entrance___OBJ__) {\n                                if (elem.innerHTML != xmlhttp.responseText) {\n                                    elem.innerHTML = xmlhttp.responseText;\n                                }\n                                if (elem.innerHTML.includes("Process finished.")) {\n                                    halt__OBJ__(elem, "#333333");\n                                } else {\n                                    entrance___OBJ__ += 1;\n                                    console.log("req");\n                                    window.setTimeout("refresh__OBJ__()", 300); \n                                }\n                            }\n                            return xmlhttp.responseText;\n                        } else {\n                            errors___OBJ__ += 1;\n                            if (!entrance___OBJ__) {\n                                if (errors___OBJ__ < 6) {\n                                    entrance___OBJ__ += 1;\n                                    console.log("req");\n                                    window.setTimeout("refresh__OBJ__()", 300); \n                                } else {\n                                    halt__OBJ__(elem, "#994444");\n                                }\n                            }\n                        }\n                    }\n                }\n                xmlhttp.open("GET", "__FILE__", true);\n                xmlhttp.setRequestHeader("Cache-Control", "no-cache");\n                xmlhttp.send();     \n            }\n        }\n        \n        if (!entrance___OBJ__) {\n            entrance___OBJ__ += 1;\n            refresh__OBJ__(); \n        }\n        </script>\n\n        <p id="__OBJ__" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">\n        </p>\n        \n        </font>\n        <!--MD_END_FILTER-->\n        <!--MD_FROM_FILE __FILE__.md -->\n        \'\'\'.replace("__OBJ__", obj).replace("__FILE__", file)\n    if return_html_string:\n        return html_string\n    display(HTML(html_string))\n\n    \nclass TInteractiveLauncher:\n    tmp_path = "./interactive_launcher_tmp"\n    def __init__(self, cmd):\n        try:\n            os.mkdir(TInteractiveLauncher.tmp_path)\n        except:\n            pass\n        name = str(random.randint(0, 1e18))\n        self.inq_path = os.path.join(TInteractiveLauncher.tmp_path, name + ".inq")\n        self.log_path = os.path.join(TInteractiveLauncher.tmp_path, name + ".log")\n        \n        os.mkfifo(self.inq_path)\n        open(self.log_path, \'w\').close()\n        open(self.log_path + ".md", \'w\').close()\n\n        self.pid = os.fork()\n        if self.pid == -1:\n            print("Error")\n        if self.pid == 0:\n            exe_cands = glob.glob("../tools/launcher.py") + glob.glob("../../tools/launcher.py")\n            assert(len(exe_cands) == 1)\n            assert(os.execvp("python3", ["python3", exe_cands[0], "-l", self.log_path, "-i", self.inq_path, "-c", cmd]) == 0)\n        self.inq_f = open(self.inq_path, "w")\n        interactive_launcher_opened_set.add(self.pid)\n        show_log_file(self.log_path)\n\n    def write(self, s):\n        s = s.encode()\n        assert len(s) == os.write(self.inq_f.fileno(), s)\n        \n    def get_pid(self):\n        n = 100\n        for i in range(n):\n            try:\n                return int(re.findall(r"PID = (\\d+)", open(self.log_path).readline())[0])\n            except:\n                if i + 1 == n:\n                    raise\n                time.sleep(0.1)\n        \n    def input_queue_path(self):\n        return self.inq_path\n        \n    def wait_stop(self, timeout):\n        for i in range(int(timeout * 10)):\n            wpid, status = os.waitpid(self.pid, os.WNOHANG)\n            if wpid != 0:\n                return True\n            time.sleep(0.1)\n        return False\n        \n    def close(self, timeout=3):\n        self.inq_f.close()\n        if not self.wait_stop(timeout):\n            os.kill(self.get_pid(), signal.SIGKILL)\n            os.waitpid(self.pid, 0)\n        os.remove(self.inq_path)\n        # os.remove(self.log_path)\n        self.inq_path = None\n        self.log_path = None \n        interactive_launcher_opened_set.remove(self.pid)\n        self.pid = None\n        \n    @staticmethod\n    def terminate_all():\n        if "interactive_launcher_opened_set" not in globals():\n            globals()["interactive_launcher_opened_set"] = set()\n        global interactive_launcher_opened_set\n        for pid in interactive_launcher_opened_set:\n            print("Terminate pid=" + str(pid), file=sys.stderr)\n            os.kill(pid, signal.SIGKILL)\n            os.waitpid(pid, 0)\n        interactive_launcher_opened_set = set()\n        if os.path.exists(TInteractiveLauncher.tmp_path):\n            shutil.rmtree(TInteractiveLauncher.tmp_path)\n    \nTInteractiveLauncher.terminate_all()\n   \nyandex_metrica_allowed = bool(globals().get("yandex_metrica_allowed", False))\nif yandex_metrica_allowed:\n    display(HTML(\'\'\'<!-- YANDEX_METRICA_BEGIN -->\n    <script type="text/javascript" >\n       (function(m,e,t,r,i,k,a){m[i]=m[i]||function(){(m[i].a=m[i].a||[]).push(arguments)};\n       m[i].l=1*new Date();k=e.createElement(t),a=e.getElementsByTagName(t)[0],k.async=1,k.src=r,a.parentNode.insertBefore(k,a)})\n       (window, document, "script", "https://mc.yandex.ru/metrika/tag.js", "ym");\n\n       ym(59260609, "init", {\n            clickmap:true,\n            trackLinks:true,\n            accurateTrackBounce:true\n       });\n    </script>\n    <noscript><div><img src="https://mc.yandex.ru/watch/59260609" style="position:absolute; left:-9999px;" alt="" /></div></noscript>\n    <!-- YANDEX_METRICA_END -->\'\'\'))\n\ndef make_oneliner():\n    html_text = \'("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований. Если вы не хотите, чтобы по вам собиралась статистика, исправьте: yandex_metrica_allowed = False" if yandex_metrica_allowed else "")\'\n    html_text += \' + "<""!-- MAGICS_SETUP_PRINTING_END -->"\'\n    return \'\'.join([\n        \'# look at tools/set_up_magics.ipynb\\n\',\n        \'yandex_metrica_allowed = True ; get_ipython().run_cell(%s);\' % repr(one_liner_str),\n        \'display(HTML(%s))\' % html_text,\n        \' #\'\'MAGICS_SETUP_END\'\n    ])\n       \n\n');display(HTML(("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований. Если вы не хотите, чтобы по вам собиралась статистика, исправьте: yandex_metrica_allowed = False" if yandex_metrica_allowed else "") + "<""!-- MAGICS_SETUP_PRINTING_END -->")) #MAGICS_SETUP_END
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



# FUSE

<table width=100%  > <tr>
    <th width=15%> <b>Видео с семинара &rarr; </b> </th>
    <th>
    <a href="https://youtu.be/__RuADlaK0k"><img src="video.jpg" width="320" 
   height="160" align="left" alt="Видео с семинара"></a>
    </th>
    <th> </th>
</tr> </table>

Сегодня в программе:
* <a href="#fs_posix" style="color:#856024"> Работа с файловой системой POSIX </a>
  * <a href="#opendir" style="color:#856024"> Просмотр содержимого директории c фильтрацией по регулярке </a>
  * <a href="#glob" style="color:#856024"> glob или история о том, как вы пишете *.cpp в терминале </a>
  * <a href="#ftw" style="color:#856024"> Рекурсивный просмотр. Правда с помощью устаревшей функции. </a>
  * <a href="#fs_stat" style="color:#856024"> Информация о файловой системе. </a>
  
* <a href="#fusepy" style="color:#856024"> Примонтируем json как read-only файловую систему. Python + fusepy </a>
* <a href="#fuse_с" style="color:#856024"> Файловая система с одним файлом на C </a>


https://ru.wikipedia.org/wiki/FUSE_(модуль_ядра)

![FUSE](https://upload.wikimedia.org/wikipedia/commons/thumb/0/08/FUSE_structure.svg/490px-FUSE_structure.svg.png)


https://habr.com/ru/post/315654/ - на питоне

https://engineering.facile.it/blog/eng/write-filesystem-fuse/




[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/fuse)
  
  
<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>



## <a name="fs_posix"></a> Работа с файловой системой в POSIX




Заголовочные файлы, в которых есть функции для работы с файловой системой ([wiki-источник](https://en.wikipedia.org/wiki/C_POSIX_library)):

| Header file | Description |
|-------------|-------------|
| `<fcntl.h>` |	File opening, locking and other operations |
| `<fnmatch.h>` |	Filename matching |
| `<ftw.h>` |	File tree traversal |
| `<sys/stat.h>` |	File information (stat et al.) |
| `<sys/statvfs.h>` |	File System information |
| `<dirent.h>` | Directories opening, traversing |


read, write, stat, fstat - это все было раньше


## <a name="opendir"></a> Просмотр содержимого директории с фильтрацией по регулярке


```python
%%cpp traverse_dir.c
%run gcc -Wall -Werror -fsanitize=address traverse_dir.c -lpthread -o traverse_dir.exe
%run ./traverse_dir.exe ..

#include <stdio.h>
#include <dirent.h>
#include <assert.h>
#include <fnmatch.h>

int main(int argc, char** argv) {
    assert(argc == 2);
    const char* dir_path = argv[1];
    DIR *pDir = opendir(dir_path);
    if (pDir == NULL) {
        fprintf(stderr, "Cannot open directory '%s'\n", dir_path);
        return 1;
    }
    int limit = 4;
    for (struct dirent *pDirent; (pDirent = readdir(pDir)) != NULL && limit > 0;) {
        // + Регулярочки
        if (fnmatch("sem2*", pDirent->d_name, 0) == 0) {
            printf("%s\n", pDirent->d_name);
            --limit;
        }
    }

    closedir(pDir);
    return 0;
}
```


Run: `gcc -Wall -Werror -fsanitize=address traverse_dir.c -lpthread -o traverse_dir.exe`



Run: `./traverse_dir.exe ..`


    sem22-dynamic-lib
    sem27-python-bindings
    sem23-extra-net-protocols
    sem28-unix-time


## <a name="glob"></a> glob или история о том, как вы пишете *.cpp в терминале

Это не совсем про файловую систему, но тем не менее интересно

glob хорошо сочетается с exec, пример тут http://man7.org/linux/man-pages/man3/glob.3.html


```python
%%cpp traverse_dir.c
%run gcc -Wall -Werror -fsanitize=address traverse_dir.c -lpthread -o traverse_dir.exe
%run ./traverse_dir.exe .. | head -n 5

#include <stdio.h>
#include <assert.h>
#include <glob.h>

int main() {
    glob_t globbuf = {0};
    glob("*.c", GLOB_DOOFFS, NULL, &globbuf);
    glob("../*/*.c", GLOB_DOOFFS | GLOB_APPEND, NULL, &globbuf);
    for (char** path = globbuf.gl_pathv; *path; ++path) {
        printf("%s\n", *path);;
    }
    globfree(&globbuf);
    return 0;
}
```


Run: `gcc -Wall -Werror -fsanitize=address traverse_dir.c -lpthread -o traverse_dir.exe`



Run: `./traverse_dir.exe .. | head -n 5`


    fs_stat.c
    traverse_dir.c
    traverse_dir_2.c
    ../sem01/heloo.c
    ../sem01/lib.c



```python
import glob
glob.glob("../*/*.c")[:4]
```




    ['../sem10-file-attributes/stat.c',
     '../sem04-asm-arm/asm_inline_example.c',
     '../sem04-asm-arm/hello.c',
     '../sem04-asm-arm/my_lib_example.c']



## <a name="ftw"></a> Рекурсивный просмотр. Правда с помощью устаревшей функции.


```python
%%cpp traverse_dir_2.c
%run gcc -Wall -Werror -fsanitize=address traverse_dir_2.c -lpthread -o traverse_dir_2.exe
%run ./traverse_dir_2.exe ..

#include <stdio.h>
#include <ftw.h>
#include <assert.h>

int limit = 4;
    
int callback(const char* fpath, const struct stat* sb, int typeflag) {
    printf("%s %ld\n", fpath, sb->st_size);
    return (--limit == 0);
}
    
int main(int argc, char** argv) {
    assert(argc == 2);
    const char* dir_path = argv[1];
    ftw(dir_path, callback, 0);
    return 0;
}
```


Run: `gcc -Wall -Werror -fsanitize=address traverse_dir_2.c -lpthread -o traverse_dir_2.exe`



Run: `./traverse_dir_2.exe ..`


    .. 4096
    ../sem10-file-attributes 4096
    ../sem10-file-attributes/file-attrib.ipynb 51578
    ../sem10-file-attributes/stat.c 517


## <a name="fs_stat"></a> Информация о файловой системе


```python
%%cpp fs_stat.c
%run gcc -Wall -Werror -fsanitize=address fs_stat.c -lpthread -o fs_stat.exe
%run ./fs_stat.exe ..
%run ./fs_stat.exe /dev

#include <stdio.h>
#include <sys/statvfs.h>
#include <assert.h>

    
int main(int argc, char** argv) {
    assert(argc == 2);
    const char* dir_path = argv[1];
    struct statvfs stat;
    statvfs(dir_path, &stat);
    
    printf("Free 1K-blocks %lu/%lu", stat.f_bavail * stat.f_bsize / 1024, stat.f_blocks * stat.f_bsize / 1024);
    return 0;
}
```


Run: `gcc -Wall -Werror -fsanitize=address fs_stat.c -lpthread -o fs_stat.exe`



Run: `./fs_stat.exe ..`


    Free 1K-blocks 120005048/154880424


Run: `./fs_stat.exe /dev`


    Free 1K-blocks 3992316/3992316


```python
!df
```

    Filesystem     1K-blocks     Used Available Use% Mounted on
    udev             3992316        0   3992316   0% /dev
    tmpfs             805924     1584    804340   1% /run
    /dev/sdb3       28705700 22408076   4816408  83% /
    tmpfs            4029604   261500   3768104   7% /dev/shm
    tmpfs               5120        4      5116   1% /run/lock
    tmpfs            4029604        0   4029604   0% /sys/fs/cgroup
    /dev/loop1         96256    96256         0 100% /snap/core/9066
    /dev/loop0         25856    25856         0 100% /snap/heroku/3929
    /dev/loop2         96128    96128         0 100% /snap/core/8935
    /dev/loop3         25856    25856         0 100% /snap/heroku/3907
    /dev/sda2          98304    31569     66735  33% /boot/efi
    /dev/sdb4      154880424 27747388 120005032  19% /home
    tmpfs             805920       80    805840   1% /run/user/1000



```python

```


```python

```

## <a name="fusepy"></a> Python + fusepy

Установк: `pip install --user fusepy`


```python
%%writefile fuse_json.py
from __future__ import print_function

import logging
import os
import json
from errno import EIO, ENOENT, EROFS
from stat import S_IFDIR, S_IFREG
from sys import argv, exit
from time import time

from fuse import FUSE, FuseOSError, LoggingMixIn, Operations

NOW = time()

DIR_ATTRS = dict(st_mode=(S_IFDIR | 0o555), st_nlink=2)
FILE_ATTRS = dict(st_mode=(S_IFREG | 0o444), st_nlink=1)

def find_json_path(j, path):
    for part in path.split('/'):
        if len(part) > 0:
            if part == '__json__':
                return json.dumps(j)
            if part not in j:
                return None
            j = j[part]
    return j
    

class FuseOperations(LoggingMixIn, Operations):

    def __init__(self, j):
        self.j = j
        self.fd = 0

    def open(self, path, flags):
        self.fd += 1
        return self.fd

    def read(self, path, size, offset, fh):
        logging.debug("Read %r %r %r", path, size, offset)
        node = find_json_path(self.j, path)
        if not isinstance(node, str):
            raise FuseOSError(EIO)
        return node[offset:offset + size]

    def readdir(self, path, fh):
        logging.debug("Readdir %r %r", path, fh)
        node = find_json_path(self.j, path)
        if node is None:
            raise FuseOSError(EROFS)
        return ['.', '..', '__json__'] + list(node.keys())

    def getattr(self, path, fh=None):
        node = find_json_path(self.j, path)
        if isinstance(node, dict):
            return DIR_ATTRS
        elif isinstance(node, str):
            attrs = dict(FILE_ATTRS)
            attrs["st_size"] = len(node)
            return attrs
        else:
            raise FuseOSError(ENOENT)

if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    j = {
        'a': 'b',
        'c': {
            'c1': '234'
        }
    }
    FUSE(FuseOperations(j), "./fuse_json", foreground=True)
```

    Writing fuse_json.py



```python
!mkdir fuse_json 2>&1 | grep -v "File exists" || true
a = TInteractiveLauncher("python2 fuse_json.py example.txt fuse_json 2>&1")
```


    ---------------------------------------------------------------------------

    KeyboardInterrupt                         Traceback (most recent call last)

    <ipython-input-13-fa6417c65cbe> in <module>
          1 get_ipython().system('mkdir fuse_json 2>&1 | grep -v "File exists" || true')
    ----> 2 a = TInteractiveLauncher("python2 fuse_json.py example.txt fuse_json 2>&1")
    

    <ipython-input-5-471907a9d97a> in __init__(self, cmd)
        184             assert(len(exe_cands) == 1)
        185             assert(os.execvp("python3", ["python3", exe_cands[0], "-l", self.log_path, "-i", self.inq_path, "-c", cmd]) == 0)
    --> 186         self.inq_f = open(self.inq_path, "w")
        187         interactive_launcher_opened_set.add(self.pid)
        188         show_log_file(self.log_path)


    KeyboardInterrupt: 



```python
!ls fuse_json
!cat fuse_json/c/__json__
```


```python

```


```bash
%%bash
echo -n -e "\n" > new_line
exec 2>&1 ; set -o xtrace

tree fuse_json --noreport 

cat fuse_json/__json__    new_line
cat fuse_json/a           new_line
cat fuse_json/c/__json__  new_line
```

    + tree fuse_json --noreport
    fuse_json
    + cat fuse_json/__json__ new_line
    cat: fuse_json/__json__: No such file or directory
    
    + cat fuse_json/a new_line
    cat: fuse_json/a: No such file or directory
    
    + cat fuse_json/c/__json__ new_line
    cat: fuse_json/c/__json__: No such file or directory
    



    ---------------------------------------------------------------------------

    CalledProcessError                        Traceback (most recent call last)

    <ipython-input-14-8040573cd978> in <module>
    ----> 1 get_ipython().run_cell_magic('bash', '', 'echo -n -e "\\n" > new_line\nexec 2>&1 ; set -o xtrace\n\ntree fuse_json --noreport \n\ncat fuse_json/__json__    new_line\ncat fuse_json/a           new_line\ncat fuse_json/c/__json__  new_line\n')
    

    /usr/local/lib/python3.6/dist-packages/IPython/core/interactiveshell.py in run_cell_magic(self, magic_name, line, cell)
       2321             magic_arg_s = self.var_expand(line, stack_depth)
       2322             with self.builtin_trap:
    -> 2323                 result = fn(magic_arg_s, cell)
       2324             return result
       2325 


    /usr/local/lib/python3.6/dist-packages/IPython/core/magics/script.py in named_script_magic(line, cell)
        140             else:
        141                 line = script
    --> 142             return self.shebang(line, cell)
        143 
        144         # write a basic docstring:


    <decorator-gen-109> in shebang(self, line, cell)


    /usr/local/lib/python3.6/dist-packages/IPython/core/magic.py in <lambda>(f, *a, **k)
        185     # but it's overkill for just that one bit of state.
        186     def magic_deco(arg):
    --> 187         call = lambda f, *a, **k: f(*a, **k)
        188 
        189         if callable(arg):


    /usr/local/lib/python3.6/dist-packages/IPython/core/magics/script.py in shebang(self, line, cell)
        243             sys.stderr.flush()
        244         if args.raise_error and p.returncode!=0:
    --> 245             raise CalledProcessError(p.returncode, cell, output=out, stderr=err)
        246 
        247     def _run_script(self, p, cell, to_close):


    CalledProcessError: Command 'b'echo -n -e "\\n" > new_line\nexec 2>&1 ; set -o xtrace\n\ntree fuse_json --noreport \n\ncat fuse_json/__json__    new_line\ncat fuse_json/a           new_line\ncat fuse_json/c/__json__  new_line\n'' returned non-zero exit status 1.



```python
!fusermount -u fuse_json
a.close()
```

    fusermount: entry for /home/dgolear/homework/caos_2019-2020/sem26-fs-fuse/fuse_json not found in /etc/mtab



    ---------------------------------------------------------------------------

    NameError                                 Traceback (most recent call last)

    <ipython-input-12-f1ecc830f46d> in <module>
          1 get_ipython().system('fusermount -u fuse_json')
    ----> 2 a.close()
    

    NameError: name 'a' is not defined



```bash
%%bash
tree fuse_json --noreport
```

    fuse_json



```python

```

## <a name="fuse_c"></a> fuse + с

Надо поставить `libfuse-dev`. Возможно, для этого нужно подаунгрейдить `libfuse2`.

Да, обращаю внимание, что у Яковлева в ридинге используется fuse3. Но что-то его пока не очень тривиально поставить в Ubuntu 16.04 (за час не справился) и мне не хочется ненароком себе что-нибудь сломать в системе :)

fuse3 немного отличается по API. В примере я поддержал компилируемость и с fuse2, и с fuse3.

Для установки на Ubuntu может оказаться полезным [Официальный репозиторий Fuse](https://github.com/libfuse/libfuse).  
В нём указаны шаги установки. Правда, может понадобиться поставить ещё [*Ninja*](https://ninja-build.org/) и [*Meson*](https://mesonbuild.com/).


```python
cmake_minimum_required(VERSION 3.15)
project(hw23 CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=leak -g")
set(FUSE_PATH "downloads/fuse")

add_executable(hw23 1task.cpp)

target_include_directories(hw23 PUBLIC ${FUSE_PATH}/include) # -I/usr/include/fuse3
target_link_libraries(hw23 ${FUSE_PATH}/build/lib/libfuse3.so) # -lfuse3 -lpthread

```

Либо, если следовать скрипту ниже, то может помочь такой CMake


```python
cmake_minimum_required(VERSION 2.7)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FUSE REQUIRED fuse3)

include_directories(${FUSE_INCLUDE_DIRS})
add_executable(main main.c)
target_link_libraries(main ${FUSE_LIBRARIES})
```


```python
!cat fuse3.sh
```

    cd ~
    mkdir fuse && cd fuse
    sudo apt-get update -y
    # sudo apt-get upgrade -y
    # sudo apt-get full-upgrade -y
    sudo apt-get install python3-pip -y
    sudo pip3 install meson
    export PATH=$PATH:~/.local/bin
    source ~/.profile
    sudo pip3 install pytest
    sudo apt-get install ninja -y
    sudo apt install pkg-config -y
    wget https://github.com/libfuse/libfuse/releases/download/fuse-3.9.1/fuse-3.9.1.tar.xz
    tar xf fuse-3.9.1.tar.xz
    rm fuse-3.9.1.tar.xz
    cd fuse-3.9.1
    mkdir build && cd build
    meson ..
    meson configure
    meson configure -D disable-mtab=true
    ninja
    sudo python3 -m pytest test/
    sudo ninja install
    sudo chown root:root util/fusermount3
    sudo chmod 4755 util/fusermount3
    python3 -m pytest test/



```python

```

Код во многом взят отсюда: https://github.com/fntlnz/fuse-example


```python
!mkdir fuse_c_example 2>&1 | grep -v "File exists" || true
!mkdir fuse_c_example/CMake 2>&1 | grep -v "File exists" || true
```


```python
%%cmake fuse_c_example/CMake/FindFUSE.cmake
# copied from https://github.com/fntlnz/fuse-example/blob/master/CMake/FindFUSE.cmake
# Кстати, вот пример модуля CMake который умеет искать библиотеку

IF (FUSE_INCLUDE_DIR)
    SET (FUSE_FIND_QUIETLY TRUE)
ENDIF (FUSE_INCLUDE_DIR)

FIND_PATH (FUSE_INCLUDE_DIR fuse.h /usr/local/include/osxfuse /usr/local/include /usr/include)

if (APPLE)
    SET(FUSE_NAMES libosxfuse.dylib fuse)
else (APPLE)
    SET(FUSE_NAMES fuse)
endif (APPLE)
FIND_LIBRARY(FUSE_LIBRARIES NAMES ${FUSE_NAMES} PATHS /lib64 /lib /usr/lib64 /usr/lib /usr/local/lib64 /usr/local/lib /usr/lib/x86_64-linux-gnu)

include ("FindPackageHandleStandardArgs")
find_package_handle_standard_args ("FUSE" DEFAULT_MSG FUSE_INCLUDE_DIR FUSE_LIBRARIES)

mark_as_advanced (FUSE_INCLUDE_DIR FUSE_LIBRARIES)
```


```python
%%cmake fuse_c_example/CMakeLists.txt
# copied from https://github.com/fntlnz/fuse-example/blob/master/CMakeLists.txt

cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FILE_OFFSET_BITS=64 -DFUSE2 -g -fsanitize=address")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH}) # Говорим, где еще можно искать модули

find_package(FUSE REQUIRED)

include_directories(${FUSE_INCLUDE_DIR})
add_executable(fuse-example main.c)
target_link_libraries(fuse-example ${FUSE_LIBRARIES})
```

---

Чтобы пользователь мог пользоваться вашим модулем Fuse, нужно добавить основные операции для взаимодействия. Они реализуются в виде колбэков, которые Fuse будет вызывать при выполнении определённого действия пользователем.  
В C/C++ это реализуется путём заполнения структурки [fuse_operations](http://libfuse.github.io/doxygen/structfuse__operations.html).  

---


```python
%%cpp fuse_c_example/main.c
%run mkdir fuse_c_example/build 2>&1 | grep -v "File exists"
%run cd fuse_c_example/build && cmake .. > /dev/null && make
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#ifdef FUSE2
    #define FUSE_USE_VERSION 26
#else
    #define FUSE_USE_VERSION 30
#endif
#include <fuse.h>

typedef struct { 
    char* filename;
    char* filecontent;
    char* log;
} my_options_t;
my_options_t my_options;


void print_cwd() {
    if (my_options.log) {
        FILE* f = fopen(my_options.log, "at");
        char buffer[1000];
        getcwd(buffer, sizeof(buffer));
        fprintf(f, "Current working dir: %s\n", buffer);
        fclose(f);
    }
}

// Самый важный колбэк. Вызывается первым при любом другом колбэке. 
// Заполняет структуру stbuf.
int getattr_callback(const char* path, struct stat* stbuf
#ifndef FUSE2
    , struct fuse_file_info *fi
#endif
) {
#ifndef FUSE2
    (void) fi;
#endif   
    if (strcmp(path, "/") == 0) {
        // st_mode(тип файла, а также права доступа)
        // st_nlink(количество ссылок на файл)
        // Интересный факт, что количество ссылок у папки = 2 + n, где n -- количество подпапок.
        *stbuf = (struct stat) {.st_nlink = 2, .st_mode = S_IFDIR | 0755};
        return 0;
    }
    if (path[0] == '/' && strcmp(path + 1, my_options.filename) == 0) {
        *stbuf = (struct stat) {.st_nlink = 2, .st_mode = S_IFREG | 0777, .st_size = (__off_t)strlen(my_options.filecontent)};
        return 0;
    }
    return -ENOENT; // При ошибке, вместо errno возвращаем (-errno).
}

// filler(buf, filename, stat, flags) -- заполняет информацию о файле и вставляет её в buf.
int readdir_callback(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi
#ifndef FUSE2
    , enum fuse_readdir_flags flags
#endif
) {
#ifdef FUSE2
    (void) offset; (void) fi;
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    filler(buf, my_options.filename, NULL, 0);
#else
    static const enum fuse_fill_dir_flags zero_fuse_fill_dir_flags = (enum fuse_fill_dir_flags)0; // c/c++ compatibility
    (void) offset; (void) fi; (void)flags;
    filler(buf, ".", NULL, 0, zero_fuse_fill_dir_flags);
    filler(buf, "..", NULL, 0, zero_fuse_fill_dir_flags);
    filler(buf, my_options.filename, NULL, 0, zero_fuse_fill_dir_flags);
#endif   
    return 0;
}

// Вызывается после успешной обработки open.
int read_callback(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
    if (strcmp(path, "/") == 0) {
        return -EISDIR;
    }
    print_cwd();
    // "/", "/my_file"
    size_t len = strlen(my_options.filecontent);
    if (offset >= len) {
        return 0;
    }
    size = (offset + size <= len) ? size : (len - offset);
    memcpy(buf, my_options.filecontent + offset, size);
    return size;
}

// Структура с колбэками. 
struct fuse_operations fuse_example_operations = {
    .getattr = getattr_callback,
    .read = read_callback,
    .readdir = readdir_callback,
};

// Аргументы, которые мы хотим, чтобы Fuse распарсил для нас.
// Как и всё в C, массив должен заканчиваться нулём -- {NULL, 0, 0}.
struct fuse_opt opt_specs[] = {
    { "--file-name %s", offsetof(my_options_t, filename), 0 },
    { "--file-content %s", offsetof(my_options_t, filecontent), 0 },
    { "--log %s", offsetof(my_options_t, log), 0 },
    FUSE_OPT_END
};

int main(int argc, char** argv) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    
    /*
    * Если не хотите создавать структурку с данными, а нужно только распарсить одну строку,
    * То можно вторым аргументом передать char*.
    * Тогда в opt_specs это можно указать как {"--src %s", 0, 0}
    */
    fuse_opt_parse(&args, &my_options, opt_specs, NULL);
    print_cwd();
    
    int ret = fuse_main(args.argc, args.argv, &fuse_example_operations, NULL);
    fuse_opt_free_args(&args);
    return ret;
}
```


Run: `mkdir fuse_c_example/build 2>&1 | grep -v "File exists"`



Run: `cd fuse_c_example/build && cmake .. > /dev/null && make`


    CMake Error at /usr/share/cmake-3.10/Modules/FindPackageHandleStandardArgs.cmake:137 (message):
      Could NOT find FUSE (missing: FUSE_INCLUDE_DIR FUSE_LIBRARIES)
    Call Stack (most recent call first):
      /usr/share/cmake-3.10/Modules/FindPackageHandleStandardArgs.cmake:378 (_FPHSA_FAILURE_MESSAGE)
      CMake/FindFUSE.cmake:19 (find_package_handle_standard_args)
      CMakeLists.txt:10 (find_package)
    
    


Запустим в синхронном режиме (программа работает, пока `fusermount -u` не будет сделан)


```python
!mkdir fuse_c || true
!truncate --size=0 err.txt || true
a = TInteractiveLauncher("fuse_c_example/build/fuse-example fuse_c -f "
                         "--file-name my_file --file-content 'My file content\n' --log `pwd`/err.txt")
```


    ---------------------------------------------------------------------------

    KeyboardInterrupt                         Traceback (most recent call last)

    <ipython-input-18-4362d234e4cc> in <module>
          1 get_ipython().system('mkdir fuse_c || true')
          2 get_ipython().system('truncate --size=0 err.txt || true')
    ----> 3 a = TInteractiveLauncher("fuse_c_example/build/fuse-example fuse_c -f "
          4                          "--file-name my_file --file-content 'My file content\n' --log `pwd`/err.txt")


    <ipython-input-1-471907a9d97a> in __init__(self, cmd)
        184             assert(len(exe_cands) == 1)
        185             assert(os.execvp("python3", ["python3", exe_cands[0], "-l", self.log_path, "-i", self.inq_path, "-c", cmd]) == 0)
    --> 186         self.inq_f = open(self.inq_path, "w")
        187         interactive_launcher_opened_set.add(self.pid)
        188         show_log_file(self.log_path)


    KeyboardInterrupt: 



```bash
%%bash
exec 2>&1 ; set -o xtrace

tree fuse_c --noreport 

cat fuse_c/my_file
```

    + tree fuse_c --noreport
    fuse_c
    └── my_file
    + cat fuse_c/my_file
    My file content



```python
!fusermount -u fuse_c
a.close()
```


```bash
%%bash
tree fuse_c --noreport
cat err.txt
```

    fuse_c
    Current working dir: /home/pechatnov/vbox/caos_2019-2020/sem26-fs-fuse
    Current working dir: /home/pechatnov/vbox/caos_2019-2020/sem26-fs-fuse


А теперь в асинхронном (в режиме демона, в параметрах запуска нет `-f`):


```python
!mkdir fuse_c || true
!truncate --size=0 err.txt || true
a = TInteractiveLauncher("fuse_c_example/build/fuse-example fuse_c "
                         "--file-name my_file --file-content 'My file content\n' --log `pwd`/err.txt")
```

    mkdir: cannot create directory ‘fuse_c’: File exists




        <!--MD_BEGIN_FILTER-->
        <script type=text/javascript>
        var entrance___interactive_launcher_tmp_213277631725109524_log_obj = 0;
        var errors___interactive_launcher_tmp_213277631725109524_log_obj = 0;
        function halt__interactive_launcher_tmp_213277631725109524_log_obj(elem, color)
        {
            elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    
        }
        function refresh__interactive_launcher_tmp_213277631725109524_log_obj()
        {
            entrance___interactive_launcher_tmp_213277631725109524_log_obj -= 1;
            if (entrance___interactive_launcher_tmp_213277631725109524_log_obj < 0) {
                entrance___interactive_launcher_tmp_213277631725109524_log_obj = 0;
            }
            var elem = document.getElementById("__interactive_launcher_tmp_213277631725109524_log_obj");
            if (elem) {
                var xmlhttp=new XMLHttpRequest();
                xmlhttp.onreadystatechange=function()
                {
                    var elem = document.getElementById("__interactive_launcher_tmp_213277631725109524_log_obj");
                    console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___interactive_launcher_tmp_213277631725109524_log_obj);
                    if (elem && xmlhttp.readyState==4) {
                        if (xmlhttp.status==200)
                        {
                            errors___interactive_launcher_tmp_213277631725109524_log_obj = 0;
                            if (!entrance___interactive_launcher_tmp_213277631725109524_log_obj) {
                                if (elem.innerHTML != xmlhttp.responseText) {
                                    elem.innerHTML = xmlhttp.responseText;
                                }
                                if (elem.innerHTML.includes("Process finished.")) {
                                    halt__interactive_launcher_tmp_213277631725109524_log_obj(elem, "#333333");
                                } else {
                                    entrance___interactive_launcher_tmp_213277631725109524_log_obj += 1;
                                    console.log("req");
                                    window.setTimeout("refresh__interactive_launcher_tmp_213277631725109524_log_obj()", 300); 
                                }
                            }
                            return xmlhttp.responseText;
                        } else {
                            errors___interactive_launcher_tmp_213277631725109524_log_obj += 1;
                            if (!entrance___interactive_launcher_tmp_213277631725109524_log_obj) {
                                if (errors___interactive_launcher_tmp_213277631725109524_log_obj < 6) {
                                    entrance___interactive_launcher_tmp_213277631725109524_log_obj += 1;
                                    console.log("req");
                                    window.setTimeout("refresh__interactive_launcher_tmp_213277631725109524_log_obj()", 300); 
                                } else {
                                    halt__interactive_launcher_tmp_213277631725109524_log_obj(elem, "#994444");
                                }
                            }
                        }
                    }
                }
                xmlhttp.open("GET", "./interactive_launcher_tmp/213277631725109524.log", true);
                xmlhttp.setRequestHeader("Cache-Control", "no-cache");
                xmlhttp.send();     
            }
        }
        
        if (!entrance___interactive_launcher_tmp_213277631725109524_log_obj) {
            entrance___interactive_launcher_tmp_213277631725109524_log_obj += 1;
            refresh__interactive_launcher_tmp_213277631725109524_log_obj(); 
        }
        </script>

        <p id="__interactive_launcher_tmp_213277631725109524_log_obj" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">
        </p>
        
        </font>
        <!--MD_END_FILTER-->
        <!--MD_FROM_FILE ./interactive_launcher_tmp/213277631725109524.log.md -->
        



```bash
%%bash
exec 2>&1 ; set -o xtrace

tree fuse_c --noreport 

cat fuse_c/my_file

fusermount -u fuse_c
```

    + tree fuse_c --noreport
    fuse_c
    └── my_file
    + cat fuse_c/my_file
    My file content
    + fusermount -u fuse_c



```python
a.close()
```


```bash
%%bash
tree fuse_c --noreport
cat err.txt
```

    fuse_c
    Current working dir: /home/pechatnov/vbox/caos_2019-2020/sem26-fs-fuse
    Current working dir: /


Парам-пам-пам, изменилась текущая директория! Учиытвайте это в ДЗ


```python

```


```python

```

# <a name="hw"></a> Комментарии к ДЗ

* Пример входных данных в первой задаче: 

```
2
a.txt 3
b.txt 5

AaAbBbBb
```

* В ejudge fuse запускается без опции `-f` поэтому текущая директория будет меняться и относительные пути могут становиться невалидными. Рекомендую: `man 3 realpath`

1) В задачах на fuse основная цель -- реализовать 3 метода(read, readdir, getattr).  
Для этого может понадобиться сохранить свои данные в какую-то глобальную переменную и доставать их оттуда в вызовах колбэка.  

2) В 23-1 Чтобы не усложнять себе жизнь, можно ходить по папкам при каждом вызове.  
Тогда задача сводится к поиску конкретного файла в каждой папке из условия и выборе из этих файлов последнего.  
Либо, в случае readdir, можно вызвать opendir/readdir/closedir к каждому пути и сформировать словарик из уникальных файлов в папках.
