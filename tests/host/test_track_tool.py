"""Independent Python/C disk compatibility fixture checks."""
from pathlib import Path
import importlib.util
import struct
import zlib

root=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('track_tool',root/'tools/track_tool.py')
tool=importlib.util.module_from_spec(spec);spec.loader.exec_module(tool)
data=(root/'tests/vectors/home_track_v1.bin').read_bytes()
info,samples=tool.decode(data)
assert samples[0]==(0,5200,-14200,0,9000,0)
assert info['sample_count']==8
assert tool.verify_motion(samples,20000,0.5,2)['peak_velocity_rad_s']==0
for offset in (0,4,12,20,36,40):
    broken=bytearray(data);broken[offset]^=1
    try:tool.decode(broken)
    except ValueError:pass
    else:raise AssertionError(f'accepted corruption at {offset}')
for broken in (data[:-1],data+b'\0'):
    try:tool.decode(broken)
    except ValueError:pass
    else:raise AssertionError('accepted wrong length')
print(f'Python format/CRC/model/length verification PASS, model=0x{tool.model_id():08x}')
