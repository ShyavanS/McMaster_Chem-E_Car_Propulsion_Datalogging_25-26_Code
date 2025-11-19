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
    {2,3,4,5,6,7,8,9}, // arduino pins for segment display (g, f, e, d, c, b, a, dp)
    {10,11,12,13,14,15,0,0} // arduino pins for digit strobes (Strobe1, Strobe2, Strobe3, Strobe4, a-d', b-d)
};

const byte MSB_pinList[5] = {14, 15, 6, 10, 12};

string displayString = "";
char displayChars[5] = {' ', ' ', ' ', ' ', '\0'};

bool strobe_1 = false;
bool strobe_2 = false;
bool strobe_3 = false;
bool strobe_4 = false;
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

// find first digit
byte get_first_digit()
{
    // int i;
    // // if the pins for the 1 in the first LED are both high
    // if (digitalRead(pinList[0][2]) == HIGH && digitalRead(pinList[0][3]) == HIGH &&)
    //     i = 1
    // else
    //     i = 0
 
    // // if the pin for c is high, check if the pins for d and d are high
    // if (digitalRead(pinList[0][2]) == HIGH)
    //     //if the pins corresponding to d and d' are LOW then make i negative
    //     if (digitalRead(pinList[0][9]) == LOW)
    //         i = i*-1

    int i;
    
    bool changed_1 = false;
    bool changed_2 = false;

    byte condition1 = 0;
    byte condition2 = 0;
    
    if(digitalRead(pinList[1][0]) && !changed_1) {
        for(i = 0; i < 5; i++)
        {
            condition1 |= (digitalRead(MSB_pinList[i]) == HIGH) << i; // active low seg
        }
        changed_1 = true;
    }

    if(digitalRead(pinList[1][2]) && !changed_2) {
        for(i = 0; i < 5; i++)
        {
            condition2 |= (digitalRead(MSB_pinList[i]) == HIGH) << i; // active low seg
        }
        changed_2 = true;
    }

    if(condition1 == 0b11110 && condition2 == 0b11101 && changed_1 && changed_2) {
        // this is a positive 1
        displayString += "+1";
    } else if(condition1 == 0b11110 && condition2 == 0b00101 && changed_1 && changed_2) {
        // this is a positive 0
        displayString += "0";
    } else if(condition1 == 0b00110 && condition2 == 0b11101 && changed_1 && changed_2) {
        // this is a negative 1
        displayString += "-1";
    } else if(condition1 == 0b00110 && condition2 == 0b00101 && changed_1 && changed_2) {
        // this is a negative 0
        displayString += "-0";
    }
}

char get_sign()
{
    byte signBits = 0;
    for (int i; i < 5; i++)
    {
        signBits |= (digitalRead(MSB_pinList[i]) == HIGH) << i;
    }

    //case 1 (positive)
    if (signBits == /*case for a +*/)
    {
        return '+';
    }
    //case 2 (negative)
    else if (signBits == /*case for a -*/)
    {
        return '-';
    }
}



// finding other 3 digits

int get_active_digit()
{
    int i;

    for(i = 0; i < 4; i++)
    {
        if (digitalRead(pinList[1][i]) == HIGH) // active high strobe
            return i;
        
    }

  return -1;
}


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
  // Loop code to run repeatedly goes here
  int digit = get_active_digit();

  while(get_active_digit() != 0) {}
  for (int i = 1; i<5; i++) //empty all the elements of displayChars
  {
      displayString = "";
  }
  displayString += get_sign(); //append the sign of the first digit

  while(get_active_digit() != 1) {}
  char temp = segment_value(read_value);

  while(get_active_digit() != 2) {}
  byte condition = 0;
  for(i = 0; i < 5; i++)
  {
      condition |= (digitalRead(MSB_pinList[i]) == HIGH) << i; // active low seg
  }
  
  //check conditions for if its a 1 or a 0
  if (condition == /*case for a 0)*/)
  {
      displayString += '0';
  }
  else if (condition == /*case for a 1*/)
  {
      displayString += '1';
  }

  displayString += temp;
  displayString = segment_value(read_value);

  while(get_active_digit() != 3) {}
  displayString = segment_value(read_value);
}

