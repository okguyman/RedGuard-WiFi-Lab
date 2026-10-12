#!/usr/bin/env python3
"""Run portable RedGuard logic tests and relevant upstream regressions."""
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[1]
compiler = shutil.which('clang++') or shutil.which('g++')
if not compiler:
    raise SystemExit('Install Clang or GCC first')
source = root / 'esp32_marauder'
with tempfile.TemporaryDirectory(prefix='redguard-tests-') as temporary:
    for name, files in [
        ('model', ['RedGuardModel.cpp', 'RedGuardText.cpp']),
        ('scan', ['RedGuardScanModel.cpp']),
        ('pmf', ['RedGuardPmf.cpp']),
        ('risk', ['RedGuardRisk.cpp']),
    ]:
        output = Path(temporary) / name
        subprocess.run([compiler, '-std=c++17', '-Wall', '-Wextra', '-Werror',
                        '-fsanitize=undefined', '-I' + str(source),
                        str(root / 'test' / f'redguard_{name}_test.cpp'),
                        *[str(source / item) for item in files], '-o', str(output)], check=True)
        subprocess.run([str(output)], check=True)
subprocess.run([sys.executable, '-m', 'unittest', 'tools.test_installer_manifest',
                'tools.test_tft_workflow_configuration', 'tools.test_redguard_audit'], cwd=root, check=True)
