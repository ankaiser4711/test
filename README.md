# based on: https://eloquentarduino.github.io/2021/06/howto-load-tensorflow-lite-tinyml-model-from-internet-on-arduino/

SDCard path not tested!

board: esp32 wroover

possible ways to load a model
 * als C/C++ headerfile
 * load from sdcard
 * load from spiffs
 * load via http

change way to load model within tflitearduino.ino:

// !!! UNCOMMENT ONLY 1 LINE !!!

 * //#define MODEL_INCLUDE_TYPE_CPP
 * //#define MODEL_INCLUDE_TYPE_SDCARD
 * #define MODEL_INCLUDE_TYPE_SPIFFS
 * //#define MODEL_INCLUDE_TYPE_HTTP

Benchmark:
==========
   ### dump cpp model - 21280
   0x1C 0x00 0x00 0x00 0x54 0x46 0x4C 0x33 0x14 0x00
  ...
   0x00 0x00 0x00 0x04 0x00 0x04 0x00 0x04 0xFF 0xFF

  * It took 1971 micros to run inference
  * It took 723 micros to run inference
  * It took 725 micros to run inference
  * It took 728 micros to run inference
  * It took 726 micros to run inference
  * It took 714 micros to run inference

  ##############

   ### dump http model - 21280
   0x1C 0x00 0x00 0x00 0x54 0x46 0x4C 0x33 0x14 0x00
  ...
   0x00 0x00 0x00 0x04 0x00 0x04 0x00 0x04 0xFF 0xFF

  * It took 561 micros to run inference
  * It took 325 micros to run inference
  * It took 316 micros to run inference
  * It took 317 micros to run inference
  * It took 316 micros to run inference
  * It took 324 micros to run inference

  ##############

   ### dump spiffs model - 21280
   0x1C 0x00 0x00 0x00 0x54 0x46 0x4C 0x33 0x14 0x00
  ...
   0x00 0x00 0x00 0x04 0x00 0x04 0x00 0x04 0xFF 0xFF

  * It took 563 micros to run inference
  * It took 312 micros to run inference
  * It took 319 micros to run inference
  * It took 312 micros to run inference
  * It took 312 micros to run inference
  * It took 321 micros to run inference

