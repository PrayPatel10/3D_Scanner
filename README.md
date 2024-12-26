# 3D_Scanner
The theory for this project is based on the principles of photogrammetry and depth mapping, which involves capturing multiple images of an object to reconstruct its 3D geometry. Photogrammetry can create point clouds from an image and display them in three-dimensional space.
The purpose of this project is to create a simple and effective system that uses software and hardware implementation to capture and process the image data into a 3D mesh. The hardware implementation automates the capturing system designed to take stationary images of an object rotating on a base on a stepper motor. These images are then taken by an Arducam 2MP camera at every point when the object is turned and stored on an  SD card allowing for ease of use and transfer between the system. The website then processes the images of the object to map out the depth of the image and create a mesh from that data. This system is designed for a practical approach to making 3D scans of an object from capture to final downloadable mesh which can be loaded into 3D modeling software for making adjustments.




The following files contain the Arduino code folder, web app code, an image of the interface, a 3D mesh image, and the complete scanner.
