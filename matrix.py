import io

class Matrix:
  def __init__(self, n):
    self.size = n
    self.data = []
    for i in range(n ** 2): self.data.append(0.0)

  def get(self, i, j): return self.data[self.size * i + j]
  def set(self, i, j, x): self.data[self.size * i + j] = float(x)
  def add(self, i, j, x): self.data[self.size * i + j] += x

  def __iter__(self):
    for i in range(self.size):
      for j in range(self.size):
        x = self.get(i, j)
        if x != 0.0: 
          yield (i, j, x)

def parse(f):
  header_line = str(f.readline())

  parts = header_line.split()

  if len(parts) == 1:
    mx = Matrix(int(parts[0]))

    for l in f.readlines():
      i, j, x = l.split()
      mx.add(int(i), int(j), float(x))

    return mx
  else:
    mx = Matrix(len(parts))

    def add_row(i, entries):
      if mx.size != len(entries):
        raise ParseError("Wrong number of columns in row %d. Expected %d but got %d" % (i, mx.size, len(entries)))
      for j in range(len(entries)): 
        mx.set(i, j, float(entries[j]))

    add_row(0, parts) 
    row = 1
    for l in f.readlines():
      add_row(row, str(l).split())
      row += 1

    return mx


class ParseError(Exception): pass    
