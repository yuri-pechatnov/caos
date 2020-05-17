```python
# look at tools/set_up_magics.ipynb
yandex_metrica_allowed = True ; get_ipython().run_cell('# one_liner_str\n\nget_ipython().run_cell_magic(\'javascript\', \'\', \n    \'// setup cpp code highlighting\\n\'\n    \'IPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\\\'reg\\\':[/^%%cpp/]} ;\'\n    \'IPython.CodeCell.options_default.highlight_modes["text/x-cmake"] = {\\\'reg\\\':[/^%%cmake/]} ;\'\n)\n\n# creating magics\nfrom IPython.core.magic import register_cell_magic, register_line_magic\nfrom IPython.display import display, Markdown, HTML\nimport argparse\nfrom subprocess import Popen, PIPE\nimport random\nimport sys\nimport os\nimport re\nimport signal\nimport shutil\nimport shlex\nimport glob\nimport time\n\n@register_cell_magic\ndef save_file(args_str, cell, line_comment_start="#"):\n    parser = argparse.ArgumentParser()\n    parser.add_argument("fname")\n    parser.add_argument("--ejudge-style", action="store_true")\n    args = parser.parse_args(args_str.split())\n    \n    cell = cell if cell[-1] == \'\\n\' or args.no_eof_newline else cell + "\\n"\n    cmds = []\n    with open(args.fname, "w") as f:\n        f.write(line_comment_start + " %%cpp " + args_str + "\\n")\n        for line in cell.split("\\n"):\n            line_to_write = (line if not args.ejudge_style else line.rstrip()) + "\\n"\n            if line.startswith("%"):\n                run_prefix = "%run "\n                if line.startswith(run_prefix):\n                    cmds.append(line[len(run_prefix):].strip())\n                    f.write(line_comment_start + " " + line_to_write)\n                    continue\n                if line.startswith("%" + line_comment_start + " "):\n                    f.write(line_comment_start + " " + line_to_write)\n                    continue\n                raise Exception("Unknown %%save_file subcommand: \'%s\'" % line)\n            else:\n                f.write(line_to_write)\n        f.write("" if not args.ejudge_style else line_comment_start + r" line without \\n")\n    for cmd in cmds:\n        display(Markdown("Run: `%s`" % cmd))\n        get_ipython().system(cmd)\n\n@register_cell_magic\ndef cpp(fname, cell):\n    save_file(fname, cell, "//")\n    \n@register_cell_magic\ndef cmake(fname, cell):\n    save_file(fname, cell, "#")\n\n@register_cell_magic\ndef asm(fname, cell):\n    save_file(fname, cell, "//")\n    \n@register_cell_magic\ndef makefile(fname, cell):\n    assert not fname\n    save_file("makefile", cell.replace(" " * 4, "\\t"))\n        \n@register_line_magic\ndef p(line):\n    line = line.strip() \n    if line[0] == \'#\':\n        display(Markdown(line[1:].strip()))\n    else:\n        try:\n            expr, comment = line.split(" #")\n            display(Markdown("`{} = {}`  # {}".format(expr.strip(), eval(expr), comment.strip())))\n        except:\n            display(Markdown("{} = {}".format(line, eval(line))))\n    \n    \ndef show_log_file(file, return_html_string=False):\n    obj = file.replace(\'.\', \'_\').replace(\'/\', \'_\') + "_obj"\n    html_string = \'\'\'\n        <!--MD_BEGIN_FILTER-->\n        <script type=text/javascript>\n        var entrance___OBJ__ = 0;\n        var errors___OBJ__ = 0;\n        function halt__OBJ__(elem, color)\n        {\n            elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    \n        }\n        function refresh__OBJ__()\n        {\n            entrance___OBJ__ -= 1;\n            if (entrance___OBJ__ < 0) {\n                entrance___OBJ__ = 0;\n            }\n            var elem = document.getElementById("__OBJ__");\n            if (elem) {\n                var xmlhttp=new XMLHttpRequest();\n                xmlhttp.onreadystatechange=function()\n                {\n                    var elem = document.getElementById("__OBJ__");\n                    console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___OBJ__);\n                    if (elem && xmlhttp.readyState==4) {\n                        if (xmlhttp.status==200)\n                        {\n                            errors___OBJ__ = 0;\n                            if (!entrance___OBJ__) {\n                                if (elem.innerHTML != xmlhttp.responseText) {\n                                    elem.innerHTML = xmlhttp.responseText;\n                                }\n                                if (elem.innerHTML.includes("Process finished.")) {\n                                    halt__OBJ__(elem, "#333333");\n                                } else {\n                                    entrance___OBJ__ += 1;\n                                    console.log("req");\n                                    window.setTimeout("refresh__OBJ__()", 300); \n                                }\n                            }\n                            return xmlhttp.responseText;\n                        } else {\n                            errors___OBJ__ += 1;\n                            if (!entrance___OBJ__) {\n                                if (errors___OBJ__ < 6) {\n                                    entrance___OBJ__ += 1;\n                                    console.log("req");\n                                    window.setTimeout("refresh__OBJ__()", 300); \n                                } else {\n                                    halt__OBJ__(elem, "#994444");\n                                }\n                            }\n                        }\n                    }\n                }\n                xmlhttp.open("GET", "__FILE__", true);\n                xmlhttp.setRequestHeader("Cache-Control", "no-cache");\n                xmlhttp.send();     \n            }\n        }\n        \n        if (!entrance___OBJ__) {\n            entrance___OBJ__ += 1;\n            refresh__OBJ__(); \n        }\n        </script>\n\n        <p id="__OBJ__" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">\n        </p>\n        \n        </font>\n        <!--MD_END_FILTER-->\n        <!--MD_FROM_FILE __FILE__.md -->\n        \'\'\'.replace("__OBJ__", obj).replace("__FILE__", file)\n    if return_html_string:\n        return html_string\n    display(HTML(html_string))\n\n    \nclass TInteractiveLauncher:\n    tmp_path = "./interactive_launcher_tmp"\n    def __init__(self, cmd):\n        try:\n            os.mkdir(TInteractiveLauncher.tmp_path)\n        except:\n            pass\n        name = str(random.randint(0, 1e18))\n        self.inq_path = os.path.join(TInteractiveLauncher.tmp_path, name + ".inq")\n        self.log_path = os.path.join(TInteractiveLauncher.tmp_path, name + ".log")\n        \n        os.mkfifo(self.inq_path)\n        open(self.log_path, \'w\').close()\n        open(self.log_path + ".md", \'w\').close()\n\n        self.pid = os.fork()\n        if self.pid == -1:\n            print("Error")\n        if self.pid == 0:\n            exe_cands = glob.glob("../tools/launcher.py") + glob.glob("../../tools/launcher.py")\n            assert(len(exe_cands) == 1)\n            assert(os.execvp("python3", ["python3", exe_cands[0], "-l", self.log_path, "-i", self.inq_path, "-c", cmd]) == 0)\n        self.inq_f = open(self.inq_path, "w")\n        interactive_launcher_opened_set.add(self.pid)\n        show_log_file(self.log_path)\n\n    def write(self, s):\n        s = s.encode()\n        assert len(s) == os.write(self.inq_f.fileno(), s)\n        \n    def get_pid(self):\n        n = 100\n        for i in range(n):\n            try:\n                return int(re.findall(r"PID = (\\d+)", open(self.log_path).readline())[0])\n            except:\n                if i + 1 == n:\n                    raise\n                time.sleep(0.1)\n        \n    def input_queue_path(self):\n        return self.inq_path\n        \n    def wait_stop(self, timeout):\n        for i in range(int(timeout * 10)):\n            wpid, status = os.waitpid(self.pid, os.WNOHANG)\n            if wpid != 0:\n                return True\n            time.sleep(0.1)\n        return False\n        \n    def close(self, timeout=3):\n        self.inq_f.close()\n        if not self.wait_stop(timeout):\n            os.kill(self.get_pid(), signal.SIGKILL)\n            os.waitpid(self.pid, 0)\n        os.remove(self.inq_path)\n        # os.remove(self.log_path)\n        self.inq_path = None\n        self.log_path = None \n        interactive_launcher_opened_set.remove(self.pid)\n        self.pid = None\n        \n    @staticmethod\n    def terminate_all():\n        if "interactive_launcher_opened_set" not in globals():\n            globals()["interactive_launcher_opened_set"] = set()\n        global interactive_launcher_opened_set\n        for pid in interactive_launcher_opened_set:\n            print("Terminate pid=" + str(pid), file=sys.stderr)\n            os.kill(pid, signal.SIGKILL)\n            os.waitpid(pid, 0)\n        interactive_launcher_opened_set = set()\n        if os.path.exists(TInteractiveLauncher.tmp_path):\n            shutil.rmtree(TInteractiveLauncher.tmp_path)\n    \nTInteractiveLauncher.terminate_all()\n   \nyandex_metrica_allowed = bool(globals().get("yandex_metrica_allowed", False))\nif yandex_metrica_allowed:\n    display(HTML(\'\'\'<!-- YANDEX_METRICA_BEGIN -->\n    <script type="text/javascript" >\n       (function(m,e,t,r,i,k,a){m[i]=m[i]||function(){(m[i].a=m[i].a||[]).push(arguments)};\n       m[i].l=1*new Date();k=e.createElement(t),a=e.getElementsByTagName(t)[0],k.async=1,k.src=r,a.parentNode.insertBefore(k,a)})\n       (window, document, "script", "https://mc.yandex.ru/metrika/tag.js", "ym");\n\n       ym(59260609, "init", {\n            clickmap:true,\n            trackLinks:true,\n            accurateTrackBounce:true\n       });\n    </script>\n    <noscript><div><img src="https://mc.yandex.ru/watch/59260609" style="position:absolute; left:-9999px;" alt="" /></div></noscript>\n    <!-- YANDEX_METRICA_END -->\'\'\'))\n\ndef make_oneliner():\n    html_text = \'("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований. Если вы не хотите, чтобы по вам собиралась статистика, исправьте: yandex_metrica_allowed = False" if yandex_metrica_allowed else "")\'\n    html_text += \' + "<""!-- MAGICS_SETUP_PRINTING_END -->"\'\n    return \'\'.join([\n        \'# look at tools/set_up_magics.ipynb\\n\',\n        \'yandex_metrica_allowed = True ; get_ipython().run_cell(%s);\' % repr(one_liner_str),\n        \'display(HTML(%s))\' % html_text,\n        \' #\'\'MAGICS_SETUP_END\'\n    ])\n       \n\n');display(HTML(("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований. Если вы не хотите, чтобы по вам собиралась статистика, исправьте: yandex_metrica_allowed = False" if yandex_metrica_allowed else "") + "<""!-- MAGICS_SETUP_PRINTING_END -->")) #MAGICS_SETUP_END
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



# Работа со временем в С/С++

Поговорим о типах времени в C/C++ и функциях для получения текущего времени, парсинга из строк, сериализации в строки.

Меня всегда дико напрягало отсутствие одного хорошего типа времени, наличие времени в разных часовых поясах и куча разных типов сериализации. Постараюсь собрать полезную информацию в одном месте, чтобы жилось проще.

<table width=100%  > <tr>
    <th width=15%> <b>Видео с семинара &rarr; </b> </th>
    <th>
    <a href="???"><img src="video.jpg" width="320" 
   height="160" align="left" alt="Видео с семинара"></a>
    </th>
    <th> </th>
</tr> </table>



Сегодня в программе:
* <a href="types_c" style="color:#856024"> Типы времени в C </a>
* <a href="funcs_c" style="color:#856024"> Функции для работы со временем в C </a>
* <a href="types_cpp" style="color:#856024"> Типы времени в C++ </a>
* <a href="funcs_cpp" style="color:#856024"> Функции для работы со временем в C++ </a>
<br><br>
* <a href="clocks_and_cpu" style="color:#856024"> Разные часы и процессорное время </a>
* <a href="benchmarking" style="color:#856024"> Время для бенчмарков </a>
<br><br>
* <a href="sleep" style="color:#856024"> Как поспать? </a>
<br><br>
* <a href="problems" style="color:#856024"> Задачки для самостоятельного решения </a>

 


## <a name="types_c"></a> Типы времени в C

Что у нас есть?

Собственно типы времени
* `time_t` - целочисленный тип, в котором хранится количество секунд с начала эпохи. В общем таймстемп в секундах. [man](https://www.opennet.ru/man.shtml?topic=time&category=2)
* `struct tm` - структурка в которой хранится год, месяц, ..., секунда [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=ctime&category=3)
* `struct timeval` пара (секунды, миллисекунды) (с начала эпохи, если используется как момент времени) [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=gettimeofday&category=2)
* `struct timespec` пара (секунды, наносекунды) [man](https://www.opennet.ru/man.shtml?topic=select&category=2&russian=)
* `struct timeb` - секунды, миллисекунды, таймзона+информация о летнем времени [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=ftime&category=3) (Я ни разу не сталкивался, но и такая есть)

Часовой пояс
* `struct timezone` - [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=gettimeofday&category=2)


## <a name="funcs_c"></a> Функции для работы с временем в C

До всего последующего хочется напомнить, что многие функции в C не потокобезопасны (если не заканчиваются на `_r`, что означает reentrant, ну и потокобезопасность). Поэтому, перед использованием, стоит посмотреть документацию.

Конвертация:
<table>
<tr>
  <th>Из чего\Во что</th>
  <th>time_t</th>
  <th>struct tm</th>
  <th>struct timeval</th>
  <th>struct timespec</th>
</tr> 
<tr> <td>time_t</td>
  <td>=</td>
  <td><a href="https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=ctime&category=3"><code>gmtime_r</code></a>/<a href="https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=ctime&category=3"><code>localtime_r</code></a></td>
  <td>{.tv_sec = x}</td>
  <td>{.tv_sec = x}</td>
</tr>
<tr> <td>struct tm</td>
  <td><a href="https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=ctime&category=3"><code>mktime</code></a> [1]</td>
  <td>=</td>
  <td>через time_t</td>
  <td>через time_t</td>
</tr>
<tr> <td>struct timeval</td>
  <td>x.tv_sec</td>
  <td>через time_t</td>
  <td>=</td>
  <td>{.tv_sec = x.tv_sec, .tv_nsec = x.tv_usec * 1000}</td>
</tr>
<tr> <td>struct timespec</td>
  <td>x.tv_sec</td>
  <td>через time_t</td>
  <td>{.tv_sec = x.tv_sec, .tv_usec = x.tv_nsec / 1000}</td>
  <td>=</td>
</tr>
</table>

[1] - `mktime` неадекватно работает, когда у вас не локальное время. Подробности и как с этим жить - в примерах. https://stackoverflow.com/questions/530519/stdmktime-and-timezone-info

Получение:
* `time` - получить время как `time_t` [man](https://www.opennet.ru/man.shtml?topic=time&category=2)
* `clock_gettime` - получить время как `struct timespec` [man](https://www.opennet.ru/man.shtml?topic=clock_gettime&category=3&russian=2)
* `gettimeofday` - получить время как `struct timeval` [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=settimeofday&category=2)

Парсинг:
* Если таймстемп - то просто читаем как число.
* `strptime` [man](https://www.opennet.ru/man.shtml?topic=strptime&category=3&russian=0) Не умеет во временные зоны, всегда локальную выставляет
* `getdate` [man](https://opennet.ru/man.shtml?topic=getdate&category=3) Не рекомендую, не очень умная функция.

Сериализация:
* Всегда можно просто записать таймстемп в секундах/миллисекундах.
* `strftime` - позволяет превратить struct tm в строку, используя printf-подобную форматную строку [man](https://www.opennet.ru/man.shtml?topic=strftime&category=3)

Арифметические операции:
* Их нет, все вручную?

Работа с часовыми поясами:
  Прежде всего замечание: в рамках этого семинара считаем, что время в GMT = время в UTC.

* Сериализация таймстемпа как локального или UTC времени - `localtime_t`/`gmtime_r`.
* Парсинг локального времени - `strptime`.
* Другие часовые пояса и парсинг human-readable строк c заданным часовым поясом только через установку локалей, переменных окружения. В общем избегайте этого


```python
# В питоне примерно то же самое, что и в С
import time
print("* Таймстемп (time_t): ", time.time())
print("* Дата (struct tm): ", time.localtime(time.time()))
print("* Дата (struct tm): ", time.gmtime(time.time()), "(обращаем внимание на разницу в часовых поясах)")
print("* tm_gmtoff для local:", time.localtime(time.time()).tm_gmtoff, 
      "и для gm: ", time.gmtime(time.time()).tm_gmtoff, "(скрытое поле, но оно используется :) )")
