EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L remote-control-rescue:MCP2562-capteur-rescue-remote-control-rescue U2
U 1 1 5D07804E
P 6700 2250
F 0 "U2" H 6300 2700 50  0000 L BNN
F 1 "MCP2562" H 6300 1750 50  0000 L BNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 6700 2400 50  0001 C CNN
F 3 "~" H 6700 2250 60  0000 C CNN
	1    6700 2250
	1    0    0    -1  
$EndComp
$Comp
L remote-control-rescue:PIC18F2xK80-capteur-rescue-remote-control-rescue U1
U 1 1 5D07804F
P 5350 5450
F 0 "U1" H 5450 6750 50  0000 L BNN
F 1 "PIC18F2XK80" H 5650 4250 50  0000 L BNN
F 2 "Package_SO:SOIC-28W_7.5x17.9mm_P1.27mm" H 5850 6450 50  0001 C CNN
F 3 "~" H 5850 6300 60  0000 C CNN
	1    5350 5450
	0    -1   -1   0   
$EndComp
$Comp
L remote-control-rescue:uA78M33-capteur-rescue-remote-control-rescue U4
U 1 1 5D078050
P 8950 4900
F 0 "U4" H 9100 4704 60  0000 C CNN
F 1 "UA78M33" H 8950 5100 60  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-223-3_TabPin2" H 8950 4900 60  0001 C CNN
F 3 "" H 8950 4900 60  0000 C CNN
	1    8950 4900
	0    -1   1    0   
$EndComp
$Comp
L remote-control-rescue:GND-RESCUE-capteur-capteur-rescue-remote-control-rescue #PWR014
U 1 1 5D078051
P 9350 4900
F 0 "#PWR014" H 9350 4900 30  0001 C CNN
F 1 "GND" H 9350 4830 30  0001 C CNN
F 2 "" H 9350 4900 60  0000 C CNN
F 3 "" H 9350 4900 60  0000 C CNN
	1    9350 4900
	0    -1   -1   0   
$EndComp
$Comp
L remote-control-rescue:+12V-power #PWR012
U 1 1 5D078052
P 8900 4400
F 0 "#PWR012" H 8900 4350 20  0001 C CNN
F 1 "+12V" H 8900 4500 30  0000 C CNN
F 2 "" H 8900 4400 60  0000 C CNN
F 3 "" H 8900 4400 60  0000 C CNN
	1    8900 4400
	1    0    0    -1  
$EndComp
$Comp
L remote-control-rescue:+3.3V-RESCUE-capteur-capteur-rescue-remote-control-rescue #PWR013
U 1 1 5D078053
P 8900 5600
F 0 "#PWR013" H 8900 5560 30  0001 C CNN
F 1 "+3.3V" H 8900 5710 30  0000 C CNN
F 2 "" H 8900 5600 60  0000 C CNN
F 3 "" H 8900 5600 60  0000 C CNN
	1    8900 5600
	-1   0    0    1   
$EndComp
$Comp
L remote-control-rescue:C-RESCUE-capteur-capteur-rescue-remote-control-rescue C4
U 1 1 5D078054
P 5250 6700
F 0 "C4" H 5250 6800 40  0000 L CNN
F 1 "100nF" H 5256 6615 40  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 5288 6550 30  0001 C CNN
F 3 "~" H 5250 6700 60  0000 C CNN
	1    5250 6700
	0    -1   -1   0   
$EndComp
$Comp
L remote-control-rescue:+3.3V-RESCUE-capteur-capteur-rescue-remote-control-rescue #PWR08
U 1 1 5D078048
P 6900 5450
F 0 "#PWR08" H 6900 5410 30  0001 C CNN
F 1 "+3.3V" H 6900 5560 30  0000 C CNN
F 2 "" H 6900 5450 60  0000 C CNN
F 3 "" H 6900 5450 60  0000 C CNN
	1    6900 5450
	0    -1   -1   0   
$EndComp
$Comp
L remote-control-rescue:+3.3V-RESCUE-capteur-capteur-rescue-remote-control-rescue #PWR09
U 1 1 5D078056
P 7350 2550
F 0 "#PWR09" H 7350 2510 30  0001 C CNN
F 1 "+3.3V" H 7350 2660 30  0000 C CNN
F 2 "" H 7350 2550 60  0000 C CNN
F 3 "" H 7350 2550 60  0000 C CNN
	1    7350 2550
	0    1    1    0   
