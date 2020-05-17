```python
# look at tools/set_up_magics.ipynb
yandex_metrica_allowed = True ; get_ipython().run_cell('# one_liner_str\n\nget_ipython().run_cell_magic(\'javascript\', \'\', \'// setup cpp code highlighting\\nIPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\\\'reg\\\':[/^%%cpp/]} ;\')\n\n# creating magics\nfrom IPython.core.magic import register_cell_magic, register_line_magic\nfrom IPython.display import display, Markdown, HTML\nimport argparse\nfrom subprocess import Popen, PIPE\nimport random\nimport sys\nimport os\nimport re\nimport signal\nimport shutil\nimport shlex\nimport glob\n\n@register_cell_magic\ndef save_file(args_str, cell, line_comment_start="#"):\n    parser = argparse.ArgumentParser()\n    parser.add_argument("fname")\n    parser.add_argument("--ejudge-style", action="store_true")\n    args = parser.parse_args(args_str.split())\n    \n    cell = cell if cell[-1] == \'\\n\' or args.no_eof_newline else cell + "\\n"\n    cmds = []\n    with open(args.fname, "w") as f:\n        f.write(line_comment_start + " %%cpp " + args_str + "\\n")\n        for line in cell.split("\\n"):\n            line_to_write = (line if not args.ejudge_style else line.rstrip()) + "\\n"\n            if line.startswith("%"):\n                run_prefix = "%run "\n                if line.startswith(run_prefix):\n                    cmds.append(line[len(run_prefix):].strip())\n                    f.write(line_comment_start + " " + line_to_write)\n                    continue\n                run_prefix = "%# "\n                if line.startswith(run_prefix):\n                    f.write(line_comment_start + " " + line_to_write)\n                    continue\n                raise Exception("Unknown %%save_file subcommand: \'%s\'" % line)\n            else:\n                f.write(line_to_write)\n        f.write("" if not args.ejudge_style else line_comment_start + r" line without \\n")\n    for cmd in cmds:\n        display(Markdown("Run: `%s`" % cmd))\n        get_ipython().system(cmd)\n\n@register_cell_magic\ndef cpp(fname, cell):\n    save_file(fname, cell, "//")\n\n@register_cell_magic\ndef asm(fname, cell):\n    save_file(fname, cell, "//")\n    \n@register_cell_magic\ndef makefile(fname, cell):\n    assert not fname\n    save_file("makefile", cell.replace(" " * 4, "\\t"))\n        \n@register_line_magic\ndef p(line):\n    try:\n        expr, comment = line.split(" #")\n        display(Markdown("`{} = {}`  # {}".format(expr.strip(), eval(expr), comment.strip())))\n    except:\n        display(Markdown("{} = {}".format(line, eval(line))))\n        \ndef show_file(file, clear_at_begin=True, return_html_string=False):\n    if clear_at_begin:\n        get_ipython().system("truncate --size 0 " + file)\n    obj = file.replace(\'.\', \'_\').replace(\'/\', \'_\') + "_obj"\n    html_string = \'\'\'\n        <!--MD_BEGIN_FILTER-->\n        <script type=text/javascript>\n        var entrance___OBJ__ = 0;\n        var errors___OBJ__ = 0;\n        function refresh__OBJ__()\n        {\n            entrance___OBJ__ -= 1;\n            var elem = document.getElementById("__OBJ__");\n            if (elem) {\n                var xmlhttp=new XMLHttpRequest();\n                xmlhttp.onreadystatechange=function()\n                {\n                    var elem = document.getElementById("__OBJ__");\n                    console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___OBJ__);\n                    if (elem && xmlhttp.readyState==4) {\n                        if (xmlhttp.status==200)\n                        {\n                            errors___OBJ__ = 0;\n                            if (!entrance___OBJ__) {\n                                elem.innerText = xmlhttp.responseText;\n                                entrance___OBJ__ += 1;\n                                console.log("req");\n                                window.setTimeout("refresh__OBJ__()", 300); \n                            }\n                            return xmlhttp.responseText;\n                        } else {\n                            errors___OBJ__ += 1;\n                            if (errors___OBJ__ < 10 && !entrance___OBJ__) {\n                                entrance___OBJ__ += 1;\n                                console.log("req");\n                                window.setTimeout("refresh__OBJ__()", 300); \n                            }\n                        }\n                    }\n                }\n                xmlhttp.open("GET", "__FILE__", true);\n                xmlhttp.setRequestHeader("Cache-Control", "no-cache");\n                xmlhttp.send();     \n            }\n        }\n        \n        if (!entrance___OBJ__) {\n            entrance___OBJ__ += 1;\n            refresh__OBJ__(); \n        }\n        </script>\n        \n        <font color="white"> <tt>\n        <p id="__OBJ__" style="font-size: 16px; border:3px #333333 solid; background: #333333; border-radius: 10px; padding: 10px;  "></p>\n        </tt> </font>\n        <!--MD_END_FILTER-->\n        <!--MD_FROM_FILE __FILE__ -->\n        \'\'\'.replace("__OBJ__", obj).replace("__FILE__", file)\n    if return_html_string:\n        return html_string\n    display(HTML(html_string))\n    \nBASH_POPEN_TMP_DIR = "./bash_popen_tmp"\n    \ndef bash_popen_terminate_all():\n    for p in globals().get("bash_popen_list", []):\n        print("Terminate pid=" + str(p.pid), file=sys.stderr)\n        p.terminate()\n    globals()["bash_popen_list"] = []\n    if os.path.exists(BASH_POPEN_TMP_DIR):\n        shutil.rmtree(BASH_POPEN_TMP_DIR)\n\nbash_popen_terminate_all()  \n\ndef bash_popen(cmd):\n    if not os.path.exists(BASH_POPEN_TMP_DIR):\n        os.mkdir(BASH_POPEN_TMP_DIR)\n    h = os.path.join(BASH_POPEN_TMP_DIR, str(random.randint(0, 1e18)))\n    stdout_file = h + ".out.html"\n    stderr_file = h + ".err.html"\n    run_log_file = h + ".fin.html"\n    \n    stdout = open(stdout_file, "wb")\n    stdout = open(stderr_file, "wb")\n    \n    html = """\n    <table width="100%">\n    <colgroup>\n       <col span="1" style="width: 70px;">\n       <col span="1">\n    </colgroup>    \n    <tbody>\n      <tr> <td><b>STDOUT</b></td> <td> {stdout} </td> </tr>\n      <tr> <td><b>STDERR</b></td> <td> {stderr} </td> </tr>\n      <tr> <td><b>RUN LOG</b></td> <td> {run_log} </td> </tr>\n    </tbody>\n    </table>\n    """.format(\n        stdout=show_file(stdout_file, return_html_string=True),\n        stderr=show_file(stderr_file, return_html_string=True),\n        run_log=show_file(run_log_file, return_html_string=True),\n    )\n    \n    cmd = """\n        bash -c {cmd} &\n        pid=$!\n        echo "Process started! pid=${{pid}}" > {run_log_file}\n        wait ${{pid}}\n        echo "Process finished! exit_code=$?" >> {run_log_file}\n    """.format(cmd=shlex.quote(cmd), run_log_file=run_log_file)\n    # print(cmd)\n    display(HTML(html))\n    \n    p = Popen(["bash", "-c", cmd], stdin=PIPE, stdout=stdout, stderr=stdout)\n    \n    bash_popen_list.append(p)\n    return p\n\n\n@register_line_magic\ndef bash_async(line):\n    bash_popen(line)\n    \n    \ndef show_log_file(file, return_html_string=False):\n    obj = file.replace(\'.\', \'_\').replace(\'/\', \'_\') + "_obj"\n    html_string = \'\'\'\n        <!--MD_BEGIN_FILTER-->\n        <script type=text/javascript>\n        var entrance___OBJ__ = 0;\n        var errors___OBJ__ = 0;\n        function halt__OBJ__(elem, color)\n        {\n            elem.setAttribute("style", "font-size: 14px; background: " + color + "; padding: 10px; border: 3px; border-radius: 5px; color: white; ");                    \n        }\n        function refresh__OBJ__()\n        {\n            entrance___OBJ__ -= 1;\n            if (entrance___OBJ__ < 0) {\n                entrance___OBJ__ = 0;\n            }\n            var elem = document.getElementById("__OBJ__");\n            if (elem) {\n                var xmlhttp=new XMLHttpRequest();\n                xmlhttp.onreadystatechange=function()\n                {\n                    var elem = document.getElementById("__OBJ__");\n                    console.log(!!elem, xmlhttp.readyState, xmlhttp.status, entrance___OBJ__);\n                    if (elem && xmlhttp.readyState==4) {\n                        if (xmlhttp.status==200)\n                        {\n                            errors___OBJ__ = 0;\n                            if (!entrance___OBJ__) {\n                                if (elem.innerHTML != xmlhttp.responseText) {\n                                    elem.innerHTML = xmlhttp.responseText;\n                                }\n                                if (elem.innerHTML.includes("Process finished.")) {\n                                    halt__OBJ__(elem, "#333333");\n                                } else {\n                                    entrance___OBJ__ += 1;\n                                    console.log("req");\n                                    window.setTimeout("refresh__OBJ__()", 300); \n                                }\n                            }\n                            return xmlhttp.responseText;\n                        } else {\n                            errors___OBJ__ += 1;\n                            if (!entrance___OBJ__) {\n                                if (errors___OBJ__ < 6) {\n                                    entrance___OBJ__ += 1;\n                                    console.log("req");\n                                    window.setTimeout("refresh__OBJ__()", 300); \n                                } else {\n                                    halt__OBJ__(elem, "#994444");\n                                }\n                            }\n                        }\n                    }\n                }\n                xmlhttp.open("GET", "__FILE__", true);\n                xmlhttp.setRequestHeader("Cache-Control", "no-cache");\n                xmlhttp.send();     \n            }\n        }\n        \n        if (!entrance___OBJ__) {\n            entrance___OBJ__ += 1;\n            refresh__OBJ__(); \n        }\n        </script>\n\n        <p id="__OBJ__" style="font-size: 14px; background: #000000; padding: 10px; border: 3px; border-radius: 5px; color: white; ">\n        </p>\n        \n        </font>\n        <!--MD_END_FILTER-->\n        <!--MD_FROM_FILE __FILE__.md -->\n        \'\'\'.replace("__OBJ__", obj).replace("__FILE__", file)\n    if return_html_string:\n        return html_string\n    display(HTML(html_string))\n\n    \nclass TInteractiveLauncher:\n    tmp_path = "./interactive_launcher_tmp"\n    def __init__(self, cmd):\n        try:\n            os.mkdir(TInteractiveLauncher.tmp_path)\n        except:\n            pass\n        name = str(random.randint(0, 1e18))\n        self.inq_path = os.path.join(TInteractiveLauncher.tmp_path, name + ".inq")\n        self.log_path = os.path.join(TInteractiveLauncher.tmp_path, name + ".log")\n        \n        os.mkfifo(self.inq_path)\n        open(self.log_path, \'w\').close()\n        open(self.log_path + ".md", \'w\').close()\n\n        self.pid = os.fork()\n        if self.pid == -1:\n            print("Error")\n        if self.pid == 0:\n            exe_cands = glob.glob("../tools/launcher.py") + glob.glob("../../tools/launcher.py")\n            assert(len(exe_cands) == 1)\n            assert(os.execvp("python3", ["python3", exe_cands[0], "-l", self.log_path, "-i", self.inq_path, "-c", cmd]) == 0)\n        self.inq_f = open(self.inq_path, "w")\n        interactive_launcher_opened_set.add(self.pid)\n        show_log_file(self.log_path)\n\n    def write(self, s):\n        s = s.encode()\n        assert len(s) == os.write(self.inq_f.fileno(), s)\n        \n    def get_pid(self):\n        n = 100\n        for i in range(n):\n            try:\n                return int(re.findall(r"PID = (\\d+)", open(self.log_path).readline())[0])\n            except:\n                if i + 1 == n:\n                    raise\n                time.sleep(0.1)\n        \n    def input_queue_path(self):\n        return self.inq_path\n        \n    def close(self):\n        self.inq_f.close()\n        os.waitpid(self.pid, 0)\n        os.remove(self.inq_path)\n        # os.remove(self.log_path)\n        self.inq_path = None\n        self.log_path = None \n        interactive_launcher_opened_set.remove(self.pid)\n        self.pid = None\n        \n    @staticmethod\n    def terminate_all():\n        if "interactive_launcher_opened_set" not in globals():\n            globals()["interactive_launcher_opened_set"] = set()\n        global interactive_launcher_opened_set\n        for pid in interactive_launcher_opened_set:\n            print("Terminate pid=" + str(pid), file=sys.stderr)\n            os.kill(pid, signal.SIGKILL)\n            os.waitpid(pid, 0)\n        interactive_launcher_opened_set = set()\n        if os.path.exists(TInteractiveLauncher.tmp_path):\n            shutil.rmtree(TInteractiveLauncher.tmp_path)\n    \nTInteractiveLauncher.terminate_all()\n   \nyandex_metrica_allowed = bool(globals().get("yandex_metrica_allowed", False))\nif yandex_metrica_allowed:\n    display(HTML(\'\'\'<!-- YANDEX_METRICA_BEGIN -->\n    <script type="text/javascript" >\n       (function(m,e,t,r,i,k,a){m[i]=m[i]||function(){(m[i].a=m[i].a||[]).push(arguments)};\n       m[i].l=1*new Date();k=e.createElement(t),a=e.getElementsByTagName(t)[0],k.async=1,k.src=r,a.parentNode.insertBefore(k,a)})\n       (window, document, "script", "https://mc.yandex.ru/metrika/tag.js", "ym");\n\n       ym(59260609, "init", {\n            clickmap:true,\n            trackLinks:true,\n            accurateTrackBounce:true\n       });\n    </script>\n    <noscript><div><img src="https://mc.yandex.ru/watch/59260609" style="position:absolute; left:-9999px;" alt="" /></div></noscript>\n    <!-- YANDEX_METRICA_END -->\'\'\'))\n\ndef make_oneliner():\n    html_text = \'("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований. Если вы не хотите, чтобы по вам собиралась статистика, исправьте: yandex_metrica_allowed = False" if yandex_metrica_allowed else "")\'\n    html_text += \' + "<""!-- MAGICS_SETUP_PRINTING_END -->"\'\n    return \'\'.join([\n        \'# look at tools/set_up_magics.ipynb\\n\',\n        \'yandex_metrica_allowed = True ; get_ipython().run_cell(%s);\' % repr(one_liner_str),\n        \'display(HTML(%s))\' % html_text,\n        \' #\'\'MAGICS_SETUP_END\'\n    ])\n       \n\n');display(HTML(("В этот ноутбук встроен код Яндекс Метрики для сбора статистики использований. Если вы не хотите, чтобы по вам собиралась статистика, исправьте: yandex_metrica_allowed = False" if yandex_metrica_allowed else "") + "<""!-- MAGICS_SETUP_PRINTING_END -->")) #MAGICS_SETUP_END
```

