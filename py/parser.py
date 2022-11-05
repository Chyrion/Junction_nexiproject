import subprocess
import re

forbidden = ["[0m] ", '', '[0m\\n']

p = subprocess.run(["./py/tlvp", "./py/data.bin"], capture_output=True)
a = str(p)[78:]
b = a.split('\\x1b')
c = []

for text in b:
  m = re.sub(r'\[\d{1,2}[m;](\d{2}m)?(\\n)?(\s*)((\\[x])(.{2}))*|(\\[x])(.{2})', '', str(text))
  c.append(m)

while "   " in c:
  c.remove("   ")

while "" in c:
  c.remove("")

c.pop(len(c)-1)

for x in c:
  x.rstrip()

for x in c:
  x.lstrip()

print(c)