$EndComp
$Comp
L remote-control-rescue:GND-RESCUE-capteur-capteur-rescue-remote-control-rescue #PWR02
U 1 1 5D078057
P 4950 6750
F 0 "#PWR02" H 4950 6750 30  0001 C CNN
F 1 "GND" H 4950 6680 30  0001 C CNN
F 2 "" H 4950 6750 60  0000 C CNN
F 3 "" H 4950 6750 60  0000 C CNN
	1    4950 6750
	1    0    0    -1  
$EndComp
$Comp
L remote-control-rescue:GND-RESCUE-capteur-capteur-rescue-remote-control-rescue #PWR04
U 1 1 5D078058
P 6700 3000
F 0 "#PWR04" H 6700 3000 30  0001 C CNN
F 1 "GND" H 6700 2930 30  0001 C CNN
F 2 "" H 6700 3000 60  0000 C CNN
F 3 "" H 6700 3000 60  0000 C CNN
	1    6700 3000
	1    0    0    -1  
$EndComp
$Comp
L remote-control-rescue:+5V-power #PWR03
U 1 1 5D078059
P 6700 1550
F 0 "#PWR03" H 6700 1640 20  0001 C CNN
F 1 "+5V" H 6700 1640 30  0000 C CNN
F 2 "" H 6700 1550 60  0000 C CNN
F 3 "" H 6700 1550 60  0000 C CNN
	1    6700 1550
	1    0    0    -1  
$EndComp
$Comp
L remote-control-rescue:C_Small-Device-remote-control-rescue C6
U 1 1 5D07805A
P 7200 2750
F 0 "C6" H 7200 2850 40  0000 L CNN
F 1 "100nF" H 7206 2665 40  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 7238 2600 30  0001 C CNN
F 3 "~" H 7200 2750 60  0000 C CNN
	1    7200 2750
	-1   0    0    1   
$EndComp
$Comp
L remote-control-rescue:GND-RESCUE-capteur-capteur-rescue-remote-control-rescue #PWR07
U 1 1 5D07805C
P 6900 5150
F 0 "#PWR07" H 6900 5150 30  0001 C CNN
F 1 "GND" H 6900 5080 30  0001 C CNN
F 2 "" H 6900 5150 60  0000 C CNN
F 3 "" H 6900 5150 60  0000 C CNN
	1    6900 5150
	0    1    1    0   
$EndComp
Text Label 6800 5050 0    60   ~ 0
~MCLR
Text Label 6550 5350 0    60   ~ 0
PGC
Text Label 6650 5250 0    60   ~ 0
PGD
$Comp
L remote-control-rescue:PWR_FLAG-power #PWR_PIC01
U 1 1 5D07805D
P 6850 6350
F 0 "#PWR_PIC01" H 6850 6445 30  0001 C CNN
F 1 "PWR_FLAG" H 6850 6530 30  0000 C CNN
F 2 "" H 6850 6350 60  0000 C CNN
F 3 "" H 6850 6350 60  0000 C CNN
	1    6850 6350
	0    1    1    0   
$EndComp
$Comp
L remote-control-rescue:CP1-RESCUE-capteur-capteur-rescue-remote-control-rescue C8
U 1 1 5D07805F
P 9100 5450
F 0 "C8" H 9150 5550 50  0000 L CNN
F 1 "10uF" H 9150 5350 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 9100 5450 60  0001 C CNN
F 3 "~" H 9100 5450 60  0000 C CNN
	1    9100 5450
	0    -1   -1   0   
$EndComp
$Comp
L remote-control-rescue:CP1-RESCUE-capteur-capteur-rescue-remote-control-rescue C2
U 1 1 5D078060
P 4700 6700
F 0 "C2" H 4750 6800 50  0000 L CNN
F 1 "10uF" H 4750 6600 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 4700 6700 60  0001 C CNN
F 3 "~" H 4700 6700 60  0000 C CNN
	1    4700 6700
	0    -1   -1   0   
$EndComp
$Comp
L remote-control-rescue:CONN_6-capteur-rescue-remote-control-rescue P1
U 1 1 00000000
P 7350 5300
F 0 "P1" V 7300 5300 60  0000 C CNN
F 1 "CONN_6" V 7400 5300 60  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x06_P2.54mm_Vertical" H 7350 5300 60  0001 C CNN
F 3 "" H 7350 5300 60  0000 C CNN
	1    7350 5300
	1    0    0    -1  
