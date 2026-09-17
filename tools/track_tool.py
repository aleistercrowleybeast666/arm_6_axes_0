"""A6TR v1 inspector. Standard library only; all disk fields are explicitly LE.

verify checks CRC, exact length, model, hard limits and timing. Optional --velocity
and --acceleration apply uniform joint limits to analytical Hermite extrema.
Those arguments are engineering inputs, not production defaults.
"""
import argparse
import csv
import json
import math
from pathlib import Path
import struct
import sys
import zlib

HEADER_BYTES = 40
MAX_SAMPLES = 4096
HARD_DEG = [(-170,170),(-15,100),(-150,100),(-180,180),(-120,120),(-180,180)]
GEOMETRY = [(0,0,0,0,0,1),(0,0,180,0,-1,0),(320,0,0,0,-1,0),
            (290,0,0,1,0,0),(60,0,0,0,-1,0),(86,0,0,1,0,0)]

def model_id():
    values=[6]
    for joint in GEOMETRY:
        values.extend(round(v*(1000 if i<3 else 1000000)) for i,v in enumerate(joint))
    values.append(90000)
    return zlib.crc32(struct.pack('<'+'i'*len(values),*values))

def decode(data):
    if len(data)<HEADER_BYTES: raise ValueError('truncated header')
    magic,version,size,joints,encoding,flags,period,count,model,payload,pcrc,reserved,hcrc=struct.unpack('<4sHHBBH7I',data[:40])
    if magic!=b'A6TR': raise ValueError('bad magic')
    if version!=1: raise ValueError(f'unsupported version {version}')
    if zlib.crc32(data[:36])!=hcrc: raise ValueError('header CRC mismatch')
    if (size,joints,encoding,flags,reserved)!=(40,6,1,0,0): raise ValueError('unsupported header/encoding/flags')
    if not 2<=count<=MAX_SAMPLES: raise ValueError('sample count out of range')
    if period not in (10000,20000,40000): raise ValueError('unsupported sample period')
    if model!=model_id(): raise ValueError('wrong mechanical model')
    if payload!=count*12 or len(data)!=40+payload: raise ValueError('payload length mismatch')
    if zlib.crc32(data[40:])!=pcrc: raise ValueError('payload CRC mismatch')
    samples=list(struct.iter_unpack('<6h',data[40:]))
    for row in samples:
        for angle,(lo,hi) in zip(row,HARD_DEG):
            if not lo*100<=angle<=hi*100: raise ValueError('joint hard limit exceeded')
    return {'version':version,'sample_period_us':period,'sample_count':count,'model_id':f'{model:08x}',
            'payload_crc32':f'{pcrc:08x}','duration_s':(count-1)*period/1e6},samples

def verify_motion(samples,period,velocity,acceleration):
    if any(v is not None and (not math.isfinite(v) or v<=0) for v in (velocity,acceleration)):
        raise ValueError('motion limits must be finite positive rad/s and rad/s^2')
    dt=period/1e6
    q=[[math.radians(v/100) for v in row] for row in samples]
    def tangent(i,j):
        if i==0 or i==len(q)-1:return 0
        a,b=q[i][j]-q[i-1][j],q[i+1][j]-q[i][j]
        return 0 if a*b<=0 else 2*a*b/(a+b)
    peak_v=peak_a=0
    for i in range(len(q)-1):
        for j in range(6):
            delta=q[i+1][j]-q[i][j];m,k=tangent(i,j),tangent(i+1,j)
            c2,c3=3*delta-2*m-k,-2*delta+m+k
            points=[0,1]
            if abs(c3)>1e-14:
                u=-c2/(3*c3)
                if 0<u<1:points.append(u)
            for u in points:
                peak_v=max(peak_v,abs((m+2*c2*u+3*c3*u*u)/dt))
                peak_a=max(peak_a,abs((2*c2+6*c3*u)/(dt*dt)))
    if velocity is not None and peak_v>velocity:raise ValueError('velocity limit exceeded')
    if acceleration is not None and peak_a>acceleration:raise ValueError('acceleration limit exceeded')
    return {'peak_velocity_rad_s':peak_v,'peak_acceleration_rad_s2':peak_a}

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command',choices=['info','verify','dump'])
    parser.add_argument('file',type=Path)
    parser.add_argument('--csv',type=Path)
    parser.add_argument('--velocity',type=float)
    parser.add_argument('--acceleration',type=float)
    args=parser.parse_args()
    try:
        info,samples=decode(args.file.read_bytes())
        info.update(verify_motion(samples,info['sample_period_us'],args.velocity,args.acceleration))
        if args.command=='dump':
            stream=args.csv.open('w',newline='',encoding='utf-8') if args.csv else sys.stdout
            try:
                writer=csv.writer(stream);writer.writerow(['time_s']+[f'j{i}_deg' for i in range(1,7)])
                for i,row in enumerate(samples):writer.writerow([i*info['sample_period_us']/1e6]+[v/100 for v in row])
            finally:
                if args.csv:stream.close()
        else:print(json.dumps(info,indent=2));print('VALID: device must still check commissioned soft/velocity/acceleration limits.')
        return 0
    except (OSError,ValueError,struct.error) as error:
        print(f'INVALID: {error}',file=sys.stderr);return 1

if __name__=='__main__':raise SystemExit(main())
