```python
# look at tools/set_up_magics.ipynb
yandex_metrica_allowed = True ; get_ipython().run_cell('# one_liner_str\n\nget_ipython().run_cell_magic(\'javascript\', \'\', \'// setup cpp code highlighting\\nIPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\\\'reg\\\':[/^%%cpp/]} ;\')\n\n# creating magics\nfrom IPython.core.magic import register_cell_magic, register_line_magic\nfrom IPython.display import display, Markdown, HTML\nimport argparse\nfrom subprocess import Popen, PIPE\nimport random\nimport sys\nimport os\nimport re\nimport signal\nimport shutil\nimport shlex\nimport glob\n\n@register_cell_magic\ndef save_file(args_str, cell, line_comment_start="#"):\n    parser = argparse.ArgumentParser()\n    parser.add_argument("fname")\n    parser.add_argument("--ejudge-style", action="store_true")\n    args = parser.parse_args(args_str.split())\n    \n    cell = cell if cell[-1] == \'\\n\' or args.no_eof_newline else cell + "\\n"\n    cmds = []\n    with open(args.fname, "w") as f:\n        f.write(line_comment_start + " %%cpp " + args_str + "\\n")\n        for line in cell.split("\\n"):\n            line_to_write = (line if not args.ejudge_style else line.rstrip()) + "\\n"\n            if line.startswith("%"):\n                run_prefix = "%run "\n                if line.startswith(run_prefix):\n                    cmds.append(line[len(run_prefix):].strip())\n                    f.write(line_comment_start + " " + line_to_write)\n                    continue\n                run_prefix = "%# "\n                if line.startswith(run_prefix):\n                    f.write(line_comment_start + " " + line_to_write)\n                    continue\n                raise Exception("Unknown %%save_file subcommand: \'%s\'" % line)\n            else:\n                f.write(line_to_write)\n        f.write("" if not args.ejudge_style else line_comment_start + r" line without \\n")\n    for cmd in cmds:\n        display(Markdown("Run: `%s`" % cmd))\n        get_ipython().system(cmd)\n\n@register_cell_magic\ndef cpp(fname, cell):\n    save_file(fname, cell, "//")\n\n@register_cell_magic\ndef asm(fname, cell):\n    save_file(fname, cell, "//")\n    \n@register_cell_magic\ndef makefile(fname, cell):\n    assert not fname\n    save_file("makefile", cell.replace(" " * 4, "\\t"))\n        \n@register_line_magic\ndef p(line):\n    try:\n        expr, comment = line.split(" #")\n        display(Markdown("`{} = {}`  # {}".format(expr.strip(), eval(expr), comment.strip())))\n    except:\n        display(Markdown("{} = {}".format(line, eval(line))))\n        \ndef show_file(file, clear_at_begin=True, return_html_string=False):\n    if clear_at_begin:\n        get_ipython().system("truncate --size 0 " + file)\n    obj = file.replace(\'.\', \'_\').replace(\'/\', \'_\') + "_obj"\n    html_string = \'\'\'\n        <!--MD_BEGIN_FILTER-->\n        <script type=text/javascript>\n        var entrance___OBJ__ = 0;\n        var errors___OBJ__ = 0;\n        function refresh__OBJ__()\n        {\n            entrance___OBJ__ -= 1;\n            var elem = document.getElementById("__OBJ__");\n            if (elem) {\n                var xmlhttp=new XMLHttpRequest();\n                xmlhttp.onreadystatechange=function()\n                {\n                    var elem = document.getElementById("__OBJ__");\n                    console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___OBJ__);\n                    if (elem && xmlhttp.readyState==4) {\n                        if (xmlhttp.status==200)\n                        {\n                            errors___OBJ__ = 0;\n                            if (!entrance___OBJ__) {\n                                elem.innerText = xmlhttp.responseText;\n                                entrance___OBJ__ += 1;\n                                console.log("req");\n                                window.setTimeout("refresh__OBJ__()", 300); \n                            }\n                            return xmlhttp.responseText;\n                        } else {\n                            errors___OBJ__ += 1;\n                            if (errors___OBJ__ < 10 && !entrance___OBJ__) {\n                                entrance___OBJ__ += 1;\n                                console.log("req");\n                                window.setTimeout("refresh__OBJ__()", 300); \n                            }\n                        }\n                    }\n                }\n                xmlhttp.open("GET", "__FILE__", true);\n                xmlhttp.setRequestHeader("Cache-Control", "no-cache");\n                xmlhttp.send();     \n            }\n        }\n        \n        if (!entrance___OBJ__) {\n            entrance___OBJ__ += 1;\n            refresh__OBJ__(); \n        }\n        </script>\n        \n        <font color="white"> <tt>\n        <p id="__OBJ__" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>\n        </tt> </font>\n        <!--MD_END_FILTER-->\n        <!--MD_FROM_FILE __FILE__ -->\n        \'\'\'.replace("__OBJ__", obj).replace("__FILE__", file)\n    if return_html_string:\n        return html_string\n    display(HTML(html_string))\n    \nBASH_POPEN_TMP_DIR = "./bash_popen_tmp"\n    \ndef bash_popen_terminate_all():\n    for p in globals().get("bash_popen_list", []):\n        print("Terminate pid=" + str(p.pid), file=sys.stderr)\n        p.terminate()\n    globals()["bash_popen_list"] = []\n    if os.path.exists(BASH_POPEN_TMP_DIR):\n        shutil.rmtree(BASH_POPEN_TMP_DIR)\n\nbash_popen_terminate_all()  \n\ndef bash_popen(cmd):\n    if not os.path.exists(BASH_POPEN_TMP_DIR):\n        os.mkdir(BASH_POPEN_TMP_DIR)\n    h = os.path.join(BASH_POPEN_TMP_DIR, str(random.randint(0, 1e18)))\n    stdout_file = h + ".out.html"\n    stderr_file = h + ".err.html"\n    run_log_file = h + ".fin.html"\n    \n    stdout = open(stdout_file, "wb")\n    stdout = open(stderr_file, "wb")\n    \n    html = """\n    <table width="100%">\n    <colgroup>\n       <col span="1" style="width: 70px;">\n       <col span="1">\n    </colgroup>    \n    <tbody>\n      <tr> <td><b>STDOUT</b></td> <td> {stdout} </td> </tr>\n      <tr> <td><b>STDERR</b></td> <td> {stderr} </td> </tr>\n      <tr> <td><b>RUN LOG</b></td> <td> {run_log} </td> </tr>\n    </tbody>\n    </table>\n    """.format(\n        stdout=show_file(stdout_file, return_html_string=True),\n        stderr=show_file(stderr_file, return_html_string=True),\n        run_log=show_file(run_log_file, return_html_string=True),\n    )\n    \n    cmd = """\n        bash -c {cmd} &\n        pid=$!\n        echo "Process started! pid=${{pid}}" > {run_log_file}\n        wait ${{pid}}\n        echo "Process finished! exit_code=$?" >> {run_log_file}\n    """.format(cmd=shlex.quote(cmd), run_log_file=run_log_file)\n    # print(cmd)\n    display(HTML(html))\n    \n    p = Popen(["bash", "-c", cmd], stdin=PIPE, stdout=stdout, stderr=stdout)\n    \n    bash_popen_list.append(p)\n    return p\n\n\n@register_line_magic\ndef bash_async(line):\n    bash_popen(line)\n    \n    \ndef show_log_file(file, return_html_string=False):\n    obj = file.replace(\'.\', \'_\').replace(\'/\', \'_\') + "_obj"\n    html_string = \'\'\'\n        <!--MD_BEGIN_FILTER-->\n        <script type=text/javascript>\n        var entrance___OBJ__ = 0;\n        var errors___OBJ__ = 0;\n        function halt__OBJ__(elem, color)\n        {\n            elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    \n        }\n        function refresh__OBJ__()\n        {\n            entrance___OBJ__ -= 1;\n            if (entrance___OBJ__ < 0) {\n                entrance___OBJ__ = 0;\n            }\n            var elem = document.getElementById("__OBJ__");\n            if (elem) {\n                var xmlhttp=new XMLHttpRequest();\n                xmlhttp.onreadystatechange=function()\n                {\n                    var elem = document.getElementById("__OBJ__");\n                    console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___OBJ__);\n                    if (elem && xmlhttp.readyState==4) {\n                        if (xmlhttp.status==200)\n                        {\n                            errors___OBJ__ = 0;\n                            if (!entrance___OBJ__) {\n                                if (elem.innerHTML != xmlhttp.responseText) {\n                                    elem.innerHTML = xmlhttp.responseText;\n                                }\n                                if (elem.innerHTML.includes("Process finished.")) {\n                                    halt__OBJ__(elem, "#333333");\n                                } else {\n                                    entrance___OBJ__ += 1;\n                                    console.log("req");\n                                    window.setTimeout("refresh__OBJ__()", 300); \n                                }\n                            }\n                            return xmlhttp.responseText;\n                        } else {\n                            errors___OBJ__ += 1;\n                            if (!entrance___OBJ__) {\n                                if (errors___OBJ__ < 6) {\n                                    entrance___OBJ__ += 1;\n                                    console.log("req");\n                                    window.setTimeout("refresh__OBJ__()", 300); \n                                } else {\n                                    halt__OBJ__(elem, "#994444");\n                                }\n                            }\n                        }\n                    }\n                }\n                xmlhttp.open("GET", "__FILE__", true);\n                xmlhttp.setRequestHeader("Cache-Control", "no-cache");\n                xmlhttp.send();     \n            }\n        }\n        \n        if (!entrance___OBJ__) {\n            entrance___OBJ__ += 1;\n            refresh__OBJ__(); \n        }\n        </script>\n\n        <p id="__OBJ__" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">\n        </p>\n        \n        </font>\n        <!--MD_END_FILTER-->\n        <!--MD_FROM_FILE __FILE__.md -->\n        \'\'\'.replace("__OBJ__", obj).replace("__FILE__", file)\n    if return_html_string:\n        return html_string\n    display(HTML(html_string))\n\n    \nclass TInteractiveLauncher:\n    tmp_path = "./interactive_launcher_tmp"\n    def __init__(self, cmd):\n        try:\n            os.mkdir(TInteractiveLauncher.tmp_path)\n        except:\n            pass\n        name = str(random.randint(0, 1e18))\n        self.inq_path = os.path.join(TInteractiveLauncher.tmp_path, name + ".inq")\n        self.log_path = os.path.join(TInteractiveLauncher.tmp_path, name + ".log")\n        \n        os.mkfifo(self.inq_path)\n        open(self.log_path, \'w\').close()\n        open(self.log_path + ".md", \'w\').close()\n\n        self.pid = os.fork()\n        if self.pid == -1:\n            print("Error")\n        if self.pid == 0:\n            exe_cands = glob.glob("../tools/launcher.py") + glob.glob("../../tools/launcher.py")\n            assert(len(exe_cands) == 1)\n            assert(os.execvp("python3", ["python3", exe_cands[0], "-l", self.log_path, "-i", self.inq_path, "-c", cmd]) == 0)\n        self.inq_f = open(self.inq_path, "w")\n        interactive_launcher_opened_set.add(self.pid)\n        show_log_file(self.log_path)\n\n    def write(self, s):\n        s = s.encode()\n        assert len(s) == os.write(self.inq_f.fileno(), s)\n        \n    def get_pid(self):\n        n = 100\n        for i in range(n):\n            try:\n                return int(re.findall(r"PID = (\\d+)", open(self.log_path).readline())[0])\n            except:\n                if i + 1 == n:\n                    raise\n                time.sleep(0.1)\n        \n    def input_queue_path(self):\n        return self.inq_path\n        \n    def close(self):\n        self.inq_f.close()\n        os.waitpid(self.pid, 0)\n        os.remove(self.inq_path)\n        # os.remove(self.log_path)\n        self.inq_path = None\n        self.log_path = None \n        interactive_launcher_opened_set.remove(self.pid)\n        self.pid = None\n        \n    @staticmethod\n    def terminate_all():\n        if "interactive_launcher_opened_set" not in globals():\n            globals()["interactive_launcher_opened_set"] = set()\n        global interactive_launcher_opened_set\n        for pid in interactive_launcher_opened_set:\n            print("Terminate pid=" + str(pid), file=sys.stderr)\n            os.kill(pid, signal.SIGKILL)\n            os.waitpid(pid, 0)\n        interactive_launcher_opened_set = set()\n        if os.path.exists(TInteractiveLauncher.tmp_path):\n            shutil.rmtree(TInteractiveLauncher.tmp_path)\n    \nTInteractiveLauncher.terminate_all()\n   \nyandex_metrica_allowed = bool(globals().get("yandex_metrica_allowed", False))\nif yandex_metrica_allowed:\n    display(HTML(\'\'\'<!-- YANDEX_METRICA_BEGIN -->\n    <script type="text/javascript" >\n       (function(m,e,t,r,i,k,a){m[i]=m[i]||function(){(m[i].a=m[i].a||[]).push(arguments)};\n       m[i].l=1*new Date();k=e.createElement(t),a=e.getElementsByTagName(t)[0],k.async=1,k.src=r,a.parentNode.insertBefore(k,a)})\n       (window, document, "script", "https://mc.yandex.ru/metrika/tag.js", "ym");\n\n       ym(59260609, "init", {\n            clickmap:true,\n            trackLinks:true,\n            accurateTrackBounce:true\n       });\n    </script>\n    <noscript><div><img src="https://mc.yandex.ru/watch/59260609" style="position:absolute; left:-9999px;" alt="" /></div></noscript>\n    <!-- YANDEX_METRICA_END -->\'\'\'))\n\ndef make_oneliner():\n    html_text = \'("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований. Если вы не хотите, чтобы по вам собиралась статистика, исправьте: yandex_metrica_allowed = False" if yandex_metrica_allowed else "")\'\n    html_text += \' + "<""!-- MAGICS_SETUP_PRINTING_END -->"\'\n    return \'\'.join([\n        \'# look at tools/set_up_magics.ipynb\\n\',\n        \'yandex_metrica_allowed = True ; get_ipython().run_cell(%s);\' % repr(one_liner_str),\n        \'display(HTML(%s))\' % html_text,\n        \' #\'\'MAGICS_SETUP_END\'\n    ])\n       \n\n');display(HTML(("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований. Если вы не хотите, чтобы по вам собиралась статистика, исправьте: yandex_metrica_allowed = False" if yandex_metrica_allowed else "") + "<""!-- MAGICS_SETUP_PRINTING_END -->")) #MAGICS_SETUP_END
```

