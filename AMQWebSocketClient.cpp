#include "AMQWebSocketClient.h"

//work as a singleton to be used by websocket layer message callback
AMQWebSocketClient* AMQWebSocketClient::instance = NULL;

//callback to handle messages coming from the websocket layer
void AMQWebSocketClient::webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            DEBUG_WEBSOCKET_MQTT("[AWSc] Disconnected!\n");
			AMQWebSocketClient::instance->stop ();			
            break;
        case WStype_CONNECTED:            
            DEBUG_WEBSOCKET_MQTT("[AWSc] Connected to url: %s\n",  payload);
			AMQWebSocketClient::instance->_connected = true;
            break;
        case WStype_TEXT:
            DEBUG_WEBSOCKET_MQTT("[WSc] get text: %s\n", payload);
            AMQWebSocketClient::instance->putMessage (payload, length);
            break;
        case WStype_BIN:
            DEBUG_WEBSOCKET_MQTT("[WSc] get binary length: %u\n", length);
            //hexdump(payload, length);
            AMQWebSocketClient::instance->putMessage (payload, length);
            break;
    }
}

//constructor
AMQWebSocketClient::AMQWebSocketClient (unsigned int bufferSize) {
    useSSL = true;
    connectionTimeout = 5000; //5 seconds
    AMQWebSocketClient:instance = this;
    onEvent(AMQWebSocketClient::webSocketEvent);
    path = NULL;
	_connected = false;	
    bb.init (bufferSize); //1000 bytes of circular buffer... maybe it is too big
}

//destructor
AMQWebSocketClient::~AMQWebSocketClient(void) {
    if (path != NULL)
        delete[] path;
}

AMQWebSocketClient& AMQWebSocketClient::setPath(const char * path) {
    int len = strlen(path) + 1;
    this->path = new char[len]();
    strcpy(this->path, path);
	return *this;
}

int AMQWebSocketClient::connect(IPAddress ip, uint16_t port){
	  return connect (awsDomain,port);
}

int AMQWebSocketClient::connect(const char *host, uint16_t port) {
	//make sure it is disconnect first
	  const char* protocol = "mqtt";
	  stop ();	
	  char* path = this->path;
	   
	  if (useSSL == true)
          //const char *host, uint16_t port, const char * url, const char * fingerprint, const char * protocol
		  beginSSL (host,port,path,"",protocol);
	  else
		  begin (host,port,path,protocol);
	  long now = millis ();
	  while ( (millis ()-now) < connectionTimeout) {
		  loop ();		  
		  if (connected () == 1) {		  
			  return 1;
		  }
		  delay (10);
	  }
	  
	  return 0;
}

//store messages arrived by websocket layer to be consumed by mqtt layer through the read funcion
void AMQWebSocketClient::putMessage (byte* buffer, int length) {
	bb.push (buffer,length);	
}

size_t AMQWebSocketClient::write(uint8_t b) {
	if (_connected == false)
	  return -1;
  return write (&b,1);
}

//write through websocket layer
size_t AMQWebSocketClient::write(const uint8_t *buf, size_t size) {
  if (_connected == false)
	  return -1;
  if (sendBIN (buf,size))
	  return size;
  return 0;
}

//return with there is bytes to consume from the circular buffer (used by mqtt layer)
int AMQWebSocketClient::available(){
  //force websocket to handle it messages
  if (_connected == false)
	  return false;
  loop ();
  return bb.getSize ();
}

//read from circular buffer (used by mqtt layer)
int AMQWebSocketClient::read() {
	if (_connected == false)
	  return -1;
	return bb.pop ();
}

//read from circular buffer (used by mqtt layer)
int AMQWebSocketClient::read(uint8_t *buf, size_t size) {
	if (_connected == false)
	  return -1;
	int s = size;
	if (bb.getSize()<s)
		s = bb.getSize ();

	bb.pop (buf,s);	
	return s;
};

int AMQWebSocketClient::peek() {
	return bb.peek ();
}

void AMQWebSocketClient::flush() {

}

void AMQWebSocketClient::stop() {
	if (_connected == true) {		
		_connected = false;
		bb.clear ();
	}
	disconnect ();
}

uint8_t AMQWebSocketClient::connected() {
return _connected;
};

AMQWebSocketClient::operator bool() {
	return _connected;
};


bool AMQWebSocketClient::getUseSSL () {
  return useSSL;
}

AMQWebSocketClient& AMQWebSocketClient::setUseSSL (bool value) {
  useSSL = value;
  return *this;
}