print("* Дата human-readable (local): ", time.strftime("%Y.%m.%d %H:%M:%S %z", time.localtime(time.time())))
print("* Дата human-readable (gmt): ", time.strftime("%Y.%m.%d %H:%M:%S %z", time.gmtime(time.time())))
```

    * Таймстемп (time_t):  1589635178.5520053
    * Дата (struct tm):  time.struct_time(tm_year=2020, tm_mon=5, tm_mday=16, tm_hour=16, tm_min=19, tm_sec=38, tm_wday=5, tm_yday=137, tm_isdst=0)
    * Дата (struct tm):  time.struct_time(tm_year=2020, tm_mon=5, tm_mday=16, tm_hour=13, tm_min=19, tm_sec=38, tm_wday=5, tm_yday=137, tm_isdst=0) (обращаем внимание на разницу в часовых поясах)
    * tm_gmtoff для local: 10800 и для gm:  0 (скрытое поле, но оно используется :) )
    * Дата human-readable (local):  2020.05.16 16:19:38 +0300
    * Дата human-readable (gmt):  2020.05.16 13:19:38 +0000



```python
%%cpp time.c
%run gcc -fsanitize=address time.c -lpthread -o time_c.exe
%run ./time_c.exe

#define _BSD_SOURCE
#define _GNU_SOURCE  // для strptime

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <assert.h>
#include <string.h>

