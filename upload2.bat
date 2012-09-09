avrdude -C ../avrdude.conf -p atmega328p -c arduino -P COM4 -b 57600 -D -V -U Release/BoardTest.hex
