# ARC Temperature Sensing Calibration Platform

In this contest, we aim to develop a machine learning method to calibrate the thermal sensor and to avoid the interference from the environment for higher accuracy level in temperature measurement.The computation complexity of our proposed method would be suitable for porting on to the ARC Embedded Starter Kit platform. 

* [Introduction](#Introduction)
	* [System Architecture](#System-Architecture)
* [Hardware and Software Setup](#Hardware-And-Software-Setup)
	* [Required Hardware](#Required-Hardware)
	* [Required Software](#Required-Software)
	* [Hardware Connection](#Hardware-Connection)
* [User Manual](#user-manual)
	* [Before Running This Application](#before-running-this-application)
	* [Data Extraction](#raw-data)
	* [Run This Application](#run-this-application)
    * [Demo Video](#DemoVideo)
# Introduction
 In most acedemic research, most of those fabricated chips are verfied through testing machine to check the input and output pattern's functionality. However for sensor design, we need a platform to check the functionality.
 
 Due to the process variation , sometimes our chip may not work as well as we do in our post-layout-simulation. So during system integration, we still need some software method (using some Machine Learning Algorithm) to calibrate the chip's functionality.
 
  Therefore we adopt ARC Embedded Stater Kit as our platform to build up our system prototype.

* Main Task Of Our Platform
    - Data Extraction
    - Chip Sensor Functionality Calibration

## System-Architecture
### Phase1: Data Extraction
![](https://i.imgur.com/idx3oCv.png)
Using ARC EMSK to build up the temperature monitor system to collect all the chip's response data in the testing chamber. All the data collected from EMSK would transmitted through UART inteface back to PC for analysis.

### Phase2: Data Analysis & Algorithm Selection
![](https://i.imgur.com/ohs1MhH.png)
After data preprocessing, we could divide those raw-data into training set and testing set, and start to build up our Machine Learning Model to get better temperature accuracy.
### Phase3: Porting ML model to ARC EMSK Platform
![](https://i.imgur.com/WsUe4UG.png)
**Currently we only developed the Terminal Display Version**
Porting the **Inference Part** of our Machine Learning Model to the ARC EMSK platform and start to check our functionality of our ML model to boost up the accuracy.

## Hardware-And-Software-Setup
### Required-Hardware
ARC EMSK Platform with USB cable
![](https://i.imgur.com/neNKqxc.png) 

Customized Temperature Sensor Chip
![](https://i.imgur.com/vPgDj5b.jpg =50x)

|                Picture                     |
| ------------------------------------------ |
| Technology:0.18u 			     |
| Temperature Range: 0 ~ 100		     |
| Area 0.7 mm^2                              |

Other Part for Sensor Read Out Circuit

| Part Name                   | Number | Purpose                                       |
| --------------------------- | ------ | --------------------------------------------- | 
| 7404 Logic IC               |   6    | Buffer from EMSK(3.3V) to CHIP IO(1.8V)       |
| NTC Themistor $2 M\Omega$   |   2    | External Thermal Sensor                       |
| Capacitor  0.5uF            |   5    | CHIP Voltage and IO PAD Voltage Stabilization |
| Dupont Line                 |   16   | ARC Connection                                |
| 34Pin 2 Connector IDE Cable |   2    | For Long Connection to Breadboard in Chamber  |

|               PC platform                 |
| ----------------------------------------- |
| Intel Core i5-4570S CPU @ 2.90GHz (4Core) |
| RAM: 12GB                                 |
| Windows 7 64-bit Operating System         |


### Required-Software
* EMSK Environment 
	* [embARC OSP 2017.12](https://github.com/foss-for-synopsys-dwc-arc-processors/embarc_osp/releases)
	* [Teraterm](https://zh-tw.osdn.net/projects/ttssh2/)
	* [Environment Build Up](http://embarc.org/embarc_osp/doc/embARC_Document/html/page_example_usage.html#EMBARC_DEVELOPMENT_REQUIREMENT)
* Python 3.5
* MachineLearning related Package
    * Jupyter (optional)
	* Numpy
	* Pandas
	* Matplot
### Hardware-Connection
* Chip Pin Definition
![](https://i.imgur.com/H2YenYD.png)
* Thermal Sensor Chip on Breadboard
![](https://i.imgur.com/2HBDdUl.jpg)
* EMSK connection
![](https://i.imgur.com/VO4smvv.jpg)



# User-Manual
### before-running-this-application
* Modify mux.c (/board/emsk/drivers/mux/mux.c)
```
line 201: change 
	set_pmod_mux(PM1_UR_UART_0 | PM1_LR_SPI_S	\
				| PM2_I2C_HRI		\
				| PM3_GPIO_AC		\
				| PM4_I2C_GPIO_D	\
				| PM5_UR_SPI_M1 | PM5_LR_GPIO_A	\
				| PM6_UR_SPI_M0 | PM6_LR_GPIO_A );
 to 
 	set_pmod_mux(PM1_UR_GPIO_C | PM1_LR_GPIO_A\
				| PM2_GPIO_AC \
				| PM3_GPIO_AC		\
				| PM4_GPIO_AC	\
				| PM5_UR_GPIO_C | PM5_LR_GPIO_A	\
				| PM6_UR_GPIO_C | PM6_LR_GPIO_A );
```
to change the default pmod setting from UART I2C SPI All to GPIO
* makefile
```
BOARD ?= emsk
BD_VER ?= 22
CUR_CORE ?= ercem11d
JTAG ?= usb
EMBARC_ROOT = setting your root path of your embARC-osp without " "
```

##### Machine Learning Data Analysis Please Reference to the Technical Document with following links.    

[Link](http://htmlpreview.github.com/?https://github.com/willtuna/embarc_contest_2018/blob/master/Data_Preprocessing_Degree.html)

### run-this-application
run the "make run" command on cmd in directory where your makefile is
![](https://i.imgur.com/J5Celhg.png)

Result:
![](https://i.imgur.com/xL69Pdh.png)

# [DemoVideo](http://v.youku.com/v_show/id_XMzYxMTM2MDgyNA==.html?spm=a2h3j.8428770.3416059.1)
