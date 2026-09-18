"""Package already-built official extension files; no caches or library copies."""
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile, ZipInfo

ROOT = Path(__file__).resolve().parents[1]
extension = ROOT/'easyeda/extension'
output = ROOT/'easyeda/arm-six-axes-reva-schematic.eext'
with ZipFile(output, 'w', compression=ZIP_DEFLATED) as archive:
    for name in ['extension.json', 'dist/index.js']:
        info = ZipInfo(name, (2026, 9, 19, 0, 0, 0))
        info.compress_type = ZIP_DEFLATED
        archive.writestr(info, (extension/name).read_bytes())
print(output)
