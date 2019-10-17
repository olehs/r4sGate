#ifndef WEB_API_H
#define WEB_API_H

#include "WebServer.h"
#include "r4scfg.h"

static WebServer webServer(WEB_SERVER_PORT);

void webHandleDefault();

void setupWeb();
void loopWeb(void);

#endif
