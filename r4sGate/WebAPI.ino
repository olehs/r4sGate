
WebServer webServer(WEB_SERVER_PORT);

void webHandleDefault();

void setupWeb() {

  webServer.onNotFound(webHandleDefault);

  webServer.begin();
  Serial.println("WebAPI server started");
}

void loopWeb(void) {
  webServer.handleClient();
}
