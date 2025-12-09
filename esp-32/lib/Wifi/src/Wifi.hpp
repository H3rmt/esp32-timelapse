#pragma once

namespace Wifi {
    // number of times to retry connecting to WIFI => 30 / 2 => 15 seconds + 4 * startTimeout
    constexpr int connectionAttempts = 30;
    // how long to wait in between conenction checks (ms) => 2 per second
    constexpr int timeout = 500;
    // how long to wait after starting a connection. (ms)
    constexpr int startTimeout = 1000;

    /// @brief Connect to WIFI network
    bool connect();

    /// @brief Disconnect from WIFI network
    void disconnect();

    /// @brief Reconnect to WIFI network
    void reconnect();
}
