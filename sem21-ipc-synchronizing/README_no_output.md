```python
# look at tools/set_up_magics.ipynb
yandex_metrica_allowed = True ; get_ipython().run_cell('# one_liner_str\n\nget_ipython().run_cell_magic(\'javascript\', \'\', \'// setup cpp code highlighting\\nIPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\\\'reg\\\':[/^%%cpp/]} ;\')\n\n# creating magics\nfrom IPython.core.magic import register_cell_magic, register_line_magic\nfrom IPython.display import display, Markdown, HTML\nimport argparse\nfrom subprocess import Popen, PIPE\nimport random\nimport sys\nimport os\nimport re\nimport signal\nimport shutil\nimport shlex\nimport glob\n\n@register_cell_magic\ndef save_file(args_str, cell, line_comment_start="#"):\n    parser = argparse.ArgumentParser()\n    parser.add_argument("fname")\n    parser.add_argument("--ejudge-style", action="store_true")\n    args = parser.parse_args(args_str.split())\n    \n    cell = cell if cell[-1] == \'\\n\' or args.no_eof_newline else cell + "\\n"\n    cmds = []\n    with open(args.fname, "w") as f:\n        f.write(line_comment_start + " %%cpp " + args_str + "\\n")\n        for line in cell.split("\\n"):\n            line_to_write = (line if not args.ejudge_style else line.rstrip()) + "\\n"\n            if line.startswith("%"):\n                run_prefix = "%run "\n                if line.startswith(run_prefix):\n                    cmds.append(line[len(run_prefix):].strip())\n                    f.write(line_comment_start + " " + line_to_write)\n                    continue\n                run_prefix = "%# "\n                if line.startswith(run_prefix):\n                    f.write(line_comment_start + " " + line_to_write)\n                    continue\n                raise Exception("Unknown %%save_file subcommand: \'%s\'" % line)\n            else:\n                f.write(line_to_write)\n        f.write("" if not args.ejudge_style else line_comment_start + r" line without \\n")\n    for cmd in cmds:\n        display(Markdown("Run: `%s`" % cmd))\n        get_ipython().system(cmd)\n\n@register_cell_magic\ndef cpp(fname, cell):\n    save_file(fname, cell, "//")\n\n@register_cell_magic\ndef asm(fname, cell):\n    save_file(fname, cell, "//")\n    \n@register_cell_magic\ndef makefile(fname, cell):\n    assert not fname\n    save_file("makefile", cell.replace(" " * 4, "\\t"))\n        \n@register_line_magic\ndef p(line):\n    try:\n        expr, comment = line.split(" #")\n        display(Markdown("`{} = {}`  # {}".format(expr.strip(), eval(expr), comment.strip())))\n    except:\n        display(Markdown("{} = {}".format(line, eval(line))))\n        \ndef show_file(file, clear_at_begin=True, return_html_string=False):\n    if clear_at_begin:\n        get_ipython().system("truncate --size 0 " + file)\n    obj = file.replace(\'.\', \'_\').replace(\'/\', \'_\') + "_obj"\n    html_string = \'\'\'\n        <!--MD_BEGIN_FILTER-->\n        <script type=text/javascript>\n        var entrance___OBJ__ = 0;\n        var errors___OBJ__ = 0;\n        function refresh__OBJ__()\n        {\n            entrance___OBJ__ -= 1;\n            var elem = document.getElementById("__OBJ__");\n            if (elem) {\n                var xmlhttp=new XMLHttpRequest();\n                xmlhttp.onreadystatechange=function()\n                {\n                    var elem = document.getElementById("__OBJ__");\n                    console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___OBJ__);\n                    if (elem && xmlhttp.readyState==4) {\n                        if (xmlhttp.status==200)\n                        {\n                            errors___OBJ__ = 0;\n                            if (!entrance___OBJ__) {\n                                elem.innerText = xmlhttp.responseText;\n                                entrance___OBJ__ += 1;\n                                console.log("req");\n                                window.setTimeout("refresh__OBJ__()", 300); \n                            }\n                            return xmlhttp.responseText;\n                        } else {\n                            errors___OBJ__ += 1;\n                            if (errors___OBJ__ < 10 && !entrance___OBJ__) {\n                                entrance___OBJ__ += 1;\n                                console.log("req");\n                                window.setTimeout("refresh__OBJ__()", 300); \n                            }\n                        }\n                    }\n                }\n                xmlhttp.open("GET", "__FILE__", true);\n                xmlhttp.setRequestHeader("Cache-Control", "no-cache");\n                xmlhttp.send();     \n            }\n        }\n        \n        if (!entrance___OBJ__) {\n            entrance___OBJ__ += 1;\n            refresh__OBJ__(); \n        }\n        </script>\n        \n        <font color="white"> <tt>\n        <p id="__OBJ__" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>\n        </tt> </font>\n        <!--MD_END_FILTER-->\n        <!--MD_FROM_FILE __FILE__ -->\n        \'\'\'.replace("__OBJ__", obj).replace("__FILE__", file)\n    if return_html_string:\n        return html_string\n    display(HTML(html_string))\n    \nBASH_POPEN_TMP_DIR = "./bash_popen_tmp"\n    \ndef bash_popen_terminate_all():\n    for p in globals().get("bash_popen_list", []):\n        print("Terminate pid=" + str(p.pid), file=sys.stderr)\n        p.terminate()\n    globals()["bash_popen_list"] = []\n    if os.path.exists(BASH_POPEN_TMP_DIR):\n        shutil.rmtree(BASH_POPEN_TMP_DIR)\n\nbash_popen_terminate_all()  \n\ndef bash_popen(cmd):\n    if not os.path.exists(BASH_POPEN_TMP_DIR):\n        os.mkdir(BASH_POPEN_TMP_DIR)\n    h = os.path.join(BASH_POPEN_TMP_DIR, str(random.randint(0, 1e18)))\n    stdout_file = h + ".out.html"\n    stderr_file = h + ".err.html"\n    run_log_file = h + ".fin.html"\n    \n    stdout = open(stdout_file, "wb")\n    stdout = open(stderr_file, "wb")\n    \n    html = """\n    <table width="100%">\n    <colgroup>\n       <col span="1" style="width: 70px;">\n       <col span="1">\n    </colgroup>    \n    <tbody>\n      <tr> <td><b>STDOUT</b></td> <td> {stdout} </td> </tr>\n      <tr> <td><b>STDERR</b></td> <td> {stderr} </td> </tr>\n      <tr> <td><b>RUN LOG</b></td> <td> {run_log} </td> </tr>\n    </tbody>\n    </table>\n    """.format(\n        stdout=show_file(stdout_file, return_html_string=True),\n        stderr=show_file(stderr_file, return_html_string=True),\n        run_log=show_file(run_log_file, return_html_string=True),\n    )\n    \n    cmd = """\n        bash -c {cmd} &\n        pid=$!\n        echo "Process started! pid=${{pid}}" > {run_log_file}\n        wait ${{pid}}\n        echo "Process finished! exit_code=$?" >> {run_log_file}\n    """.format(cmd=shlex.quote(cmd), run_log_file=run_log_file)\n    # print(cmd)\n    display(HTML(html))\n    \n    p = Popen(["bash", "-c", cmd], stdin=PIPE, stdout=stdout, stderr=stdout)\n    \n    bash_popen_list.append(p)\n    return p\n\n\n@register_line_magic\ndef bash_async(line):\n    bash_popen(line)\n    \n    \ndef show_log_file(file, return_html_string=False):\n    obj = file.replace(\'.\', \'_\').replace(\'/\', \'_\') + "_obj"\n    html_string = \'\'\'\n        <!--MD_BEGIN_FILTER-->\n        <script type=text/javascript>\n        var entrance___OBJ__ = 0;\n        var errors___OBJ__ = 0;\n        function halt__OBJ__(elem, color)\n        {\n            elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    \n        }\n        function refresh__OBJ__()\n        {\n            entrance___OBJ__ -= 1;\n            if (entrance___OBJ__ < 0) {\n                entrance___OBJ__ = 0;\n            }\n            var elem = document.getElementById("__OBJ__");\n            if (elem) {\n                var xmlhttp=new XMLHttpRequest();\n                xmlhttp.onreadystatechange=function()\n                {\n                    var elem = document.getElementById("__OBJ__");\n                    console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___OBJ__);\n                    if (elem && xmlhttp.readyState==4) {\n                        if (xmlhttp.status==200)\n                        {\n                            errors___OBJ__ = 0;\n                            if (!entrance___OBJ__) {\n                                if (elem.innerHTML != xmlhttp.responseText) {\n                                    elem.innerHTML = xmlhttp.responseText;\n                                }\n                                if (elem.innerHTML.includes("Process finished.")) {\n                                    halt__OBJ__(elem, "#333333");\n                                } else {\n                                    entrance___OBJ__ += 1;\n                                    console.log("req");\n                                    window.setTimeout("refresh__OBJ__()", 300); \n                                }\n                            }\n                            return xmlhttp.responseText;\n                        } else {\n                            errors___OBJ__ += 1;\n                            if (!entrance___OBJ__) {\n                                if (errors___OBJ__ < 6) {\n                                    entrance___OBJ__ += 1;\n                                    console.log("req");\n                                    window.setTimeout("refresh__OBJ__()", 300); \n                                } else {\n                                    halt__OBJ__(elem, "#994444");\n                                }\n                            }\n                        }\n                    }\n                }\n                xmlhttp.open("GET", "__FILE__", true);\n                xmlhttp.setRequestHeader("Cache-Control", "no-cache");\n                xmlhttp.send();     \n            }\n        }\n        \n        if (!entrance___OBJ__) {\n            entrance___OBJ__ += 1;\n            refresh__OBJ__(); \n        }\n        </script>\n\n        <p id="__OBJ__" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">\n        </p>\n        \n        </font>\n        <!--MD_END_FILTER-->\n        <!--MD_FROM_FILE __FILE__.md -->\n        \'\'\'.replace("__OBJ__", obj).replace("__FILE__", file)\n    if return_html_string:\n        return html_string\n    display(HTML(html_string))\n\n    \nclass TInteractiveLauncher:\n    tmp_path = "./interactive_launcher_tmp"\n    def __init__(self, cmd):\n        try:\n            os.mkdir(TInteractiveLauncher.tmp_path)\n        except:\n            pass\n        name = str(random.randint(0, 1e18))\n        self.inq_path = os.path.join(TInteractiveLauncher.tmp_path, name + ".inq")\n        self.log_path = os.path.join(TInteractiveLauncher.tmp_path, name + ".log")\n        \n        os.mkfifo(self.inq_path)\n        open(self.log_path, \'w\').close()\n        open(self.log_path + ".md", \'w\').close()\n\n        self.pid = os.fork()\n        if self.pid == -1:\n            print("Error")\n        if self.pid == 0:\n            exe_cands = glob.glob("../tools/launcher.py") + glob.glob("../../tools/launcher.py")\n            assert(len(exe_cands) == 1)\n            assert(os.execvp("python3", ["python3", exe_cands[0], "-l", self.log_path, "-i", self.inq_path, "-c", cmd]) == 0)\n        self.inq_f = open(self.inq_path, "w")\n        interactive_launcher_opened_set.add(self.pid)\n        show_log_file(self.log_path)\n\n    def write(self, s):\n        s = s.encode()\n        assert len(s) == os.write(self.inq_f.fileno(), s)\n        \n    def get_pid(self):\n        n = 100\n        for i in range(n):\n            try:\n                return int(re.findall(r"PID = (\\d+)", open(self.log_path).readline())[0])\n            except:\n                if i + 1 == n:\n                    raise\n                time.sleep(0.1)\n        \n    def input_queue_path(self):\n        return self.inq_path\n        \n    def close(self):\n        self.inq_f.close()\n        os.waitpid(self.pid, 0)\n        os.remove(self.inq_path)\n        # os.remove(self.log_path)\n        self.inq_path = None\n        self.log_path = None \n        interactive_launcher_opened_set.remove(self.pid)\n        self.pid = None\n        \n    @staticmethod\n    def terminate_all():\n        if "interactive_launcher_opened_set" not in globals():\n            globals()["interactive_launcher_opened_set"] = set()\n        global interactive_launcher_opened_set\n        for pid in interactive_launcher_opened_set:\n            print("Terminate pid=" + str(pid), file=sys.stderr)\n            os.kill(pid, signal.SIGKILL)\n            os.waitpid(pid, 0)\n        interactive_launcher_opened_set = set()\n        if os.path.exists(TInteractiveLauncher.tmp_path):\n            shutil.rmtree(TInteractiveLauncher.tmp_path)\n    \nTInteractiveLauncher.terminate_all()\n   \nyandex_metrica_allowed = bool(globals().get("yandex_metrica_allowed", False))\nif yandex_metrica_allowed:\n    display(HTML(\'\'\'<!-- YANDEX_METRICA_BEGIN -->\n    <script type="text/javascript" >\n       (function(m,e,t,r,i,k,a){m[i]=m[i]||function(){(m[i].a=m[i].a||[]).push(arguments)};\n       m[i].l=1*new Date();k=e.createElement(t),a=e.getElementsByTagName(t)[0],k.async=1,k.src=r,a.parentNode.insertBefore(k,a)})\n       (window, document, "script", "https://mc.yandex.ru/metrika/tag.js", "ym");\n\n       ym(59260609, "init", {\n            clickmap:true,\n            trackLinks:true,\n            accurateTrackBounce:true\n       });\n    </script>\n    <noscript><div><img src="https://mc.yandex.ru/watch/59260609" style="position:absolute; left:-9999px;" alt="" /></div></noscript>\n    <!-- YANDEX_METRICA_END -->\'\'\'))\n\ndef make_oneliner():\n    html_text = \'("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований. Если вы не хотите, чтобы по вам собиралась статистика, исправьте: yandex_metrica_allowed = False" if yandex_metrica_allowed else "")\'\n    html_text += \' + "<""!-- MAGICS_SETUP_PRINTING_END -->"\'\n    return \'\'.join([\n        \'# look at tools/set_up_magics.ipynb\\n\',\n        \'yandex_metrica_allowed = True ; get_ipython().run_cell(%s);\' % repr(one_liner_str),\n        \'display(HTML(%s))\' % html_text,\n        \' #\'\'MAGICS_SETUP_END\'\n    ])\n       \n\n');display(HTML(("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований. Если вы не хотите, чтобы по вам собиралась статистика, исправьте: yandex_metrica_allowed = False" if yandex_metrica_allowed else "") + "<""!-- MAGICS_SETUP_PRINTING_END -->")) #MAGICS_SETUP_END
```

