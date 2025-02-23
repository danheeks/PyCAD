import cad
import geom


def GetFileName(path):
    p2 = path.replace('\\', '/')
    slash = p2.rfind('/')
    if slash == -1:
        return path
    return p2[slash+1:]
    
  
    
def Export(path):
    f = open(path, 'w')
    
    f.write("// shape code created by danheeks software https://github.com/danheeks/PyCAD\n\n")
    doc = cad.GetApp()
    object = doc.GetFirstChild()
    
    f.write('    ctx.fillStyle = color;\n')
    f.write('    ctx.beginPath();\n')
    
    while object:
        if object.GetType() == cad.OBJECT_TYPE_SKETCH:
            span = object.GetFirstChild()
            if span:
                s = span.GetStartPoint()
                f.write('    ctx.moveTo(' + "{:.2f}".format(s.x) + '+x,' + "{:.2f}".format(32 - s.y) + '+y);\n')
                while span:
                    e = span.GetEndPoint()
                    f.write('    ctx.lineTo(' + "{:.2f}".format(e.x) + '+x,' + "{:.2f}".format(32 - e.y) + '+y);\n')
                    span = object.GetNextChild()                
                
        object = doc.GetNextChild()

    f.write('    ctx.fill();\n')
    
    f.close()
    