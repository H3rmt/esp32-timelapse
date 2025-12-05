#pragma once

#include <Arduino.h>

namespace Storage {
    void init();

    void reset();

    int incLayerCounter();

    int incMinuteCounter();

    int getCurrentLayerCounter();

    int getCurrentMinuteCounter();

    void setIdent(const String &);

    String getIdent();
}


