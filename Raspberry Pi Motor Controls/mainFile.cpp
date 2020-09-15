int main()
{
    float angle;
    
    int stepsPerRevolution = 1600;
    wiringPiSetup();
    Stepper myStepper(stepsPerRevolution, 0, 2);
    
    cin>>angle;
        void Stepper::stepAngle(float angles_to_move, int stepsPerRevolution)
    myStepper.stepAngle(angle, stepsPerRevolution);
    
    return 0;
}

