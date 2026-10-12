#!/usr/bin/env python3
"""Offline evaluation of a voluntarily supplied password. No radio or network IO."""
import argparse
import getpass
import hashlib
import json
import os
from pathlib import Path
import re
import sqlite3
import sys
import tempfile
import unicodedata
import warnings

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_DB = ROOT / '.redguard-local' / 'weakwords.sqlite3'
MAX_SOURCE = 16 * 1024 * 1024
MAX_WORDS = 100000
MAX_DB = 32 * 1024 * 1024
# Small, original demonstration list; this is not a comprehensive breach database.
COMMON = {'password', 'password123', '12345678', '123456789', '1234567890',
          'qwerty', 'qwerty123', 'qwertyuiop', 'admin', 'admin123', 'administrator',
          'welcome', 'welcome123', 'letmein', 'iloveyou', 'changeme', 'abc123',
          'abcdefgh', '88888888', '00000000', '11111111', 'wifi123456'}


def normalize(value):
    return unicodedata.normalize('NFKC', value).casefold()


def digest(value):
    return hashlib.sha256(normalize(value).encode('utf-8')).hexdigest()


def import_words(source, destination):
    """Replace the local dictionary atomically; only normalized word digests persist."""
    source, destination = Path(source), Path(destination)
    if source.resolve() == destination.resolve():
        raise ValueError('词库源文件与数据库不能相同')
    destination.parent.mkdir(parents=True, exist_ok=True)
    fd, temporary = tempfile.mkstemp(prefix='.words-', dir=destination.parent)
    os.close(fd)
    total = count = 0
    connection = None
    try:
        connection = sqlite3.connect(temporary)
        connection.execute('CREATE TABLE metadata (version INTEGER NOT NULL)')
        connection.execute('INSERT INTO metadata VALUES (1)')
        connection.execute('CREATE TABLE words (digest TEXT PRIMARY KEY) WITHOUT ROWID')
        with source.open('rb') as stream:
            while True:
                raw = stream.readline(1025)
                if not raw:
                    break
                total += len(raw)
                if len(raw) > 1024 or total > MAX_SOURCE:
                    raise ValueError('词库超出限制：总计 16 MiB，每行 1024 字节')
                word = raw.decode('utf-8-sig' if count == 0 else 'utf-8').rstrip('\r\n')
                if not word:
                    continue
                if any(unicodedata.category(c).startswith('C') for c in word):
                    raise ValueError('词库含控制字符；请使用每行一个词的 UTF-8 文本')
                count += 1
                if count > MAX_WORDS:
                    raise ValueError('词库最多允许 100000 个非空行')
                connection.execute('INSERT OR IGNORE INTO words VALUES (?)', (digest(word),))
        if not count:
            raise ValueError('词库没有有效条目')
        unique = connection.execute('SELECT count(*) FROM words').fetchone()[0]
        connection.commit()
        connection.close()
        connection = None
        os.replace(temporary, destination)
        return unique
    finally:
        if connection is not None:
            connection.close()
        Path(temporary).unlink(missing_ok=True)


class Dictionary:
    def __init__(self, path=None):
        self.connection = None
        self.count = 0
        if path is not None:
            path = Path(path).resolve()
            if not path.is_file() or path.stat().st_size > MAX_DB:
                raise ValueError('词库数据库不存在或超过 32 MiB')
            self.connection = sqlite3.connect(path.as_uri() + '?mode=ro', uri=True)
            try:
                self.connection.execute('PRAGMA trusted_schema=OFF')
                self.connection.execute('PRAGMA query_only=ON')
                # A supplied SQLite file must not define views, triggers or virtual tables.
                schema = self.connection.execute(
                    "SELECT name, sql FROM sqlite_master WHERE type != 'index' ORDER BY name").fetchall()
                expected = [('metadata', 'CREATE TABLE metadata (version INTEGER NOT NULL)'),
                            ('words', 'CREATE TABLE words (digest TEXT PRIMARY KEY) WITHOUT ROWID')]
                if schema != expected or self.connection.execute('SELECT version FROM metadata').fetchall() != [(1,)]:
                    raise ValueError('词库格式或版本不受支持，请重新导入文本')
                self.count = self.connection.execute('SELECT count(*) FROM words').fetchone()[0]
                if not 0 < self.count <= MAX_WORDS:
                    raise ValueError('词库条目数无效')
            except Exception:
                self.close()
                raise

    def contains(self, word):
        if normalize(word) in COMMON:
            return True
        return bool(self.connection and self.connection.execute(
            'SELECT 1 FROM words WHERE digest=?', (digest(word),)).fetchone())

    def close(self):
        if self.connection:
            self.connection.close()
            self.connection = None


