#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "http_request.h"

#define MAX_REQUEST_SIZE 1024
#define SSID ""
#define PASSWORD ""

WiFiClient client;
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println();

  WiFi.begin(SSID, PASSWORD);

  Serial.println("connecting...");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(10000);
  }

  Serial.println();

  Serial.print("connected! ipaddr: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

uint8_t *readRequest (WiFiClient client) {
  static uint8_t buf[MAX_REQUEST_SIZE];
  int crlfcount = 0, position = 0;
  char c;

  // while there is still data, read it
  while (client.connected() && client.available() && position < MAX_REQUEST_SIZE && crlfcount < 4) {
    c = client.read();

    if (c == '\n' || c == '\r') {
      crlfcount++;
    } else {
      crlfcount = 0;
    }

    buf[position] = (uint8_t) c;
    position++;
  }

  if (position == 0) {
    return NULL;
  } else {
    buf[position] = '\0';
    return buf;
  }
}

void loop () {
  // Check if a client has connected
  WiFiClient connection = server.available();

  if (!connection) {
    delay(25);
    return;
  }

  // read the request, returns a buffer
  uint8_t *buf = readRequest(connection);

  // if there's an error, bail
  if (buf == NULL) {
    connection.flush();
    delay(25);
    return;
  }

  // parse the request, get the results back
  Request *request = parse_request(buf);

  // this allows for better path matching
  String path = String((char *) request->path);

  if (path.compareTo("/ping") == 0) {
    connection.println("HTTP/1.1 200 OK\r\nContent-type: text/plain\r\n\r\npong\r\n");
  }
  else {
    connection.println("HTTP/1.1 404 Not Found\r\n");
  }

  connection.flush();

  delay(25);
}
