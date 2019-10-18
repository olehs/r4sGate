#include "WebAPI.h"

#include "r4scfg.h"

WebServer webServer(WEB_SERVER_PORT);

void setupWeb() {

  webServer.onNotFound(webHandleDefault);

  webServer.begin();
  log_i("WebAPI server started");
}

void loopWeb(void) {
  webServer.handleClient();
}
