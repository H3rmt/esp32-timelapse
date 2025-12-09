#pragma once

namespace Storage {
    /// Initialise Storage connection
    void init();

    /// Reset Storage values
    void reset();

    /// Increment layer counter
    int incLayerCounter();

    /// Increment minute counter
    int incMinuteCounter();

    /// Get current layer counter
    int getCurrentLayerCounter();

    /// Get current minute counter
    int getCurrentMinuteCounter();

    /// Set identifier for this timelapse session
    void setIdent(const String &);

    /// Get identifier for this timelapse session
    String getIdent();
}


