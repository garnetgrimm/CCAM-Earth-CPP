import numpy
coords = [
    (0, 0),
    (1, 0.98),
    (2, 1.93),
    (3, 2.927),
    (4, 3.9),
    (4.917, 4.79)
]

xs = [c[1] for c in coords]
ys = [c[0] for c in coords]
p = numpy.poly1d(numpy.polyfit(xs, ys, deg=2))
print(p)