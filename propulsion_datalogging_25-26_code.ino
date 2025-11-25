// McMaster Chem-E Car Team
// Propulsion Datalogging Code - 24/25

/*
Some datalogging code for the Propulsion sub-team to obtain information on
their reaction and the power it produces. Hijacks an HP3435A multimeter to
add serial output with timestamps for datalogging rather than simply
reading the screen. All data is output via serial to Excel Data Streamer.
*/

const byte pinList[2][8] =
    {
        {2, 3, 4, 5, 6, 7, 8, 9},      // arduino pins for segment display (g, f, e, d, c, b, a, dp)
        {10, 11, 12, 13, 14, 15, 0, 0} // arduino pins for digit strobes (Strobe1, Strobe2, Strobe3, Strobe4, a-d', b-d)
};

const byte MSB_pinList[3] = {14, 15, 6}; // arduino pins for first digit (a-d', b-d, c)

String displayString = "";

bool strobe_1 = false;
bool strobe_2 = false;
bool strobe_3 = false;
bool strobe_4 = false;

byte sign = 0;
byte first_digit = 0;
byte second_digit = 0;

// 7 segment switch case
void segment_value(byte segment_bit)
{
    byte dp = segment_bit & 0b10000000;
    byte num = segment_bit & 0b01111111;

    switch (dp)
    {
    case 0b10000000:
        displayString += ".";
        break;
    case 0b00000000:
        break;
    default:
        break;
    }

    switch (num)
    {
    case 0b01111110:
        displayString += "0";
        break;
    case 0b00110000:
        displayString += "1";
        break;
    case 0b01101101:
        displayString += "2";
        break;
    case 0b01111001:
        displayString += "3";
        break;
    case 0b00110011:
        displayString += "4";
        break;
    case 0b01011011:
        displayString += "5";
        break;
    case 0b00011111:
        displayString += "6";
        break;
    case 0b01110000:
        displayString += "7";
        break;
    case 0b01111111:
        displayString += "8";
        break;
    case 0b01110011:
        displayString += "9";
        break;
    case 0b00001110:
        displayString += "L";
        break;
    default:
        displayString += " ";
        break;
    }
}

inline uint8_t fast_read(uint8_t pin)
{
    if (pin < 8)
    { // PORTD pins 0–7
        return (PIND & (1 << pin)) ? HIGH : LOW;
    }
    else if (pin < 14)
    { // PORTB pins 8–13
        return (PINB & (1 << (pin - 8))) ? HIGH : LOW;
    }
    else if (pin < 20)
    { // PORTC pins 14–19 (A0–A5)
        return (PINC & (1 << (pin - 14))) ? HIGH : LOW;
    }
    return LOW;
}

// put all segments into byte
byte read_value()
{
    byte bits = 0;

    // bit 7: dp, and bit 0: a

    for (int i = 0; i < 8; i++)
    {
        bits |= (fast_read(pinList[0][i]) == HIGH) << i; // active high seg
    }

    return bits;
}

// put all segments into byte
byte read_first_value()
{
    byte bits = 0;

    // bit 0: a-d', bit 1: b-d, bit 2: c

    for (int i = 0; i < 3; i++)
    {
        bits |= (fast_read(MSB_pinList[i]) == HIGH) << i; // active high seg
    }

    return bits;
}

// find first digit
void get_first_segment_value(byte condition_1, byte condition_2)
{
    switch (condition_1)
    {
    case 0b110:
        displayString += "+";
        break;
    case 0b111:
        displayString += "+";
        break;
    case 0b100:
        displayString += "-";
        break;
    case 0b101:
        displayString += "-";
        break;
    default:
        displayString += " ";
        break;
    }

    switch (condition_2)
    {
    case 0b000:
        displayString += "0";
        break;
    case 0b010:
        displayString += "0";
        break;
    case 0b100:
        displayString += "0";
        break;
    case 0b110:
        displayString += "0";
        break;
    case 0b001:
        displayString += "1";
        break;
    case 0b011:
        displayString += "1";
        break;
    case 0b101:
        displayString += "1";
        break;
    case 0b111:
        displayString += "1";
        break;
    default:
        displayString += " ";
        break;
    }
}

// finding other 3 digits
int get_active_digit()
{
    for (int i = 0; i < 4; i++)
    {
        if (fast_read(pinList[1][i]) == HIGH) // active high strobe
            return i;
    }

    return -1;
}

void setup()
{
    Serial.begin(115200);

    for (int i = 0; i < 8; i++)
    {
        pinMode(pinList[0][i], INPUT);
    }

    for (int i = 0; i < 6; i++)
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
    displayString = "";
    sign = 0;
    first_digit = 0;
    second_digit = 0;

    while (get_active_digit() != 0)
    {
    }

    sign = read_first_value();

    while (get_active_digit() != 1)
    {
    }

    second_digit = read_value();

    while (get_active_digit() != 2)
    {
    }

    first_digit = read_first_value();

    get_first_segment_value(sign, first_digit);
    segment_value(second_digit);
    segment_value(read_value());

    while (get_active_digit() != 3)
    {
    }

    segment_value(read_value());

    Serial.print(micros() / 1e6, 6);
    Serial.print(",");
    Serial.println(displayString);
    delayMicroseconds(147750);
}