# Потоки и их использование

<br>
<div style="text-align: right"> Спасибо <a href="https://github.com/Disadvantaged">Голяр Димитрису</a> за участие в написании текста </div>
<br>


Сегодня в программе:
* <a href="#ptread_create" style="color:#856024">Создание и join потоков</a>
* <a href="#pthread_result" style="color:#856024">Аргументы и возвращаемое значение потока</a>
* <a href="#pthread_cancel" style="color:#856024">Прерывание/отмена/cancel потока</a>
* <a href="#pthread_attr" style="color:#856024">Атрибуты потока</a>
* <a href="#coro" style="color:#856024">Корутины</a>


<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>

[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/pthread)


Атрибуты процесса (полнота списков не гарантируется):
* Виртуальное адресное пространство и данные в этой витруальной памяти
* Файловые дескрипторы, блокировки файлов
* PID
* argc, argv
* ulimit

Атрибуты потока:
* Маски сигналов (Маска сигналов наследует маску потока-родителя, изменения будут сохраняться только внутри потока)
* Состояние процесса R, S, T, Z
* Состояние регистров (какая ф-я сейчас выполняется) (состояние стека скорее входит в состояние вииртуального адресного пространства)
* TID

https://unix.stackexchange.com/questions/47595/linux-max-threads-count - про максимальное количество процессов и потоков в системе.


