#pragma once

#include <Arduino.h>

namespace Net {
    constexpr int netRetries = 7;
    constexpr int netRetryDelay = 3000;

    inline volatile bool abortSendFlag = false;

    bool sendPic(const String &pic, const String &pictureNumber, const String &ident, bool layer);

    bool sendFinish(int layerCount, int minuteCount, const String &ident);

    bool sendStart(String &ident);

    /// @brief abort any send that is currently in progress
    void abortSend();
}
