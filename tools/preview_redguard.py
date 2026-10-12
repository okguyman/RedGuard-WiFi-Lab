"""Render a desktop preview from the exact generated monochrome glyph data."""
import json,re,sys
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1]
strings=json.loads((root/'assets/redguard/text.json').read_text())
glyphs={}
for cp,advance,packed in re.findall(r'  \{(\d+), (\d+), \{([0-9,]+)\}\},',(root/'esp32_marauder/RedGuardText.cpp').read_text()):
 glyphs[int(cp)]=(int(advance),bytes(map(int,packed.split(','))))
def rgb(v):return ((v>>11)*255//31,((v>>5)&63)*255//63,(v&31)*255//31)
bg,panel,accent=map(rgb,[0x0841,0x18e3,0xf982])
def render(page,selected):
 im=Image.new('RGB',(240,320),bg);d=ImageDraw.Draw(im)
 def text(value,x,y,width,color=(255,255,255),center=False):
  size=sum(glyphs.get(ord(c),glyphs[63])[0] for c in value)
  right=x+width
  if center and size<width:x+=(width-size)//2
  for c in value:
   advance,bits=glyphs.get(ord(c),glyphs[63])
   if x+advance>right or x+20>240:break
   for gy in range(24):
    for gx in range(20):
     if bits[gy*3+gx//8]&(0x80>>(gx%8)):im.putpixel((x+gx,y+gy),color)
   x+=advance
 def tr(k):return strings[k]
 def button(k,y,h=44,on=False):
  d.rounded_rectangle((10,y,229,y+h-1),radius=5,fill=panel,outline=accent if on else panel)
  text(tr(k),14,y+(h-24)//2,212,accent if on else (255,255,255),True)
 text(tr('Title' if page<2 else 'Settings'),8,8,224,accent,True);d.line((10,41,229,41),fill=accent)
 if page<2:
  keys=['WifiScan','Security','Password','Signal','Ble','Network','Report','Settings'][page*4:page*4+4]
  for i,k in enumerate(keys):button(k,54+i*48,on=i==selected)
  text(tr('Navigation'),10,252,190,(211,211,211));text(f'{page+1}/2',204,252,30,(211,211,211));button('Advanced',290,26)
 else:
  button('DeviceInfo',54,on=selected==0);button('DeviceSettings',102,on=selected==1);button('Back',290,26,on=selected==2)
 return im
frames=[render(0,0),render(1,3),render(2,0)]
out=Image.new('RGB',(240*3+12*2,320),(35,35,35))
for i,frame in enumerate(frames):out.paste(frame,(i*252,0))
out.resize((out.width*2,out.height*2),Image.Resampling.NEAREST).save(sys.argv[1])
# Every displayed resource must fit its reserved width.
for key,value in strings.items():
 limit=190 if key=='Navigation' else 216
 assert sum(glyphs[ord(c)][0] for c in value)<=limit,(key,value)
print('Preview generated; all resource widths fit')
