#!/usr/bin/env python3
"""Reproducible CYD 2USB build. Never erases, uploads or flashes a device."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import shutil
import subprocess
import zipfile

ROOT = Path(__file__).resolve().parents[1]
DEPENDENCIES = [
    ('ESP32Ping', 'marian-craciunescu/ESP32Ping', '1.6'),
    ('AsyncTCP', 'ESP32Async/AsyncTCP', 'v3.4.8'),
    ('MicroNMEA', 'stevemarple/MicroNMEA', 'v2.0.6'),
    ('ESPAsyncWebServer', 'ESP32Async/ESPAsyncWebServer', 'v3.8.1'),
    ('TFT_eSPI', 'Bodmer/TFT_eSPI', 'V2.5.34'),
    ('XPT2046_Touchscreen', 'PaulStoffregen/XPT2046_Touchscreen', 'v1.4'),
    ('lv_arduino', 'lvgl/lv_arduino', '3.0.0'),
    ('JPEGDecoder', 'Bodmer/JPEGDecoder', '1.8.0'),
    ('NimBLE-Arduino', 'h2zero/NimBLE-Arduino', '2.3.8'),
    ('Adafruit_NeoPixel', 'adafruit/Adafruit_NeoPixel', '1.12.0'),
    ('ArduinoJson', 'bblanchon/ArduinoJson', 'v6.18.2'),
    ('LinkedList', 'ivanseidel/LinkedList', 'v1.3.3'),
    ('EspSoftwareSerial', 'plerup/espsoftwareserial', '8.1.0'),
    ('Adafruit_BusIO', 'adafruit/Adafruit_BusIO', '1.15.0'),
    ('Adafruit_MAX1704X', 'adafruit/Adafruit_MAX1704X', '1.0.2'),
]

def run(*args):
    subprocess.run([str(arg) for arg in args], check=True)

def install_library(environment, name, repository, ref):
    dest = environment / 'user/libraries' / name
    archive = environment / 'archives' / (name + '.zip')
    archive.parent.mkdir(parents=True, exist_ok=True)
    if not archive.exists():
        run('curl', '--retry', '3', '--retry-all-errors', '--fail', '--location',
            '--silent', '--show-error', '--connect-timeout', '20', '--max-time', '300',
            f'https://codeload.github.com/{repository}/zip/{ref}', '-o', archive)
    with zipfile.ZipFile(archive) as z:
        prefix = z.namelist()[0].split('/')[0] + '/'
        for entry in z.infolist():
            rel = PurePosixPath(entry.filename[len(prefix):])
            if not str(rel) or str(rel) == '.':
                continue
            if rel.is_absolute() or '..' in rel.parts:
                raise ValueError('Unsafe archive path')
            target = dest / str(rel)
            if entry.is_dir():
                target.mkdir(parents=True, exist_ok=True)
            else:
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(z.read(entry))
    return dict(name=name, repository=repository, ref=ref,
                sha256=hashlib.sha256(archive.read_bytes()).hexdigest())

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--setup', action='store_true', help='Install pinned core/libraries before compiling')
    parser.add_argument('--environment', type=Path, default=ROOT / '.redguard-build')
    parser.add_argument('--output-dir', type=Path, default=ROOT / 'build/redguard')
    args = parser.parse_args()
    env = args.environment.resolve()
    output = args.output_dir.resolve()
    env.mkdir(parents=True, exist_ok=True)
    if not shutil.which('arduino-cli'):
        parser.error('Install official arduino-cli first; on macOS: brew install arduino-cli')
    config = env / 'arduino-cli.yaml'
    # JSON is valid YAML; structured serialization safely quotes paths with spaces.
    config.write_text(json.dumps({
        'board_manager': {'additional_urls': ['https://github.com/espressif/arduino-esp32/releases/download/3.3.4/package_esp32_dev_index.json']},
        'directories': {key: str(env / value) for key, value in [('data', 'data'), ('downloads', 'downloads'), ('user', 'user')]},
    }, indent=2))
    cli = ['arduino-cli', '--config-file', config]
    if args.setup:
        run(*cli, 'core', 'update-index')
        run(*cli, 'core', 'install', 'esp32:esp32@3.3.4')
        receipt = [install_library(env, *dependency) for dependency in DEPENDENCIES]
        shutil.copytree(ROOT / 'libraries/Adafruit_TCA8418', env / 'user/libraries/Adafruit_TCA8418', dirs_exist_ok=True)
        (env / 'dependencies.json').write_text(json.dumps(receipt, indent=2))
    core = env / 'data/packages/esp32/hardware/esp32/3.3.4'
    if not core.is_dir():
        parser.error('Core missing. Run with --setup.')
    tft = env / 'user/libraries/TFT_eSPI'
    if not tft.is_dir():
        parser.error('Libraries missing. Run with --setup.')
    for header in ROOT.glob('User*.h'):
        shutil.copy2(header, tft / header.name)
    setup = tft / 'User_Setup_Select.h'
    setup.write_text(setup.read_text().replace('//#include <User_Setup_cyd_2usb.h>', '#include <User_Setup_cyd_2usb.h>'))
    # Match upstream v1.18.0 CI for Arduino-ESP32 3.3.4; isolated environment only.
    platform = core / 'platform.txt'
    content = platform.read_text()
    if 'compiler.c.elf.extra_flags=-Wl,-zmuldefs ' not in content:
        platform.write_text(content.replace('compiler.c.elf.extra_flags=', 'compiler.c.elf.extra_flags=-Wl,-zmuldefs '))
    for flags in (env / 'data/packages/esp32/tools/esp32-arduino-libs').rglob('cpp_flags'):
        content = flags.read_text()
        if '-fexceptions' in content:
            flags.write_text(content.replace('-fexceptions', '-fno-exceptions'))
    run(*cli, 'compile', '--fqbn', 'esp32:esp32:d32:PartitionScheme=min_spiffs',
        '--warnings', 'none', '--build-property', 'compiler.cpp.extra_flags=-DMARAUDER_CYD_2USB',
        '--build-path', env / 'build', '--output-dir', output, ROOT / 'esp32_marauder')
    artifact = output / 'esp32_marauder.ino.bin'
    size = artifact.stat().st_size
    if size > 0x1e0000:
        raise RuntimeError('Application exceeds the original min_spiffs app slot')
    print(f'Application: {size} bytes; remaining: {0x1e0000-size} bytes')
    print('SHA256:', hashlib.sha256(artifact.read_bytes()).hexdigest())
    print('Build only. No flash operation performed.')

if __name__ == '__main__':
    main()
