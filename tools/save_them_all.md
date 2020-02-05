```python
import glob
import os
import subprocess

def convert(n, d):
    get_ipython().system("jupyter nbconvert {} --to markdown --output {}".format(n, d))
    #subprocess.check_call(["jupyter", "nbconvert", n, "--to", "markdown", "--output", d])

highlevel_dirs = ["../tools"] + sorted(glob.glob("../sem16*"))

print("Highlevel dirs:", highlevel_dirs)
```

    Highlevel dirs: ['../tools', '../sem16-fcntl-dup-pipe']



```python
for subdir in highlevel_dirs:
    notebooks = glob.glob(subdir + "/*.ipynb")
    print(subdir, notebooks)
    for m in glob.glob(subdir + "/*.md"):
        os.remove(m)
    if len(notebooks) == 1:
        convert(notebooks[0], "README")
    else:
        for n in notebooks:
            convert(n, os.path.basename(n.replace(".ipynb", "")))
        
```

    ../tools ['../tools/set_up_magics.ipynb', '../tools/set_up_magics_dev.ipynb', '../tools/save_them_all.ipynb']
    [NbConvertApp] Converting notebook ../tools/set_up_magics.ipynb to markdown
    [NbConvertApp] Writing 50748 bytes to ../tools/set_up_magics.md
    [NbConvertApp] Converting notebook ../tools/set_up_magics_dev.ipynb to markdown
    [NbConvertApp] Writing 32 bytes to ../tools/set_up_magics_dev.md
    [NbConvertApp] Converting notebook ../tools/save_them_all.ipynb to markdown
    [NbConvertApp] Writing 10010 bytes to ../tools/save_them_all.md
    ../sem16-fcntl-dup-pipe ['../sem16-fcntl-dup-pipe/16.ipynb', '../sem16-fcntl-dup-pipe/fcntl-dup-pipe.ipynb']
    This application is used to convert notebook files (*.ipynb) to various other
    formats.
    
    WARNING: THE COMMANDLINE INTERFACE MAY CHANGE IN FUTURE RELEASES.
    
    Options
    -------
    
    Arguments that take values are actually convenience aliases to full
    Configurables, whose aliases are listed on the help line. For more information
    on full configurables, see '--help-all'.
    
    --debug
        set log level to logging.DEBUG (maximize logging output)
    --no-input
        Exclude input cells and output prompts from converted document. 
        This mode is ideal for generating code-free reports.
    -y
        Answer yes to any questions instead of prompting.
    --stdin
        read a single notebook file from stdin. Write the resulting notebook with default basename 'notebook.*'
    --inplace
        Run nbconvert in place, overwriting the existing notebook (only 
        relevant when converting to notebook format)
    --no-prompt
        Exclude input and output prompts from converted document.
    --stdout
        Write notebook output to stdout instead of files.
    --execute
        Execute the notebook prior to export.
    --generate-config
        generate default config file
    --clear-output
        Clear output of current file and save in place, 
        overwriting the existing notebook.
    --allow-errors
        Continue notebook execution even if one of the cells throws an error and include the error message in the cell output (the default behaviour is to abort conversion). This flag is only relevant if '--execute' was specified, too.
    --writer=<DottedObjectName> (NbConvertApp.writer_class)
        Default: 'FilesWriter'
        Writer class used to write the  results of the conversion
    --to=<Unicode> (NbConvertApp.export_format)
        Default: 'html'
        The export format to be used, either one of the built-in formats
        ['asciidoc', 'custom', 'html', 'latex', 'markdown', 'notebook', 'pdf',
        'python', 'rst', 'script', 'slides'] or a dotted object name that represents
        the import path for an `Exporter` class
    --reveal-prefix=<Unicode> (SlidesExporter.reveal_url_prefix)
        Default: ''
        The URL prefix for reveal.js (version 3.x). This defaults to the reveal CDN,
        but can be any url pointing to a copy  of reveal.js.
        For speaker notes to work, this must be a relative path to a local  copy of
        reveal.js: e.g., "reveal.js".
        If a relative path is given, it must be a subdirectory of the current
        directory (from which the server is run).
        See the usage documentation
        (https://nbconvert.readthedocs.io/en/latest/usage.html#reveal-js-html-
        slideshow) for more details.
    --output-dir=<Unicode> (FilesWriter.build_directory)
        Default: ''
        Directory to write output(s) to. Defaults to output to the directory of each
        notebook. To recover previous default behaviour (outputting to the current
        working directory) use . as the flag value.
    --config=<Unicode> (JupyterApp.config_file)
        Default: ''
        Full path of a config file.
    --template=<Unicode> (TemplateExporter.template_file)
        Default: ''
        Name of the template file to use
    --output=<Unicode> (NbConvertApp.output_base)
        Default: ''
        overwrite base name use for output files. can only be used when converting
        one notebook at a time.
    --post=<DottedOrNone> (NbConvertApp.postprocessor_class)
        Default: ''
        PostProcessor class used to write the results of the conversion
    --nbformat=<Enum> (NotebookExporter.nbformat_version)
        Default: 4
        Choices: [1, 2, 3, 4]
        The nbformat version to write. Use this to downgrade notebooks.
    --log-level=<Enum> (Application.log_level)
        Default: 30
        Choices: (0, 10, 20, 30, 40, 50, 'DEBUG', 'INFO', 'WARN', 'ERROR', 'CRITICAL')
        Set the log level by value or name.
    
    To see all available configurables, use `--help-all`
    
    Examples
    --------
    
        The simplest way to use nbconvert is
        
        > jupyter nbconvert mynotebook.ipynb
        
        which will convert mynotebook.ipynb to the default format (probably HTML).
        
        You can specify the export format with `--to`.
        Options include ['asciidoc', 'custom', 'html', 'latex', 'markdown', 'notebook', 'pdf', 'python', 'rst', 'script', 'slides'].
        
        > jupyter nbconvert --to latex mynotebook.ipynb
        
        Both HTML and LaTeX support multiple output templates. LaTeX includes
        'base', 'article' and 'report'.  HTML includes 'basic' and 'full'. You
        can specify the flavor of the format used.
        
        > jupyter nbconvert --to html --template basic mynotebook.ipynb
        
        You can also pipe the output to stdout, rather than a file
        
        > jupyter nbconvert mynotebook.ipynb --stdout
        
        PDF is generated via latex
        
        > jupyter nbconvert mynotebook.ipynb --to pdf
        
        You can get (and serve) a Reveal.js-powered slideshow
        
        > jupyter nbconvert myslides.ipynb --to slides --post serve
        
        Multiple notebooks can be given at the command line in a couple of 
        different ways:
        
        > jupyter nbconvert notebook*.ipynb
        > jupyter nbconvert notebook1.ipynb notebook2.ipynb
        
        or you can specify the notebooks list in a config file, containing::
        
            c.NbConvertApp.notebooks = ["my_notebook.ipynb"]
        
        > jupyter nbconvert --config mycfg.py
    
    [NbConvertApp] CRITICAL | Bad config encountered during initialization:
    [NbConvertApp] CRITICAL | The 'output_base' trait of a NbConvertApp instance must be a unicode string, but a value of 16 <class 'int'> was specified.
    [NbConvertApp] Converting notebook ../sem16-fcntl-dup-pipe/fcntl-dup-pipe.ipynb to markdown
    [NbConvertApp] Writing 184941 bytes to ../sem16-fcntl-dup-pipe/fcntl-dup-pipe.md



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
    r = r.replace('\n', "SUPER_SLASH" + "_N_REPLACER")
    r = re.sub(r'\<\!--MD_BEGIN_FILTER--\>.*?\<\!--MD_END_FILTER--\>', "", r)
    r = re.sub(r'(\<too much code>)', "<too much code>", r)
    r = r.replace("SUPER_SLASH" + "_N_REPLACER", '\n')
    
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

    dos2unix: converting file ./../tools/set_up_magics.ipynb to Unix format ...
    dos2unix: converting file ./../tools/set_up_magics_dev.ipynb to Unix format ...
    dos2unix: converting file ./../tools/save_them_all.ipynb to Unix format ...
    dos2unix: converting file ./../sem16-fcntl-dup-pipe/16.ipynb to Unix format ...
    dos2unix: converting file ./../sem16-fcntl-dup-pipe/fcntl-dup-pipe.ipynb to Unix format ...
    dos2unix: converting file ./../tools/set_up_magics_dev.md to Unix format ...
    dos2unix: converting file ./../tools/set_up_magics.md to Unix format ...
    dos2unix: converting file ./../tools/save_them_all.md to Unix format ...
    dos2unix: converting file ./../sem16-fcntl-dup-pipe/fcntl-dup-pipe.md to Unix format ...



