#ifndef __SERIAL_OPERATE_H__
#define __SERIAL_OPERATE_H__
#include "tvutils/CThread.h"

class CTv2d4GHeadSetDetect: public CThread
{

public:
    CTv2d4GHeadSetDetect();
    ~CTv2d4GHeadSetDetect();

    int startDetect();

    class IHeadSetObserver
    {
    public:
        IHeadSetObserver()
        {};
        virtual ~IHeadSetObserver()
        {};
        virtual void onHeadSetDetect(int state, int para) {};
        virtual void onThermalDetect(int state) {};

    };
    void setObserver ( IHeadSetObserver *pOb )
    {
        mpObserver = pOb;
    };

private:
    bool  threadLoop();
    IHeadSetObserver *mpObserver;

};

#endif//__SERIAL_OPERATE_H__