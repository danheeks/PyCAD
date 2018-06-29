import wx
from wx import glcanvas
import sys
import math
import cad

import os
full_path_here = os.path.abspath( __file__ )
full_path_here = full_path_here.replace("\\", "/")
slash = full_path_here.rfind("/")
res_folder = full_path_here
if slash != -1:
    res_folder = full_path_here[0:slash]

class myGLCanvas(wxGLCanvas):
   def __init__(self, parent):
      glcanvas.GLCanvas.__init__(self, parent,-1, attribList=[glcanvas.WX_GL_RGBA, glcanvas.WX_GL_DOUBLEBUFFER, glcanvas.WX_GL_DEPTH_SIZE, 24])
      self.context = glcanvas.GLContext(self)
      self.Bind(wx.EVT_PAINT, self.OnPaint)
      self.Bind(wx.EVT_SIZE, self.OnSize)
      self.Bind(wx.EVT_MOUSE_EVENTS, self.OnMouse)
      self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
      self.Bind(wx.EVT_MENU, self.OnMenu, None, 10000, 12000)
      self.viewport= cad.Viewport()
      self.Resize()
      self.paint_callbacks = []
      self.context_menu_enabled = True

   def OnSize(self, event):
       self.Resize()
       event.Skip()

   def OnMenu(self, event):
      index = event.GetId() - 10000
      tool = self.tools[index]
      tool.Run()

   def AppendToolsToMenu(self, menu, tools):
      for tool in tools:
         if tool.IsSeparator():
            menu.AppendSeparator()
         elif tool.IsAToolList():
            sub_menu = wx.Menu()
            self.AppendToolsToMenu(sub_menu, tool.GetChildTools())
            menu.AppendMenu(wx.ID_ANY, tool.GetTitle(), sub_menu)
         else:
            item = wx.MenuItem(menu, 10000 + self.next_tool_id, text = tool.GetTitle(), help = tool.GetToolTip())
            str = tool.BitmapPath()
            if len(str)>0:
                try:
                    image = wx.Image(res_folder + '/bitmaps/' + str + '.png')
                    image.Rescale(24, 24)
                    item.SetBitmap(wx.BitmapFromImage(image))
                except:
                    pass
            menu.AppendItem(item)
            self.next_tool_id = self.next_tool_id + 1
            self.tools.append(tool)

   def OnMouse(self, event):
      if event.RightUp():
         if self.context_menu_enabled:
             tools = cad.GetDropDownTools(event.GetX(), event.GetY(), False, event.m_controlDown)
             if len(tools) > 0:
                self.next_tool_id = 0
                self.tools = []
                menu = wx.Menu()
                self.AppendToolsToMenu(menu, tools)
                self.PopupMenu(menu)
      else:
         self.cad.OnMouseEvent(self.viewport, event)

      if self.viewport.need_update: self.Update()
      if self.viewport.need_refresh: self.Refresh()
      event.Skip()

   def OnEraseBackground(self, event):
      pass # Do nothing, to avoid flashing on MSW

   def Resize(self):
      s = self.GetClientSize()
      self.viewport.WidthAndHeightChanged(s.GetWidth(), s.GetHeight())
      self.Refresh()

   def OnPaint(self, event):
      dc = wx.PaintDC(self)
      self.SetCurrent(self.context)
      self.viewport.glCommands()
      for callback in self.paint_callbacks:
          callback()
      self.SwapBuffers()
      return
