# this script resizes bitmaps, so that they are square shaped
import wx
from os import walk
BITMAP_SIZE = 96

app = wx.App()

def ResizeBitmap(filepath):
    img = wx.Image(filepath)
    if img.Width == BITMAP_SIZE and img.Height == BITMAP_SIZE:
        print('Skipped ' + filepath)
        return
    img.Resize(wx.Size(BITMAP_SIZE, BITMAP_SIZE), wx.Point((BITMAP_SIZE - img.Width) * 0.5, (BITMAP_SIZE - img.Height) * 0.5))
    img.SaveFile(filepath)
    print('Resized ' + filepath)

def FileIsBitmapSuffix(filename):
    dot = filename.rfind('.')
    if dot == -1:
        return False
    suffix = filename[dot:]
    if suffix.lower() == '.png':
        return True
    return False

def ResizeBitmapsInFolder(folder):
    for (dirpath, dirnames, filenames) in walk(folder):
        for filename in filenames:
            if FileIsBitmapSuffix(filename):
                ResizeBitmap(folder + '/' + filename)
        break

ResizeBitmapsInFolder('./')
