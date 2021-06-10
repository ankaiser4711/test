# based on: https://eloquentarduino.github.io/2021/06/howto-load-tensorflow-lite-tinyml-model-from-internet-on-arduino/

SDCard path not tested!

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

