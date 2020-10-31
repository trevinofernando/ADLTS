#ifndef IRLASER_H
#define IRLASER_H

const int IRfrequency = 38;
const int IRt = 9;
const int IRpulse = 600;
const int IRpulses = 23;

class IRLaser
{
    public:
    IRLaser(int GPIO);
    ~IRLaser();

    int pin;

    void SendPulse(int length);
    void Shoot(int color);
};

#endif
