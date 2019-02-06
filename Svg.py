import cad

def GetFileName(path):
    p2 = path.replace('\\', '/')
    slash = p2.rfind('/')
    if slash == -1:
        return path
    return p2[slash+1:]
    
def Export(path):
    f = open(path, 'w')
    
    f.write("""<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<!-- Created with Inkscape (http://www.inkscape.org/) -->

<svg
   xmlns:dc="http://purl.org/dc/elements/1.1/"
   xmlns:cc="http://creativecommons.org/ns#"
   xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   width="210mm"
   height="297mm"
   viewBox="0 0 210 297"
   version="1.1"
   id="svg1348"
   inkscape:version="0.92.1 r15371"
   sodipodi:docname""")
    f.write('="'+GetFileName(path)+'">\n')
    f.write("""  <defs
     id="defs1342" />
  <sodipodi:namedview
     id="base"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     inkscape:pageopacity="0.0"
     inkscape:pageshadow="2"
     inkscape:zoom="3.959798"
     inkscape:cx="203.20224"
     inkscape:cy="997.78291"
     inkscape:document-units="mm"
     inkscape:current-layer="layer1"
     showgrid="false"
     inkscape:window-width="1920"
     inkscape:window-height="1017"
     inkscape:window-x="-8"
     inkscape:window-y="-8"
     inkscape:window-maximized="1" />
  <metadata
     id="metadata1345">
    <rdf:RDF>
      <cc:Work
         rdf:about="">
        <dc:format>image/svg+xml</dc:format>
        <dc:type
           rdf:resource="http://purl.org/dc/dcmitype/StillImage" />
        <dc:title />
      </cc:Work>
    </rdf:RDF>
  </metadata>
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">\n""")
    doc = cad.GetApp()
    object = doc.GetFirstChild()
    path_id = 1350
    
    cols = ['ff0000', '00ff00', '0000ff', 'ffff00', '00ffff', 'ff00ff']
    col_index = 0
    
    while object:
        if object.GetType() == cad.OBJECT_TYPE_SKETCH:
            f.write('    <path\n       style="fill:none;stroke:#' + cols[col_index] + ';stroke-width:0.26458332px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"\n')
            col_index += 1
            if col_index == len(cols):
                col_index = 0
            f.write('       d="m')
            first = True
            line = object.GetFirstChild()
            while line:
                if first:
                    s = line.GetStartPoint()
                    f.write(' ' + str(s.x) + ',' + str(-s.y))
                e = line.GetEndPoint()
                f.write(' ' + str(e.x - s.x) + ',' + str(-(e.y - s.y)))
                line = object.GetNextChild()
                s = e
                first = False
            f.write('"\n       id="path' + str(path_id) + '"\n')
            f.write('       inkscape:connector-curvature="0" />\n')
            path_id += 1
        object = doc.GetNextChild()
    f.write('  </g>\n</svg>\n')
    
    f.close()
    