#ifndef WEB_API_H
#define WEB_API_H

#include "WebServer.h"

extern WebServer webServer;

void webHandleDefault();

void setupWeb();
void loopWeb(void);

#endif