# Адресация памяти в ассемблере

[Ридинг от Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/asm/arm_load_store)

Добавление от меня:

1. `str r0, [r1, #4]! (C-style: *(r1 += 4) = r0)` - то же самое, что и `str r0, [r1, #4] (C-style: *(r1 + 4) = r0)`, но в `r1`, будет сохранено `r1 + #4` после выполнения команды. Другими словами префиксный инкремент на 4.
1. `ldr r0, [r1], #4` - то же самое, что и `ldr r0, [r1] (C-style: r0 = *r1)` с последующим `add r1, r1, #4 (C-style: r1 += 4)`. Другими словами постфиксный инкремент.


```python
# Add path to compilers to PATH
import os
os.environ["PATH"] = os.environ["PATH"] + ":" + \
    "/home/pechatnov/Downloads/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi/bin/"

```

# Пример работы с массивом из ассемблера


```python
%%cpp is_sorted.c
%run arm-linux-gnueabi-gcc -marm is_sorted.c -o is_sorted.exe
%run qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./is_sorted.exe

#include <stdio.h>
#include <assert.h>

int is_sorted(int n, unsigned int* x);
__asm__ (R"(
.global is_sorted
is_sorted:
    // r0 - n, r1 - x
    cmp r0, #1
    bls is_sorted_true
    sub r0, r0, #1
    add r1, r1, #4
    ldr r2, [r1, #-4]
    ldr r3, [r1]
    cmp r2, r3
    bhi is_sorted_false
    b is_sorted
is_sorted_false:
    mov r0, #0
    bx lr
is_sorted_true:
    mov r0, #1
    bx  lr
)");

#define check(result, ...) {\
    unsigned int a[] = {__VA_ARGS__}; \
    int r = is_sorted(sizeof(a) / sizeof(int), a); \
    printf("is_sorted({" #__VA_ARGS__ "}) = %d ?= %d\n", r, result);\
    assert(r == result); \
}

#define check_sorted(...) check(1, __VA_ARGS__)
#define check_not_sorted(...) check(0, __VA_ARGS__)


int test() {
    check_sorted();
    check_sorted(10);
    check_sorted(10, 20);
    check_sorted(10, 20, 30);
    check_sorted(30, 30, 30);
    check_not_sorted(20, 10);
    check_not_sorted(10, 30, 20);
}

int main() {
    test();
    return 0;
}
```