```python

```

# <a name="pthread_create"></a> Создание и join потока


```python
%%cpp pthread_create.c
%run gcc -fsanitize=thread pthread_create.c -lpthread -o pthread_create.exe
%run ./pthread_create.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

const char* log_prefix(const char* file, int line) {
    struct timeval tp; gettimeofday(&tp, NULL); struct tm ltime; localtime_r(&tp.tv_sec, &ltime);
    static __thread char prefix[100]; 
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", &ltime);
    sprintf(prefix + time_len, ".%03ld %s:%d [tid=%ld]", tp.tv_usec / 1000, file, line, syscall(__NR_gettid));
    return prefix;
}

#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FILE__, __LINE__), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// thread-aware assert
#define ta_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed"); exit(EXIT_FAILURE); }

// Возвращаемое значение потока (~код возврата процесса) -- любое машинное слово.
static void* thread_func(void* arg) 
{
    log_printf("  Thread func started\n");
    log_printf("  Thread func finished\n");
    return NULL;
}

int main()
{
    log_printf("Main func started\n");
    pthread_t thread;
    log_printf("Thread creating\n");
    ta_assert(pthread_create(&thread, NULL, thread_func, 0) == 0); // В какой-то момент будет создан поток и в нем вызвана функция
    // Начиная отсюда неизвестно в каком порядке выполняются инструкции основного и дочернего потока
    ta_assert(pthread_join(thread, NULL) == 0); // -- аналог waitpid. Второй аргумент -- указатель в который запишется возвращаемое значение
    log_printf("Thread joined\n");
    log_printf("Main func finished\n");
    return 0;
}
```


