# ADLTS
The Automated Drone Laser Tracking System (ADLTS) will be a fully automated system that integrates different technologies, hardware, and firmware in order to detect and track a threat in motion. We have decided to use a flying drone as the threat, which our system will target and aim a laser at as a countermeasure. While there are many different methods of detection, our system will be using a camera and computer vision algorithm in order to detect, track, and predict the movement of the threat. Using these coordinates and vector data, our 2-degrees of freedom turret motor system will smoothly guide our laser onto the target drone’s sensor.

![alt text](https://raw.githubusercontent.com/trevinofernando/ADLTS/master/Readme_Images/Tracking_Demo.gif)

<img src="https://raw.githubusercontent.com/trevinofernando/ADLTS/master/Readme_Images/ProjectBreakDown.png" width="550" />

## Computer Vision
We initially used the manual bounding box method to attempt to start integration as soon as possible and then transitioned to using the Hough Circle Transform paired with a background subtraction technique for circle detection. For tracking we started out testing a few different algorithms including CSRT, KCF, MOSSE, and MedianFlow. In the end we stuck with the MedianFlow tracker as it provided the best speed to accuracy ratio and was capable of being run at a near real-time framerate on the raspberry pi, capable of processing ~28 frames per second.
## Middleware
The “Middleware” is the connecting bridge between the camera and motors. It  translates pixel coordinates from an image (given by the detection system) to angles for the motors to rotate to point at the target. The Middleware will also estimate the velocity of the target as well as reduce noise from the input in order to achieve better tracking.

## Simulation
One of the first problem we faced was that because the Middleware is dependant in the other components , we can't test it without a detection system giving it input and the motor to confirm that the angles are correct. And to solve that problem, a simulation was created as a testing environment and to speed development.

Here is a side by side comparison of the main Middleware feature "Velocity Prediction":
### **Without** Velocity Prediction
![alt text](https://raw.githubusercontent.com/trevinofernando/ADLTS/master/Readme_Images/Simulation_NoVelocity.gif)


### **With** Velocity Prediction
![alt text](https://raw.githubusercontent.com/trevinofernando/ADLTS/master/Readme_Images/Simulation_WithVelocity.gif)
## Motors Controls
In order to move any mechanical part, it is essential to decide the type of motors suitable for an application. We need to rotate the camera by a certain angles, however speed of motor is not a crucial thing. For such applications, motors usually used are either servo motor or stepper motor. We chose the stepper motor.

![alt text](https://raw.githubusercontent.com/trevinofernando/ADLTS/master/Readme_Images/ChassisAssembly.png)

## Laser Sensor
The laser sensor is independent from the main system, which is the laser turret and its processing unit. It will required its own Arduino microcontroller board and battery source. We wanted to use a spherical sensor to help the camera vision detect our target easier. This is because at any angle or elevation the target would still be a 2D “circle” in the “eyes” of the camera vision. This would allow our algorithms of the camera vision to easily detect our target and transmit that data to our laser turret.

![alt text](https://raw.githubusercontent.com/trevinofernando/ADLTS/master/Readme_Images/IRSignals.png)