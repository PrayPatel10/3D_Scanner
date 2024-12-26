# 3D_Scanner
The theory for this project is based on the principles of photogrammetry and depth mapping, which involves capturing multiple images of an object to reconstruct its 3D geometry. Photogrammetry can create point clouds from an image and display them in three-dimensional space.
Develop the hardware setup for to initial prototype that automates the physical capture of objects using a stepper motor and camera modules.
The initial setup will require the configuration of the distance between the object and the camera and the height of the camera to the center of the object.
Programming the Arduino microcontroller to ensure the libraries are functioning properly with the equipment and the function is desirable. The camera module libraries are all on the Arducam website which links to the git hub repository where the library is attached. Since this is an OV2640 2MP camera, the libraries will have all the available products produced in the library. The only change that needs to be done is commenting out the camera module being used and running the code.
The code will be run to ensure the camera is working using the multi-shot SD card example program. This program will produce multiple images taken from the camera and store them on the SD card. This code can be used and modified to fit the desired capture limits.
This automation will be done by a simple push button, when the button is pressed it will engage the system to begin the capturing process. A blue LED indicates the process has started and turns off when the desired number of images has reached. The combination of the led and push buttons will simplify by soldering the circuit on a PCB will get rid of the breadboard and make it easy to mount it under the ABS styrene sheet using M2 screws.
The SD module will be used as a form of storing and collecting the data from the automated system.
The system will use an A4988 stepper motor driver which needs to be adjusted to limit the current for the Nema 17 stepper motor. The equation for calculating the current limit is I = Vref/R (resistor on the driver). The use of digital multimeter will be used for this precise adjustment and the 12v power is also connected to ensure the digital multimeter is reading the voltage going into the driver.
The rotating base will also be built out of the same material with a circular-shaped green paper glued on top of it and a green screen background being used for removing the background from the object itself, isolating it for post-processing.
The use of voltage regulator will be used to make this system even more portable allowing for one single 12v adapter to be connected to the whole system. The voltage regulator is only used for the Arduino and the 12v will power the stepper motor.
The Arduino code will include the Wire.h, ArduCAM.h, SPI.h, SD.h,"memorysaver.h".
The wire.h library is used for communication with SDA and SCL. These are used for a master to control other devices. Read and write instructions are given via this library.
ArduCam.h library interacts with the camera module and uses its differentiation definitions for desired functions like taking a picture. (ArduCam).
SPI.h library is used to send large data transfers between sensors.
SD library is used to read and write into the SD module attached to the Arduino R3.
Memorysaver.h library is primarily used to shrink the data being written into the SD card.
After libraries are written, the program starts by including the pin definition of the camera, SD card, push button, LED, direction, and step of the stepper motor. 
The void setup function will test if the correct camera module is attached and write a serial output message if the connection between the camera and SPI is there. It will also see if it can read the SD card connection and output a message indicating the SD card module is present and ready.
In the void loop, the main program will start by pressing the push button which it turn on the blue LED. The loop within will go to the take_picture function which will begin by clearing the memory register attached to the camera and setting the image size which is 1600 x 1200. The picture will be taken and within this function, the program will be sent to read_fifo_burst where this image will begin the process of storing the image into the SD and name it based on how many times this function is accessed like “1.jpg”.  The program will back to the void loop.
The void loop will send a signal to the Move_stepper function where the direction pin will be high and the step amount will loop by turning the step pin high and low until the desired step has been reached.
This process will continue until the set variable amount of picture in the main loop has reached. After the set amount of 10 images with the combined movement of the stepper motor at each image is taken the process will stop and the blue LED will turn off.
The SD attached will have all 10 images stored on it, which can be accessed via an SD card reader and attached to a pc to inspect the images. 
After the system is performing well and the distance is well-calibrated for precise imaging. The system can be built out of ABS styrene sheet to complete the portability of the system.
The hardware setup is complete and the web development setup is.
The web development will bring in Visual Studio code
The libraries will be added to the top of the .py file. The library needed is os, cv2, torch, numpy as np, Flask request render_template send_from_directory, torchvision.transforms import Compose, Resize, ToTensor, Normalize, rembg import remove, trimesh, and time 
Starting with the OS library is the standard library that helps you interact with the path and directory located in the folder.
CV2 stands for open-source computer vision library primarily used for image recognition and object detection. This library helps the program use the image inserted in the webpage to be transferred to other applications for post-processing. (OpenCV).
Torch will be critical for this application. The library allows for deep learning models to be implemented and controlled on the web page. (Paszke, Gross, Chintala, 2017).
The Numpy library is used for renaming the images being uploaded and naming them according to where their destination is like the depth map folder, uploads folder, and outputs folder. (Numpy, 2020).
 The main framework that will be used is Flask, it is used for a Python-based web framework that can be used for developing Python-based web applications. (Grinberg, 2018).
The torch vision library allows this system to send the images into the deep learning model. (Marinas).
The image will be sent into MiDas with the help of the torch vision library and with it the reconstruction using depth imaging. (Ranftl, Mildenhall, Noch, 2020).
Trimesh is a library primarily used for working with 3D meshes and geometry. 
Time library is used for naming the outputs with the given time attached to see when was the last downloaded mesh created.
These libraries are what create the software development side of this project. 
The web page will initially start with the implementation of the html side where the drop box and upload button will be created and a title representing the project.
The Python code will begin by including all the libraries. The next step is to import the MIDAS model that will be used for the post-processing of the image.
The directories will be named and the folder will created inside the project folder with the same name given in the program directory.
One of the directories is named. The app route will be used to access the location of each directory and used for rendering the template directory which will have the webpage.html file.
The definition upload_file will have code that will used the the image post in the drop box of the webpage store it into the uploaded folder and send it to the next definition called process_image.
The definition process_image will use the uploaded image folder and be the depth imaging processing. It will start by removing the background of the image and send the image into the MiDas model where it will come up with depth image data and store it in the depth image folder. 
The definition create_mesh will be the final process in creating the mesh. It will start by using the depth image data to read the height and width of the image. It will loop through the width range and then hthe eight range to find the depth distance. The process will create the point cloud which later on will be combined with numpy. The point cloud will used again to triangle the mesh in a similar fashion using a nested for loop to go through the height(y) > width(x) > depth(z). The final output will be stored and downloaded with a button once the depth image is created. The process of mesh creation can take a few seconds to reach the final calculation. 
Test many different objects to ensure the accuracy and reliability of the system.