// Я не уверен, что так делать норм
time_t as_utc_timestamp(struct tm timeTm) {
    time_t timestamp = mktime(&timeTm); // mktime распарсит как локальное время, даже если tm_gmtoff в 0 сбросить
    //               ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ Извращение, чтобы получить нормальный таймстемп UTC
    return timestamp + timeTm.tm_gmtoff; // mktime выставит tm_gmtoff в соответствии с текущей таймзоной
}

int main() {
    { // (1)
        struct timespec spec = {0}; 
        clock_gettime(CLOCK_REALTIME, &spec);
        
        time_t timestamp = spec.tv_sec;
        struct tm local_tm = {0};
        localtime_r(&timestamp, &local_tm);
        
        char time_str[100]; 
        size_t time_len = strftime(time_str, sizeof(time_str), "%Y.%m.%d %H:%M:%S", &local_tm);
        time_len += snprintf(time_str + time_len, sizeof(time_str) - time_len, ".%09ld", spec.tv_nsec);
        time_len += strftime(time_str + time_len, sizeof(time_str) - time_len, " %Z", &local_tm);
        printf("(1) Current time: %s\n", time_str);
    }
    
    { // (2)
        const char* utc_time = "2020.08.15 12:48:06";
        
        struct tm local_tm = {0};
        strptime(utc_time, "%Y.%m.%d %H:%M:%S", &local_tm); // распарсит как локальное время
        
        time_t timestamp = as_utc_timestamp(local_tm); 
        localtime_r(&timestamp, &local_tm);
        
        char time_str[100]; 
        size_t time_len = strftime(time_str, sizeof(time_str), "%Y.%m.%d %H:%M:%S%z", &local_tm);
        printf("(2) Recovered time by strptime: %s (given utc time: %s)\n", time_str, utc_time);
    }
    
    { // (3)
        time_t timestamps[] = {1589227667, 840124800, -1};
        for (time_t* timestamp = timestamps; *timestamp != -1; ++timestamp) {
            struct tm local_time = {0};
            localtime_r(timestamp, &local_time);
            char time_str[100]; 
            size_t time_len = strftime(time_str, sizeof(time_str), "%Y.%m.%d %H:%M:%S", &local_time);
            printf("(3) Timestamp %ld -> %s\n", *timestamp, time_str);
        }
    }

    return 0;
}
```


Run: `gcc -fsanitize=address time.c -lpthread -o time_c.exe`


    In file included from [01m[K/usr/include/x86_64-linux-gnu/bits/libc-header-start.h:33[m[K,
                     from [01m[K/usr/include/stdio.h:27[m[K,
                     from [01m[Ktime.c:8[m[K:
    [01m[K/usr/include/features.h:187:3:[m[K [01;35m[Kwarning: [m[K#warning "_BSD_SOURCE and _SVID_SOURCE are deprecated, use _DEFAULT_SOURCE" [[01;35m[K-Wcpp[m[K]
      187 | # [01;35m[Kwarning[m[K "_BSD_SOURCE and _SVID_SOURCE are deprecated, use _DEFAULT_SOURCE"
          |   [01;35m[K^~~~~~~[m[K



Run: `./time_c.exe`


    (1) Current time: 2020.05.16 16:19:41.907912649 MSK
    (2) Recovered time by strptime: 2020.08.15 15:48:06+0300 (given utc time: 2020.08.15 12:48:06)
    (3) Timestamp 1589227667 -> 2020.05.11 23:07:47
    (3) Timestamp 840124800 -> 1996.08.15 20:00:00



```python

