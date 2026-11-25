class Alert {
    private:
        int AlarmPin = 0;

        bool AlarmState = false;

        int Timeout = 10000; //10s

        int Period = 1000; //in ms
        int TimeoutPreviousMillis = 0;
        int BlinkPreviousMillis = 0;

        double *TemperatureTolerance = nullptr;

        bool *ReachedSetpoint = nullptr;

    public:
        Alert(int AlarmPin, bool *reachedSetpoint);
        Alert(int AlarmPin, bool *reachedSetpoint, int timeout);
        Alert(int AlarmPin, bool *reachedSetpoint, int timeout, int period);
        Alert(int AlarmPin, bool *reachedSetpoint, int timeout, int period, double *temperatureTolerance);

        void Begin();

        void SetReachedSetpoint(bool State);
        bool ReadReachedSetpoint();

        bool TimeoutCheck();

        bool OverHumidityAlert(double  CurrentHumidity, double  HumidityMax);
        bool OverTempAlert(double  Temperature, double  SetPoint);

        void MakeNoise();
        void Silence();
};