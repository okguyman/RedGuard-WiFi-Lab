"""Desktop preview using generated glyphs and firmware logo coordinates."""
import json,re,sys
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1]
strings=json.loads((root/'assets/redguard/text.json').read_text())
glyphs={}
for cp,advance,packed in re.findall(r'  \{(\d+), (\d+), \{([0-9,]+)\}\},',(root/'esp32_marauder/RedGuardText.cpp').read_text()):
 glyphs[int(cp)]=(int(advance),bytes(map(int,packed.split(','))))
source=(root/'esp32_marauder/RedGuardRender.cpp').read_text()
def points(name):
 block=re.search(r'const int '+name+r'\[\]\[2\] = \{(.*?)\};',source).group(1)
 return [tuple(map(int,p)) for p in re.findall(r'\{(\d+),(\d+)\}',block)]
def rgb(v):return ((v>>11)*255//31,((v>>5)&63)*255//63,(v&31)*255//31)
bg,accent=rgb(0x0841),rgb(0xf982)
def frame(stage):
 im=Image.new('RGB',(240,320),bg);d=ImageDraw.Draw(im)
 def text(value,y,color):
  size=sum(glyphs.get(ord(c),glyphs[63])[0] for c in value);assert size<=220
  x=10+(220-size)//2
  for c in value:
   advance,bits=glyphs.get(ord(c),glyphs[63])
   for gy in range(24):
    for gx in range(20):
     if bits[gy*3+gx//8]&(0x80>>(gx%8)):
      assert 0<=x+gx<240 and 0<=y+gy<320
      im.putpixel((x+gx,y+gy),color)
   x+=advance
 shield=points('shield');wifi=points('wifi')
 for p in shield+wifi:assert 0<=p[0]<240 and 0<=p[1]<320
 d.line(shield,fill=accent);d.line([(x,y+1) for x,y in shield],fill=accent)
 d.line(wifi,fill='white');d.line([(96,108),(106,102),(120,99),(134,102),(144,108)],fill='white')
 d.line([(108,120),(120,116),(132,120)],fill='white');d.ellipse((117,129,123,135),fill=accent)
 text('RedGuard WiFi Lab',184,(255,255,255));text(strings['Title'],211,accent)
 text(strings[stage],240,(211,211,211));text(strings['BootChecking'],270,(80,80,80))
 return im
out=Image.new('RGB',(492,320),(35,35,35));out.paste(frame('BootSettings'),(0,0));out.paste(frame('BootWireless'),(252,0));out.resize((984,640),Image.Resampling.NEAREST).save(sys.argv[1])
print('Boot preview generated; glyph and geometry bounds passed')
