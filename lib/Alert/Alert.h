class Alert {
    private:
        int AlarmPin = 0;

        bool AlarmState = false;

        int Timeout = 10000; //10s

        int Period = 1000; //in ms
        int TimeoutPreviousMillis = 0;
        int BlinkPreviousMillis = 0;

        float TemperatureTolerance = 2;

        bool *ReachedSetpoint = nullptr;

    public:
        Alert(int AlarmPin, bool *reachedSetpoint);
        Alert(int AlarmPin, bool *reachedSetpoint, int timeout);
        Alert(int AlarmPin, bool *reachedSetpoint, int timeout, int period);
        Alert(int AlarmPin, bool *reachedSetpoint, int timeout, int period, float temperatureTolerance);

        void Begin();

        void SetReachedSetpoint(bool State);
        bool ReadReachedSetpoint();

        bool TimeoutCheck();

        bool OverHumidityAlert(float CurrentHumidity, float HumidityMax);
        bool OverTempAlert(float Temperature, float SetPoint);

        void MakeNoise();
};