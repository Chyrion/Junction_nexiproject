import subprocess
import re
import platform

if (platform.system() == 'Linux'):
  p = subprocess.run(["./py/tlvp", "./py/data.txt"], capture_output=True)
elif (platform.system() == 'Windows'):
  p = subprocess.run(["./py/tlvp.exe", "./py/data.txt"], capture_output=True)
a = str(p)[78:]
b = a.split('\\x1b')
c = []

for text in b:
  m = re.sub(r'\[\d{1,2}[m;](\d{2}m)?(\\n)?(\s*)((\\[x])(.{2}))*|(\\[x])(.{2})|\[|\]|(Size:.*\n)|^\s/gm', '', str(text))
  m = re.sub(r'^\s*', '', m)
  m = re.sub(r'(S[a-z]{3}:\s)(\d{2})(\sbyte(s)?)', '', m)
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