# Пример приема более, чем 4 аргументов в функции

ip = r12. Почему он тут портится? У меня нет ответа `¯\_(ツ)_/¯ `


```python
%%cpp more_than_4.c
%run arm-linux-gnueabi-gcc -marm more_than_4.c -O2 -S -o more_than_4.s
%run cat more_than_4.s | grep -v "^\\s*\\." | grep -v "^\\s*@"

int mega_sum(int a1, int a2, int a3, int a4, int a5, int a6) {
    return a1 + a2 + a3 + a4 + a5 + a6;
}
```

# Пример чтения структуры из ассемблера


```python
%%cpp cut_struct.c
%run arm-linux-gnueabi-gcc -marm cut_struct.c -o cut_struct.exe
%run qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./cut_struct.exe

#include <stdio.h>
#include <assert.h>

struct Obj {
    char c;
    int i;
    short s;
    char c2;
} __attribute__((packed));

int cut_struct(struct Obj* obj, char* c, int* i, short* s, char* c2);
__asm__ (R"(
.global cut_struct
cut_struct:
    push {r4, r5} // notice that we decrease sp by pushing
    ldr r4, [sp, #8] // get last arg from stack (in fact we use initial value of sp)
    // r0 - obj, r1 - c, r2 - i, r3 - s, r4 - c2
    ldrb r5, [r0, #0]
    strb r5, [r1]
    ldr r5, [r0, #1]
    str r5, [r2]
    ldrh r5, [r0, #5]
    strh r5, [r3]
    ldrb r5, [r0, #7]
    strb r5, [r4]
    pop {r4, r5}
    bx  lr
)");

int test() {
    // designated initializers: https://en.cppreference.com/w/c/language/struct_initialization
    struct Obj obj = {.c = 123, .i = 100500, .s = 15000, .c2 = 67};
    char c = 0; int i = 0; short s = 0; char c2 = 0; // bad codestyle
    cut_struct(&obj, &c, &i, &s, &c2);
    fprintf(stderr, "Got c=%d, i=%d, s=%d, c2=%d", (int)c, (int)i, (int)s, (int)c2);
    assert(c == obj.c && i == obj.i && s == obj.s && c2 == obj.c2);
}

int main() {
    test();
    return 0;
}
```

