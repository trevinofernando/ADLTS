#ifndef MOTOR_H
#define MOTOR_H


#define totalStepsPerRevolution 1600
#define connectionMotor1 0
#define connectionMotor2 2
#define connectionMotor3 3
#define connectionMotor4 4

class Stepper {
  public:
    // constructors:
    Stepper(int number_of_steps, int motor_pin_1, int motor_pin_2);

    // speed setter method:
    void setSpeed(long whatSpeed);

    // mover method:
    void step(int number_of_steps);
    void stepAngle(float angles_to_move, int stepsPerRevolution);

  private:
    void stepMotor(int this_step);

    int direction;            // Direction of rotation
    unsigned long step_delay; // delay between steps, in ms, based on speed
    int number_of_steps;      // total number of steps this motor can take
    int step_number;          // which step the motor is on

    // motor pin numbers:
    int motor_pin_1;
    int motor_pin_2;

    unsigned long last_step_time; // time stamp in us of when the last step was taken
};

#endif
