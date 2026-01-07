// McMaster Chem-E Car Team
// Propulsion Datalogging Code - 24/25

/*
Some datalogging code for the Propulsion sub-team to obtain information on
their reaction and the power it produces. Hijacks an HP3435A multimeter to
add serial output with timestamps for datalogging rather than simply reading
the screen. Makes use of an Arduino UNO and a comparator with the output
tied to a pull up resistor strobe pins 1 & 2 due to a lower logic level
being present on them. The comparator circuit acts as a logic level shifter
to let the Arduino read them. All data is output via serial to Excel Data Streamer.
*/

// List of pins used on the Arduino UNO to interface
const byte pinList[2][8] =
    {
        {2, 3, 4, 5, 6, 7, 8, 9},      // arduino pins for segment display (g, f, e, d, c, b, a, dp)
        {10, 11, 12, 13, 14, 15, 0, 0} // arduino pins for digit strobes (Strobe1, Strobe2, Strobe3, Strobe4, a-d', b-d)
};

// List of pins used for the most significant digit which operates differently than the others
const byte MSB_pinList[3] = {14, 15, 6}; // arduino pins for first digit (a-d', b-d, c)

// Output variable
String display_string = "";

// Temporary variables
byte sign = 0;
byte first_digit = 0;
byte second_digit = 0;

/*
Description: Subroutine to interpret the signal sent to a single display unit input as a byte into decimal numbers.
Inputs:      void
Outputs:     (String)display_string
Parameters:  (byte)segment_bit
Returns:     void
*/
void segment_value(byte segment_bit)
{
    // Split the recieved byte of data representing a display unit into the decimal point and number
    byte dp = segment_bit & 0b10000000;
    byte num = segment_bit & 0b01111111;

    // Switch for decimal point
    switch (dp)
    {
    case 0b10000000:
        display_string += ".";
        break;
    case 0b00000000:
        break;
    default:
        break;
    }

    // Switch for 7-segment display
    switch (num)
    {
    case 0b01111110:
        display_string += "0";
        break;
    case 0b00110000:
        display_string += "1";
        break;
    case 0b01101101:
        display_string += "2";
        break;
    case 0b01111001:
        display_string += "3";
        break;
    case 0b00110011:
        display_string += "4";
        break;
    case 0b01011011:
        display_string += "5";
        break;
    case 0b00011111:
        display_string += "6";
        break;
    case 0b01110000:
        display_string += "7";
        break;
    case 0b01111111:
        display_string += "8";
        break;
    case 0b01110011:
        display_string += "9";
        break;
    case 0b00001110:
        display_string += "L";
        break;
    default:
        display_string += " ";
        break;
    }
}

/*
Description: Subroutine to perform a faster digital read by directly poking the registers rather than using the Arduino subroutine.
Inputs:      void
Outputs:     void
Parameters:  (uint8_t)pin
Returns:     (uint8_t)
*/
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

/*
Description: Subroutine to read all pins for a single display unit and represent the signal as a single byte.
Inputs:      void
Outputs:     void
Parameters:  void
Returns:     (byte)bits
*/
byte read_value()
{
    byte bits = 0;

    // bit 7: dp, and bit 0: a
    for (int i = 0; i < 8; i++)
    {
        bits |= (fast_read(pinList[0][i]) == HIGH) << i; // Active high segment
    }

    return bits;
}

/*
Description: Subroutine to read all pins for the most significant display unit and represent the signal as a single byte.
Inputs:      void
Outputs:     void
Parameters:  void
Returns:     (byte)bits
*/
byte read_first_value()
{
    byte bits = 0;

    // bit 0: a-d', bit 1: b-d, bit 2: c
    for (int i = 0; i < 3; i++)
    {
        bits |= (fast_read(MSB_pinList[i]) == HIGH) << i; // Active high segment
    }

    return bits;
}

/*
Description: Subroutine to interpret the signal sent to the most significant display unit input as a byte into the corresponding value.
Inputs:      void
Outputs:     (string)display_string
Parameters:  (byte)condition_1, (byte)condition_2
Returns:     void
*/
void get_first_segment_value(byte condition_1, byte condition_2)
{
    // Switch for sign
    switch (condition_1)
    {
    case 0b110:
        display_string += "+";
        break;
    case 0b111:
        display_string += "+";
        break;
    case 0b100:
        display_string += "-";
        break;
    case 0b101:
        display_string += "-";
        break;
    default:
        display_string += " ";
        break;
    }

    // Switch for number
    switch (condition_2)
    {
    case 0b000:
        display_string += "0";
        break;
    case 0b010:
        display_string += "0";
        break;
    case 0b100:
        display_string += "0";
        break;
    case 0b110:
        display_string += "0";
        break;
    case 0b001:
        display_string += "1";
        break;
    case 0b011:
        display_string += "1";
        break;
    case 0b101:
        display_string += "1";
        break;
    case 0b111:
        display_string += "1";
        break;
    default:
        display_string += " ";
        break;
    }
}

/*
Description: Subroutine to identify the current display unit being actively driven by probing the strobe pins.
Inputs:      void
Outputs:     void
Parameters:  void
Returns:     (int)
*/
int get_active_digit()
{
    for (int i = 0; i < 4; i++)
    {
        if (fast_read(pinList[1][i]) == HIGH) // Active high strobe
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
    Serial.begin(115200); // Start serial communication (adjust baud rate as needed)

    // Define all input pins
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
    // Reset display and temporary variables on each loop
    display_string = "";
    sign = 0;
    first_digit = 0;
    second_digit = 0;

    // Read the digit(s) corresponding to the currently active strobe pin
    while (get_active_digit() != 0)
    {
    }

    sign = read_first_value(); // Save sign temporarily

    while (get_active_digit() != 1)
    {
    }

    second_digit = read_value(); // Save second most significant digit tempoararily

    while (get_active_digit() != 2)
    {
    }

    first_digit = read_first_value(); // Save most significant digit temporarily

    // Process first & second digits with sign and read third digit
    get_first_segment_value(sign, first_digit);
    segment_value(second_digit);
    segment_value(read_value());

    while (get_active_digit() != 3)
    {
    }

    segment_value(read_value()); // Read fourth digit

    // Output obtained value to serial along with timestamp
    Serial.print(micros() / 1e6, 6);
    Serial.print(",");
    Serial.println(display_string);

    // Delay to ensure accurate timing & usability on Excel's data streamer
    delayMicroseconds(147750);
    delay(1000);
}
