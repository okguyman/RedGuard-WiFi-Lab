"""Synthetic layout preview, not a screenshot or real scan result."""
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
def frame(kind):
 im=Image.new('RGB',(240,320),bg);d=ImageDraw.Draw(im)
 def text(value,x,y,width=220,color=(255,255,255),center=False):
  size=sum(glyphs.get(ord(c),glyphs[63])[0] for c in value);right=x+width
  if center and size<width:x+=(width-size)//2
  for c in value:
   advance,bits=glyphs.get(ord(c),glyphs[63])
   if x+advance>right or x+20>240:break
   for gy in range(24):
    for gx in range(20):
     if bits[gy*3+gx//8]&(0x80>>(gx%8)):im.putpixel((x+gx,y+gy),color)
   x+=advance
 def button(key,y,h=44,on=False):
  d.rounded_rectangle((10,y,229,y+h-1),radius=5,fill=panel,outline=accent if on else panel)
  text(strings[key],14,y+(h-24)//2,212,accent if on else (255,255,255),True)
 text(strings['Channels' if kind=='channels' else 'WifiScan'],8,8,224,accent,True);d.line((10,41,229,41),fill=accent)
 if kind=='hub':
  for i,key in enumerate(['StartScan','ViewResults','Channels','ExportCsv']):button(key,54+i*48,on=i==0)
  text(strings['ScanComplete'],10,252,color=(211,211,211));button('Back',290,26)
 elif kind=='list':
  for i,(rssi,ch) in enumerate([(-42,6),(-54,1),(-68,11),(-76,6)]):
   y=54+i*48;d.rounded_rectangle((10,y,229,y+43),radius=5,fill=panel,outline=accent if i==0 else panel)
   text('LAB_WIFI_'+str(i+1),14,y-1,212,accent if i==0 else (255,255,255))
   d.text((14,y+27),f'{rssi} dBm   CH:{ch}   WPA2',fill=(211,211,211),font_size=8)
  text('4/4  CH:ALL',10,252,color=(211,211,211));button('Back',290,26)
 else:
  counts=[2,1,0,1,0,4,1,0,0,1,3,0,0,0];highest=max(counts)
  d.text((10,54),'AP:13/13  max:4',fill=(211,211,211),font_size=8)
  for index,n in enumerate(counts):
   x=10+index*16;height=n*116//highest
   if height:d.rectangle((x,204-height,x+11,203),fill=accent)
   d.text((x,210),str(index+1),fill='white',font_size=8)
   if n:d.text((x,190-height),str(n),fill='white',font_size=8)
  text(strings['CountOnly'],10,244,color=(211,211,211));button('Back',290,26,on=True)
 return im
out=Image.new('RGB',(744,320),(35,35,35))
for i,kind in enumerate(['hub','list','channels']):out.paste(frame(kind),(252*i,0))
out.resize((1488,640),Image.Resampling.NEAREST).save(sys.argv[1])
print('Synthetic scan layout generated')
