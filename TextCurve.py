import geom

def GetCharCurves(c, x, curves):
    if c == 32:
        return 0.3932235970718831;
    elif c == 33:
        c = geom.Curve()
        c.Append(geom.Point(x + 0.076404075922666, 1.0134529471612979))
        c.Append(geom.Point(x + 0.06902099358041801, 0.3302862280156991))
        curves.append(c)
        c = geom.Curve()
        c.Append(geom.Point(x + 0.072712534751542, 0.09394747113073515))
        c.Append(geom.Point(x + 0.076404075922666, 0.053326747349465145))
        curves.append(c)
        return 0.14402406447227198;        

def GetTextCurves(str):
    # given a text string, this returns a list of Curve objects
    
    curves = []
    
    x = 0.0
    for c in str:
        x += GetCharCurves(ord(c), x, curves)
        
    return curves