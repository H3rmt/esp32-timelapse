#pragma once

namespace Net {
    // number of retries for each http request
    constexpr int netRetries = 30;
    // dealy in between net requests
    constexpr int netRetryDelay = 500;

    // if true aborts reqeust send loop
    inline volatile bool abortSendFlag = false;

    /// @brief send a picture to server
    /// @param pic string representation of the jpg image
    /// @param pictureNumber picture number
    /// @param ident identifier for this timelapse session
    /// @param layer type of image (layer = image from layer, !layer = image from minute)
    /// @return true on success, false on failure
    bool sendPic(const String &pic, const String &pictureNumber, const String &ident, bool layer);

    /// @brief send the finish request to server
    /// @param layerCount number of layers that were registered
    /// @param minuteCount number of minutes that were registered
    /// @param ident identifier for this timelapse session
    /// @return true on success, false on failure
    bool sendFinish(int layerCount, int minuteCount, const String &ident);

    /// @brief send start request to server
    /// @param ident gets set to identifier for this timelapse session
    /// @return true on success, false on failure
    bool sendStart(String &ident);

    /// @brief abort any send that is currently in progress
    void abortSend();
}
