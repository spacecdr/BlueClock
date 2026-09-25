#include "../src/display_fonts.h"
#include <cassert>
#include <cstdio>
#include <string>
#include <set>
int main() {
  std::set<std::string> fontSignatures;
  for(int f=0;f<5;f++) {
    ExtraFont font=extraFont(f);
    std::set<std::string> digits;
    std::string signature;
    for(int d=0;d<10;d++) {
      std::string digit;
      for(int row=0;row<2;row++) for(int c=0;c<font.width;c++) {
        unsigned code=pgm_read_byte(font.digits+row*(3+10*font.width)+3+d*font.width+c);
        assert(code<font.patternCount||code==0xfe||code==0xff);
        digit+=char(code);
      }
      digits.insert(digit);signature+=digit;
    }
    assert(digits.size()==10);fontSignatures.insert(signature);
    for(int h=0;h<24;h++)for(int m=0;m<60;m++)for(int mode=0;mode<3;mode++)for(int sec=0;sec<2;sec++) {
      struct {uint8_t before=0xab, rows[2][16], after=0xcd;} frame;
      bool visible=colonVisible(mode,sec);
      renderExtraFont(f,h,m,visible,frame.rows);
      assert(frame.before==0xab&&frame.after==0xcd);
      for(auto &row:frame.rows) {
        assert(row[8]==(visible?'.':' '));
        for(auto code:row)assert(code<font.patternCount||code==' '||code=='.'||code==0xff);
      }
    }
  }
  assert(fontSignatures.size()==5);
  assert(colonVisible(0,0)&&!colonVisible(0,1));
  assert(colonVisible(1,0)&&colonVisible(1,1));
  assert(!colonVisible(2,0)&&!colonVisible(2,1));
  puts("PASS: 5 distinct fonts, 10 distinct digits each, 43200 frames, all separator modes");
}
