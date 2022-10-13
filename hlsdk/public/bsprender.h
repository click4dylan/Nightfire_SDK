  #pragma once
  #include <vector>
  
  
  struct BSPLine
  {
      vec3_t start;
      vec3_t end;
      unsigned char red, green, blue, alpha;
      BSPLine(const vec3_t& s, const vec3_t& e, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
      {
          start[0] = s[0];
          start[1] = s[1];
          start[2] = s[2];
          end[0] = e[0];
          end[1] = e[1];
          end[2] = e[2];
          red = r;
          green = g;
          blue = b;
          alpha = a;
      }
  };
  
  extern std::vector<BSPLine> bsp_wires;