$EndComp
NoConn ~ 5450 4450
$Comp
L remote-control-rescue:uA78M05-capteur-rescue-remote-control-rescue U5
U 1 1 5D078038
P 9800 4900
F 0 "U5" H 9950 4704 60  0000 C CNN
F 1 "UA78M05" H 9800 5100 60  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-223-3_TabPin2" H 9800 4900 60  0001 C CNN
F 3 "" H 9800 4900 60  0000 C CNN
	1    9800 4900
	0    -1   1    0   
$EndComp
$Comp
L remote-control-rescue:GND-RESCUE-capteur-capteur-rescue-remote-control-rescue #PWR017
U 1 1 5D078039
P 10200 4900
F 0 "#PWR017" H 10200 4900 30  0001 C CNN
F 1 "GND" H 10200 4830 30  0001 C CNN
F 2 "" H 10200 4900 60  0000 C CNN
F 3 "" H 10200 4900 60  0000 C CNN
	1    10200 4900
	0    -1   -1   0   
$EndComp
$Comp
L remote-control-rescue:+12V-power #PWR015
U 1 1 5D07803A
P 9750 4400
F 0 "#PWR015" H 9750 4350 20  0001 C CNN
F 1 "+12V" H 9750 4500 30  0000 C CNN
F 2 "" H 9750 4400 60  0000 C CNN
F 3 "" H 9750 4400 60  0000 C CNN
	1    9750 4400
	1    0    0    -1  
$EndComp
$Comp
L remote-control-rescue:CP1-RESCUE-capteur-capteur-rescue-remote-control-rescue C9
U 1 1 5D07802E
P 9950 5450
F 0 "C9" H 10000 5550 50  0000 L CNN
F 1 "10uF" H 10000 5350 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 9950 5450 60  0001 C CNN
F 3 "~" H 9950 5450 60  0000 C CNN
	1    9950 5450
	0    -1   -1   0   
$EndComp
$Comp
L remote-control-rescue:+5V-power #PWR016
U 1 1 5D07803C
P 9750 5600
F 0 "#PWR016" H 9750 5690 20  0001 C CNN
F 1 "+5V" H 9750 5690 30  0000 C CNN
F 2 "" H 9750 5600 60  0000 C CNN
F 3 "" H 9750 5600 60  0000 C CNN
	1    9750 5600
	-1   0    0    1   
$EndComp
NoConn ~ 4550 4450
NoConn ~ 4650 4450
NoConn ~ 4950 4450
NoConn ~ 5050 4450
NoConn ~ 5150 4450
Text Label 5950 3250 1    60   ~ 0
CANTX
Text Label 6050 3250 1    60   ~ 0
CANRX
Wire Wire Line
	9300 5450 9300 4900
Connection ~ 8900 5450
Wire Wire Line
	5200 6450 5450 6450
Connection ~ 5450 6700
Wire Wire Line
	5050 6700 5050 6450
Wire Wire Line
	4900 6700 4950 6700
Wire Wire Line
	4950 6700 4950 6750
Connection ~ 5050 6700
Wire Wire Line
	4500 6450 4750 6450
Connection ~ 4950 6700
Wire Wire Line
	6700 2850 6700 2900
Wire Wire Line
	6700 1550 6700 1650
Wire Wire Line
	6100 2450 6100 2900
Wire Wire Line
	6100 2900 6700 2900
Connection ~ 6700 2900
Wire Wire Line
	6750 5550 6750 6350
Wire Wire Line
	6750 6700 5450 6700
Wire Wire Line
	5450 6700 5450 6450
Wire Wire Line
	4150 5050 4150 4450
Wire Wire Line
	6350 4450 6350 5350
Wire Wire Line
	6450 4450 6450 5250
Wire Wire Line
	6850 6350 6750 6350
Connection ~ 6750 6350
Wire Wire Line
	7200 2550 7350 2550
Wire Wire Line
	7200 2550 7200 2650
Wire Wire Line
	9200 4900 9300 4900
Connection ~ 9300 4900
Wire Wire Line
	8900 5300 8900 5450
Wire Wire Line
	4500 6450 4500 6700
Wire Wire Line
	4900 6700 4900 6450
