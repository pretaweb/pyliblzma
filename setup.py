#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Python Bindings for libLZMA
#
# Copyright (c) 2008 Per Øyvind Karlsen <peroyvind@mandriva.org>
# liblzma Copyright (C) 2007-2008  Lasse Collin
# LZMA SDK Copyright (C) 1999-2007 Igor Pavlov
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
import sys, os
from warnings import warn
from setuptools import setup, Extension
from distutils.ccompiler import get_default_compiler

try:
    import subprocess
    def popen(command):
        return subprocess.Popen(command,
               shell=True, stdout=subprocess.PIPE, close_fds=True).stdout
except ImportError:
    popen = os.popen

descr = "Python bindings for liblzma"
long_descr = """PylibLZMA provides a python interface for the liblzma library
to read and write data that has been compressed or can be decompressed
by Lasse Collin's xz / lzma utils."""
version = '0.5.3'
version_define = [('VERSION', '"%s"' % version)]

modules = ['liblzma']
c_files = ['liblzma.c', 'liblzma_compressobj.c', 'liblzma_decompressobj.c', 'liblzma_file.c', 'liblzma_fileobj.c', 'liblzma_options.c', 'liblzma_util.c']
for i in range(len(c_files)):
    c_files[i] = os.path.join('src', c_files[i])

compile_args = []
link_args = []
libraries = ['lzma']

p = subprocess.Popen("pkg-config --version", shell=True)
p.communicate()[0]
if p.returncode == 0:
    HAS_PKG_CONFIG = True
else:
    HAS_PKG_CONFIG = False

if get_default_compiler() in ('cygwin', 'emx', 'mingw32', 'unix'):
    warnflags = ['-Wall', '-Wextra', '-pedantic', '-Wswitch-enum', '-Wswitch-default']
    compile_args.extend(warnflags)
    
    if not subprocess.Popen('touch gcc_c89-test.c; gcc -std=c89 -E gcc_c89-test.c > /dev/null; rm -f gcc_c89-test.c',
            shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True).stdout.read():
        compile_args.extend(['-std=c89', '-Wno-long-long'])
    
    if HAS_PKG_CONFIG:
        p = subprocess.Popen("pkg-config --cflags liblzma", shell=True, stdout=subprocess.PIPE, close_fds=True)
        pc_cflags = p.communicate()[0].strip()
        if p.returncode == 1:
            raise RuntimeError("System library 'liblzma' not installed properly.")
        elif p.returncode == 0 and pc_cflags:
            compile_args.extend(str(pc_cflags).split(' '))
    
        p = subprocess.Popen("pkg-config --libs liblzma", shell=True, stdout=subprocess.PIPE, close_fds=True)
        pc_libs = p.communicate()[0].strip()
        if p.returncode == 1:
            raise RuntimeError("System library 'liblzma' not installed properly.")
        elif p.returncode == 0 and pc_libs:
            pc_libs = str(pc_libs).split(' ')
            pc_libs.remove(str('-llzma'))
            link_args.extend(pc_libs)

link_args.append('-lpython%i.%i' % sys.version_info[:2])

extens=[Extension('lzma', c_files, extra_compile_args=compile_args, libraries=libraries, extra_link_args=link_args, define_macros=version_define)]

setup(
    name = "pyliblzma",
    version = version,
    description = descr,
    author = "Per Øyvind Karlsen",
    author_email = "peroyvind@mandriva.org",
    url = "https://launchpad.net/pyliblzma",
    license = 'PSF',
    keywords = "xz lzma compression",
    long_description = long_descr,
    platforms = sys.platform,
    classifiers = [
        'Development Status :: 4 - Beta',
        'Programming Language :: Python',
        'Topic :: Software Development :: Libraries :: Python Modules',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: Python Software Foundation License',
        'Operating System :: POSIX :: Linux'
    ],
    py_modules = modules,
    ext_modules = extens,
    test_suite = 'tests',
)

