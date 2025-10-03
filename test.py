import geom

a1 = geom.Area()
c = geom.Curve()
c.Append(geom.Point(0,0))
c.Append(geom.Point(10,0))
c.Append(geom.Point(5,10))
c.Append(geom.Point(0,0))
a1.Append(c)

a2 = geom.Area()
c = geom.Curve()
c.Append(geom.Point(5,5))
c.Append(geom.Point(10,15))
c.Append(geom.Point(0,15))
c.Append(geom.Point(5,5))
a2.Append(c)

a1.Union(a2)

print(str(a1))