#include "Screen.h"

Screen::Screen() : lcd(19, 23, 18, 17, 16, 15)
{
    Pins[0] = 19;
    Pins[1] = 23;
    Pins[2] = 18;
    Pins[3] = 17;
    Pins[4] = 16;
    Pins[5] = 15;
}

Screen::Screen(int RS, int EN, int D4, int D5, int D6, int D7)
    : lcd(RS, EN, D4, D5, D6, D7)
{
    Pins[0] = RS;
    Pins[1] = EN;
    Pins[2] = D4;
    Pins[3] = D5;
    Pins[4] = D6;
    Pins[5] = D7;
}

void Screen::Begin() {
    lcd.begin(Lcd_MAX[0], Lcd_MAX[1]);
}

void Screen::Display(String Text, int Row) 
{
    TextSent[Row] = Text;
    lcd.setCursor(0, Row);
    lcd.print(Text);
}