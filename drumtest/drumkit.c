#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/songlib.h"

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "drumkit";
char *PROGRAM_VERSION = "0.01";

int
main()
    {
    int spot;

    songInit();

    openOutput("drumkit.rra",0,0);

    setTempo(150);
    setTime(4,4);
    setStride(0.05);
    setSustain(0.99999);
    setAmplitude(0.4);

    setSlopTiming(0.05);
    setSlopAmplitude(0.5);
    setSlopPitch(0.8);

    setPrimaryEmphasis(1);
    setSecondaryEmphasis(1);

    drumkitStick(1,1,"xxxx",(char *) 0); //intro

    spot = getLocation();
    drumkitKick   (10,1,"x---------------",(char *) 0); setLocation(spot);
    drumkitSnare  (10,1,"--x-------------",(char *) 0); setLocation(spot);
    drumkitHHPedal(10,1,"----x-----------",(char *) 0); setLocation(spot);
    drumkitTomHi  (10,1,"------x---------",(char *) 0); setLocation(spot);
    drumkitTom    (10,1,"--------x-------",(char *) 0); setLocation(spot);
    drumkitTomLo  (10,1,"----------x-----",(char *) 0); setLocation(spot);
    drumkitHHOpen (10,1,"------------x---",(char *) 0); setLocation(spot);
    drumkitRide   (10,1,"--------------x-",(char *) 0);

    closeOutput();

    return 0;
    }

