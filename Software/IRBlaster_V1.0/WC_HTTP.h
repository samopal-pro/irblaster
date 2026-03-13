/**
* Проект контроллера автомоек. Версия 4 от 2020
* Copyright (C) 2020 Алексей Шихарбеев
* http://samopal.pro
*/

#ifndef WS_HTTP_h
#define WS_HTTP_h
#include "SHTTP.h"

extern SHTTP_Content http;
void HTTP_style();
void HTTP_head(const char *_title, uint16_t _refresh=0);
#endif
