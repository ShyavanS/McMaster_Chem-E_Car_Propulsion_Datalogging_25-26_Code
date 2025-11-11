// McMaster Chem-E Car Team
// Propulsion Datalogging Code - 24/25

/*
Some datalogging code for the Propulsion sub-team to obtain information on
their reaction and the power it produces. Hijacks an HP3435A multimeter to
add serial output with timestamps for datalogging rather than simply
reading the screen. All data is output via serial to Excel Data Streamer.
*/

/*
TODO:
- Make it so it reads in all 4 digits in order and only once all 4 have been read, it pushes a string to serial
- Make cases for first digit
- Add case for dp
*/

const byte pinList[2][8] = 
{
  {2,3,4,5,6,7,8,9}, // arduino pins for segment display
  {10,11,12,13,0,0,0,0} // arduino pins for digit strobes
};

char displayString[5] = {' ', ' ', ' ', ' ', '\0'};

// 7 segment switch case

char segment_value(byte segment_bit)
{
  switch (segment_bit)
  {
    case 0b0111111: return '0';
    case 0b0000110: return '1';
    case 0b1011011: return '2';
    case 0b1001111: return '3';
    case 0b1100110: return '4';
    case 0b1101101: return '5';
    case 0b1111101: return '6';
    case 0b0000111: return '7';
    case 0b1111111: return '8';
    case 0b1101111: return '9';
    case 0b1000000: return '-';
    
    default: 
      return ' ';
  }
}

// put all segments into byte

byte read_value()
{
  byte bits = 0;

  // bit 7: dp, and bit 0: a

  for(int i = 0; i < 8; i++)
  {
    bits |= digitalRead(pinList[0][i]) << i; // active low seg
  }

  return bits;
}

// finding active digit

int get_active_digit()
{
  for(int i = 0; i < 4; i++)
  {
    if (digitalRead(pinList[1][i])) // active high strobe
      return i;
  }

  return -1;
}

/*
Description: Arduino setup subroutine.
Inputs:      void
Outputs:     void
Parameters:  void
Returns:     void
*/

void setup()
{
  Serial.begin(115200);

  for(int i = 0; i < 8; i++) 
  {
    pinMode(pinList[0][i], INPUT);
  }

  for(int i = 0; i < 4; i++) 
  {
    pinMode(pinList[1][i], INPUT);
  }
}

/*
Description: Arduino loop subroutine.
Inputs:      void
Outputs:     void
Parameters:  void
Returns:     void
*/
void loop()
{
  int digit = get_active_digit();

  if(digit >= 0)
  {
    byte segment_bit = read_value();
    displayString[digit] = segment_value(segment_bit);
  }
}