```python

```

# <a name="pthread_result"></a> Смотрим на возвращаемое потоком значение.


```python
%%cpp pthread_create.c
%run clang -fsanitize=memory pthread_create.c -lpthread -o pthread_create.exe
%run ./pthread_create.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

const char* log_prefix(const char* file, int line) {
    struct timeval tp; gettimeofday(&tp, NULL); struct tm ltime; localtime_r(&tp.tv_sec, &ltime);
    static __thread char prefix[100]; 
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", &ltime);
    sprintf(prefix + time_len, ".%03ld %s:%d [tid=%ld]", tp.tv_usec / 1000, file, line, syscall(__NR_gettid));
    return prefix;
}

#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FILE__, __LINE__), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// thread-aware assert
#define ta_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed"); exit(EXIT_FAILURE); }


typedef struct {
    int a;
    int b;
} thread_task_args_t;

// На самом деле проще записать результат в структуру аргументов
typedef struct {
    int c;
} thread_task_result_t;

static thread_task_result_t* thread_func(const thread_task_args_t *arg)
{
    log_printf("  Thread func started\n");
    thread_task_result_t* result = 
        (thread_task_result_t*)malloc(sizeof(thread_task_result_t));
    result->c = arg->a + arg->b;
    log_printf("  Thread func finished\n");
    return result;
}

int main()
{
    log_printf("Main func started\n");
    pthread_t thread;
    
    thread_task_args_t args = {.a = 35, .b = 7};
    log_printf("Thread creating, args are: a=%d b=%d\n", args.a, args.b);
    ta_assert(pthread_create(
        &thread, NULL, 
        (void* (*)(void*))thread_func, // Важно понимать, что тут происходит
        (void*)&args
    ) == 0);
    
    thread_task_result_t* result;
    ta_assert(pthread_join(thread, (void**)&result) == 0);
    log_printf("Thread joined. Result: c=%d\n", result->c);
    free(result);
    
    log_printf("Main func finished\n");
    return 0;
}
```

