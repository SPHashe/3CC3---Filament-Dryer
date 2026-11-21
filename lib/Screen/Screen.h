//This code is to streamline the Screen code :)
#include <Wire.h>
#include <LiquidCrystal.h>

class Screen {
    private:
        int Pins[6] = {0, 0, 0, 0, 0, 0}; //RS, EN, D4, D5, D6, D7
        const int Lcd_MAX[2] = {16, 2}; //First is Rows, then columns
        LiquidCrystal lcd;
        String TextSent[2];

    public:
        Screen();

        Screen(int RS, int EN, int D4, int D5, int D6, int D7);

        void Begin();

        void Display(String Text, int Row);
        
};