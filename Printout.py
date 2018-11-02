import wx
import copy
import cad

class Printout(wx.Printout):
    def __init__(self):
        wx.Printout.__init__(self, 'Heeks printout')
    
    # overriding
    def GetPageInfo(self):
        return (1,1,1,1) # minPage, maxPage, pageFrom, pageTo
    
    def HasPage(self, page):
        return page == 1 or page == 2
    
    def OnPrintPage(self, page):
        wx.GetApp().frame.printout = self
        dc = self.GetDC()
        if dc:
            self.SetUnitsFactor()
            cad.SetGetLinesPixelsPerMm(1.0/self.scale)
            
            for start, x, y, z in cad.GetApp().GetLines():
                if not start:
                    self.GetDC().DrawLine(self.xoff + (px * self.logUnitsFactorx + 0.5), self.yoff - (py * self.logUnitsFactory + 0.5), self.xoff + (x * self.logUnitsFactorx + 0.5), self.yoff - (y * self.logUnitsFactory + 0.5))
                px = x
                py = y

            return True
        else:
            return False
        
    def SetUnitsFactor(self):
        dc = self.GetDC()
        
        ppiScreenX, ppiScreenY = self.GetPPIScreen()
        ppiPrinterX, ppiPrinterY = self.GetPPIPrinter()
        
        scalex = float(ppiPrinterX)/ppiScreenX
        w, h = dc.GetSize()
        pageWidth, pageHeight = self.GetPageSizePixels()
        
        overallScalex = scalex * float(w) / pageWidth
        
        self.logUnitsFactorx = float(ppiPrinterX) / 25.4 * float(w) / pageWidth
        self.logUnitsFactory = float(ppiPrinterY) / 25.4 * float(h) / pageHeight
        
        self.scale = overallScalex
        
        fitRect = self.GetLogicalPageMarginsRect(wx.GetApp().pageSetupData)
        self.xoff = fitRect.x + fitRect.width / 2
        self.yoff = fitRect.y + fitRect.height / 2
        
    def SetColor(self, c):
        col = copy.copy(c)
        if c == cad.Color(255, 255, 255): col = cad.Color(0,0,0)
        pen = wx.Pen(col.ref(), self.scale + 0.99)
        self.GetDC().SetPen(pen)