# Дизассемблируем и смотрим на то, как код и данные размещаются в памяти


```python
%%cpp layout_example.c
%run arm-linux-gnueabi-gcc -marm layout_example.c -c -o layout_example.o

__asm__ (R"(
.global cut_struct
cut_struct:
    push {r4, r5} // notice that we decrease sp by pushing
    ldr r4, [sp, #8] // get last arg from stack (in fact we use initial value of sp)
    // r0 - obj, r1 - c, r2 - i, r3 - s, r4 - c2
    ldrb r5, [r0, #0]
    strb r5, [r1]
    ldr r5, [r0, #1]
    str r5, [r2]
    ldrh r5, [r0, #5]
    strh r5, [r3]
    ldrb r5, [r0, #7]
    strb r5, [r4]
    pop {r4, r5}
    push {r1-r12} // still one instruction
    pop {r1-r12}
    bx  lr
  
s1_ptr:
    .word s1
s2_ptr:
    .word s2
s1:
    .ascii "%d\n\0" // 4 bytes
s2:
    .ascii "%d%d\0" // 5 bytes
d1:
    .word 1234 // no padding
)");
```


```python
!arm-linux-gnueabi-objdump -D layout_example.o 2>&1 | grep cut_struct\> -A 35

```


```python

```

Можно сравнить с дизассемблером


