import hashlib
import json
from pathlib import Path
import sqlite3
import tempfile
import unittest
from tools.redguard_audit import Dictionary, evaluate, import_words, save_report, MAX_WORDS


class AuditTests(unittest.TestCase):
    def test_common_and_trivial_suffix(self):
        dictionary = Dictionary()
        for value in ('Password123!', 'qwertyuiop', '0000000000000000', '12345678'):
            self.assertEqual(evaluate(value, dictionary)['assessment'], '建议修改')
        self.assertTrue(any('重复' in s for s in evaluate('ab'*12, dictionary)['findings']))

    def test_no_randomness_claim_or_composition_rule(self):
        report = evaluate('ottermangobreezefalcon', Dictionary())
        self.assertEqual(report['assessment'], '已查项未发现明显弱点')
        self.assertEqual(report['format'], 'ascii_passphrase')
        self.assertNotIn('entropy', report)
        self.assertEqual(evaluate('a'*64, Dictionary())['format'], 'raw_psk')
        self.assertNotEqual(evaluate('a'*64, Dictionary())['assessment'], '已查项未发现明显弱点')
        for text in ('short', '测试密码1234567890', 'a'*65, 'abc\n1234567890123'):
            self.assertEqual(evaluate(text, Dictionary())['format'], 'unsupported')

    def test_import_normalization_and_redaction(self):
        with tempfile.TemporaryDirectory() as tmp:
            source, db = Path(tmp)/'words.txt', Path(tmp)/'words.db'
            source.write_text('\ufeffHarborTangerine\r\nHARBORTANGERINE\nＡｄｍｉｎ\n', encoding='utf-8')
            self.assertEqual(import_words(source, db), 2)
            dictionary = Dictionary(db)
            try:
                value = 'HarborTangerine2026!'
                report = evaluate(value, dictionary)
                self.assertTrue(any('词库' in s for s in report['findings']))
                serialized = json.dumps(report)
                self.assertNotIn(value, serialized)
                self.assertNotIn(hashlib.sha256(value.encode()).hexdigest(), serialized)
                self.assertTrue(dictionary.contains('harbortangerine'))
                self.assertEqual(dictionary.count, 2)
            finally:
                dictionary.close()
            self.assertNotIn(b'HarborTangerine', db.read_bytes())

    def test_failed_import_keeps_existing_database(self):
        with tempfile.TemporaryDirectory() as tmp:
            source, db = Path(tmp)/'words.txt', Path(tmp)/'words.db'
            source.write_text('exampleword\n')
            import_words(source, db)
            original = db.read_bytes()
            for payload in (b'', b'a'*1025, b'\xff', b'abc\x00xyz\n'):
                source.write_bytes(payload)
                with self.assertRaises((ValueError, UnicodeError)):
                    import_words(source, db)
                self.assertEqual(db.read_bytes(), original)
            source.write_bytes(b'a\n'*(MAX_WORDS+1))
            with self.assertRaises(ValueError):
                import_words(source, db)
            self.assertEqual(db.read_bytes(), original)
            self.assertEqual(list(Path(tmp).glob('.words-*')), [])

    def test_reject_unrecognized_database(self):
        with tempfile.TemporaryDirectory() as tmp:
            db = Path(tmp)/'words.db'
            with sqlite3.connect(db) as c:
                c.execute('CREATE VIEW words AS SELECT 1')
            with self.assertRaises(ValueError):
                Dictionary(db)

    def test_report_does_not_overwrite(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp)/'report.json'
            report = evaluate('ottermangobreezefalcon', Dictionary())
            save_report(path, report)
            self.assertEqual(json.loads(path.read_text()), report)
            self.assertEqual(path.stat().st_mode & 0o777, 0o600)
            with self.assertRaises(FileExistsError):
                save_report(path, report)


if __name__ == '__main__':
    unittest.main()
