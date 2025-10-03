import geom

# two triangles

#a1 = geom.Area()
#c = geom.Curve()
#c.Append(geom.Point(0,0))
#c.Append(geom.Point(10,0))
#c.Append(geom.Point(5,10))
#c.Append(geom.Point(0,0))
#a1.Append(c)

#a2 = geom.Area()
#c = geom.Curve()
#c.Append(geom.Point(5,5))
#c.Append(geom.Point(10,15))
#c.Append(geom.Point(0,15))
#c.Append(geom.Point(5,5))
#a2.Append(c)

#a1.Union(a2)

#a3 = geom.Area()
#c = geom.Curve()
#c.Append(geom.Point(0, 0))
#c.Append(geom.Point(10, 0))
#c.Append(geom.Point(6.25, 7.5))
#c.Append(geom.Point(10, 15))
#c.Append(geom.Point(0, 15))
#c.Append(geom.Point(3.75, 7.5))
#c.Append(geom.Point(0, 0))
#a3.Append(c)

#print(str(a1) == str(a3))

# two doughnuts
a1 = geom.Area()
c = geom.Curve()
c.Append(geom.Point(0, -14))
c.Append(geom.Vertex(1, geom.Point(0,14), geom.Point(0,0)))
c.Append(geom.Vertex(1, geom.Point(0,-14), geom.Point(0,0)))
a1.Append(c)
c = geom.Curve()
c.Append(geom.Point(0, -10))
c.Append(geom.Vertex(-1, geom.Point(0,10), geom.Point(0,0)))
c.Append(geom.Vertex(-1, geom.Point(0,-10), geom.Point(0,0)))
a1.Append(c)

a2 = geom.Area()
c = geom.Curve()
c.Append(geom.Point(10, -14))
c.Append(geom.Vertex(1, geom.Point(10,14), geom.Point(10,0)))
c.Append(geom.Vertex(1, geom.Point(10,-14), geom.Point(10,0)))
a2.Append(c)
c = geom.Curve()
c.Append(geom.Point(10, -10))
c.Append(geom.Vertex(-1, geom.Point(10,10), geom.Point(10,0)))
c.Append(geom.Vertex(-1, geom.Point(10,-10), geom.Point(10,0)))
a2.Append(c)

a1.Union(a2)

print(str(a1))