# Inter Process Communications (IPC)

<br>
<div style="text-align: right"> Спасибо <a href="https://github.com/SyrnikRebirth">Сове Глебу</a> и <a href="https://github.com/Disadvantaged">Голяр Димитрису</a> за участие в написании текста </div>
<br>


Сегодня в программе:
* <a href="#mmap" style="color:#856024">`mmap` для IPC</a>
  <br> Используем примитивы межпоточной синхронизации для межпроцессной. Через разделяемую память создаем правильные мьютексы.
  <br> [Ссылка про правильные мьютексы](https://linux.die.net/man/3/pthread_mutexattr_init)
* <a href="#shm" style="color:#856024">Объекты разделяемой памяти POSIX</a>
  <br>Это почти то же самое, что и обычные файлы, но у них ортогональное пространство имен и они не сохраняются на диск.
  <br>Вызовы `shm_open` (открывает/создает объект разделяемой памяти, аналогично `open`) и `shm_unlink` (удаляет ссылку на объект, аналогично `unlink`)  
  <br>[Документашка](https://www.opennet.ru/man.shtml?topic=shm_open&category=3&russian=0). [Отличия от `open`](https://stackoverflow.com/questions/24875257/why-use-shm-open)
  <br><br>
* Семафоры
  <br><a href="#sem_anon" style="color:#856024">Неименованные</a>
  <br><a href="#sem_named" style="color:#856024">Именованные</a>
* <a href="#sem_signal" style="color:#856024">Сочетаемость семафоров и сигналов</a> 

  
  
<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>

[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/posix_ipc)

# <a name="mmap"></a> `mmap`


Разделяемая память - это когда два региона виртуальной памяти (один в одном процессе, другой в другом) 
ссылаются на одну и ту же физическую память. То есть могут обмениваться информацией через нее.

Межпроцессное взаимодействие через разделяемую память нужно, 
когда у нас есть две различные программы (могут быть написаны на разных языках)
и когда нам не подходит взаимодействие через сокеты (такое взаимодействие не очень эффективно).




```python
%%cpp mmap.c
%run gcc -Wall -fsanitize=thread mmap.c -lpthread -o mmap.exe
%run ./mmap.exe

#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>
#include <stdatomic.h>

const char* log_prefix(const char* file, int line) {
    struct timeval tp; gettimeofday(&tp, NULL); struct tm ltime; localtime_r(&tp.tv_sec, &ltime);
    static __thread char prefix[100]; 
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", &ltime);
    sprintf(prefix + time_len, ".%03ld %s:%3d [pid=%d]", tp.tv_usec / 1000, file, line, getpid());
    return prefix;
}

#define log_printf_impl(fmt, ...) { dprintf(2, "%s: " fmt "%s", log_prefix(__FILE__, __LINE__), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// process-aware assert
#define pa_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed\n"); exit(EXIT_FAILURE); }

typedef enum {
    VALID_STATE = 0,
    INVALID_STATE = 1
} state_t;


typedef struct {
    pthread_mutex_t mutex; 
    state_t current_state; // protected by mutex
} shared_state_t;

shared_state_t* state; // interprocess state

// process_safe_func и process_func - функции-примеры с прошлого семинара (с точностью до замены thread/process)
void process_safe_func() {
    // all function is critical section, protected by mutex
    pthread_mutex_lock(&state->mutex); // try comment lock&unlock out and look at result
    pa_assert(state->current_state == VALID_STATE);
    state->current_state = INVALID_STATE; // do some work with state. 
    sched_yield();
    state->current_state = VALID_STATE;
    pthread_mutex_unlock(&state->mutex);
}

void process_func(int process_num) 
{
    log_printf("  Process %d started\n", process_num);
    for (int j = 0; j < 10000; ++j) {
        process_safe_func();
    }
    log_printf("  Process %d finished\n", process_num);
}

 
shared_state_t* create_state() {
    // Создаем кусок разделяемой памяти. Он будет общим для данного процесса и его дочерних
    shared_state_t* state = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ sizeof(shared_state_t), // Размер разделяемого фрагмента памяти
        /* access attributes, prot = */ PROT_READ | PROT_WRITE, 
        /* flags = */ MAP_SHARED | MAP_ANONYMOUS,
        /* fd = */ -1,
        /* offset in file, offset = */ 0
    );
    pa_assert(state != MAP_FAILED);
    
    // create and initialize interprocess mutex
    pthread_mutexattr_t mutex_attrs; 
    pa_assert(pthread_mutexattr_init(&mutex_attrs) == 0);
    // Важно! Без этого атрибута один из процессов навсегда зависнет в lock мьютекса
    // Вероятно этот атрибут влияет на отсутствие флага FUTEX_PRIVATE_FLAG в операциях с futex
    // Если он стоит, то ядро может делать некоторые оптимизации в предположении, что futex используется одним процессом
    pa_assert(pthread_mutexattr_setpshared(&mutex_attrs, PTHREAD_PROCESS_SHARED) == 0);
    pa_assert(pthread_mutex_init(&state->mutex, &mutex_attrs) == 0);
    pa_assert(pthread_mutexattr_destroy(&mutex_attrs) == 0);
    
    state->current_state = VALID_STATE; // Инициализирем защищаемое состояние
    return state;
}

void delete_state(shared_state_t* state) {
    pa_assert(pthread_mutex_destroy(&state->mutex) == 0);
    pa_assert(munmap(state, sizeof(shared_state_t)) == 0);
}

int main()
{
    log_printf("Main process started\n");
    state = create_state(); // Создаем разделяемое состояние
    const int process_count = 2;
    pid_t processes[process_count];
    // Создаем дочерние процессы
    for (int i = 0; i < process_count; ++i) {
        log_printf("Creating process %d\n", i);
        // дочерние процессы унаследуют разделяемое состояние (оно не скопируется, а будет общим)
        pa_assert((processes[i] = fork()) >= 0); 
        if (processes[i] == 0) {
            process_func(i); // Имитируем работу из разных процессов
            exit(0);
        }
    }
    for (int i = 0; i < process_count; ++i) {
        int status;
        pa_assert(waitpid(processes[i], &status, 0) != -1)
        pa_assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
        log_printf("Process %d 'joined'\n", i);
    }
    delete_state(state);
    log_printf("Main process finished\n");
    return 0;
}
```

# Ну и spinlock давайте. А почему бы и нет?

Отличие только в замене инициализации и в взятии/снятии локов.


```python
%%cpp mmap.c
%run gcc -Wall -fsanitize=thread mmap.c -lpthread -o mmap.exe
%run ./mmap.exe

#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>
#include <stdatomic.h>

const char* log_prefix(const char* file, int line) {
    struct timeval tp; gettimeofday(&tp, NULL); struct tm ltime; localtime_r(&tp.tv_sec, &ltime);
    static __thread char prefix[100]; 
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", &ltime);
    sprintf(prefix + time_len, ".%03ld %s:%3d [pid=%d]", tp.tv_usec / 1000, file, line, getpid());
    return prefix;
}

#define log_printf_impl(fmt, ...) { dprintf(2, "%s: " fmt "%s", log_prefix(__FILE__, __LINE__), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// process-aware assert
#define pa_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed\n"); exit(EXIT_FAILURE); }

typedef enum {
    VALID_STATE = 0,
    INVALID_STATE = 1
} state_t;


typedef struct {
    _Atomic int lock; 
    state_t current_state; // protected by mutex
} shared_state_t;

shared_state_t* state; // interprocess state

void sl_lock(_Atomic int* lock) { 
    int expected = 0;
    while (!atomic_compare_exchange_weak(lock, &expected, 1)) {
        expected = 0;
    }
}

void sl_unlock(_Atomic int* lock) {
    atomic_fetch_sub(lock, 1);
}

void process_safe_func() {
    // all function is critical section, protected by spinlock
    sl_lock(&state->lock);
    pa_assert(state->current_state == VALID_STATE);
    state->current_state = INVALID_STATE; // do some work with state. 
    sched_yield();
    state->current_state = VALID_STATE;
    sl_unlock(&state->lock);
}

void process_func(int process_num) 
{
    log_printf("  Process %d started\n", process_num);
    for (int j = 0; j < 10000; ++j) {
        process_safe_func();
    }
    log_printf("  Process %d finished\n", process_num);
}

 
shared_state_t* create_state() {
    shared_state_t* state = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ sizeof(shared_state_t), 
        /* access attributes, prot = */ PROT_READ | PROT_WRITE, 
        /* flags = */ MAP_SHARED | MAP_ANONYMOUS,
        /* fd = */ -1,
        /* offset in file, offset = */ 0
    );
    pa_assert(state != MAP_FAILED);
    
    state->lock = 0;
    state->current_state = VALID_STATE;
    return state;
}

void delete_state(shared_state_t* state) {
    pa_assert(munmap(state, sizeof(shared_state_t)) == 0);
}

int main()
{
    log_printf("Main process started\n");
    state = create_state();
    const int process_count = 2;
    pid_t processes[process_count];
    for (int i = 0; i < process_count; ++i) {
        log_printf("Creating process %d\n", i);
        pa_assert((processes[i] = fork()) >= 0);
        if (processes[i] == 0) {
            process_func(i);
            exit(0);
        }
    }
    for (int i = 0; i < process_count; ++i) {
        int status;
        pa_assert(waitpid(processes[i], &status, 0) != -1)
        pa_assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
        log_printf("Process %d 'joined'\n", i);
    }
    delete_state(state);
    log_printf("Main process finished\n");
    return 0;
}
```

# <a name="shm"></a> `shm_open`

Сделаем то же самое, что и в предыдущем примере, но на этот раз не из родственных процессов. Воспользуемся именноваными объектами разделяемой памяти.


```python
%%cpp shm.c
%# Обратите внимание: -lrt. Здесь нужна новая разделяемая библиотека
%run gcc -Wall -fsanitize=thread shm.c -lrt -lpthread -o s.exe
%run ./s.exe create_shm /my_shm
%run ./s.exe work 1 /my_shm & PID=$! ; ./s.exe work 2 /my_shm ; wait $PID
%run ./s.exe remove_shm /my_shm

#define _GNU_SOURCE 
#include <stdio.h> 
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>

const char* log_prefix(const char* file, int line) {
    struct timeval tp; gettimeofday(&tp, NULL); struct tm ltime; localtime_r(&tp.tv_sec, &ltime);
    static __thread char prefix[100]; 
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", &ltime);
    sprintf(prefix + time_len, ".%03ld %s:%3d [pid=%d]", tp.tv_usec / 1000, file, line, getpid());
    return prefix;
}

#define log_printf_impl(fmt, ...) { dprintf(2, "%s: " fmt "%s", log_prefix(__FILE__, __LINE__), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// process-aware assert
#define pa_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed\n"); exit(EXIT_FAILURE); }

typedef enum {
    VALID_STATE = 0,
    INVALID_STATE = 1
} state_t;


typedef struct {
    pthread_mutex_t mutex; 
    state_t current_state; // protected by mutex
} shared_state_t;

void process_safe_func(shared_state_t* state) {
    // all function is critical section, protected by mutex
    pthread_mutex_lock(&state->mutex); // try comment lock&unlock out and look at result
    pa_assert(state->current_state == VALID_STATE);
    state->current_state = INVALID_STATE; // do some work with state. 
    sched_yield();
    state->current_state = VALID_STATE;
    pthread_mutex_unlock(&state->mutex);
}
 
shared_state_t* load_state(const char* shm_name, bool do_create) {
    // открываем / создаем объект разделяемой памяти
    // по сути это просто open, только для виртуального файла (без сохранения данных на диск + ортогональное пространство имен)
    int fd = shm_open(shm_name, O_RDWR | (do_create ? O_CREAT : 0), 0644);
    pa_assert(fd >= 0);
    pa_assert(ftruncate(fd, sizeof(shared_state_t)) == 0);
    shared_state_t* state = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ sizeof(shared_state_t), 
        /* access attributes, prot = */ PROT_READ | PROT_WRITE, 
        /* flags = */ MAP_SHARED,
        /* fd = */ fd,
        /* offset in file, offset = */ 0
    );
    pa_assert(state != MAP_FAILED);
    if (!do_create) {
        return state;
    }
    // create interprocess mutex
    pthread_mutexattr_t mutex_attrs;
    pa_assert(pthread_mutexattr_init(&mutex_attrs) == 0);
    // Важно!
    pa_assert(pthread_mutexattr_setpshared(&mutex_attrs, PTHREAD_PROCESS_SHARED) == 0);
    pa_assert(pthread_mutex_init(&state->mutex, &mutex_attrs) == 0);
    pa_assert(pthread_mutexattr_destroy(&mutex_attrs) == 0);
    
    state->current_state = VALID_STATE;
    return state;
}

void unload_state(shared_state_t* state) {
    pa_assert(munmap(state, sizeof(shared_state_t)) == 0);
}

int main(int argc, char** argv)
{
    pa_assert(argc >= 2);
    if (strcmp("create_shm", argv[1]) == 0) {
        log_printf("  Creating state: %s\n", argv[2]);
        unload_state(load_state(argv[2], /*do_create=*/ 1));
        log_printf("  State created\n");
    } else if (strcmp("remove_shm", argv[1]) == 0) {
        log_printf("  Removing state: %s\n", argv[2]);
        // Файлы shm существуют пока не будет вызвана unlink.
        pa_assert(shm_unlink(argv[2]) == 0)
        log_printf("  State removed\n");   
    } else if (strcmp("work", argv[1]) == 0) {
        pa_assert(argc >= 3);
        int worker = strtol(argv[2], 0, 10);
        log_printf("  Worker %d started\n", worker);
        shared_state_t* state = load_state(argv[3], /*do_create=*/ 0);
       
        for (int j = 0; j < 10000; ++j) {
            process_safe_func(state);
        }

        unload_state(state);
        log_printf("  Worker %d finished\n", worker);
    } else {
        pa_assert(0 && "unknown command")
    }
    return 0;
}
```

Проблема: как решить, кто из независимых процессов будет создавать участок?

Способ разрешить конфликт создания участка разделяемой памяти:
  * Все процессы создают файлы с флагом O_EXCL | O_CREAT
  * Из-за О_EXCL выкинет ошибку для всех процессов кроме одного
  * Этот один процесс создаст файл, выделит память, создаст спинлок на инициализацию и начнёт инициализировать
  * Другие, которые получили ошибку попытаются открыть файл ещё раз, без этих флагов уже.
  * Далее они будут ждать (регулярно проверять) пока не изменится размер файла, потом откроют его, и дальше будут ждать инициализации на спинлоке.

# <a name="sem_anon"></a> Анонимные семафоры

Игровое сравнение: семафор это ящик с шариками.

<pre>
|   |
|   |
|   |
|_*_|</pre>
    ^
    |
Это семафор со значением 1 (ящик с одним шариком)

У семафора такая семантика:
* Операция post() кладет шарик в ящик. Работает мгновенно.
* Операция wait() извлекает шарик из ящика. Если шариков нет, то блокируется пока не появится шарик и затем его извлекает.
* Еще есть try_wait(), timed_wait() - они соответствуют названиям.

Шарики можно так же рассматривать как свободные ресурсы.

Семафор с одним шариком можно использовать как мьютекс. В данном случае шарик - это ресурс на право входить в критическую секцию. Соответственно lock - это wait. А unlock это post.


### Пример использования с многими шариками:    Построение очереди.

Создаём 2 семафора, semFree и semElementsInside.

При добавлении берём ресурс (~шарик) из semFree, под lock добавляем элемент, кладём ресурс в semElementsInside

При удалении берём ресурс из semElementsInside, под локом удаляем элемент, кладём ресурс в semFree<br>


```python
%%cpp sem_anon.c
%run gcc -Wall -fsanitize=thread -lrt sem_anon.c -o sem_anon.exe
%run ./sem_anon.exe

#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>

const char* log_prefix(const char* file, int line) {
    struct timeval tp; gettimeofday(&tp, NULL); struct tm ltime; localtime_r(&tp.tv_sec, &ltime);
    static __thread char prefix[100]; 
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", &ltime);
    sprintf(prefix + time_len, ".%03ld %s:%3d [pid=%d]", tp.tv_usec / 1000, file, line, getpid());
    return prefix;
}

#define log_printf_impl(fmt, ...) { dprintf(2, "%s: " fmt "%s", log_prefix(__FILE__, __LINE__), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// process-aware assert
#define pa_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed\n"); exit(EXIT_FAILURE); }

typedef enum {
    VALID_STATE = 0,
    INVALID_STATE = 1
} state_t;


typedef struct {
    sem_t semaphore; 
    state_t current_state; // protected by semaphore
} shared_state_t;

shared_state_t* state; // interprocess state

void process_safe_func() {
    // all function is critical section, protected by mutex
    sem_wait(&state->semaphore); // ~ lock
    pa_assert(state->current_state == VALID_STATE);
    state->current_state = INVALID_STATE; // do some work with state. 
    sched_yield();
    state->current_state = VALID_STATE;
    sem_post(&state->semaphore); // ~ unlock
}

void process_func(int process_num) 
{
    log_printf("  Process %d started\n", process_num);
    for (int j = 0; j < 10000; ++j) {
        process_safe_func();
    }
    log_printf("  Process %d finished\n", process_num);
}

 
shared_state_t* create_state() {
    shared_state_t* state = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ sizeof(shared_state_t), 
        /* access attributes, prot = */ PROT_READ | PROT_WRITE, 
        /* flags = */ MAP_SHARED | MAP_ANONYMOUS,
        /* fd = */ -1,
        /* offset in file, offset = */ 0
    );
    pa_assert(state != MAP_FAILED);
    
    // create interprocess semaphore
    pa_assert(sem_init(
        &state->semaphore,
        1, // interprocess? (0 if will be used in one process)
        1  // initial value
    ) == 0);
    
    state->current_state = VALID_STATE;
    return state;
}

void delete_state(shared_state_t* state) {
    pa_assert(sem_destroy(&state->semaphore) == 0);
    pa_assert(munmap(state, sizeof(shared_state_t)) == 0);
}

int main()
{
    log_printf("Main process started\n");
    state = create_state();
    const int process_count = 2;
    pid_t processes[process_count];
    for (int i = 0; i < process_count; ++i) {
        log_printf("Creating process %d\n", i);
        pa_assert((processes[i] = fork()) >= 0);
        if (processes[i] == 0) {
            process_func(i);
            exit(0);
        }
    }
    for (int i = 0; i < process_count; ++i) {
        int status;
        pa_assert(waitpid(processes[i], &status, 0) != -1)
        pa_assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
        log_printf("Process %d 'joined'\n", i);
    }
    delete_state(state);
    log_printf("Main process finished\n");
    return 0;
}
```

# <a name="sem_named"></a> Именнованные семафоры

В примере про именованные объекты разделяемой памяти мы явно запускали процесс для инициализации состояния до процессов-воркеров, чтобы избежать гонки инициализации состояния.

В этом примере предлагается способ избежать гонки используя именованный семафор.

В примере используется одно и то же имя для объекта разделяемой памяти и семафора. Это безопасно, так как имя семафора автоматически расширяется префиксом или суффиксом `sem`. То есть в результате имена разные.


```python
%%cpp sem_named.c
%# Обратите внимание: -lrt. Здесь нужна новая разделяемая библиотека
%run gcc -Wall -fsanitize=thread sem_named.c -lrt -lpthread -o s.exe
%run ./s.exe work 1 /s42 & PID=$! ; ./s.exe work 2 /s42 ; wait $PID
%run ./s.exe cleanup /s42 # необязательная команда. Будет работать и без нее

#define _GNU_SOURCE 
#include <stdio.h> 
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

const char* log_prefix(const char* file, int line) {
    struct timeval tp; gettimeofday(&tp, NULL); struct tm ltime; localtime_r(&tp.tv_sec, &ltime);
    static __thread char prefix[100]; 
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", &ltime);
    sprintf(prefix + time_len, ".%03ld %s:%3d [pid=%d]", tp.tv_usec / 1000, file, line, getpid());
    return prefix;
}

#define log_printf_impl(fmt, ...) { dprintf(2, "%s: " fmt "%s", log_prefix(__FILE__, __LINE__), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// process-aware assert
#define pa_warn_if_not(stmt) if (stmt) {} else { log_printf("WARNING: '" #stmt "' failed\n"); }
#define pa_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed\n"); exit(EXIT_FAILURE); }

typedef enum {
    VALID_STATE = 0,
    INVALID_STATE = 1
} state_t;


typedef struct {
    pthread_mutex_t mutex; 
    state_t current_state; // protected by mutex
} shared_state_t;

void process_safe_func(shared_state_t* state) {
    // all function is critical section, protected by mutex
    pthread_mutex_lock(&state->mutex); // try comment lock&unlock out and look at result
    pa_assert(state->current_state == VALID_STATE);
    state->current_state = INVALID_STATE; // do some work with state. 
    sched_yield();
    state->current_state = VALID_STATE;
    pthread_mutex_unlock(&state->mutex);
}
 
shared_state_t* load_state(const char* shm_name, bool do_create) {
    // открываем / создаем объект разделяемой памяти
    int fd = shm_open(shm_name, O_RDWR | (do_create ? O_CREAT : 0), 0644);
    if (do_create) {
        pa_assert(ftruncate(fd, sizeof(shared_state_t)) == 0);
    }
    shared_state_t* state = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ sizeof(shared_state_t), 
        /* access attributes, prot = */ PROT_READ | PROT_WRITE, 
        /* flags = */ MAP_SHARED,
        /* fd = */ fd,
        /* offset in file, offset = */ 0
    );
    pa_assert(state != MAP_FAILED);
    if (do_create) {
        // create interprocess mutex
        pthread_mutexattr_t mutex_attrs;
        pa_assert(pthread_mutexattr_init(&mutex_attrs) == 0);
        // Важно!
        pa_assert(pthread_mutexattr_setpshared(&mutex_attrs, PTHREAD_PROCESS_SHARED) == 0);
        pa_assert(pthread_mutex_init(&state->mutex, &mutex_attrs) == 0);
        pa_assert(pthread_mutexattr_destroy(&mutex_attrs) == 0);

        state->current_state = VALID_STATE;
    }
    return state;
}

void unload_state(shared_state_t* state) {
    pa_assert(munmap(state, sizeof(shared_state_t)) == 0);
}

shared_state_t* process_safe_init_and_load(const char* name) {
    // succeeded only for first process. This process will initalize state
    sem_t* init_semaphore = sem_open(
        name, O_CREAT | O_EXCL, 0644, 0); // Создаем семафор с изначальным значением 0. Если семафор уже есть, то команда пофейлится
    if (init_semaphore != SEM_FAILED) { // Если смогли сделать семафор, то мы - главный процесс, ответственный за инициализацию
        // initializing branch for initializing process
        shared_state_t* state = load_state(name, /*do_create=*/ 1);
        sem_post(init_semaphore); // Кладем в "ящик" весточку, что стейт проинициализирован
        sem_close(init_semaphore);
        return state;
    } else { // Если мы не главные процесс, то подождем инициализацию
        // branch for processes waiting initialisation
        init_semaphore = sem_open(name, 0);
        pa_assert(init_semaphore != SEM_FAILED);
        sem_wait(init_semaphore); // ждем весточку, что стейт готов
        sem_post(init_semaphore); // возвращаем весточку на место, чтобы другим процессам тоже досталось
        sem_close(init_semaphore);
        return load_state(name, /*do_create=*/ 0);
    }
}

int main(int argc, char** argv)
{
    pa_assert(argc >= 2);
    if (strcmp("cleanup", argv[1]) == 0) {
        log_printf("  Cleanup sem and shm: %s\n", argv[2]);
        pa_warn_if_not(shm_unlink(argv[2]) == 0);
        pa_warn_if_not(sem_unlink(argv[2]) == 0);
        log_printf("  State created\n");
    } else if (strcmp("work", argv[1]) == 0) {
        pa_assert(argc >= 3);
        int worker = strtol(argv[2], 0, 10);
        log_printf("  Worker %d started\n", worker);
        
        shared_state_t* state = process_safe_init_and_load(argv[3]);
       
        for (int j = 0; j < 10000; ++j) {
            process_safe_func(state);
        }
     
        unload_state(state);
        log_printf("  Worker %d finished\n", worker);
    } else {
        pa_assert(0 && "unknown command")
    }
    return 0;
}
```

### Важное замечание про именованные и неименованные семафоры

Для открытия/закрытия именованных семафоров используются `sem_open` и `sem_close`.

А для неименованных `sem_init` и `sem_destroy`. 

Смешивать эти операции определенно не стоит, если конечно, вы где-нибудь не найдете документацию, подтверждающую обратное. Делать `sem_open`, а затем `sem_destroy`, это как создавать объект конструктором одного класса, а уничтожать деструктором другого (для родственных классов, без виртуального деструктора).


```python

```

# <a name="sem_signal"></a> Сочетаемость семафоров и сигналов

Нет этой сочетаемости.

```
process 1
> send signal to process 2
> sem_post

process 2
> sem_wait
> check variable that is set in signal handler
```

Не гарантируется, что сигнал будет доставлен и обработан до того, как отработает sem_wait.


```python
%%cpp sem_and_signal.c
%run gcc -Wall -fsanitize=thread -lrt sem_and_signal.c -o sem_and_signal.exe
%run ./sem_and_signal.exe

#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>

const char* log_prefix(const char* file, int line) {
    struct timeval tp; gettimeofday(&tp, NULL); struct tm ltime; localtime_r(&tp.tv_sec, &ltime);
    static __thread char prefix[100]; 
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", &ltime);
    sprintf(prefix + time_len, ".%03ld %s:%3d [pid=%d]", tp.tv_usec / 1000, file, line, getpid());
    return prefix;
}

#define log_printf_impl(fmt, ...) { dprintf(2, "%s: " fmt "%s", log_prefix(__FILE__, __LINE__), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// process-aware assert
#define pa_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed\n"); exit(EXIT_FAILURE); }

volatile sig_atomic_t signal_count = 0;

static void handler(int signum) {
    signal_count += 1;
}

typedef struct {
    sem_t semaphore_1;
    sem_t semaphore_2;
} shared_state_t;

shared_state_t* state;

 
shared_state_t* create_state() {
    shared_state_t* state = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ sizeof(shared_state_t), 
        /* access attributes, prot = */ PROT_READ | PROT_WRITE, 
        /* flags = */ MAP_SHARED | MAP_ANONYMOUS,
        /* fd = */ -1,
        /* offset in file, offset = */ 0
    );
    pa_assert(state != MAP_FAILED);
    
    pa_assert(sem_init(&state->semaphore_1, 1, 0) == 0);
    pa_assert(sem_init(&state->semaphore_2, 1, 0) == 0);
    
    return state;
}

void delete_state(shared_state_t* state) {
    pa_assert(sem_destroy(&state->semaphore_1) == 0);
    pa_assert(sem_destroy(&state->semaphore_2) == 0);
    pa_assert(munmap(state, sizeof(shared_state_t)) == 0);
}

int main()
{
    log_printf("Main process started\n");
    state = create_state();
    pid_t process = fork();
    if (process == 0) {
        sigaction(SIGUSR1, &(struct sigaction){.sa_handler = handler, .sa_flags = SA_RESTART}, NULL);
        sleep(1); // imitate synchronous start
        for (int i = 0; ; ++i) {
            sem_wait(&state->semaphore_1); 
            int cnt = signal_count;
            if (cnt != i + 1) {
                fprintf(stderr, "Signals and semaphors are not ordered... i = %d, signals_count = %d\n", i, cnt);
                exit(-1);
            }
            if (i % 100000 == 0) {
                fprintf(stderr, "i = %d\n", i);
            }
            sem_post(&state->semaphore_2); 
        }
    } else {
        sleep(1); // imitate synchronous start
        int status;
        int ret;
        while ((ret = waitpid(process, &status, WNOHANG)) == 0) {
            kill(process, SIGUSR1);
            sem_post(&state->semaphore_1);
        
            while (sem_timedwait(&state->semaphore_2, &(struct timespec){.tv_nsec = 500000000}) == -1 
                   && (ret = waitpid(process, &status, WNOHANG)) == 0) {
            }
        }
        pa_assert(ret != -1)
        pa_assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    }
   
    delete_state(state);
    log_printf("Main process finished\n");
    return 0;
}
```

Ломается, долго, но ломается. Так, что нельзя рассчитывать, что сигнал отрпавленный раньше, будет обработан до того, как дойдет событие через sem_post/sem_wait отправленное позже.

Что неудивительно, так как семафоры работают напрямую через разделяемую память, а в обработке сигналов принимает участие еще и планировщик задач.

Если система многоядерная и два процесса выполняются одновременно, то между отправкой сигнала и получением события через семафор может не случиться переключений процессов планировщиком - тогда сигнал будет доставлен позже.

А на одноядерной системе представленная схема скорее всего будет работать. Так как между sem_post в одном процессе и завершением sem_wait в другом должно случиться переключение на второй процесс. В ходе которого вызовутся обработчики.


```python

```


```python

```


```python

```

# <a name="hw"></a> Комментарии к ДЗ

* 


```python

```


```python

```
