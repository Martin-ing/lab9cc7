#!/usr/bin/env python3
"""
visualize.py – Lee la salida de ./lab009 y genera ram_visual.html
con una grilla 2D de colores de los 100 frames.

Uso:
    ./lab009 8 example_addresses.txt 12345 | python3 visualize.py
  o con archivo:
    ./lab009 8 example_addresses.txt 12345 > output.txt
    python3 visualize.py output.txt
"""

import sys, re

# ---------- leer input ----------
text = open(sys.argv[1]).read() if len(sys.argv) > 1 else sys.stdin.read()

# ---------- parsear frames ----------
# tokens como:  0:F   16:X   8:2  ...
frames = {}
for token in re.findall(r'(\d+):([FX12])', text):
    idx, state = int(token[0]), token[1]
    if 0 <= idx <= 99:
        frames[idx] = state

# ---------- parsear PFNs por proceso ----------
pfn_proc = {}
for m in re.finditer(r'Load process \[PID=(\d+)\].*?PFNs \[(.*?)\]', text):
    pid = int(m.group(1))
    for pfn in re.findall(r'\d+', m.group(2)):
        pfn_proc[int(pfn)] = pid

# ---------- metadatos ----------
seed_m = re.search(r'seed=(\d+)', text)
free_m = re.search(r'FREE=(\d+)', text)
occ_m  = re.search(r'OCCUPIED=(\d+)', text)
seed_val = seed_m.group(1) if seed_m else '?'
free_val = free_m.group(1) if free_m else '?'
occ_val  = occ_m.group(1)  if occ_m  else '?'

# ---------- colores por estado ----------
COLOR = {
    'F': ('#dfe6e9', '#636e72', 'Libre'),
    'X': ('#e17055', '#ffffff', 'Ocupado (hueco)'),
    '1': ('#0984e3', '#ffffff', 'Proceso 1'),
    '2': ('#00b894', '#ffffff', 'Proceso 2'),
}

def cell(idx):
    state = frames.get(idx, 'F')
    if idx in pfn_proc:
        state = str(pfn_proc[idx])
    bg, fg, _ = COLOR[state]
    return (f'<td style="background:{bg};color:{fg}" title="Frame {idx}">'
            f'{idx}</td>')

# ---------- tabla 10x10 ----------
rows = ''
for r in range(10):
    rows += '<tr>' + ''.join(cell(r*10+c) for c in range(10)) + '</tr>\n'

# ---------- leyenda ----------
legend = ''.join(
    f'<span style="background:{bg};color:{fg};padding:4px 10px;'
    f'border-radius:4px;margin:4px;display:inline-block">{label}</span>'
    for bg, fg, label in COLOR.values()
)

# ---------- HTML ----------
html = f"""<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<title>Lab009 RAM Visual</title>
<style>
  body  {{ font-family: monospace; padding: 24px; background: #f8f9fa; }}
  h2    {{ color: #2d3436; }}
  table {{ border-collapse: collapse; margin-top: 16px; }}
  td    {{ width:52px; height:48px; text-align:center; font-size:13px;
           font-weight:bold; border:1px solid #b2bec3; border-radius:3px; cursor:default; }}
  td:hover {{ opacity:.8; outline:2px solid #fdcb6e; }}
  .stats   {{ margin:8px 0 16px; color:#636e72; font-size:14px; }}
  .legend  {{ margin-top:16px; }}
</style>
</head>
<body>
<h2>Physical RAM &ndash; 100 frames &nbsp; (seed={seed_val})</h2>
<div class="stats">FREE={free_val} &nbsp;|&nbsp; OCCUPIED={occ_val}</div>
<table>{rows}</table>
<div class="legend"><strong>Leyenda:</strong><br>{legend}</div>
</body>
</html>
"""

out = 'ram_visual.html'
with open(out, 'w') as f:
    f.write(html)

print(f'[visualize.py] Generado: {out}  ({len(frames)} frames parseados)')
