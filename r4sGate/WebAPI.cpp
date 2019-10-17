#include "WebAPI.h"

void setupWeb() {

  webServer.onNotFound(webHandleDefault);

  webServer.begin();
  log_i("WebAPI server started");
}

void loopWeb(void) {
  webServer.handleClient();
}