# <a name="pthread_cancel"></a> Прерывание потока

Пусть это возможно сделать, но с этим нужно быть очень осторожным, особенно если поток, который вы прерываете владеет какими-либо ресурсами


```python
%%cpp pthread_cancel.c
%run gcc -fsanitize=thread pthread_cancel.c -lpthread -o pthread_cancel.exe
%run ./pthread_cancel.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

const char* log_prefix(const char* file, int line) {
    struct timeval tp; gettimeofday(&tp, NULL); struct tm ltime; localtime_r(&tp.tv_sec, &ltime);
    static __thread char prefix[100]; 
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", &ltime);
    sprintf(prefix + time_len, ".%03ld %s:%d [tid=%ld]", tp.tv_usec / 1000, file, line, syscall(__NR_gettid));
    return prefix;
}

#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FILE__, __LINE__), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// thread-aware assert
#define ta_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed"); exit(EXIT_FAILURE); }

static void* thread_func(void* arg)
{
    log_printf("  Thread func started\n");
    // В системных функциях разбросаны Cancellation points, в которых может быть прерван поток.
    sleep(2);
    log_printf("  Thread func finished\n"); // not printed because thread canceled
    return NULL;
}

int main()
{
    log_printf("Main func started\n");
    pthread_t thread;
    log_printf("Thread creating\n");
    ta_assert(pthread_create(&thread, NULL, thread_func, 0) == 0);
    sleep(1);
    log_printf("Thread canceling\n");
    ta_assert(pthread_cancel(thread) == 0); // принимает id потока и прерывает его.
    ta_assert(pthread_join(thread, NULL) == 0); // Если не сделать join, то останется зомби-поток.
    log_printf("Thread joined\n");
    log_printf("Main func finished\n");
    return 0;
}
```