```

## <a name="types_cpp"></a> Типы времени в C++

Для начала нам доступно все то же, что было в С.

Новые типы времени
* `std::tm = struct tm`, `std::time_t = struct tm` - типы старые, но способ написания новый :)
* `std::chrono::time_point` [doc](https://en.cppreference.com/w/cpp/chrono/time_point)
* `std::chrono::duration` [doc](https://en.cppreference.com/w/cpp/chrono/duration)


Скажу откровенно, добавились не самые удобные типы. Единственное, что сделано удобно - арифметика времени.

## <a name="funcs_cpp"></a> Функции для работы с временем в C++


Конвертация:
* `std::chrono::system_clock::to_time_t`, `std::chrono::system_clock::from_time_t`

Сериализация и парсинг:
* `std::get_time` / `std::put_time` - примерно то же самое, что `strftime` и `strptime` в C. Работают с `std::tm`. [doc](https://en.cppreference.com/w/cpp/io/manip/get_time)

Арифметические операции:
* Из коробки, обычными +/*



```python
%%cpp time.cpp
%run clang++ -std=c++14 -fsanitize=address time.cpp -lpthread -o time_cpp.exe
%run ./time_cpp.exe

#include <iostream>
#include <sstream>
#include <locale>
#include <iomanip>
#include <chrono>
#include <time.h> // localtime_r

