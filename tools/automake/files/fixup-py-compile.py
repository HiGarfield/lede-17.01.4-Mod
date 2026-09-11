#!/usr/bin/env python3
# Fix automake 1.15's py-compile for Python 3.12+.
#
# py-compile is a shell script that byte-compiles installed .py files by
# running two inline "$PYTHON -c '...'" snippets.  Those snippets import the
# removed 'imp' module (imp.cache_from_source / hasattr(imp, 'get_tag')),
# which no longer exists on Python 3.12, so every package that installs
# python sources via automake (e.g. glib's gdbus-codegen) fails at
# "make install" with "ModuleNotFoundError: No module named 'imp'".
#
# py_compile.compile() already computes the correct cache path itself,
# honouring sys.flags.optimize for the "-O" invocation, so the imp branch
# can simply be dropped.  This rewrites the installed script in place and is
# safe to run more than once.

import re
import sys

path = sys.argv[1]
with open(path, "r") as f:
    s = f.read()

s = s.replace("import sys, os, py_compile, imp", "import sys, os, py_compile")

s = re.sub(
    r"\n\s*if hasattr\(imp, 'get_tag'\):\n"
    r"\s*py_compile\.compile\(filepath, imp\.cache_from_source\(filepath(?:, False)?\), path\)\n"
    r"\s*else:\n"
    r"\s*py_compile\.compile\(filepath, filepath \+ '[co]', path\)",
    "\n   py_compile.compile(filepath, dfile=path)",
    s,
)

with open(path, "w") as f:
    f.write(s)
