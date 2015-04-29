MICO
====

###Mico-controller based Internet Connectivity Operation System


###Feathers
* Designed for embedded devices
* Based on a Real time operation system
* Support abundant MCUs (Plan)
* Wi-Fi connectivity total solution
* Build-in protocols for cloud service (Plan)
* State-of-the-art low power management
* Application framework for I.O.T product, apple homekit compatiable
* Rich tools and mobile APP to accelerate development

###Contents:
	* Demos: Demos applications and application framework
		COM.MXCHIP.SPP: Data transparent transimission between serial and wlan
		COM.MXCHIP.HA: Data that conform to HA package definition can be delivered between serial and wlan
		COM.Apple.HomeKit: Application demo that is ompatiable to Apple Homekit protocol beta release 1
	* External: External library and tools
		Curve25519: New Diffie-Hellman Speed Records Booktitle Public Key Cryptography
		GladmanAES: AES algorithm (not used in MICO, MICO has build-in AES algorithm)
		JSON-C: Implements a reference counting object model that allows you to easily construct JSON objects in C
	* Library: MICO library, handeling RTOS, algorithms, Wi-Fi driver and TCP/IP stack
		Support: Open source tools and functions
	* MICO: MICO's main entrance and basic services
		EasyLink: MXCHIP one-click Wi-Fi provisioning
		WAC: Apple MFi wireless accessory configuration library
	* Platform: Hardware operation on different platform
	* Projects: IAR workbench project

###How to use:
	1. Install IAR workbench for ARM v6.7 or higher
	2. Select target: EMW3162, EMW3161 or Open1081
	3. Build and download MICO demo application
		Note: MICO demo runs on EMB-380-S2 + EMW3162/EMW3161: http://joinmx.com/tool.php?class_id=23&id=24
			Open1081: http://item.taobao.com/item.htm?spm=a230r.1.14.32.BVcJ4C&id=38941373196&ns=1#detail
	4、Download and run Easylink APP from apple APP store, 
		https://itunes.apple.com/cn/app/easylink/id820801172?mt=8
	5. Press "add" button on Easylink APP to add a new device
	6. Input ssid and password of the wlan that assoiated to iPhone, and start easylink
	7. Press easylink button on your module (User button 1 on Open1081 dev board)
	8. A new device appears on iPhone's screen, press this device, change and save these settings
	9. This new device appears on EasyLink app's main page, you can interact with it now!
	10. HomeKit demo cannot display on EasyLink app's main page, but it can play with any app based on HomeKit API on iOS8

![github](https://raw.githubusercontent.com/MXCHIP/MICO/master/Picture/Demo1.jpg) ![github](https://raw.githubusercontent.com/MXCHIP/MICO/master/Picture/Demo2.jpg) ![github](https://raw.githubusercontent.com/MXCHIP/MICO/master/Picture/Demo3.jpg) ![github](https://raw.githubusercontent.com/MXCHIP/MICO/master/Picture/Demo4.jpg) ![github](https://raw.githubusercontent.com/MXCHIP/MICO/master/Picture/Demo5.jpg) 