time_t as_utc_timestamp(struct tm t) {
    time_t timestamp = mktime(&t); // mktime распарсит как локальное время, даже если tm_gmtoff в 0 сбросить
    //               ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ Извращение, чтобы получить нормальный таймстемп UTC
    return timestamp + t.tm_gmtoff; // mktime выставит tm_gmtoff в соответствии с текущей таймзоной
}

int main() {
    { // (0)
        using namespace std::literals;
        auto nowChrono = std::chrono::system_clock::now();
        std::time_t timestamp = std::chrono::system_clock::to_time_t(nowChrono);
        std::tm timeTm = {};
        timestamp = 1589401219;
        localtime_r(&timestamp, &timeTm); 
        uint64_t nowMs = (nowChrono.time_since_epoch() % 1s) / 1ms;
        std::cout << "(0) Current time: " 
                  << std::put_time(&timeTm, "%Y.%m.%d %H:%M:%S") 
                  << "." << std::setfill('0') << std::setw(3) << nowMs << " "
                  << std::put_time(&timeTm, "%z") << " "
                  << ", timestamp = " << timestamp << "'\n";
    }

    { // (1)
        std::string timeStr = "2011-Jan-18 23:12:34";
        
        std::tm timeTm = {};
        
        std::istringstream timeStrStream{timeStr};
        timeStrStream.imbue(std::locale("en_US.utf-8"));
        timeStrStream >> std::get_time(&timeTm, "%Y-%b-%d %H:%M:%S");
        
        if (timeStrStream.fail()) {
            std::cout << "(1) Parse failed\n";
        } else {
            std::cout << "(1) Parsed time '" << std::put_time(&timeTm, "%Y.%m.%d %H:%M:%S %z") << "'"
                      << " from '" << timeStr << "''\n";
        }
    }
    
    { // (2)
        using namespace std::literals;
        auto nowChrono = std::chrono::system_clock::now();
        for (int i = 0; i < 2; ++i, nowChrono += 23h + 55min) {
            std::time_t nowTimestamp = std::chrono::system_clock::to_time_t(nowChrono);
            std::tm localTm = {};
            localtime_r(&nowTimestamp, &localTm); // кажись в C++ нет потокобезопасной функции
            std::cout << "(2) Composed time: " << std::put_time(&localTm, "%Y.%m.%d %H:%M:%S %z") << "\n";
        }
    }
    
    { // (3)
        using namespace std::literals;
        
        std::string timeStr = "1977.01.11 22:35:22";
        
        std::tm timeTm = {};
        std::istringstream timeStrStream{timeStr};
        timeStrStream >> std::get_time(&timeTm, "%Y.%m.%d %H:%M:%S"); // read as UTC/GMT time
        
        std::cout << "(3) Original time: " << std::put_time(&timeTm, "%Y.%m.%d %H:%M:%S %z") << "\n";
        if (timeStrStream.fail()) {
            std::cout << "(3) Parse failed\n";
        } else {
            std::time_t timestamp = as_utc_timestamp(timeTm);
            auto instantChrono = std::chrono::system_clock::from_time_t(timestamp);
            instantChrono += 23h + 55min;
            std::time_t anotherTimestamp = std::chrono::system_clock::to_time_t(instantChrono);
            std::tm localTm = {};
            gmtime_r(&timestamp, &localTm); // вот эта фигня проинтерпретировала время как локальное
            std::tm anotherLocalTm = {};
            gmtime_r(&anotherTimestamp, &anotherLocalTm); 
            
            std::cout << "(3) Take '" 
                      << std::put_time(&localTm, "%Y.%m.%d %H:%M:%S %z") << "', add 23:55, and get '"
                      << std::put_time(&anotherLocalTm, "%Y.%m.%d %H:%M:%S %z") << "'\n";
        }
    }

    return 0;
}
```


Run: `clang++ -std=c++14 -fsanitize=address time.cpp -lpthread -o time_cpp.exe`



Run: `./time_cpp.exe`


    (0) Current time: 2020.05.13 23:20:19.592 +0300 , timestamp = 1589401219'
    (1) Parsed time '2011.01.18 23:12:34 +0000' from '2011-Jan-18 23:12:34''
    (2) Composed time: 2020.05.16 16:22:41 +0300
    (2) Composed time: 2020.05.17 16:17:41 +0300
    (3) Original time: 1977.01.11 22:35:22 +0000
    (3) Take '1977.01.11 22:35:22 +0000', add 23:55, and get '1977.01.12 22:30:22 +0000'


Стоит обратить внимание, что в С++ не навязывается местный часовой пояс при парсинге времени. Хорошо это или плохо - не знаю.





## <a name="clocks_and_cpu"></a> Разные часы и процессорное время

[Проблема 2038 года](https://ru.wikipedia.org/wiki/Проблема_2038_года), связанная с переполнением 32-битного time_t. Просто обозначаю, что она есть.

[iana](https://www.iana.org/time-zones) - база данных временных зон.

Хардверные часы. Обычные кварцевые часы, для которых на материнской плате есть отдельная батарейка. Они не очень точные. А еще разные системы могут хранить там время по-разному. Поэтому при перезагрузках между ubuntu и windows время может прыгать на 3 часа (если выбрано Московское время).
```
  -> sudo hwclock