```python
%%cpp cut_struct_disasm.c
%run arm-linux-gnueabi-gcc -marm cut_struct_disasm.c -O2 -S -o cut_struct_disasm.s
%run cat cut_struct_disasm.s | grep -v "^\\s*\\." | grep -v "^\\s*@"

struct Obj {
    char c;
    int i;
    short s;
    char c2;
} __attribute__((packed));

int cut_struct(struct Obj* obj, char* c, int* i, short* s, char* c2) {
    *c = obj->c;
    *i = obj->i;
    *s = obj->s;
    *c2 = obj->c2;
}
```

# Размещение структур в памяти

Не все всегда расположено очевидным образом: для более эффективного выполнения программы бывает выгодно выровненное расположение объектов в памяти, например считывать ui64 из памяти выгоднее, если адрес делится на 8.

Примерные правила:
* выравнивание (то, на что адрес должен делиться) равно размеру для простых арифметических типов (указатели тоже здесь)
* в union берется максимум для выравнивания (и максимум из размеров округенный вверх, чтобы делиться на выравнивание)
* в struct члены располагаются в том порядке, в котором указаны. Выравнивание структуры - максимум выравниваний. Каждый член располагается так, чтобы удовлетворять собственному выравниванию. Итоговый размер структуры делится на выравнивание структуры. С учетом этого размер струкуры минимизируется.

