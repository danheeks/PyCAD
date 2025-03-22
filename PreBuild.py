INCREMENT_BUILD_NUMBER = False
UPDATE_VERSION_ON_PICTURE = True

def draw_numbers_on_image(input_file, output_file, heeks_vstring, four_axis_vstring):
    import wx
    app = wx.App(False)

    # Load image
    bitmap = wx.Bitmap(input_file, wx.BITMAP_TYPE_ANY)
    
    # Create memory device context
    dc = wx.MemoryDC(bitmap)
    
    # Set font and color
    font = wx.Font(28, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL)
    dc.SetFont(font)
    dc.SetTextForeground(wx.Colour(0, 0, 0))  # Black color

    # Draw some numbers at different positions
    dc.DrawText(heeks_vstring, 509, 44)
    dc.DrawText(four_axis_vstring, 612, 102)

    # Free the memory device context
    dc.SelectObject(wx.NullBitmap)

    # Convert to wx.Image and save
    image = bitmap.ConvertToImage()
    image.SaveFile(output_file, wx.BITMAP_TYPE_PNG)

    print(f"Image saved as {output_file}")

# read build number
# increment build number
f = open('Build Defines Auto Generated.txt', 'r')
line1 = f.readline()
line2 = f.readline()

s = line2.split(' ')
v = s[2][3:-2]

f.close()

if INCREMENT_BUILD_NUMBER:
    v = str(int(v) + 1)

    f = open('Build Defines Auto Generated.txt', 'w')

    f.write('#define MyAppName "Four Axis Heeks"\n')
    f.write('#define MyVersion "0.' + v + '"\n')

    f.close()
    
if UPDATE_VERSION_ON_PICTURE:
    draw_numbers_on_image('SplashNoNumbers.png', 'Splash.png', '2.0', '0.' + v)
