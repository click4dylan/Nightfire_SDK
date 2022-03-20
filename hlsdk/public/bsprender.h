  #pragma once
  #include <vector>
  
  
  struct BSPLine
  {
      vec3_t start;
      vec3_t end;
      BSPLine(const vec3_t& s, const vec3_t& e)
      {
          start[0] = s[0];
          start[1] = s[1];
          start[2] = s[2];
          end[0] = e[0];
          end[1] = e[1];
          end[2] = e[2];
      }
  };
  
  extern std::vector<BSPLine> bsp_wires;