Для экспериментов можно использовать `sizeof()` и `_Alignof()`, чтобы получить размер и выравнивание.


```python
%%cpp structs_in_memory.c
%run arm-linux-gnueabi-gcc -marm structs_in_memory.c -o structs_in_memory.exe
%run qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./structs_in_memory.exe

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define print_int(x) printf(#x " = %d\n", (int)x)

#define print_offset(type, field) {\
    type o; \
    printf("Shift of ." #field " in " #type ": %d\n", (int)((void*)&o.field - (void*)&o)); \
}

int main() {
    print_int(sizeof(char));
    print_int(_Alignof(char));
    print_int(sizeof(short));
    print_int(_Alignof(short));
    print_int(sizeof(int));
    print_int(_Alignof(int));
    print_int(sizeof(long long));
    print_int(_Alignof(long long));
    print_int(sizeof(double));
    print_int(_Alignof(double));

    typedef struct { // максимальное выравнивание у инта, значит выравнивание структуры 4
        char c;      // 0 байт
        int i;       // 4-7 байты
        char c2;     // 8 байт
    } Obj1_t;        // 9-11 - padding байты, чтобы размер делился на выравнивание
    print_int(sizeof(Obj1_t));
    print_int(_Alignof(Obj1_t));
    print_offset(Obj1_t, c);
    print_offset(Obj1_t, i);
    print_offset(Obj1_t, c2);
    
    typedef struct { // тут все правила про выравнивание не применимы, так как указан аттрибут упаковки
        char c;
        int i;
        char c2;
    } __attribute__((packed)) Obj2_t;
    print_int(sizeof(Obj2_t));
    print_int(_Alignof(Obj2_t));
    print_offset(Obj2_t, c);
    print_offset(Obj2_t, i);
    print_offset(Obj2_t, c2);
    
    typedef struct {  // максимальное выравнивание члена - 8, так что и у всей структуры такое же
        char c8;      // 0 байт 
        uint64_t u64; // 8-15 байты
    } Obj3_t;         // всего 16 байт, выравнивание 8
    print_int(sizeof(Obj3_t));
    print_int(_Alignof(Obj3_t));
    print_offset(Obj3_t, u64);
    print_offset(Obj3_t, c8);
    
    typedef struct {
        char c8;
        char c8_1;
        char c8_2;
    } Obj4_t;
    print_int(sizeof(Obj4_t));
    print_int(_Alignof(Obj4_t));
    
    typedef struct {     // тут пример двух структур равного размера, но с разным выравниванием
        long long a;
    } ObjS8A8;
    print_int(sizeof(ObjS8A8));
    print_int(_Alignof(ObjS8A8));
    typedef struct {
        int a;
        int b;
    } ObjS8A4;
    print_int(sizeof(ObjS8A4));
    print_int(_Alignof(ObjS8A4));
    
    typedef struct {    // и вот тут разное выравнивание ObjS8A8 и ObjS8A4 себя покажет
        ObjS8A8 o;
        char c;
    } Obj5_t;
    print_int(sizeof(Obj5_t)); // обратите внимание на разницу с Obj6_t!
    print_int(_Alignof(Obj5_t));
    
    typedef struct {
        ObjS8A4 o;
        char c;
    } Obj6_t;
    print_int(sizeof(Obj6_t));
    print_int(_Alignof(Obj6_t));
    
    typedef union {
        unsigned long long u;
        int i[3];
    } Obj7_t;
    print_int(sizeof(Obj7_t));
    print_int(_Alignof(Obj7_t));
    
    return 0;
}
```