По умолчанию pthread_cancel может прерывать поток, только в cancelation points (то есть в функциях, в реализациях которых есть проверка на это). 

Поэтому, если эти функции не вызывать, то поток не сможет быть прерван.

Но можно воспользоваться `pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);`. Тогда поток может быть прерван на уровне планировщика. (То есть поток скорее всего доработает текущий выделенный квант времени, но на следующий квант уже не запустится)


```python
%%cpp pthread_cancel_fail.c
%run gcc -fsanitize=thread pthread_cancel_fail.c -lpthread -o pthread_cancel_fail.exe
%run timeout 3 ./pthread_cancel_fail.exe  # will fail (cancelation at cancelation points)
%run gcc -fsanitize=thread  -DASYNC_CANCEL pthread_cancel_fail.c -lpthread -o pthread_cancel_fail.exe
%run timeout 3 ./pthread_cancel_fail.exe  # ok, async cancelation

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

const char* log_prefix(const char* file, int line) {
    struct timeval tp; gettimeofday(&tp, NULL); struct tm ltime; localtime_r(&tp.tv_sec, &ltime);
    static __thread char prefix[100]; 
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", &ltime);
    sprintf(prefix + time_len, ".%03ld %s:%d [tid=%ld]", tp.tv_usec / 1000, file, line, syscall(__NR_gettid));
    return prefix;
}

#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FILE__, __LINE__), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// thread-aware assert
#define ta_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed"); exit(EXIT_FAILURE); }

static void *
thread_func(void *arg)
{
    log_printf("  Thread func started\n");
    #ifdef ASYNC_CANCEL
    ta_assert(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) == 0); // Включаем более жесткий способ остановки потока
    #endif
    // Без опции ASYNC_CANCEL поток не может быть остановлен во время своей работы.
    while (1); // зависаем тут. В процессе явно не будет cancelation points
    log_printf("  Thread func finished\n"); 
    return NULL;
}

int main()
{
    log_printf("Main func started\n");
    pthread_t thread;
    log_printf("Thread creating\n");
    ta_assert(pthread_create(&thread, NULL, thread_func, 0) == 0);
    sleep(1);
    log_printf("Thread canceling\n");
    ta_assert(pthread_cancel(thread) == 0);
    log_printf("Thread joining\n");
    ta_assert(pthread_join(thread, NULL) == 0);
    log_printf("Thread joined\n");
    log_printf("Main func finished\n");
    return 0;
}
```


```python

```

## А можно ли приджойнить основной поток?