def evaluate(password, dictionary):
    """Return redacted observations, never the password, candidates or its digest."""
    issues = []
    advice = []
    if len(password) > 256:
        raise ValueError('输入过长，最多 256 个字符')
    raw_key = bool(re.fullmatch(r'[0-9a-fA-F]{64}', password))
    ascii_phrase = 8 <= len(password) <= 63 and all(32 <= ord(c) <= 126 for c in password)
    result = {'schema_version': 1, 'scope': 'offline_supplied_password',
              'format': 'raw_psk' if raw_key else 'ascii_passphrase' if ascii_phrase else 'unsupported',
              'dictionary': {'builtin_demo_entries': len(COMMON), 'imported_entries': dictionary.count},
              'findings': issues, 'advice': advice,
              'limitations': ['不估算破解时间，不保证安全，不验证路由器实际密码。',
                              '不检测重复使用、个人信息或全部词库变体；词库未命中不能证明强度。']}
    if raw_key:
        result['assessment'] = '原始密钥：无法确认生成质量'
        advice.append('64 位十六进制值可能是原始 PSK；请使用可信随机生成器，不能由外观推断随机性。')
        return result
    if not ascii_phrase:
        issues.append('不符合本工具支持的 8–63 位可打印 ASCII Wi-Fi 口令格式')
        advice.append('非 ASCII 口令的兼容性需要核查路由器与客户端；本工具不自动转换口令。')
    normalized = normalize(password)
    # Detect only exact words and a word with an appended numeric/punctuation suffix.
    base = re.sub(r'[0-9\W_]+$', '', normalized)
    if dictionary.contains(password) or (len(base) >= 4 and dictionary.contains(base)):
        issues.append('命中内置示例／导入弱词库，或仅在弱词尾追加数字符号')
    if len(password) < 16:
        issues.append('长度不足 16 位，建议增加长度')
    if password.isdecimal():
        issues.append('全部由数字组成')
    if password and any(len(password) % n == 0 and password == password[:n] * (len(password)//n)
                        for n in range(1, min(4, len(password)//2)+1)):
        issues.append('由短片段重复组成')
    sequences = ('0123456789', 'abcdefghijklmnopqrstuvwxyz', 'qwertyuiop', 'asdfghjkl', 'zxcvbnm')
    if any(sequence[i:i+6] in normalized or sequence[i:i+6][::-1] in normalized
           for sequence in sequences for i in range(len(sequence)-5)):
        issues.append('含连续数字、字母或常见键盘序列')
    result['assessment'] = '建议修改' if issues else '已查项未发现明显弱点'
    advice.append('优先使用密码管理器生成独立的长随机口令，或随机选择多个无关词；不要仅添加年份和感叹号。')
    advice.append('仅凭字符种类和长度不能判断真实随机性；还需检查是否复用或公开。')
    return result


def save_report(path, report):
    # Exclusive creation avoids silently overwriting another file or following a symlink.
    with os.fdopen(os.open(path, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600), 'w') as stream:
        json.dump(report, stream, ensure_ascii=False, indent=2)
        stream.write('\n')


def main(argv=None):
    parser = argparse.ArgumentParser(description='红盾离线密码评估：仅输入本人或已同意测试者主动提供的口令。')
    sub = parser.add_subparsers(dest='command', required=True)
    imp = sub.add_parser('import', help='导入每行一词的 UTF-8 弱词库；替换原词库')
    imp.add_argument('source', type=Path)
    imp.add_argument('--db', type=Path, default=DEFAULT_DB)
    check = sub.add_parser('password', help='在终端隐藏输入，口令不进入命令行参数')
    check.add_argument('--db', type=Path)
    check.add_argument('--report', type=Path, help='可选：保存不含口令及其摘要的 JSON 报告')
    args = parser.parse_args(argv)
    try:
        if args.command == 'import':
            count = import_words(args.source, args.db)
            print(f'已导入 {count} 条唯一弱词摘要。仅用于离线匹配，不尝试连接网络。')
            return 0
        db = args.db if args.db is not None else DEFAULT_DB if DEFAULT_DB.exists() else None
        dictionary = Dictionary(db)
        try:
            # Never fall back to echoed input or accept a password in argv/stdin pipelines.
            if not sys.stdin.isatty():
                raise ValueError('请在交互式终端运行，以便隐藏输入密码')
            with warnings.catch_warnings():
                warnings.simplefilter('error', getpass.GetPassWarning)
                password = getpass.getpass('请输入主动提供的 Wi-Fi 口令（不回显）：')
            report = evaluate(password, dictionary)
            password = None  # Python cannot guarantee secure erasure of immutable strings.
            print(report['assessment'])
            for item in report['findings'] + report['advice'] + report['limitations']:
                print('• ' + item)
            print(f'词库覆盖：内置示例 {len(COMMON)} 条，导入 {dictionary.count} 条。')
            if args.report:
                save_report(args.report, report)
                print('已保存脱敏报告。')
        finally:
            dictionary.close()
        return 0
    except (ValueError, UnicodeError, OSError, sqlite3.Error, getpass.GetPassWarning):
        print('操作未完成：请检查终端、UTF-8 词库格式／大小和文件路径。原词库导入失败时保持不变。', file=sys.stderr)
        return 1
    except (EOFError, KeyboardInterrupt):
        print('\n已取消。', file=sys.stderr)
        return 130


if __name__ == '__main__':
    raise SystemExit(main())