Пт 24 апр 2020 00:28:52  .356966 seconds
  -> date
Пн май  4 14:28:24 MSK 2020
```

Процессорное время:
* [C/C++: как измерять процессорное время / Хабр](https://habr.com/ru/post/282301/)
* `clock_t clock(void);` - время затраченное процессором на исполнение потока/программы. Измеряется в непонятных единицах, связанных с секундами через CLOCKS_PER_SEC. [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=clock&category=3)
* `clock_gettime` c параметрами `CLOCK_PROCESS_CPUTIME_ID`, `CLOCK_THREAD_CPUTIME_ID` - процессорное время программы и потока.
* 


Тип часов
* `clockid_t` - тип часов [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=clock_gettime&category=3)
* `CLOCK_MONOTONIC` - тип часов, который стоит отдельно выделить. Это монотонные часы, то есть время, которое они показывают всегда возрастает несмотря ни на какие переводы времени. Их правильно использовать для замеров интервалов времени.


```python
for time_type in (time.CLOCK_REALTIME, time.CLOCK_MONOTONIC, time.CLOCK_PROCESS_CPUTIME_ID):
    print(time.clock_gettime(time_type))
```

    1589635370.4755588
    1073.3746712
    1.256146465



```python

```

## <a name="benchmarking"></a> Время для бенчмарков

#### Что измерять?
Стоит измерять процессорное время. В зависимости от того, делаете ли вы в измеряемой части программы системные вызовы или нет, имеет смысл измерять только пользовательское время или пользовательское и системное вместе.

#### Как измерять?

Чтобы замеры были максимально точными, стоит минимизировать влияние среды и максимизировать стабильность измерений. 

Какие есть способы повысить стабильность?

0. Повторить замер столько раз, сколько можете себе позволить по времени, и усреднить.
1. Увеличить минимальное время, которое шедулер гарантирует процессу, если он сам не отдает управления. Его можно увеличить до 1с.
2. Запускать бенчмарк на выделенном ядре. 
То есть запретить шедулеру запускать что-то еще на ядре, 
где будет работать бенчмарк, и его парном гипертрединговом.

А теперь подбробнее
1. `sudo sysctl -w kernel.sched_min_granularity_ns='999999999'` - выкручиваем квант времени шедулера.
2. В конфиге grub (`/etc/default/grub`) добавляем `isolcpu=2,3` (у меня это второе физическое ядро) в строку параметров запуска.
  <br> Обновляем grub. `sudo grub-mkconfig`, `sudo grub-mkconfig -o /boot/grub/grub.cfg`. Перезапускаем систему.
  <br> Теперь запускаем бенчмарк как `taskset 0x4 ./my_benchmark`. (4 == 1 << 2, 2 - номер виртуального ядра, на котором запускаем процесс)


#### Чем измерять?
* perf stat

perf вообще очень мощная штука, помимо бенчмаркинга позволяет профилировать программу, смотреть, какие функции сколько работают.

Устанавливается так:

```bash
$ sudo apt install linux-tools-$(uname -r) linux-tools-generic
$ echo -1 > /proc/sys/kernel/perf_event_paranoid # under `sudo -i`
```

* time



```bash
%%bash
exec 2>&1 ; set -o xtrace