```python
%%cpp join_main_thread.c
%run gcc join_main_thread.c -lpthread -o join_main_thread.exe
%run timeout 3 ./join_main_thread.exe ; echo "Exit code: $?"
%run gcc -fsanitize=thread join_main_thread.c -lpthread -o join_main_thread.exe
%run timeout 3 ./join_main_thread.exe ; echo "Exit code: $?"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

const char* log_prefix(const char* file, int line) {
    struct timeval tp; gettimeofday(&tp, NULL); struct tm ltime; localtime_r(&tp.tv_sec, &ltime);
    static __thread char prefix[100]; 
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", &ltime);
    sprintf(prefix + time_len, ".%03ld %s:%d [tid=%ld]", tp.tv_usec / 1000, file, line, syscall(__NR_gettid));
    return prefix;
}

#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FILE__, __LINE__), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// thread-aware assert
#define ta_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed"); exit(EXIT_FAILURE); }

pthread_t main_thread;

static void* thread_func(void* arg)
{
    log_printf("  Thread func started\n");
  
    log_printf("  Main thread joining\n");
    ta_assert(pthread_join(main_thread, NULL) == 0);
    log_printf("  Main thread joined\n");

    log_printf("  Thread func finished\n");

    _exit(42);
}

int main()
{
    log_printf("Main func started\n");
    main_thread = pthread_self();
    
    pthread_t thread;
    log_printf("Thread creating\n");
    ta_assert(pthread_create(&thread, NULL, thread_func, 0) == 0);
    
    pthread_exit(NULL);
}
```

Без санитайзера можно, с санитайзером - нет. Не знаю есть ли тут какое-то принципиальное нарушение, но не надо так делать)


```python

```

# <a name="pthread_attr"></a> Атрибуты потока

* Размер стека
* Местоположение стека
* Размер защитной области после стека. Вот тут можно прокомментировать: это область ниже стека, которая является дырой в виртуальном адресном пространстве программы. То есть при попытке обращения к этой области произойдет segfault. Для чего необходима защитная область? Чтобы при переполнении стека получать segfault, а не неотлавливаемый проезд по памяти.


В следующем примере создадим поток двумя способами. С параметрами по умолчанию и указав минимальный размер стека. И посмотрим на потребления памяти. 

(Да, потреблениЯ. Там все не так просто, как кажется на первый взгляд :). Загляните в `/proc/<pid>/status`)


```python
%%cpp pthread_stack_size.c
%run gcc -fsanitize=thread pthread_stack_size.c -lpthread -o pthread_stack_size.exe
%run ./pthread_stack_size.exe 
%run gcc -fsanitize=thread -DMY_STACK_SIZE=16384 pthread_stack_size.c -lpthread -o pthread_stack_size.exe
%run ./pthread_stack_size.exe 
%run # Во второй раз (VM delta size) не 16кб потому что имеются накладные расходы.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

const char* log_prefix(const char* file, int line) {
    struct timeval tp; gettimeofday(&tp, NULL); struct tm ltime; localtime_r(&tp.tv_sec, &ltime);
    static __thread char prefix[100]; 
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", &ltime);
    sprintf(prefix + time_len, ".%03ld %s:%d [tid=%ld]", tp.tv_usec / 1000, file, line, syscall(__NR_gettid));
    return prefix;
}

#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FILE__, __LINE__), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// thread-aware assert
#define ta_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed"); exit(EXIT_FAILURE); }

long int get_maxrss() {
    struct rusage usage;
    ta_assert(getrusage(RUSAGE_SELF, &usage) == 0);
    return usage.ru_maxrss;
}

const char* run_and_get_output(const char* bash_cmd) {
    int fds[2];
    pipe(fds);
    int pid = fork();
    if (pid == 0) {
        dup2(fds[1], 1);
        close(fds[0]); close(fds[1]);
        execlp("bash", "bash", "-c", bash_cmd, NULL);
        ta_assert(0 && "unreachable");
    }
    close(fds[1]);
    static __thread char buffer[100];
    int size = 0, rd = 0;
    while ((rd = read(fds[0], buffer, sizeof(buffer) - size)) != 0) {
        if (rd > 0) {
            size += rd;
        }
    }
    buffer[size] = 0;
    return buffer;
}

long int get_vm_usage() {  
    char cmd1[10000];
    sprintf(cmd1, "cat /proc/%d/status | grep VmData", getpid());
    const char* vm_usage_s = run_and_get_output(cmd1);
    long int vm_usage;
    sscanf(vm_usage_s, "VmData: %ld kB", &vm_usage);
    return vm_usage;
}

static void *
thread_func(void *arg)
{
//     int a[800000];
//     for (int i = 0; i < sizeof(a) / sizeof(int); ++i) {
//         a[i] = i;
//     }   
    log_printf("  Thread func started\n");
    sleep(2);
    log_printf("  Thread func finished\n"); 
    return NULL;
}

int main()
{
    long int initial_rss = get_maxrss();
    long int initial_vm_size = get_vm_usage();
    log_printf("Main func started. Initial RSS = %ldkb, initial VM usage = %ldkb\n", 
               initial_rss, initial_vm_size);
    pthread_t thread;
    pthread_attr_t thread_attr; 
    ta_assert(pthread_attr_init(&thread_attr) == 0); // Атрибуты нужно инициализировать
    #ifdef MY_STACK_SIZE
    ta_assert(pthread_attr_setstacksize(&thread_attr, MY_STACK_SIZE) == 0); // В структуру сохраняем размер стека
    #endif
    log_printf("Thread creating\n");
    ta_assert(pthread_create(&thread, &thread_attr, thread_func, 0) == 0);
    ta_assert(pthread_attr_destroy(&thread_attr) == 0); // И уничтожить
    sleep(1);
    
    log_printf("Thread working. RSS = %ldkb, delta RSS = %ldkb\n", 
               get_maxrss(), get_maxrss() - initial_rss);
    log_printf("Thread working. VM size = %ldkb, VM delta size = %ldkb (!)\n", 
               get_vm_usage(), get_vm_usage() - initial_vm_size); 
    
    ta_assert(pthread_join(thread, NULL) == 0);
    log_printf("Thread joined\n");
    log_printf("Main func finished\n");
    return 0;
}
```


