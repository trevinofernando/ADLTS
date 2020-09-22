# Testing Data
## Unit Testing
### Computer Vision: 
A folder called "CV_Test_Set" was created with GIMP (sourcefile: "CV_Unit_Test.xcf") which contains an array of generated images to test the computer vision algorithm on single image cases and streamed images with the intention to verify it's efficiency on simple cases. 

Each image in the folder contains a circle of color C, radius R and pixel coordinates x,y. The name of the file will describe it's content as follows:

            CR(x-y).png

where C is the color, R the radius, x the number of horizontal pixels from left to right of the center of the circle, and y the number of vertical pixels from top to bottom of the center of the circle.

One can test the Detection system with single images or choose an array of images in a specific order to mimic movement.

For reference, a combined image of all the images in the folder is "CV_Test_Set_Stacked.png"

