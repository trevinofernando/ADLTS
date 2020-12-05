# ADLTS
The Automated Drone Laser Tracking System (ADLTS) will be a fully automated system that integrates different technologies, hardware, and firmware in order to detect and track a threat in motion. We have decided to use a flying drone as the threat, which our system will target and aim a laser at as a countermeasure. While there are many different methods of detection, our system will be using a camera and computer vision algorithm in order to detect, track, and predict the movement of the threat. Using these coordinates and vector data, our 2-degrees of freedom turret motor system will smoothly guide our laser onto the target drone’s sensor.

<img src="https://raw.githubusercontent.com/trevinofernando/ADLTS/master/Readme_Images/ProjectBreakDown.png" width="450" />

## Computer Vision
TODO
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
TODO

## Laser Sensor
TODO

