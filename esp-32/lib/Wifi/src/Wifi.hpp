#pragma once

namespace CWifi {
    // number of times to retry connecting to WIFI => 30 seconds + 4 * startTimeout
    constexpr int connectionAttempts = 30;
    // how long to wait in between conenction checks (ms) => 1 per second
    constexpr int timeout = 1000;
    // how long to wait after starting a connection. (ms)
    constexpr int startTimeout = 1000;

    /// @brief Connect to WIFI network
    bool connect();

    /// @brief Disconnect from WIFI network
    void disconnect();

    /// @brief Reconnect to WIFI network
    void reconnect();
}