perf stat sleep 1
time sleep 1
```

    + perf stat sleep 1
    
     Performance counter stats for 'sleep 1':
    
                  0,79 msec task-clock                #    0,001 CPUs utilized          
                     1      context-switches          #    0,001 M/sec                  
                     0      cpu-migrations            #    0,000 K/sec                  
                    63      page-faults               #    0,080 M/sec                  
       <not supported>      cycles                                                      
       <not supported>      instructions                                                
       <not supported>      branches                                                    
       <not supported>      branch-misses                                               
    
           1,036695202 seconds time elapsed
    
           0,001625000 seconds user
           0,000000000 seconds sys
    
    
    + sleep 1
    
    real	0m1,012s
    user	0m0,001s
    sys	0m0,002s


## <a name="sleep"></a> Как поспать?

`sleep`, `nanosleep` - просто поспать. <s>На практике</s> В хороших продовых проектах такие функции нужны редко, из-за того, что такие ожидания нельзя корректно прервать внешним событием. На деле, конечно, постоянно используется.

`timerfd` - позволяет создавать таймеры, которые при срабатывании будут приходить записями, которые можно прочесть из файлового дескриптора.

`select`, `epoll_wait` - одновременное ожидание по таймауту и по файловым дескрипторам.

`pthread_cond_timedwait` - одновременное ожидание по таймауту и условной переменной.

`sigtimedwait` - одновременное ожидание по таймауту и сигнала. (Лучше все-таки свести прием сигнала к чтению из файлового дескриптора и не использовать это.)



```python

```


```python

```


```python

```
