// Render the actual LCD character tables as an SVG documentation asset.
#include "../src/display_fonts.h"
#include <fstream>

int main(int argc, char **argv) {
  if (argc != 2) return 1;
  std::ofstream out(argv[1]);
  if (!out) return 1;
  const char *names[] = {"Slanciato", "Trek", "Classico", "Arrotondato", "Geometrico"};
  out << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"640\" height=\"850\" viewBox=\"0 0 640 850\">"
      << "<title>BlueClock: cinque font LCD aggiuntivi alle 12:48</title>"
      << "<rect width=\"640\" height=\"850\" rx=\"16\" fill=\"#101724\"/>"
      << "<text x=\"30\" y=\"40\" fill=\"#e5edf7\" font-family=\"sans-serif\" font-size=\"24\">BlueClock · Font LCD</text>"
      << "<text x=\"30\" y=\"64\" fill=\"#aabbd1\" font-family=\"sans-serif\" font-size=\"13\">Simulazione dei pixel del firmware, non una fotografia</text>";
  for (int index = 0; index < 5; ++index) {
    const int y = 105 + index * 146;
    out << "<text x=\"30\" y=\"" << y << "\" fill=\"#e5edf7\" font-family=\"sans-serif\" font-size=\"18\">" << names[index] << "</text>";
    out << "<rect x=\"29\" y=\"" << y + 12 << "\" width=\"582\" height=\"111\" rx=\"6\" fill=\"#11339a\"/>";
    ExtraFont f = extraFont(index);
    uint8_t rows[2][16];
    renderExtraFont(index, 12, 48, true, rows);
    for (int row=0; row<2; ++row) for (int col=0; col<16; ++col) {
      const uint8_t code=rows[row][col];
      for (int py=0; py<8; ++py) {
        uint8_t bits=0;
        if(code < f.patternCount) bits=pgm_read_byte(&f.patterns[code][py]);
        else if(code==0xff) bits=31;
        else if(code=='.' && py>=6) bits=6;
        for(int px=0; px<5; ++px) if(bits & (16>>px))
          out << "<rect x=\"" << 36+col*36+px*6 << "\" y=\"" << y+20+row*54+py*6 << "\" width=\"5\" height=\"5\" fill=\"#e6f4ff\"/>";
      }
    }
  }
  out << "</svg>\n";
  return out ? 0 : 1;
}
