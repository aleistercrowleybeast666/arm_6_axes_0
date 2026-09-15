"""Write build-time version metadata. Source SHA denotes HEAD, with -dirty when modified."""
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def git(*args):
    r=subprocess.run(['git','-C',str(ROOT),*args],text=True,capture_output=True)
    return r.stdout.strip() if r.returncode==0 else 'uncommitted'
sha=git('rev-parse','HEAD')
if git('status','--porcelain'):sha+='-dirty'
mechanical=(ROOT/'docs/mechanical_source/SOURCE_SHA').read_text().strip()
out=ROOT/'build/generated';out.mkdir(parents=True,exist_ok=True)
(out/'version_build.h').write_text(f'#ifndef __VERSION_BUILD_H\n#define __VERSION_BUILD_H\n#define ARM_FIRMWARE_SHA "{sha}"\n#define ARM_MECHANICAL_SHA "{mechanical}"\n#endif\n')
print(f'firmware {sha}; mechanical {mechanical}')
