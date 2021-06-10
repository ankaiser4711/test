#include <EloquentTinyML.h>

/*
possible ways to load a model
 * als C/C++ headerfile
 * load from sdcard
 * load from spiffs
 * load via http
*/

// !!! UNCOMMENT ONLY 1 LINE !!!

//#define MODEL_INCLUDE_TYPE_CPP
//#define MODEL_INCLUDE_TYPE_SDCARD
#define MODEL_INCLUDE_TYPE_SPIFFS
//#define MODEL_INCLUDE_TYPE_HTTP

const int TENSOR_ARENA_SIZE = 8*1024;

void dumpModel(const char* lpszTitle, uint8_t *model, int model_len)
{
    Serial.print("\n ### dump ");
    Serial.print(lpszTitle);
    Serial.print(" - ");
    Serial.println(model_len);

    for (size_t i = 0; i < 10; i++)
    {
        int val = model[i];
        Serial.print(" 0x");
        if (val < 16)
          Serial.print("0");
        Serial.print(val, HEX);
    }

    Serial.println("\n...");

    for (size_t i = model_len-1; i > model_len-11; i--)
    {
        int val = model[i];
        Serial.print(" 0x");
        if (val < 16)
          Serial.print("0");
        Serial.print(val, HEX);
    }

    Serial.println();
}

#ifdef MODEL_INCLUDE_TYPE_CPP

  #include "model_data.h"
  //#include "model_data_xxd.h"

  const int NUMBER_OF_INPUTS  = CNN_MODEL_INPUTS;
  const int NUMBER_OF_OUTPUTS = CNN_MODEL_OUTPUTS;

  const char* getLabel(int idx)
  {
      return CNN_MODEL_getLabel(idx);
  }

#else

  const int NUMBER_OF_INPUTS  = 26;
  const int NUMBER_OF_OUTPUTS = 12;

  const char* getLabel(int idx) //location
  {
      switch (idx)
      {
      case 0: return "arbeitszimmer";
      case 1: return "bad_oben";
      case 2: return "bad_unten";
      case 3: return "elektrowerkstatt";
      case 4: return "gaestezimmer";
      case 5: return "hof";
      case 6: return "kueche";
      case 7: return "loetraum";
      case 8: return "schlafzimmer";
      case 9: return "terrasse";
      case 10: return "werkstatt";
      case 11: return "wohnzimmer";
      default: return "Houston we have a problem";
      }
  }

#endif

///////////

#ifdef MODEL_INCLUDE_TYPE_CPP

  Eloquent::TinyML::TfLite<CNN_MODEL_INPUTS, CNN_MODEL_OUTPUTS, TENSOR_ARENA_SIZE> ml;

  uint8_t dummy = 1;
  uint8_t *model = &dummy; //indicate valid model

  void subsystemsetup()
  {
      if (!ml.begin(model_data))
      {
          Serial.println("Cannot inialize model");
          Serial.println(ml.errorMessage());
      }
      else
      {
          Serial.println("Model started");
      }

      dumpModel("cpp model", (uint8_t*)model_data, model_data_len);
  }

#endif

///////////

#ifdef MODEL_INCLUDE_TYPE_SDCARD

  #include <SPI.h>
  #include <SD.h>

  const char* MODELFILE = "/model_data.tflite";

  uint8_t *model;
  Eloquent::TinyML::TfLite<HTTP_NUMBER_OF_INPUTS, HTTP_NUMBER_OF_OUTPUTS, TENSOR_ARENA_SIZE> ml;

  void loadModel()
  {
      File file = SD.open(MODELFILE, FILE_READ);
      size_t modelSize = file.size();

      Serial.print("load model '");
      Serial.print(MODELFILE);
      Serial.print("' from sdcard. size=");
      Serial.println(modelSize);

      // allocate memory
      model = (uint8_t*) malloc(modelSize);

      // copy data from file
      for (size_t i = 0; i < modelSize; i++)
          model[i] = file.read();

      file.close();

      dumpModel("sdcard model", model, modelSize);
  }

  void subsystemsetup()
  {
      SPI.begin();
      delay(500);

      if (!SD.begin(4))
      {
          Serial.println("Cannot init SD");
          return;
      }

      loadModel();

      if (!model)
      {
          Serial.println("model not loaded!");
          return;
      }
      else if (!ml.begin(model))
      {
          Serial.println("Cannot inialize model");
          Serial.println(ml.errorMessage());
      }
      else
      {
          Serial.println("Model started");
      }
  }

#endif

///////////

#ifdef MODEL_INCLUDE_TYPE_SPIFFS

  #include <FS.h>
  #include <SPIFFS.h>

  const char* MODELFILE = "/model_data.tflite";

  uint8_t *model;
  Eloquent::TinyML::TfLite<NUMBER_OF_INPUTS, NUMBER_OF_OUTPUTS, TENSOR_ARENA_SIZE> ml;

  void loadModel()
  {
      File file = SPIFFS.open(MODELFILE, "rb");
      size_t modelSize = file.size();

      Serial.print("load model '");
      Serial.print(MODELFILE);
      Serial.print("' from spiffs. size=");
      Serial.println(modelSize);

      // allocate memory
      model = (uint8_t*) malloc(modelSize);

      // copy data from file
      for (size_t i = 0; i < modelSize; i++)
          model[i] = file.read();

      file.close();
      SPIFFS.end();

      dumpModel("spiffs model", model, modelSize);
  }

  void subsystemsetup()
  {
      SPIFFS.begin(true);
      delay(500);

      loadModel();

      if (!model)
      {
          Serial.println("model not loaded!");
          return;
      }
      else if (!ml.begin(model))
      {
          Serial.println("Cannot inialize model");
          Serial.println(ml.errorMessage());
      }
      else
      {
          Serial.println("Model started");
      }
  }

