"""Independent double precision homogeneous-matrix reference. Python standard library only.

Input is a pinned mechanical snapshot. No C constants or quaternion algorithm are reused.
"""
import csv
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

def multiply(a, b):
    return [[sum(a[i][k]*b[k][j] for k in range(4)) for j in range(4)] for i in range(4)]

def identity():
    return [[float(i==j) for j in range(4)] for i in range(4)]

def rotation(axis, angle):
    x,y,z=axis;c=math.cos(angle);s=math.sin(angle);d=1-c
    return [[c+x*x*d,x*y*d-z*s,x*z*d+y*s,0],
            [y*x*d+z*s,c+y*y*d,y*z*d-x*s,0],
            [z*x*d-y*s,z*y*d+x*s,c+z*z*d,0],[0,0,0,1]]

def quaternion(m):
    # Largest-diagonal extraction remains stable at 180 degrees.
    scores=[1+m[0][0]+m[1][1]+m[2][2],1+m[0][0]-m[1][1]-m[2][2],
            1-m[0][0]+m[1][1]-m[2][2],1-m[0][0]-m[1][1]+m[2][2]]
    k=max(range(4),key=lambda i:scores[i]);q=[0.0]*4;q[k]=math.sqrt(max(0,scores[k]))/2;den=4*q[k]
    if k==0:q[1:]=[(m[2][1]-m[1][2])/den,(m[0][2]-m[2][0])/den,(m[1][0]-m[0][1])/den]
    elif k==1:q[0]=(m[2][1]-m[1][2])/den;q[2]=(m[0][1]+m[1][0])/den;q[3]=(m[0][2]+m[2][0])/den
    elif k==2:q[0]=(m[0][2]-m[2][0])/den;q[1]=(m[0][1]+m[1][0])/den;q[3]=(m[1][2]+m[2][1])/den
    else:q[0]=(m[1][0]-m[0][1])/den;q[1]=(m[0][2]+m[2][0])/den;q[2]=(m[1][2]+m[2][1])/den
    return q if q[0]>=0 else [-v for v in q]

def solve(dim, degrees):
    offsets=[(0,0,0),(0,0,dim['BaseHeight']),(dim['UpperArm'],0,0),(dim['Forearm'],0,0),(dim['Wrist'],0,0),(dim['Flange'],0,0)]
    axes=[(0,0,1),(0,-1,0),(0,-1,0),(1,0,0),(0,-1,0),(1,0,0)]
    m=identity();origins=[];world_axes=[];joint_quaternions=[]
    for t,axis,q in zip(offsets,axes,degrees):
        shift=identity()
        for i in range(3):shift[i][3]=t[i]
        m=multiply(m,shift);origins.append([m[i][3] for i in range(3)])
        world_axes.append([sum(m[i][j]*axis[j] for j in range(3)) for i in range(3)])
        m=multiply(m,rotation(axis,math.radians(q)));joint_quaternions.append(quaternion(m))
    tcp=[m[i][3]+m[i][0]*dim['Tool'] for i in range(3)]
    return dict(q_rad=[math.radians(q) for q in degrees],joint_origins_mm=origins,joint_axes=world_axes,joint_quaternions=joint_quaternions,tcp_mm=tcp,tcp_quaternion=quaternion(m))

def initializer(value):
    if isinstance(value,list):return '{'+','.join(initializer(x) for x in value)+'}'
    s=format(value,'.12g')
    if '.' not in s and 'e' not in s:s+='.0'
    return s+'f'

def main():
    base=ROOT/'docs/mechanical_source';p=json.loads((base/'arm_0_parameters.json').read_text(encoding='utf-8'))
    vectors={name:solve(p['dimensions'],q) for name,q in p['poses'].items()}
    vectors['ZERO']=solve(p['dimensions'],[0]*6)
    # CAD HOME datum positions and axes are an external check of the Python chain.
    with (base/'joint_datums.csv').open(newline='',encoding='utf-8-sig') as f:
        for i,row in enumerate(csv.DictReader(f)):
            for j,axis in enumerate('xyz'):
                assert abs(float(row[f'origin_{axis}_mm'])-vectors['HOME']['joint_origins_mm'][i][j])<1e-8
                assert abs(float(row[f'axis_{axis}'])-vectors['HOME']['joint_axes'][i][j])<1e-8
    assert math.dist(vectors['CAKE_APPROACH']['tcp_mm'],p['cake_center_mm'])<1e-7
    out=ROOT/'tests/vectors';out.mkdir(parents=True,exist_ok=True)
    payload={'mechanical_sha':(base/'SOURCE_SHA').read_text().strip(),'units':'mm/rad','poses':vectors}
    (out/'mechanical_baseline.json').write_text(json.dumps(payload,indent=2)+'\n')
    keys=['q_rad','joint_origins_mm','joint_axes','joint_quaternions','tcp_mm','tcp_quaternion']
    header='#ifndef __MECHANICAL_BASELINE_H\n#define __MECHANICAL_BASELINE_H\n'
    header+='typedef struct { float q[6],origins[6][3],axes[6][3],rotations[6][4],tcp[3],quat[4]; } BaselineVector;\n'
    header+='static const BaselineVector baseline_vectors[] = {\n'
    header+=',\n'.join('{'+','.join(initializer(v[k]) for k in keys)+'}' for v in vectors.values())+'\n};\n#endif\n'
    (out/'mechanical_baseline.h').write_text(header)
    for name,v in vectors.items():print(name, 'TCP mm:', ', '.join(f'{x:.9f}' for x in v['tcp_mm']))
    print('Independent matrix reference and CAD HOME datums PASS')

if __name__=='__main__':main()
