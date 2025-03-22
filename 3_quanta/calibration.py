import numpy
coords = [
    (0, 0.035),
    (0.2, 0.20),
    (0.5, 0.492),
    (1.0, 0.98),
    (2.0, 1.953),
    (3.0, 2.928),
    (4.0, 3.93),
    (4.5, 4.38)
]

xs = [c[1] for c in coords]
ys = [c[0] for c in coords]
p = numpy.poly1d(numpy.polyfit(xs, ys, deg=2))
print(p)