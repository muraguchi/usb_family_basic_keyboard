# usb_family_basic_keyboard

This is USB Family BASIC keyboard bridge with AVR328p and V-USB.

# Circuit 

AVR 328p pinout
|Pin|Func|
|1|RESET|	
|2|PD0 RXD (Unused)	
|3|PD1 TXD	(Unused)
|4|PD2	USB D+|
|5|PD3	USB D-|
|6|PD4	(Unused)|
|7|VCC	|
|8|GND	|
|9|XTAL1	16MHz crystal|
|10|XTAL2	16MHz crystal|
|11|PD5	NC|
|12|PD6	NC|
|13|PD7	NC|
|14|PB0	(ACT LED)|
|15|PB1	NC|
|16|PB2	NC|
|17|PB3	NC|
|18|PB4	NC|
|19|PB5	NC|
|20|AVCC	|
|21|AREF NC|
|22|GND	|
|23|PC0	D0 (FC CON P7)|
|24|PC1	D1 (FC CON P6)|
|25|PC2	D2 (FC CON P5)|
|26|PC3	D3 (FC CON P4)|
|27|PC4	OUT0 (FC CON P12)|
|28|PC5	OUT1 (FC CON P11)|

FC controller 15pin pinout

|1|GND|
|2|NC|
|3|NC|
|4|D3 (AVR PIN26 PC3)|
|5|D2 (AVR PIN25 PC2)|
|6|D1 (AVR PIN24 PC1)|
|7|D0 (AVR PIN23 PC0)|
|8|NC|
|9|NC|
|10|OUT2 (pull up to high)|
|11|OUT1 (AVR PIN28 PC5)|
|12|OUT0 (AVR PIN27 PC4)|
|13|NC|
|14|NC|
|15|+5V|
