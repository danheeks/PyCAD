import wx
import cad
import os

ButtonTypePlus = 1
ButtonTypeMinus = 2
ButtonTypeLabelBefore = 3
ButtonTypeLabel = 4

pycad_dir = os.path.dirname(os.path.realpath(__file__))

class TreeButton:
    def __init__(self, type = 0, rect = wx.Rect(0,0,0,0), obj = None, paste_into = None, paste_before = None):
        self.type = type
        self.rect = rect
        self.obj = obj
        self.paste_into = paste_into
        self.paste_before = paste_before
        
    def __str__(self):
        s = 'Tree Button, type = ' + str(self.type) + ', rect = ' + str(self.rect)
        return s
        
class TreeObserver(cad.Observer):
    def __init__(self, tree_canvas):
        cad.Observer.__init__(self)
        self.tree_canvas = tree_canvas
        
    def OnRemoved(self, removed):
        self.tree_canvas.OnRemoved(removed)

    def OnAdded(self, added):
        self.tree_canvas.OnAdded(added)

    def OnModified(self, modified):
        self.tree_canvas.OnModified(modified)
    
    def OnSelectionChanged(self, added, removed):
        self.tree_canvas.Refresh()
        
    def OnClear(self):
        self.tree_canvas.SetVirtualSize(self.tree_canvas.GetRenderSize())
        self.tree_canvas.Refresh()
        
