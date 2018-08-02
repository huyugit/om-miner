#ifndef SPI_TRANSMITTER_H
#define SPI_TRANSMITTER_H
/*
 * Contains SpiTransmitter class declaration.
 */

#include "base/NonCopyable.h"
#include "sys/FileDescriptor.h"

#include <stdint.h>


// The purpose of this class is to send/receive data via the SPI bus.
//
class SpiTransmitter
    : private NonCopyable  // Prevent copy and assignment.
{
// Construction/destruction.
public:
    // Constructs SpiTransmitter object.
    SpiTransmitter();

// Public interface.
public:
    // Returns true if the SPI device is open or false otherwise.
    inline bool isOpen() const  { return !m_dev.isInvalid(); }

    // Opens the device file named spiDev (e.g. "/dev/spidev0.0").
    // Returns true on success and false on error.
    void open(const char* spiDev);

    // Sends/receives data via the SPI.
    // Returns true on success and false on error.
    void transfer(const uint8_t* wrbuf, uint8_t* rdbuf, size_t size);
    void transferHw(const uint8_t* wrbuf, uint8_t* rdbuf, size_t size);
    void transferSw(const uint8_t* wrbuf, uint8_t* rdbuf, size_t size);

    // Closes the device.
    // Returns true on success and false on error.
    bool close() throw()  { return m_dev.close(); }

    // Exchanges the content of this object with "other".
    void swap(SpiTransmitter& other) throw()
    {
        m_dev.swap(other.m_dev);
    }

// Member variables.
private:
    // Device file descriptor.
    FileDescriptor m_dev;

    // SPI mode (SPI_MODE_0..SPI_MODE_3, default SPI_MODE_0) 
    int m_mode;
    
    // SPI device word length (default 8).
    int m_bits;
    
    // SPI device default max speed (hz, default 2000000).
    int m_speed;

    void delay();
};

#endif  // SPI_TRANSMITTER_H