```python
add_cmd = "git add --ignore-errors "
add_cmd_f = "git add"
for subdir in highlevel_dirs:
    for sfx in [".ipynb", ".md", ".c", ".cpp"]:
        add_cmd += " {}/*{}".format(subdir, sfx)
    add_cmd_f += " -f {}/bash_popen_tmp/*.html".format(subdir)
    
def execute_cmd(cmd):
    print(">", cmd)
    get_ipython().system(cmd)
    
execute_cmd(add_cmd)
execute_cmd(add_cmd_f)
execute_cmd("git add -u")
execute_cmd("git commit -m 'yet another update'")
execute_cmd("git push origin master")
```

    > git add --ignore-errors  ../tools/*.ipynb ../tools/*.md ../tools/*.c ../tools/*.cpp ../sem16-fcntl-dup-pipe/*.ipynb ../sem16-fcntl-dup-pipe/*.md ../sem16-fcntl-dup-pipe/*.c ../sem16-fcntl-dup-pipe/*.cpp
    fatal: pathspec '../tools/*.c' did not match any files
    > git add -f ../tools/bash_popen_tmp/*.html -f ../sem16-fcntl-dup-pipe/bash_popen_tmp/*.html
    fatal: pathspec '../sem16-fcntl-dup-pipe/bash_popen_tmp/*.html' did not match any files
    > git add -u
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/README.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/dup.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/dup2.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/fcntl-dup-pipe.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/fcntl_dup.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/fcntl_flags.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/fcntl_open_flags.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/pipe.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/pipe2.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/save_them_all.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/save_them_all.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/set_up_magics.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/set_up_magics.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/set_up_magics_dev.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/set_up_magics_dev.md.
    The file will have its original line endings in your working directory.
    > git commit -m 'yet another update'
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/README.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/fcntl-dup-pipe.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/fcntl_dup.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/fcntl_flags.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/pipe2.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/save_them_all.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/save_them_all.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/pipe2.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/save_them_all.md.
    The file will have its original line endings in your working directory.
    [master dc90c38] yet another update
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/README.md.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/fcntl-dup-pipe.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/fcntl_dup.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/fcntl_flags.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in sem16-fcntl-dup-pipe/pipe2.cpp.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/save_them_all.ipynb.
    The file will have its original line endings in your working directory.
    warning: LF will be replaced by CRLF in tools/save_them_all.md.
    The file will have its original line endings in your working directory.
     7 files changed, 659 insertions(+), 441 deletions(-)
     rewrite tools/save_them_all.md (86%)
    > git push origin master
    Counting objects: 11, done.
    Compressing objects: 100% (11/11), done.
    Writing objects: 100% (11/11), 16.82 KiB | 0 bytes/s, done.
    Total 11 (delta 8), reused 0 (delta 0)
    remote: Resolving deltas: 100% (8/8), completed with 7 local objects.[K
    To git@github.com:yuri-pechatnov/caos_2019-2020.git
       2d11e0f..dc90c38  master -> master



```python

```


```python

```


```python

```