class TreeCanvas(wx.ScrolledCanvas):
    def __init__(self, parent):
        wx.ScrolledCanvas.__init__(self, parent)
        self.observer = TreeObserver(self)
        cad.RegisterObserver(self.observer)
        self.LButton = False
        self.CurrentPoint = wx.Point(0,0)
        self.frozen = False
        self.refresh_wanted_on_thaw = False
        self.expanded = {}
        self.collapsed = {}
        self.dc = None
        self.dragged_list = []
        self.dragging = False
        self.button_down_point = wx.Point(0,0)
        self.drag_position = wx.Point(0,0)
        self.drag_paste_rect = wx.Rect(0,0,0,0)
        self.waiting_until_left_up = False
        self.xpos = 0
        self.ypos = 0
        self.max_xpos = 0
        self.tree_buttons = []
        self.clicked_object = None
        self.rendered_objects = []
        
        self.bmp_branch_plus = wx.Bitmap(pycad_dir + "/icons/branch_plus.png", wx.BITMAP_TYPE_ANY)
        self.bmp_branch_minus = wx.Bitmap(pycad_dir + "/icons/branch_minus.png", wx.BITMAP_TYPE_ANY)
        self.bmp_branch_end_plus = wx.Bitmap(pycad_dir + "/icons/branch_end_plus.png", wx.BITMAP_TYPE_ANY)
        self.bmp_branch_end_minus = wx.Bitmap(pycad_dir + "/icons/branch_end_minus.png", wx.BITMAP_TYPE_ANY)
        self.bmp_branch_split = wx.Bitmap(pycad_dir + "/icons/branch_split.png", wx.BITMAP_TYPE_ANY)
        self.bmp_branch_end = wx.Bitmap(pycad_dir + "/icons/branch_end.png", wx.BITMAP_TYPE_ANY)
        self.bmp_plus = wx.Bitmap(pycad_dir + "/icons/plus.png", wx.BITMAP_TYPE_ANY)
        self.bmp_minus = wx.Bitmap(pycad_dir + "/icons/minus.png", wx.BITMAP_TYPE_ANY)
        self.bmp_branch_trunk = wx.Bitmap(pycad_dir + "/icons/branch_trunk.png", wx.BITMAP_TYPE_ANY)

        self.render_just_for_calculation = False
        self.render_labels = True
        self.end_child_list = []

        self.SetScrollRate(10, 10)
        self.SetVirtualSize(wx.Size(92, 97))
        
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_MOUSE_EVENTS, self.OnMouse)
        self.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        self.Bind(wx.EVT_KEY_UP, self.OnKeyUp)
        
    def ObjectRenderedIndex(self, object):
        # returns index number; 0 for first item, if object is in rendered objects
        # else -1
        i = 0
        for o in self.rendered_objects:
            if o.GetIndex() == object.GetIndex():
                return i
            i += 1
        return -1
        
    def OnKeyDown(self, e):
        k = e.GetKeyCode()
        if k == wx.WXK_UP:
            sel = cad.GetSelectedObjects()
            index = -1
            if len(sel) > 0:
                index = self.ObjectRenderedIndex(sel[-1])
                if index > 0:
                    index -= 1
            if (index == -1) and (len(self.rendered_objects) > 0):
                index = len(self.rendered_objects) - 1
            
            cad.ClearSelection(True)
            
            if len(self.rendered_objects):
                cad.Select(self.rendered_objects[index])
        if k == wx.WXK_DOWN:
            sel = cad.GetSelectedObjects()
            index = -1
            if len(sel) > 0:
                index = self.ObjectRenderedIndex(sel[-1])
                if index < len(self.rendered_objects)-1:
                    index += 1
            if (index == -1) and (len(self.rendered_objects) > 0):
                index = 0
            
            cad.ClearSelection(True)
            
            if len(self.rendered_objects):
                cad.Select(self.rendered_objects[index])
        
    def OnKeyUp(self, e):
        pass
        
    def OnRemoved(self, removed):
        self.SetVirtualSize(self.GetRenderSize())
        self.Refresh()

    def OnAdded(self, added):
        self.SetVirtualSize(self.GetRenderSize())
        self.Refresh()

    def OnModified(self, modified):
        self.SetVirtualSize(self.GetRenderSize())
        self.Refresh()
    
    def OnPaint(self, event):
        self.dc = wx.PaintDC(self)
        self.DoPrepareDC(self.dc)
        self.Render()
        self.dc = None

    def OnEraseBackground(self, event):
        pass # Do nothing, to avoid flashing on MSW
        
    def HitTest(self, pt):
        for b in self.tree_buttons:
            unscrolled_pt = self.CalcUnscrolledPosition(pt)
            if b.rect.Contains(unscrolled_pt):
                return b

        return None
    
    def OnMouse(self, event):
        # to do? if(wxGetApp().m_property_grid_validation)return;
        if event.Entering():
            self.SetFocus()  # so middle wheel works
            
        if event.LeftDown():
            button = self.HitTest(event.GetPosition())
 
            if button:
                if button.type == ButtonTypePlus or button.type == ButtonTypeMinus:
                    self.SetExpanded(button.obj, button.type == ButtonTypePlus)
                    self.SetVirtualSize(self.GetRenderSize())
                    self.Refresh()
                else:
                    self.OnLabelLeftDown(button.obj, event)
                    self.clicked_object = button.obj
            else:
                cad.ClearSelection(True);

            self.button_down_point = event.GetPosition()

        if event.LeftUp():
            if self.dragging:
                self.dragging = False

                # find the object to drop on to
                button = self.HitTest(event.GetPosition())

                if (button == None) or not cad.ObjectMarked(button.obj): # can only drop on to an item other than one of the items being dragged
                    # test drop possible
                    drag_possible = True

                    add_to = cad.GetApp()
                    if button and button.paste_into: add_to = button.paste_into
                    for object in self.dragged_list:
                        if not add_to.CanAdd(object) or not object.CanAddTo(add_to):
                            drag_possible = False
                            break

                    if drag_possible:
                        cad.StartHistory()

                        # cut the objects
                        cad.DeleteListUndoably(self.dragged_list)

                        # paste the objects
                        for object in self.dragged_list:
                            if object.OneOfAKind():
                                one_found = False
                                child = add_to.GetFirstChild()
                                while child:
                                    if child.GetType() == object.GetType():
                                        child.CopyFrom(object)
                                        one_found = True
                                        break

                                    child = add_to.GetNextChild()

                                if not one_found:
                                    cad.AddUndoably(object, add_to, button.paste_before if (button != None) else None)
                            else:
                                cad.AddUndoably(object, add_to, button.paste_before if (button != None) else None)

                        cad.EndHistory()
                    else:
                        self.Refresh()
                else:
                    self.Refresh()
            else:
                if self.waiting_until_left_up:
                    cad.ClearSelection(False)
                    cad.Select(self.clicked_object, True)
            self.waiting_until_left_up = False

        if event.RightDown():
            button = self.HitTest(event.GetPosition())
            self.clicked_object = None
            if (button != None) and (button.type == ButtonTypeLabelBefore or button.type == ButtonTypeLabel):
                self.clicked_object = button.obj
                self.OnLabelRightDown(button.obj, event)

        if event.RightUp():
            if self.dragging:
                self.dragging = False
            else:
                if self.clicked_object:
                    tools = wx.GetApp().GetObjectTools(self.clicked_object, True)
                    menu = wx.Menu()
                    for tool in tools:
                        wx.GetApp().AddToolToListAndMenu(tool, menu)
                    self.PopupMenu(menu, event.GetPosition())

        if event.LeftDClick():
             if self.clicked_object:
                 wx.GetApp().EditUndoably(self.clicked_object)

        event.Skip()
    
    def IsExpanded(self, object):
        if object.AutoExpand():
            # assume it is expanded if it hasn't been collapsed
            return object.GetIndex() not in self.collapsed
        
        # it is expanded, if it is in the expanded set
        return object.GetIndex() in self.expanded
        
    def SetExpanded(self, object, bExpanded):
        if bExpanded:
            self.expanded[object.GetIndex()] = True
            if object.GetIndex() in self.collapsed:
                del self.collapsed[object.GetIndex()]
        else:
            if object.GetIndex() in self.expanded:
                del self.expanded[object.GetIndex()]
            self.collapsed[object.GetIndex()] = True
    
    def AddPlusOrMinusButton(self, object, plus):
        self.tree_buttons.append( TreeButton(ButtonTypePlus if plus else ButtonTypeMinus, wx.Rect(self.xpos, self.ypos, 16, 18), object, None, None) )
        
    def AddLabelButton(self, expanded, prev_object, prev_object_expanded, object, next_object, label_start_x, label_end_x):
        x = label_start_x
        width = label_end_x - label_start_x
        if prev_object and not prev_object_expanded:
            y = self.ypos - 4
            height = 8
        else:
            y = self.ypos
            height = 4
        type = ButtonTypeLabelBefore
        obj = object
        paste_into = object.GetOwner()
        paste_before = object
        self.tree_buttons.append( TreeButton(type, wx.Rect(x, y, width, height), obj, paste_into, paste_before) )

        type = ButtonTypeLabel
        y = self.ypos + 4
        height = 10
        paste_into = object
        paste_before = None
        self.tree_buttons.append( TreeButton(type, wx.Rect(x, y, width, height), obj, paste_into, paste_before) )

        y += height
        height = 4
        if next_object == None and not expanded:
            type = ButtonTypeLabelBefore
            paste_into = object.GetOwner()
            paste_before = next_object
            self.tree_buttons.append( TreeButton(type, wx.Rect(x, y, width, height), obj, paste_into, paste_before) )

    def OnLabelLeftDown(self, object, event):
        if event.ShiftDown():
            self.waiting_until_left_up = cad.ShiftSelect(object, event.ControlDown())
        else:
            # shift not down
            if event.ControlDown():
                if cad.ObjectMarked(object):
                    cad.Unselect(object, True)
                else:
                    cad.Select(object, True)
            else:
                if cad.ObjectMarked(object):
                    self.waiting_until_left_up = True
                    cad.Select(object, True)
                else:
                    cad.ClearSelection(False)
                    cad.Select(object, True)

    def OnLabelRightDown(self, object, event):
        if not cad.ObjectMarked(object):
            cad.ClearSelection(False)
            cad.Select(object, True)
    
    def RenderBranchIcon(self, object, next_object, expanded, level):
        num_children = object.GetNumChildren()
        if num_children > 0:
            if level:
                # with branches
                if next_object:
                    # not at end
                    if expanded:
                        self.dc.DrawBitmap(self.bmp_branch_minus, self.xpos, self.ypos)
                        if self.render_labels: self.AddPlusOrMinusButton(object, False)
                    else:
                        self.dc.DrawBitmap(self.bmp_branch_plus, self.xpos, self.ypos)
                        if self.render_labels: self.AddPlusOrMinusButton(object, True)
                else:
                    # not at end
                    if expanded:
                        self.dc.DrawBitmap(self.bmp_branch_end_minus, self.xpos, self.ypos)
                        if self.render_labels: self.AddPlusOrMinusButton(object, False)
                    else:
                        self.dc.DrawBitmap(self.bmp_branch_end_plus, self.xpos, self.ypos)
                        if self.render_labels: self.AddPlusOrMinusButton(object, True)
            else:
                # without branches
                if expanded:
                    self.dc.DrawBitmap(self.bmp_minus, self.xpos, self.ypos)
                    if self.render_labels: self.AddPlusOrMinusButton(object, False)
                else:
                    self.dc.DrawBitmap(self.bmp_plus, self.xpos, self.ypos)
                    if self.render_labels:self.AddPlusOrMinusButton(object, True)
        else:
            if level > 0:
                # just branches
                if next_object: self.dc.DrawBitmap(self.bmp_branch_split, self.xpos, self.ypos)
                else: self.dc.DrawBitmap(self.bmp_branch_end, self.xpos, self.ypos)


    def RenderBranchIcons(self, object, next_object, expanded, level):
        # render initial branches
        for i in range(0, level):
            if not self.render_just_for_calculation and i > 0:
                end_child = self.end_child_list[i]
                if not end_child: self.dc.DrawBitmap(self.bmp_branch_trunk, self.xpos, self.ypos)
            self.xpos += 16

        # render + or -
        if not self.render_just_for_calculation: self.RenderBranchIcon(object, next_object, expanded, level)
        self.xpos += 16

    def RenderObject(self, expanded, prev_object, prev_object_expanded, object, next_object, level):
        save_x = self.xpos

        self.RenderBranchIcons(object, next_object, expanded, level)

        label_start_x = self.xpos
        
        # find icon info
        if not self.render_just_for_calculation:
            self.dc.DrawBitmap(wx.Bitmap(object.GetIconFilePath(), wx.BITMAP_TYPE_ANY), self.xpos, self.ypos)
            self.rendered_objects.append(object)

        self.xpos += 16

        str = object.GetTitle()
        
        if not self.render_just_for_calculation:
            if self.render_labels and cad.ObjectMarked(object):
                self.dc.SetBackgroundMode(wx.SOLID)
                self.dc.SetTextBackground(wx.BLUE)
                self.dc.SetTextForeground(wx.WHITE)
            else:
                self.dc.SetBackgroundMode(wx.TRANSPARENT)
                self.dc.SetTextForeground(wx.BLACK)
            self.dc.DrawText(str, self.xpos, self.ypos)

        text_width = 0
        if self.render_just_for_calculation or not self.render_labels:
            # just make a guess, we don't have a valid m_dc
            text_width = 10 * len(str)
        else:
            text_size = self.GetTextExtent(str)
            text_width = text_size.GetWidth()

        label_end_x = self.xpos + 8 + text_width
        if not self.render_just_for_calculation and self.render_labels:
            self.AddLabelButton(expanded, prev_object, prev_object_expanded, object, next_object, label_start_x, label_end_x)

        if label_end_x > self.max_xpos: self.max_xpos = label_end_x
        
        self.ypos += 18

        end_object = (next_object == None)
        self.end_child_list.append(end_object)

        self.xpos = save_x

        if expanded:
            prev_child = None
            prev_child_expanded = False
            child = object.GetFirstChild()

            while child != None:
                next_child = object.GetNextChild()
                expanded = self.IsExpanded(child)
                self.RenderObject(expanded, prev_child, prev_child_expanded, child, next_child, level + 1)
                prev_child = child
                prev_child_expanded = expanded
                child = next_child

        self.end_child_list.pop()

    def Render(self, just_for_calculation = False):
        self.render_just_for_calculation = just_for_calculation
        if not just_for_calculation:
            if os.name == 'nt':
                font = wx.Font(10,wx.SWISS,wx.NORMAL,wx.NORMAL)
                self.dc.SetFont(font)

                # draw a white background rectangle
                size = self.GetClientSize()
                pTopLeft = self.CalcUnscrolledPosition(0, 0)
                pBottomRight = self.CalcUnscrolledPosition(size.x, size.y)
                self.dc.SetBrush(wx.Brush("white"))
                self.dc.SetPen(wx.Pen("white"))
                self.dc.DrawRectangle(wx.Rect(pTopLeft, pBottomRight))

            # set background
            self.dc.SetBackgroundMode(wx.SOLID)
            self.dc.SetBackground(wx.Brush("white", wx.SOLID))
            self.dc.Clear()

            self.tree_buttons = []
            self.rendered_objects = []

        self.xpos = 0 # start at the left
        self.ypos = 0 # -scroll_y_pos; // start at the top
        self.max_xpos = 0

        prev_object = None
        prev_object_expanded = None
        doc = cad.GetApp()
        object = doc.GetFirstChild()

        while object != None:
            next_object = doc.GetNextChild()
            expanded = self.IsExpanded(object)
            self.RenderObject(expanded, prev_object, prev_object_expanded, object, next_object, 0)
            prev_object = object
            prev_object_expanded = expanded
            object = next_object

        # draw the dragged objects
        if self.dragging:
            drag_size = self.GetDraggedListSize()
            self.dc.SetBrush(wx.Brush("orange"))
            self.dc.SetPen(wx.Pen("blue"))
            self.dc.DrawRectangle(self.drag_position, drag_size)
            self.RenderDraggedList()
            if self.drag_paste_rect.width > 0:
                self.dc.SetPen(wx.Pen("black"))
                self.dc.SetBrush(wx.Brush("black"))
                self.dc.DrawRectangle(self.drag_paste_rect)

    def GetRenderSize(self):
        just_for_calculation = True
        self.Render(just_for_calculation)
        return wx.Size(self.max_xpos, self.ypos)

