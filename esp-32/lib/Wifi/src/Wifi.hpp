#pragma once

namespace Wifi {
    // number of times to retry connecting to WIFI => 60 / 4 => 15 seconds + 4 * startTimeout
    constexpr int connectionAttempts = 60;
    // how long to wait in between conenction checks (ms) => 4 per second
    constexpr int timeout = 250;
    // how long to wait after starting a connection. (ms)
    constexpr int startTimeout = 1000;

    /// @brief Connect to WIFI network
    bool connect();

    /// @brief Disconnect from WIFI network
    void disconnect();
}
