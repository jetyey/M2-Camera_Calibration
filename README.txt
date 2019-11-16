#Camera Calibration

Camera calibration is a program that outputs a text file containing a camera's matrix 
and it's distortion coefficients using opencv libraries

##Usage
In the terminal of the Calib.cpp directory. Enter

g++ -o Calib Calib.cpp $(pkg-config opencv --cflags --libs)

1. Camera will open.
2. Use spacebar to save image in a matrix (minimum 15 images).
3. Press s to run the calbration.
4. See CalbrationCamera.txt