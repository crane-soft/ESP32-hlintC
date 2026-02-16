# ESP32-hlintC
## ESP32 High Priority Interrupts using C

### Preface 
The ESP-IDF, and therefore also the Arduino framework which is based on ESP-IDF, makes it possible to write [high-priority interrupts](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-guides/hlinterrupts.html "high-priority interrupts") in assembly language, resulting in very low interrupt latencies.

A demo  [example](https://github.com/espressif/esp-idf/blob/v4.4.2/components/esp_system/port/soc/esp32s3/highint_hdl.S "example") can be found here.

But this project demonstrates a way to write the interrupt service routine in C especially for the ESP-S3 by using PlatformIO.
The ESP-S3 is based on the Xtensa CPU core, which features a so-called [windowed register function](https://sachin0x18.github.io/posts/demystifying-xtensa-isa/#fn:1 "windowed register function"). This makes it considerably more difficult to save the correct context and switch stacks without significantly increasing interrupt latency.

Therefore, I decided to switch off this feature for the Interrupt Service Routine, which makes things significantly easier.
We can do this with the [Xtensa compiler option](Xtensa compiler option "Xtensa compiler option") **-mabi=call0** 

However, we must be take care not to call any C functions that were not compiled with these options, nor any functions from the IDF library.
I think for time critical interrupt service this not a real limitation and definitely better than programming in assembler.

My test board is a **[XIAO-ESP-S3](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/ "XIAO-ESP-S3")**, and my test application is an **I2C sniffer**`` that uses the GPIO interrupt to capture the I2C signals. 


### How it works

In platformio.ini you will find the extra sript parameter:
> extra_scripts = pre:compile_hli.py

Which is called at the beginning of a build process before the main script of [Development Platforms.](https://docs.platformio.org/en/latest/scripting/launch_types.html "Development Platforms.")

> #### compile_hli.py does the following:
> - Searches for all C source files in the project folder ./hli
> - Checks which one should be compiled and compiles it using the special  compiler option **-mabi=call0 **
> - The result is an assembler file which is saved in the ./src folder.
> - All generated assembler files are scanned to determine the registers usage.
> - The interrupt assembler template ./hli/hlevelint.S is modified by adding assembler instructions  to  save and restore all used registers. 
> - The modified interrrupt file is also stored in the ./src folder.

The build sytem now will compile hopefully the generated assembler files together with all other profect files.

### Getting started

- Create a folder named **./hli** next to the folder **./scr**.
- Make sure that the interrupt assembler template named **hlevelint.S** from this project is located in this folder.
- Create your own interrupt C source code in this folder. You can use one or more files.
- Your interrupt entry function must be named **hlintC_main()**. Alternatively, you can change the call in hlevelint.S.
- Take care not to call any C functions that were not compiled with these options, nor any functions from the IDF library.
- Add the exttra_script instruction into the **platformio.ini **file.
- Make sure the **compile_hli.py** file is located in the root directory.
