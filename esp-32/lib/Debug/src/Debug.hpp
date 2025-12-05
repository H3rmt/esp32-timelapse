#pragma once

namespace Debug {
    /// @brief Flash onboard flash with power and sleep in between
    void flash(int power, int sleep);

    /// @brief Indicate error and halt execution with onboard led
    [[noreturn]] void errorExit(int code);

    /// @brief Indicate error with onboard led
    void error(int code);
} // namespace Debug