Wire Wire Line
	6900 5150 7000 5150
Wire Wire Line
	6450 5250 7000 5250
Wire Wire Line
	6350 5350 7000 5350
Wire Wire Line
	10150 5450 10150 4900
Connection ~ 9750 5450
Wire Wire Line
	10050 4900 10150 4900
Connection ~ 10150 4900
Wire Wire Line
	9750 5300 9750 5450
Wire Wire Line
	9750 4400 9750 4500
Wire Wire Line
	8900 4400 8900 4500
$Comp
L remote-control-rescue:R-RESCUE-capteur-capteur-rescue-remote-control-rescue R2
U 1 1 5D078047
P 7550 5200
F 0 "R2" V 7630 5200 40  0000 C CNN
F 1 "10K" V 7557 5201 40  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 7480 5200 30  0001 C CNN
F 3 "~" H 7550 5200 30  0000 C CNN
	1    7550 5200
	1    0    0    -1  
$EndComp
Wire Wire Line
	7550 5250 7550 5450
Connection ~ 7000 5250
Wire Wire Line
	7550 5150 7550 4950
Connection ~ 7000 5150
Wire Wire Line
	8900 5450 8900 5600
Wire Wire Line
	4950 6700 5050 6700
Wire Wire Line
	6700 2900 6700 3000
Wire Wire Line
	6750 6350 6750 6700
Wire Wire Line
	9300 4900 9350 4900
Wire Wire Line
	9750 5450 9750 5600
Wire Wire Line
	10150 4900 10200 4900
Wire Wire Line
	7000 5250 7550 5250
Wire Wire Line
	7000 5150 7550 5150
$Comp
L remote-control-rescue:Crystal_Small-Device Y1
U 1 1 5D0873A6
P 4800 4300
F 0 "Y1" H 4800 4400 50  0000 C CNN
F 1 "10Mhz" H 4800 4200 50  0000 C CNN
F 2 "Crystal:Crystal_HC18-U_Vertical" H 4800 4300 50  0001 C CNN
F 3 "" H 4800 4300 50  0001 C CNN
	1    4800 4300
	1    0    0    -1  
$EndComp
$Comp
L remote-control-rescue:C_Small-Device-remote-control-rescue C1
U 1 1 5D089370
P 4700 4000
F 0 "C1" H 4500 4050 50  0000 L CNN
F 1 "22pF" H 4500 3950 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D3.4mm_W2.1mm_P2.50mm" H 4700 4000 50  0001 C CNN
F 3 "" H 4700 4000 50  0001 C CNN
	1    4700 4000
	1    0    0    -1  
$EndComp
$Comp
L remote-control-rescue:C_Small-Device-remote-control-rescue C3
U 1 1 5D0896AC
P 4900 4000
F 0 "C3" H 4992 4046 50  0000 L CNN
F 1 "22pF" H 4992 3955 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D3.4mm_W2.1mm_P2.50mm" H 4900 4000 50  0001 C CNN
F 3 "" H 4900 4000 50  0001 C CNN
	1    4900 4000
	1    0    0    -1  
$EndComp
$Comp
L remote-control-rescue:GND-RESCUE-capteur-capteur-rescue-remote-control-rescue #PWR01
U 1 1 5D08A8C6
P 4800 3800
F 0 "#PWR01" H 4800 3800 30  0001 C CNN
F 1 "GND" H 4800 3730 30  0001 C CNN
F 2 "" H 4800 3800 60  0000 C CNN
F 3 "" H 4800 3800 60  0000 C CNN
	1    4800 3800
	-1   0    0    1   
$EndComp
Wire Wire Line
	4800 3800 4800 3900
Wire Wire Line
	4800 3900 4900 3900
Wire Wire Line
	4800 3900 4700 3900
Connection ~ 4800 3900
Wire Wire Line
	4700 4100 4700 4300
Wire Wire Line
	4900 4300 4900 4100
Wire Wire Line
	4900 4300 4900 4450
Wire Wire Line
	4900 4450 4850 4450
Connection ~ 4900 4300
Wire Wire Line
	4700 4300 4700 4450
Wire Wire Line
	4700 4450 4750 4450