Попробовать _Atomic + `__packed__`

# Вызов функций


```python
%%cpp call.c
%run arm-linux-gnueabi-gcc -marm call.c -O2 -S -o call.s
%run cat call.s | grep -v "^\\s*\\." | grep -v "^\\s*@"

#include <stdio.h>

int print_a(char a) {
    fputc(a, stdout);
    return 1;
}
```

https://stackoverflow.com/questions/17214962/what-is-the-difference-between-label-equals-sign-and-label-brackets-in-ar

https://stackoverflow.com/questions/14046686/why-use-ldr-over-mov-or-vice-versa-in-arm-assembly


```python
%%cpp test_call.c
%run arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe
%run qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./test_call.exe

#include <stdio.h>

int print_a(char a);
__asm__(R"(
print_a:
    push {lr}
    ldr r3, =stdout
    ldr r1, [r3]
    bl fputc
    mov r0, #1
    pop {pc}
)");

int main() {
    print_a('?');
}
```

### Форматированный вывод


```python
%%cpp call.c
%run arm-linux-gnueabi-gcc -marm call.c -O2 -S -o call.s
%run cat call.s

#include <stdio.h>

int print_a(int a) {
    fprintf(stdout, "%d\n", a);
    return 42;
}
```


```python
%%cpp test_call.c
%run arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe
%run qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./test_call.exe

#include <stdio.h>

int print_a(int a);
__asm__(R"(
    .text
    .global print_a
print_a:
    mov r2, r0
        
    ldr r0, =stdout
    ldr r0, [r0]
    ldr r1, =.format_string
    
    push {lr}
    bl fprintf
    mov r0, #42
    pop {pc}
.format_string:
    .ascii "%d\n"
    .ascii "\0"
)");

int main() {
    print_a(100500);
}
```


```python

```

### Форматированное чтение


```python
%%cpp call.c
%run arm-linux-gnueabi-gcc -marm call.c -O2 -S -o call.s
%run cat call.s

#include <stdio.h>

int scan_a(int* a) {
    fscanf(stdin, "%d", a);
    return 42;
}
```


```python
%%cpp test_call.c
%run arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe
%run echo "123 124 125" | qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./test_call.exe

#include <stdio.h>

int scan_a(int* a);
__asm__(R"(
    .text
    .global scan_a
scan_a:
    mov r2, r0
    mov r3, r0
    ldr r0, =stdin
    ldr r0, [r0]
    ldr r1, =.format_string
    push {lr}
    push {r2}
    bl __isoc99_fscanf
    pop {r2}
    mov r0, #42
    pop {pc}
.format_string:
    .ascii "%d %d %d\0"
)");

int main() {
    int a = 100500;
    scan_a(&a);
    printf("a = %d\n", a);
}
```


```python
!arm-linux-gnueabi-objdump -D test_call.exe 2>&1 | grep 0001047c -A 20
```


```python
%%cpp test_call.c
%run arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe
%run echo "123 124 125" | qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./test_call.exe

#include <stdio.h>

int ret_eof();
__asm__(R"(
#include <stdio.h>
    .text
    .global ret_eof
ret_eof:
    mov r0, =EOF
    bx lr
)");

int main() {
    printf("%d\n", ret_eof());
}
```

# Решение одной домашней задачи


```python
%%asm sol.S
%run arm-linux-gnueabi-gcc -marm sol.S -O2 -o sol.exe
%run echo "123 124 125" | qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./sol.exe

.global main
main:
    push {lr}

cin_symb:
    ldr r0, =stdin
    ldr r0, [r0]
    bl fgetc

    cmp r0, #0
    blt out

    cmp r0, #'0'
    ble cin_symb

    cmp r0, #'9'
    bge cin_symb

    ldr r1, =stdout
    ldr r1, [r1]
    bl fputc
    b cin_symb
out:
    pop {pc}
```


```python

```


```python

```