#endif

///////////

#ifdef MODEL_INCLUDE_TYPE_HTTP

  #include <WiFi.h>
  #include <HTTPClient.h>

  const char* SSID = "WLAN-3BD843";
  const char* PASS = "4335655491736668";
  const char* REQUESTMODEL = "http://scherwel.dyndns.org:8000/model_data.tflite";

  uint8_t *model;
  Eloquent::TinyML::TfLite<NUMBER_OF_INPUTS, NUMBER_OF_OUTPUTS, TENSOR_ARENA_SIZE> ml;

  void wifi_connect()
  {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(SSID);
      WiFi.begin(SSID, PASS);
      delay(500);

      while (WiFi.status() != WL_CONNECTED)
      {
        delay(500);
        Serial.print(".");
      }

      Serial.println("Connected to wifi");
  }

  void http_get()
  {
      Serial.print("load model '");
      Serial.print(REQUESTMODEL);
      Serial.println("...");

      if (WiFi.status() != WL_CONNECTED)
      {
        Serial.println("WiFi Disconnected");
        return;
      }

      HTTPClient http;

      http.begin(REQUESTMODEL);

      int httpResponseCode = http.GET();

      if (httpResponseCode <= 0)
      {
        /*
#define HTTPC_ERROR_CONNECTION_REFUSED  (-1)
#define HTTPC_ERROR_SEND_HEADER_FAILED  (-2)
#define HTTPC_ERROR_SEND_PAYLOAD_FAILED (-3)
#define HTTPC_ERROR_NOT_CONNECTED       (-4)
#define HTTPC_ERROR_CONNECTION_LOST     (-5)
#define HTTPC_ERROR_NO_STREAM           (-6)
#define HTTPC_ERROR_NO_HTTP_SERVER      (-7)
#define HTTPC_ERROR_TOO_LESS_RAM        (-8)
#define HTTPC_ERROR_ENCODING            (-9)
#define HTTPC_ERROR_STREAM_WRITE        (-10)
#define HTTPC_ERROR_READ_TIMEOUT        (-11)
        */

        Serial.print("ERR: load model failed - Error code: ");
        Serial.println(httpResponseCode);
        http.end();
        return;
      }

      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      int modelSize = http.getSize();
      model = (uint8_t*) malloc(modelSize);

      Serial.print("Model size is: ");
      Serial.println(modelSize);

      WiFiClient * stream = http.getStreamPtr();
      int nLoaded = stream->readBytes(model, modelSize);

      if (nLoaded != modelSize)
      {
        Serial.print("Error to less bytes loaded: ");
        Serial.print(nLoaded);
        http.end();
        return;
      }

      http.end();

      Serial.println("model loaded!");

      dumpModel("http model", model, modelSize);
  }

  void subsystemsetup()
  {
      delay(500);

      wifi_connect();
      http_get();

      if (!model)
      {
          Serial.println("model not loaded!");
          return;
      }
      else if (!ml.begin(model))
      {
          Serial.println("Cannot inialize model");
          Serial.println(ml.errorMessage());
      }
      else
      {
          Serial.println("Model started");
      }
  }

#endif

//############################

void setup()
{
  Serial.begin(115200);
  delay(50);
  while (Serial.available())
    Serial.read();
  Serial.println();

  Serial.println("*** tflitearduino (c) a.kaiser 2021 ***");

  Serial.print("Sketch:   "); Serial.println(__FILE__);
  Serial.print("Uploaded: "); Serial.println(__DATE__);
  Serial.println();

  subsystemsetup();
}

void loop()
{
    if (!model)
    {
        Serial.println("model not loaded!");
        delay(1000);
        return;
    }

    float x_test[][NUMBER_OF_INPUTS] = {
/* elektrowerkstatt */ { -85.00,0.00,0.00,0.00,0.00,-66.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,-74.00,-74.00,0.00,0.00,0.00,-89.00,0.00,0.00,0.00,0.00,0.00},
/* gaestezimmer     */ { 0.00,0.00,0.00,-87.00,-78.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00},
/* schlafzimmer     */ { 0.00,-77.00,0.00,0.00,-60.00,0.00,0.00,-85.00,-88.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,-84.00,0.00,0.00,0.00,0.00,0.00,0.00},
        };

    float y_pred[NUMBER_OF_OUTPUTS] = {0};

    int cnt = sizeof(x_test) / sizeof(x_test[0]);
    for (uint8_t i = 0; i < cnt; i++)
    {
      uint32_t start = micros();

      ml.predict(x_test[i], y_pred);

      uint32_t timeit = micros() - start;
      Serial.println();
      Serial.print("It took ");
      Serial.print(timeit);
      Serial.println(" micros to run inference");

      Serial.print("Predicted proba are: ");

      for (int i = 0; i < NUMBER_OF_OUTPUTS; i++)
      {
          Serial.print(y_pred[i]);
          Serial.print(i == 9 ? '\n' : ',');
      }

      uint8_t pred_class = 0;
      float max_proba = y_pred[0];

      for (int i = 1; i < NUMBER_OF_OUTPUTS; i++)
      {
          if (y_pred[i] > max_proba)
          {
              pred_class = i;
              max_proba = y_pred[i];
          }
      }

      Serial.print("Predicted output is: ");
      Serial.print(pred_class);
      Serial.print(" - label: '");
      Serial.print(getLabel(pred_class));

      Serial.println("'");
    }

    Serial.println("\n#############\n");
    delay(1000);
}
