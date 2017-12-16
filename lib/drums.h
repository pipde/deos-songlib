#ifndef INCLUDED_DRUMLINES_H
#define INCLUDED_DRUMLINES_H

extern void loadDrumKit(void);

extern int setKick(int);
extern int setTomLo(int);
extern int setTom(int);
extern int setTomHi(int);
extern int setSnare(int);
extern int setHHOpen(int);
extern int setHHClosed(int);
extern int setHHPedal(int);
extern int setCrash(int);
extern int setRide(int);
extern int setRim(int);
extern int setCowbell(int);
extern int setStick(int);
extern int setSplash(int);

extern int getKick(void);
extern int getTomLo(void);
extern int getTom(void);
extern int getTomHi(void);
extern int getSnare(void);
extern int getHHOpen(void);
extern int getHHClosed(void);
extern int getHHPedal(void);
extern int getCrash(void);
extern int getRide(void);
extern int getRim(void);
extern int getCowbell(void);
extern int getStick(void);
extern int getSplash(void);

extern double setRollSpeed(double);
extern double getRollSpeed(void);
extern int setRollEmphasis(int);
extern int getRollEmphasis(void);

extern void drumkitKick(int,double,char *,...);
extern void drumkitTomLo(int,double,char *,...);
extern void drumkitTom(int,double,char *,...);
extern void drumkitTomHi(int,double,char *,...);
extern void drumkitSnare(int,double,char *,...);
extern void drumkitHHOpen(int,double,char *,...);
extern void drumkitHHClosed(int,double,char *,...);
extern void drumkitHHPedal(int,double,char *,...);
extern void drumkitCrash(int,double,char *,...);
extern void drumkitSplash(int,double,char *,...);
extern void drumkitRide(int,double,char *,...);
extern void drumkitRim(int,double,char *,...);
extern void drumkitStick(int,double,char *,...);
extern void drumkitCowbell(int,double,char *,...);
extern void drumkitPat(int,int,double,char *,...);

extern void drumpat(int,int,char *,...);

extern void drumline(int,void (*)(void),void (*)(void));

extern void d44a(void);
extern void d44b(void);
extern void d44c(void);
extern void d44d(void);
extern void d44e(void);
extern void d44f(void);
extern void d44g(void);
extern void d44h(void);
extern void d44i(void);
extern void d44j(void);

extern void d44afill(void);
extern void d44bfill(void);
extern void d44cfill(void);
extern void d44dfill(void);
extern void d44efill(void);
extern void d44ffill(void);
extern void d44gfill(void);
extern void d44hfill(void);
extern void d44ifill(void);
extern void d44jfill(void);


extern void d24a(void);

extern void roll(double,int);
extern void flam(double,int,int,int);
extern void drag(double,int,int,int);
extern void ghost(double,int,int);
extern void drum(double,int,int);
extern int getDrumNote(void);

#endif