Connection ~ 4700 4300
NoConn ~ 5350 4450
NoConn ~ 5250 4450
NoConn ~ 4250 4450
NoConn ~ 4350 4450
NoConn ~ 4450 4450
$Comp
L remote-control-rescue:ZPT-RFsol-remote-control-rescue U3
U 1 1 5D06B913
P 7750 3750
F 0 "U3" H 7750 4225 50  0000 C CNN
F 1 "ZPT" H 7750 4134 50  0000 C CNN
F 2 "local:RFsolutions_ZPT-D" H 7600 3750 50  0001 C CNN
F 3 "" H 7600 3750 50  0001 C CNN
	1    7750 3750
	1    0    0    -1  
$EndComp
$Comp
L remote-control-rescue:C-RESCUE-capteur-capteur-rescue-remote-control-rescue C5
U 1 1 5D06C1C4
P 6800 3700
F 0 "C5" H 6800 3800 40  0000 L CNN
F 1 "100nF" H 6806 3615 40  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 6838 3550 30  0001 C CNN
F 3 "~" H 6800 3700 60  0000 C CNN
	1    6800 3700
	1    0    0    -1  
$EndComp
Wire Wire Line
	6800 3500 7000 3500
Wire Wire Line
	7000 3500 7000 3600
Wire Wire Line
	7000 3600 7200 3600
Wire Wire Line
	7200 3800 7000 3800
Wire Wire Line
	7000 3800 7000 3900
Wire Wire Line
	7000 3900 6800 3900
$Comp
L remote-control-rescue:GND-RESCUE-capteur-capteur-rescue-remote-control-rescue #PWR06
U 1 1 5D0711B2
P 6800 3400
F 0 "#PWR06" H 6800 3400 30  0001 C CNN
F 1 "GND" H 6800 3330 30  0001 C CNN
F 2 "" H 6800 3400 60  0000 C CNN
F 3 "" H 6800 3400 60  0000 C CNN
	1    6800 3400
	-1   0    0    1   
$EndComp
Wire Wire Line
	6800 3400 6800 3500
Connection ~ 6800 3500
$Comp
L remote-control-rescue:+3.3V-RESCUE-capteur-capteur-rescue-remote-control-rescue #PWR05
U 1 1 5D0739C8
P 6750 3900
F 0 "#PWR05" H 6750 3860 30  0001 C CNN
F 1 "+3.3V" H 6750 4010 30  0000 C CNN
F 2 "" H 6750 3900 60  0000 C CNN
F 3 "" H 6750 3900 60  0000 C CNN
	1    6750 3900
	0    -1   -1   0   
$EndComp
Wire Wire Line
	6750 3900 6800 3900
Connection ~ 6800 3900
NoConn ~ 8300 3900
NoConn ~ 8300 3800
NoConn ~ 8300 3700
NoConn ~ 8300 3600
NoConn ~ 8300 3500
Wire Wire Line
	5650 4200 5650 4450
$Comp
L remote-control-rescue:R_Small-Device R1
U 1 1 5D06D0F4
P 7150 4000
F 0 "R1" H 7209 4046 50  0000 L CNN
F 1 "330" H 7209 3955 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 7150 4000 50  0001 C CNN
F 3 "~" H 7150 4000 50  0001 C CNN
	1    7150 4000
	1    0    0    -1  
$EndComp
$Comp
L remote-control-rescue:LED-Device D1
U 1 1 5D0700DD
P 6950 4100
F 0 "D1" H 6950 4000 50  0000 C CNN
F 1 "LED" H 6950 4200 50  0001 C CNN
F 2 "LED_THT:LED_D3.0mm" H 6950 4100 50  0001 C CNN
F 3 "~" H 6950 4100 50  0001 C CNN
	1    6950 4100
	-1   0    0    1   
$EndComp
Wire Wire Line
	7200 4000 7200 4200
Wire Wire Line
	5650 4200 7200 4200
Wire Wire Line
	6800 3900 6800 4100
Wire Wire Line
	7150 3900 7200 3900
Wire Wire Line
	7150 4100 7100 4100
$Comp
L remote-control-rescue:Conn_Coaxial-Connector J1
U 1 1 5D0839F1
P 7200 3250
F 0 "J1" V 7128 3351 50  0000 L CNN
F 1 "Conn_Coaxial" V 7083 3350 50  0001 L CNN
F 2 "Connector_Coaxial:SMA_Molex_73251-2200_Horizontal" H 7200 3250 50  0001 C CNN
F 3 " ~" H 7200 3250 50  0001 C CNN
F 4 "Molex 73251" H 7200 3250 50  0001 C CNN "P/N"
	1    7200 3250
	0    1    -1   0   
