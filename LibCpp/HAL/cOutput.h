#ifndef COUTPUT_H
#define COUTPUT_H

namespace LibCpp
{

class cOutput
{
public:
    cOutput();

    virtual void set(bool on = true);
    virtual bool get();

    inline  void reset() {set(false);};
    inline  void on() {set();};
    inline  void off() {set(false);};

protected:
    bool state;
};

}

#endif
