from ssd1325 import *
#Open the SPI bus 0
spibus0 = spibus(32766,0)

#Now test the OLED screen
spibus0.oled_init()

if len(sys.argv) > 1:
	spibus0.setOverflowMode(int(sys.argv[1]), 1)

if len(sys.argv) > 2:
	spibus0.write_string(2, 12, sys.argv[2])
#spibus0.write_string(2, 30, "Should scroll.")
#spibus0.write_string(2, 50, "No Hor scroll.")
#spibus0.write_string(2, 70, 'Esto va afuera.')
spibus0.write_paragraph("This is the first line of the paragraph.\nThis is the second one.\nA thrid line fits well.\nLast one.")
spibus0.spi_write_80()

#Test lights:
spibus0.gpioexport(69, 'C5')
spibus0.gpioexport(70, 'C6')
spibus0.gpioexport(71, 'C7')

spibus0.gpiosetdir('C5',"out");
spibus0.gpiosetdir('C6',"out");
spibus0.gpiosetdir('C7',"out");

spibus0.gpioclearbits('C5')
spibus0.gpioclearbits('C6')
spibus0.gpioclearbits('C7')

spibus0.gpiosetbits('C5')
if spibus0.yCursor > 64:
	spibus0.setVerticalScroll(0x00, 3, 2, 200)
spibus0.gpioclearbits('C5')
spibus0.gpiosetbits('C7')
#spibus0.setVerticalScroll(0x00, 2, 5, 200)
#spibus0.setHorizontalScroll(2, 40, 5, 200)