$EndComp
Wire Wire Line
	7200 3450 7200 3500
Wire Wire Line
	7000 3250 7000 3500
Connection ~ 7000 3500
Wire Wire Line
	6050 2250 6100 2250
Wire Wire Line
	5950 2050 6100 2050
Wire Wire Line
	7200 3700 5550 3700
Wire Wire Line
	5550 3700 5550 4450
NoConn ~ 5750 4450
NoConn ~ 5850 4450
NoConn ~ 6150 4450
Wire Wire Line
	5950 4450 5950 2050
Wire Wire Line
	6050 4450 6050 2250
Connection ~ 7200 2550
Wire Wire Line
	7200 2850 7200 2900
Wire Wire Line
	7200 2900 6700 2900
Wire Wire Line
	7200 2250 7850 2250
Wire Wire Line
	7200 2050 7650 2050
Wire Wire Line
	8450 1550 8450 1850
Wire Wire Line
	7550 1950 8150 1950
Wire Wire Line
	7550 1550 7750 1550
Wire Wire Line
	8150 1950 8250 1950
Wire Wire Line
	8450 1950 9200 1950
Wire Wire Line
	8450 2050 9200 2050
Wire Wire Line
	8450 2150 9200 2150
Wire Wire Line
	7550 1450 7550 1550
Wire Wire Line
	8450 1550 8250 1550
Wire Wire Line
	7450 1550 7550 1550
Wire Wire Line
	7650 2150 8450 2150
Wire Wire Line
	7650 2050 7650 2150
Wire Wire Line
	7850 2250 7850 2050
Wire Wire Line
	7850 2050 8450 2050
Connection ~ 8450 1550
Connection ~ 8450 1850
Connection ~ 7550 1950
Connection ~ 7550 1550
Connection ~ 8150 1950
Wire Wire Line
	8150 1450 8150 1950
Wire Wire Line
	8450 1450 8450 1550
Connection ~ 8450 1950
Connection ~ 8450 2050
Connection ~ 8450 2150
Wire Wire Line
	7400 1950 7550 1950
Wire Wire Line
	8450 1850 9200 1850
$Comp
L remote-control-rescue:PWR_FLAG-power #U01
U 1 1 5D07803E
P 7550 1450
F 0 "#U01" H 7550 1545 30  0001 C CNN
F 1 "PWR_FLAG" H 7550 1630 30  0000 C CNN
F 2 "" H 7550 1450 60  0000 C CNN
F 3 "" H 7550 1450 60  0000 C CNN
	1    7550 1450
	1    0    0    -1  
$EndComp
$Comp
L remote-control-rescue:D_Schottky_Small-Device D2
U 1 1 5D07803D
P 7850 1550
F 0 "D2" H 7850 1650 40  0000 C CNN
F 1 "PMEG4005EGWX" H 7850 1450 40  0000 C CNN
F 2 "Diode_SMD:D_SOD-123" H 7850 1550 60  0001 C CNN
F 3 "~" H 7850 1550 60  0000 C CNN
F 4 "PMEG4005EGWX" H 7850 1550 50  0001 C CNN "P/N"
	1    7850 1550
	1    0    0    -1  
$EndComp
$Comp
L remote-control-rescue:CP1-RESCUE-capteur-capteur-rescue-remote-control-rescue C7
U 1 1 5D07805E
P 7550 1750
F 0 "C7" H 7600 1850 50  0000 L CNN
F 1 "10uF" H 7600 1650 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 7550 1750 60  0001 C CNN
F 3 "~" H 7550 1750 60  0000 C CNN
	1    7550 1750
	1    0    0    -1  
$EndComp
$Comp
L remote-control-rescue:+12V-power #PWR011
U 1 1 5D07804D
P 7450 1550
F 0 "#PWR011" H 7450 1500 20  0001 C CNN
F 1 "+12V" H 7450 1650 30  0000 C CNN
F 2 "" H 7450 1550 60  0000 C CNN
F 3 "" H 7450 1550 60  0000 C CNN
	1    7450 1550
	0    -1   -1   0   
