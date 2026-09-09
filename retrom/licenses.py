"""Preserve upstream license texts without relabeling the combined work."""
import pathlib
import sys
root = pathlib.Path(__file__).resolve().parent.parent
paths = [('COPYING', 'utf-8'), ('doc/kero_src.txt', 'cp932'), ('fmgen/readme.txt', 'euc_jp')]
parts = ['PX68K contains components under different terms. See the complete upstream notices below.\n']
for name, encoding in paths:
    parts.append('\n===== ' + name + ' =====\n' + (root / name).read_text(encoding=encoding, errors='strict'))
pathlib.Path(sys.argv[1]).write_text('\n'.join(parts), encoding='utf-8')
