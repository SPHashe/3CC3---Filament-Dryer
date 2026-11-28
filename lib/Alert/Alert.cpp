#include <Arduino.h>
#include <Alert.h>

Alert::Alert(int alarmPin, bool *reachedSetpoint)
{
    AlarmPin = alarmPin;
    ReachedSetpoint = reachedSetpoint;
}

Alert::Alert(int alarmPin, bool *reachedSetpoint, int timeout)
{
    AlarmPin = alarmPin;
    ReachedSetpoint = reachedSetpoint;
    Timeout = timeout;
}

Alert::Alert(int alarmPin, bool *reachedSetpoint, int timeout, int period)
{
    AlarmPin = alarmPin;
    ReachedSetpoint = reachedSetpoint;
    Timeout = timeout;
    Period = period;
}

Alert::Alert(int alarmPin, bool *reachedSetpoint, int timeout, int period, double *temperatureTolerance)
{
    AlarmPin = alarmPin;
    ReachedSetpoint = reachedSetpoint;
    Timeout = timeout;
    Period = period;
    TemperatureTolerance = temperatureTolerance;
}


void Alert::Begin()
{
    pinMode(AlarmPin, OUTPUT);
}

void Alert::SetReachedSetpoint(bool State)
{
    *ReachedSetpoint = State;
}

bool Alert::ReadReachedSetpoint()
{
    return *ReachedSetpoint;
}

bool Alert::TimeoutCheck()
{
    if (*ReachedSetpoint == false && (millis() - TimeoutPreviousMillis) >= Timeout)
    {
        MakeNoise();
        return true;
    }
    else
    {
        TimeoutPreviousMillis = millis();
        return false;
    }
}


bool Alert::OverHumidityAlert(double  CurrentHumidity, double  HumidityMax)
{
    if ((CurrentHumidity - HumidityMax) > 0.00)
    {
        MakeNoise();
        return true;
    }
    else
    {
        return false;
    }
}

bool Alert::OverTempAlert(double  Temperature, double  SetPoint)
{
    if (ReadReachedSetpoint())
    {
        if (Temperature >= (SetPoint + *TemperatureTolerance) || Temperature <= (SetPoint - *TemperatureTolerance))
        {
            MakeNoise();
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

void Alert::MakeNoise()
{
    if ((millis() - BlinkPreviousMillis) >= (Period))
    {
        BlinkPreviousMillis = millis();
        AlarmState = !AlarmState;
        digitalWrite(AlarmPin, AlarmState);
    }
}

void Alert::Silence()
{
    AlarmState = false;
    digitalWrite(AlarmPin, LOW);
}