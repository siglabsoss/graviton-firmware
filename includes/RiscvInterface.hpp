#ifndef __ETH_INTERFACE_HPP__
#define __ETH_INTERFACE_HPP__

// believe it or not mbed already defines an EthInterface which was my first idea
// for naming this file

template <class T>
class RiscvInterface {
public:
    T *fpga;
    // SPI *spi;
    // DigitalOut *cs;
    // DigitalOut *rst;

    // RiscvInterface(void) : fpga(0) {
    // }

    RiscvInterface(T* f) : fpga(f) {
        
    }

    void boot() {
        fpga->printf("boot\r\n");
    }
    void notifyError(){
        fpga->printf("error\r\n");
    }

    void alive() {
        fpga->printf("hello\r\n");
    }

};



#endif