import cad
import xml.etree.ElementTree as ET
import wx
import os

def Export(path):
    # make model xml structure
    root = ET.Element('model')
    root.set('unit', 'millimeter')

    resources = ET.SubElement(root, 'resources')
    xmlobject = ET.SubElement(resources, 'object')
    xmlobject.set('id', '1')
    xmlobject.set('type', 'model')
    
    doc = cad.GetApp()
    object = doc.GetFirstChild()
    
    while object:
        tris = object.GetTris(0.01)
        if tris != None:
            mesh = tris.GetMesh()
            xmlmesh = ET.SubElement(xmlobject, 'mesh')
            xmlvertices = ET.SubElement(xmlmesh, 'vertices')
            vertices, faces = mesh.GetFaces()
            for vertex in vertices:
                xmlvertex = ET.SubElement(xmlvertices, 'vertex')
                xmlvertex.set('x', str(vertex[0]))
                xmlvertex.set('y', str(vertex[1]))
                xmlvertex.set('z', str(vertex[2]))
            xmltriangles = ET.SubElement(xmlmesh, 'triangles')
            for face in faces:
                if len(face) == 3:
                    xmltriangle = ET.SubElement(xmltriangles, 'triangle')
                    xmltriangle.set('v1', str(face[0]))
                    xmltriangle.set('v2', str(face[1]))
                    xmltriangle.set('v3', str(face[2]))
                
        object = doc.GetNextChild()
        
    # add a build section
    build = ET.SubElement(root, 'build')
    item = ET.SubElement(build, 'item')
    item.set('objectid', '1')
     
    if os.path.exists(path):
        os.remove(path)
        
    import zipfile
    import zlib        
    zf = zipfile.ZipFile(path, mode='w',compression=zipfile.ZIP_DEFLATED)
    zf.writestr('[Content_Types].xml','''<?xml version="1.0" encoding="UTF-8"?> 
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types"><Default ContentType="application/vnd.openxmlformats-package.relationships+xml" Extension="rels" /><Default ContentType="application/vnd.ms-package.3dmanufacturing-3dmodel+xml" Extension="model" /></Types>'''.encode('utf-8'))
    zf.writestr('3D/3dmodel.model', ET.tostring(root, encoding='utf8'))
    zf.writestr('_rels/.rels','''<?xml version="1.0" encoding="UTF-8"?> 
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships"><Relationship Id="rel0" Target="/3D/3dmodel.model" Type="http://schemas.microsoft.com/3dmanufacturing/2013/01/3dmodel" /></Relationships>'''.encode('utf-8'))
    
 