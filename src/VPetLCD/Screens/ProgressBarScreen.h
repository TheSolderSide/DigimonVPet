/////////////////////////////////////////////////////////////////
/*
  Created by Berat Özdemir, January 24 , 2021.
*/
/////////////////////////////////////////////////////////////////

#include "../VPetLCD.h"

namespace V20{
  
  class ProgressBarScreen : public VPetLCD::Screen{
    private:
      char* text;
      uint16_t barLength;
      uint16_t fillPercentage;

    public:
      ProgressBarScreen(const char _text[], uint16_t _barLength, uint16_t _fillPercentage);
      void draw(VPetLCD *lcd);
      void setFillPercentage(uint16_t _fillPercentage){fillPercentage=_fillPercentage;};

  };

}