
#pragma once

#include "../libs/geom.h"
#include "ads1115.h"

#define FULL_SCALE_RANGE    ADS1115_FSR_4_096V  // 4.096 is closest to 3.3V
#define JOYSTICK_X          ADS1115_PIN_A0
#define JOYSTICK_Y          ADS1115_PIN_A1
#define JOYSTICK_BUTTONA    ADS1115_PIN_A2
#define JOYSTICK_BUTTONB    ADS1115_PIN_A3
#define JOYSTICK_INVERT     point2D(1, -1)      // invert y axis
#define JOYSTICK_VMAX       3.3f                // volts
#define JOYSTICK_VTRIG      3.0f                // voltage above this is high


class Joystick {
public:
    Joystick(int address, float ignoreRadius) : m_ADC(address, FULL_SCALE_RANGE) {
        SetIgnoreRadius(ignoreRadius);
    }
    
    void Update() {
        UpdatePosition();
        UpdateButtons();
    }
    const point2D&  Position_XY()       { return m_Position_XY; }
    const polar&    Position_Polar()    { return m_Position_Polar; }
    bool            ButtonA()           { return m_ButtonA; }
    bool            ButtonB()           { return m_ButtonB; }

    void SetIgnoreRadius(float ignoreRadius) { m_IgnoreRadius = ignoreRadius; }
        
private:
    ADS1115 m_ADC;
    // position of joystick
    point2D m_Position_XY;     // (x, y)
    polar m_Position_Polar;  // (r, th)
    // button press
    bool m_ButtonA = false;
    bool m_ButtonB = false;
    
    float m_IgnoreRadius;
    
    void UpdatePosition() {
        point2D p = { (float)m_ADC.Read(JOYSTICK_X), (float)m_ADC.Read(JOYSTICK_Y) };
        std::cout << p << std::endl;
        // normalise to -1 to 1, and translate from (0 to 2) to (-1 to 1), and invert x/y
        p = JOYSTICK_INVERT * ((p * 2 / JOYSTICK_VMAX) - 1.0f);
        polar pol = polar(p);
        // Ignore if within small radius
        m_Position_XY       = (pol.r < m_IgnoreRadius) ?    point2D(0, 0)  : p;
        m_Position_Polar    = (pol.r < m_IgnoreRadius) ?    polar(0, 0)    : pol;
    }
    void UpdateButtons() {
        m_ButtonA = m_ADC.Read(JOYSTICK_BUTTONA) > JOYSTICK_VTRIG;
        m_ButtonB = m_ADC.Read(JOYSTICK_BUTTONB) > JOYSTICK_VTRIG;
    }
};
