# TI_FuelGaugeProgrammer
Programs firmware into a TI Lithium-ion fuel gauge such as the BQ27542.  Status information is sent over serial port.  The user must send 'f' to start programming.

# Instructions
First, use TI's Battery Management Studio to develop firmware settings for your
fuel gauge.  Export BQ.FS or DQ.FS files.

Choose either the BQ.FS or the DQ.FS file to use for programming.  Rename the
desired file "firmware.fs".

Use the FuelGaugeProgrammer.py Python script, which will convert "firmware.fs" to "data.h", which is used by the Arduino/Energia sketch.

Use the FuelGaugeProgrammer.ino sketch in Arduino or Energia to compile and upload to a suitable Arduino or Energia board.

Connect the Arduino or Energia board to the target fuel gauge (if using a pack-side gauge, a battery must be connected as well), as well as a serial port (which is used to control and monitor the programming process).

Using a PC terminal program, connect to the Arduino/Energia's serial port at a baud rate of 115200.  If the board is reset, you should see the text "Press 'f' to program fuel gauge...".

Press 'f'.  The programming progress will be shown on the serial port, followed by a success or fail message.

If the program fails, the fuel gauge will be unresponsive.  You can use TI's EV2400 along with TI's Battery Management Studio to attempt to recover the gauge by reprogramming.