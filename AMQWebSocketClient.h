#ifndef __AMQWEBSOCKETCLIENT_H_
#define __AMQWEBSOCKETCLIENT_H_

#include <Arduino.h>
#include <Stream.h>
#include "Client.h"
#include "WebSocketsClient.h"
#include "CircularByteBuffer.h"
#include "sha256.h"
#include "Utils.h"
#include "AWSClient2.h"


//#define DEBUG_WEBSOCKET_MQTT(...) os_printf( __VA_ARGS__ )
#define DEBUG_WEBSOCKETS(...) Serial.printf( __VA_ARGS__ )

// #ifndef DEBUG_WEBSOCKET_MQTT
// #define DEBUG_WEBSOCKET_MQTT(...)
// #define NODEBUG_WEBSOCKET_MQTT
// #endif

class AMQWebSocketClient : public Client, private WebSocketsClient {
public:

  //bufferSize defines the size of the circular byte buffer that provides the interface between messages arrived in websocket layer and byte reads from mqtt layer	
  AMQWebSocketClient (unsigned int bufferSize = 1000);
  ~AMQWebSocketClient();

  

  int connect(IPAddress ip, uint16_t port);
  int connect(const char *host, uint16_t port);

  void putMessage (byte* buffer, int length);
  size_t write(uint8_t b);
  size_t write(const uint8_t *buf, size_t size);
  int available();
  int read();
  int read(uint8_t *buf, size_t size);

  int peek();
  void flush();
  void stop();
  uint8_t connected() ;
  operator bool();

  bool getUseSSL ();
  
  AMQWebSocketClient& setUseSSL (bool value);
  AMQWebSocketClient& setPath(const char * path);
        
  protected:
  //generate AWS signed path
  char* generateAWSPath (uint16_t port);
  
  //convert the month info
  String getMonth(String sM);
  //get current time (UTC) from aws service (used to sign)
  char* getCurrentTime(void);
  
  //static instance of aws websocket client
  static AMQWebSocketClient* instance;
  //keep the connection state
  bool _connected;  
  //websocket callback
  static void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
  
  private:
  
  //enable ssl... if your using mqtt over websockets at AMQ IoT service, it must be enabled
  bool useSSL;

  //connection timeout (but it seems it is not working as I've expected... many I should control it by the receipt of the connection message)
  long connectionTimeout;

  char* path;
  
  //circular buffer to keep incoming messages from websocket
  CircularByteBuffer bb;  
};

#endif