```python

```


```python

```


```python

```

# <a name="coro"></a> Coroutines

Корутины -- это потоки внутри одного юзерспейса. То есть, это потоки внутри потока.
Для этого используется программный (реализованный в коде пользователя), а не системный scheduler.

Файберы (=корутины, =потоки в юзерспейсе):
<br>`+` Известно, когда может быть вызвано переключение контекста. Файберы работающие внутри одного потока могут не пользоваться межпоточной синхронизацией при общении друг с другом.
<br>`+` Низкие затраты на переключение контекста. Это очень эффективно, если есть много потоков перекладывающих друг другу данные.
<br>`+` ...
<br>`-` Привязанность к фреймворку. Нельзя использовать блокирующие вызовы не через этот фреймворк.
<br>`-` Нельзя подключиться к процессу с помощью gdb и посмотреть на все потоки. 
<br>`-` ...


```python
!rm -rf ./libtask
!git clone git@github.com:0intro/libtask.git
!cd libtask && make 
```


```python
%%cpp coro.cpp
%run gcc -I ./libtask coro.cpp ./libtask/libtask.a -lpthread -o coro.exe
%run ./coro.exe 300 100 200 1000

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <task.h>


const char* log_prefix() {
    struct timeval tp; gettimeofday(&tp, NULL);
    static __thread char prefix[100];
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", localtime(&tp.tv_sec));
    sprintf(prefix + time_len, ".%03ld [tid=%ld]", tp.tv_usec / 1000, syscall(__NR_gettid));
    return prefix;
}

#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// thread-aware assert
#define ta_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed"); exit(EXIT_FAILURE); }


const int STACK_SIZE = 32768;

Channel *c;

void delaytask(void *v)
{
    int ms = *(int*)(void*)&v;
    taskdelay(ms);
    log_printf("Task %dms is launched\n", ms);
    chansendul(c, 0);
}

void taskmain(int argc, char **argv)
{    
    c = chancreate(sizeof(unsigned long), 0);

    for(int i = 1; i < argc; i++){
        int ms = atoi(argv[i]);
        log_printf("Schedule %dms task\n", ms);
        taskcreate(delaytask, *(void**)&ms, STACK_SIZE);
    }
    
    int a = 1;
    // a == 1

    for(int i = 1; i < argc; i++){
        log_printf("Some task is finished\n");
        chanrecvul(c);
    }
    taskexitall(0);
}
```


```python

```

# <a name="hw"></a> Комментарии к ДЗ

* 


```python

```


```python

```


```python

```


```python

```