$EndComp
$Comp
L remote-control-rescue:GND-RESCUE-capteur-capteur-rescue-remote-control-rescue #PWR010
U 1 1 5D07804C
P 7400 1950
F 0 "#PWR010" H 7400 1950 30  0001 C CNN
F 1 "GND" H 7400 1880 30  0001 C CNN
F 2 "" H 7400 1950 60  0000 C CNN
F 3 "" H 7400 1950 60  0000 C CNN
	1    7400 1950
	0    1    1    0   
$EndComp
$Comp
L remote-control-rescue:PWR_FLAG-power #U02
U 1 1 5D07804B
P 8450 1450
F 0 "#U02" H 8450 1545 30  0001 C CNN
F 1 "PWR_FLAG" H 8450 1630 30  0000 C CNN
F 2 "" H 8450 1450 60  0000 C CNN
F 3 "" H 8450 1450 60  0000 C CNN
	1    8450 1450
	1    0    0    -1  
$EndComp
$Comp
L remote-control-rescue:PWR_FLAG-power #GND01
U 1 1 5D07804A
P 8150 1450
F 0 "#GND01" H 8150 1545 30  0001 C CNN
F 1 "PWR_FLAG" H 8150 1630 30  0000 C CNN
F 2 "" H 8150 1450 60  0000 C CNN
F 3 "" H 8150 1450 60  0000 C CNN
	1    8150 1450
	1    0    0    -1  
$EndComp
Text Label 9000 2050 0    60   ~ 0
CANN
Text Label 9000 2150 0    60   ~ 0
CANP
Text Label 9050 1950 0    60   ~ 0
GND
Text Label 9050 1850 0    60   ~ 0
12Vin
$Comp
L remote-control-rescue:CONN_4-capteur-rescue-remote-control-rescue P2
U 1 1 5D078049
P 8800 2000
F 0 "P2" V 8750 2000 50  0000 C CNN
F 1 "CONN_4" V 8850 2000 50  0000 C CNN
F 2 "TerminalBlock_Phoenix:TerminalBlock_Phoenix_PTSM-0,5-4-2.5-V-THR_1x04_P2.50mm_Vertical" H 8800 2000 60  0001 C CNN
F 3 "" H 8800 2000 60  0000 C CNN
F 4 "1770979" H 8800 2000 50  0001 C CNN "P/N"
	1    8800 2000
	1    0    0    -1  
$EndComp
$Comp
L remote-control-rescue:CONN_4-capteur-rescue-remote-control-rescue P3
U 1 1 5D078055
P 9550 2000
F 0 "P3" V 9500 2000 50  0000 C CNN
F 1 "CONN_4" V 9600 2000 50  0000 C CNN
F 2 "TerminalBlock_Phoenix:TerminalBlock_Phoenix_PTSM-0,5-4-2.5-V-THR_1x04_P2.50mm_Vertical" H 9550 2000 60  0001 C CNN
F 3 "" H 9550 2000 60  0000 C CNN
F 4 "1770979" H 9550 2000 50  0001 C CNN "P/N"
	1    9550 2000
	1    0    0    -1  
$EndComp
Wire Wire Line
	4150 5050 7000 5050
$Comp
L remote-control-rescue:1.5KExxA-Diode TVS1
U 1 1 5D0EC305
P 8250 1750
F 0 "TVS1" V 8204 1829 50  0000 L CNN
F 1 "SMBJ16A" V 8295 1829 50  0000 L CNN
F 2 "Diode_SMD:D_SMB_Handsoldering" H 8250 1550 50  0001 C CNN
F 3 "" H 8200 1750 50  0001 C CNN
F 4 "SMBJ16A" H 8250 1750 50  0001 C CNN "P/N"
	1    8250 1750
	0    1    1    0   
$EndComp
Wire Wire Line
	8250 1600 8250 1550
Connection ~ 8250 1550
Wire Wire Line
	8250 1550 7950 1550
Wire Wire Line
	8250 1900 8250 1950
Connection ~ 8250 1950
Wire Wire Line
	8250 1950 8450 1950
Wire Wire Line
	7000 5550 6750 5550
Wire Wire Line
	6250 4450 6250 5550
Connection ~ 6750 5550
Wire Wire Line
	6750 5550 6250 5550
Wire Wire Line
	6900 5450 7000 5450
Text Label 7200 3500 0    50   ~ 0
ANT
$EndSCHEMATC
