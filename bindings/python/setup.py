#!/usr/bin/env python

from distutils.core import setup

setup(name='graviton',
  version='0.1.0',
  description='Graviton pulls your network together',
  author='Trever Fischer',
  author_email='tdfischer@phrobo.net',
  url='http://aether.phrobo.net/',
  py_modules=['graviton'],
  scripts=['graviton-browse', 'graviton-call'],
)

