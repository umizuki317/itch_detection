# itch_detection

## Project Overview
This GitHub repository contains the code and documentation for the development of a system consisting of three components: 

### ① spresense_main_vel1.0
main_vel.io is responsible for setting up communication with the two subcores and driving the vibration component (beep sound) based on the processing results from the subcores. Additionally, it sets up Wi-Fi communication.

### ② sub_spresense_test
sub_spresense_test.io focuses on analyzing microphone input using FFT and establishes communication with the host system via packets. It operates based on predefined threshold values.

### ③ sub2.io
sub2.io utilizes the accelerometer sensor (kx122) to measure arm movements. Similar to sub_spresense_test.io, it communicates with the host system via packets, employing threshold values for decision making.

## Setup and Usage
Clone the repository to your local machine.

Make sure you have the required hardware components, accelerometer, mic, and addon board.

Follow the setup instructions provided in each individual file to configure the specific functionalities and communication protocols.

Compile and upload the code to each respective core.

Observe the system's behavior, which includes driving the vibration component based on the processing results from subscores and performing Wi-Fi communication.

## Note
This project is a work in progress, and further updates and improvements may be added to enhance its functionality and performance.

Please refer to the individual code files for more detailed instructions and comments on the implementation of specific functionalities.
