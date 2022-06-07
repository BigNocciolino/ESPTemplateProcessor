#ifndef ESP_TEMPLATE_PROCESSOR_H
#define ESP_TEMPLATE_PROCESSOR_H

#ifdef ESP8266
  #define WebServer ESP8266WebServer
  #include <ESP8266WebServer.h>
#else
  #include <WebServer.h>
#endif

// Comment this line to use SPIFSS
#define USE_LITTLEFS

// Uncomment this line to enable serial debug
//#define ESP_TEMPLATE_PROCESSOR_DEBUG

#include <FS.h>

// Move to little fs
#ifdef USE_LITTLEFS
  #define SPIFFS LittleFS
  #include <LittleFS.h>
#else
  #include <SPIFFS.h>
#endif

typedef String ProcessorCallback(const String& key);

class ESPTemplateProcessor {
  public:
    ESPTemplateProcessor(WebServer& _server) :
      server(_server)
    {
    }

    bool send(const String& filePath, ProcessorCallback& processor, char bookend = '%')
    {
      // Open file.
      if(!SPIFFS.exists(filePath)) {
        #ifdef ESP_TEMPLATE_PROCESSOR_DEBUG
          Serial.print("Cannot process "); Serial.print(filePath); Serial.println(": Does not exist.");
        #endif
        return false;
      }

      File file = SPIFFS.open(filePath, "r");
      if (!file) {
        #ifdef ESP_TEMPLATE_PROCESSOR_DEBUG
          Serial.print("Cannot process "); Serial.print(filePath); Serial.println(": Failed to open.");
        #endif
        return false;
      }

      server.setContentLength(CONTENT_LENGTH_UNKNOWN);
      server.sendHeader("Content-Type","text/html",true);
      server.sendHeader("Cache-Control","no-cache");
      server.send(200);
      //server.sendContent(<chunk>)

      // Process!
      static const uint16_t MAX = 100;
      String buffer;
      int bufferLen = 0;
      String keyBuffer;
      int val;
      char ch;
      while ((val = file.read()) != -1) {
        ch = char(val);
        
        // Lookup expansion.
        if (ch == bookend) {
          // Clear out buffer.
          server.sendContent(buffer);
          buffer = "";
          bufferLen = 0;

          // Process substitution.
          keyBuffer = "";
          bool found = false;
          while (!found && (val = file.read()) != -1) {
            ch = char(val);
            if (ch == bookend) {
              found = true;
            } else {
              keyBuffer += ch;
            }
          }
          
          // Check for bad exit.
          if (val == -1 && !found) {
            #ifdef ESP_ESP_TEMPLATE_PROCESSOR_DEBUG
              Serial.print("Cannot process "); Serial.print(filePath); Serial.println(": Unable to parse.");
            #endif
            return false;
          }

          // Get substitution
          String processed = processor(keyBuffer);
          #ifdef ESP_ESP_ESP_TEMPLATE_PROCESSOR_DEBUG
            Serial.print("Lookup '"); Serial.print(keyBuffer); Serial.print("' received: "); Serial.println(processed);
          #endif
          server.sendContent(processed);
        } else {
          bufferLen++;
          buffer += ch;
          if (bufferLen >= MAX) {
            server.sendContent(buffer);
            bufferLen = 0;
            buffer = "";
          }
        }
      }

      if (val == -1) {
        server.sendContent(buffer);
        server.sendContent("");
        return true;
      } else {
        #ifdef ESP_ESP_TEMPLATE_PROCESSOR_DEBUG
          Serial.print("Failed to process '"); Serial.print(filePath); Serial.println("': Didn't reach the end of the file.");
        #endif
        return false;
      }
    }


  private:
    WebServer &server;
};

#endif
