avrdude -C ../avrdude.conf -p atmega168 -c arduino -P COM3 -b 19200 -D -V -U Release/BoardTest.hex
