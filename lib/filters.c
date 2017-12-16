#include "filters.h"
#include "util.h"
#include "math.h"

/********** delay unit *****************/

delay *
newDelay(int delta)
    {
    int i;
    delay *d = New(sizeof(delay));
    d->index = 0;
    d->length = delta;
    d->store = New(sizeof(int) * delta);
    for (i = 0; i < delta; ++i) d->store[i] = 0;
    return d;
    }

int
getDelay(delay *d)
    {
    int index = (d->index+1) % d->length;

    //printf("getting %d from index %d\n",d->store[index],index);
    return d->store[index];
    }

int
updateDelay(delay *d,int in)
    {
    int old = getDelay(d);
    //printf("setting %d at index %d\n",in,d->index);
    d->store[d->index] = in;
    d->index = (d->index + 1) % d->length;
    return old;
    }

int
getVarDelay(delay *d,double back)
    {
    //printf("length is %d, back is %f\n",d->length,back);
    int hi = (int) back;
    int lo = hi + 1;
    int vhi = d->store[(d->index + (d->length - hi)) % d->length];
    int vlo = d->store[(d->index + (d->length - lo)) % d->length];
    //printf("lo is %d (value is %d), vhi is %d (value is %d)\n",
    //   lo,vlo,hi,vhi);
    //getchar();
    return (int)(vlo + (lo - back) * (vhi - vlo));
    }

/********** first order filter *****************/

fof *
newFOF(double freq,int style)
    {
    double theta,gamma;
    fof *f;
    
    f = New(sizeof(fof));

    f->freq = freq;
    theta = 2 * M_PI * freq / SampleRate;
    if (style == LOWPASS)
        {
        gamma = 2 - cos(theta);
        f->b1 = pow(gamma * gamma - 1,0.5) - gamma;
        f->a0 = 1 + f->b1;
        }
    else
        {
        gamma = 2 + cos(theta);
        f->b1 = gamma - pow(gamma * gamma - 1,0.5);
        f->a0 = 1 - f->b1;
        }
    f->sdelay = newDelay(1);
    return f;
    }

int
getFOF(fof *f,int in)
    {
    return f->a0 * in - f->b1 * getDelay(f->sdelay);
    }

int
updateFOF(fof *f,int in)
    {
    int out = getFOF(f,in);
    updateDelay(f->sdelay,out);
    return out;
    }

/********** biquad filter *****************/

biquad *
newBiquad(double a0,double a1,double a2,double b1,double b2)
    {
    biquad *f = New(sizeof(biquad));
    f->a0 = a0;
    f->a1 = a1;
    f->a2 = a2;
    f->b1 = b1;
    f->b2 = b2;
    f->ldelay = newDelay(2);
    f->rdelay = newDelay(2);
    return f;
    }

int
getBiquad(biquad *f,int in)
    {
    double result;
    //printf("getting %d from ldelay\n",getVarDelay(f->ldelay,1));
    //printf("getting %d from ldelay\n",getVarDelay(f->ldelay,2));
    //printf("getting %d from rdelay\n",getVarDelay(f->rdelay,1));
    //printf("getting %d from rdelay\n",getVarDelay(f->rdelay,2));
    //result = f->a0 * in;
    //printf("result is %f\n",result);
    //printf("%f\n",f->a1 * getVarDelay(f->ldelay,1));
    //result += f->a1 * getVarDelay(f->ldelay,1);
    //printf("result is %f\n",result);
    //printf("%f\n",f->a2 * getVarDelay(f->ldelay,2));
    //result += f->a2 * getVarDelay(f->ldelay,2);
    //printf("result is %f\n",result);
    //printf("-%f\n",f->b1 * getVarDelay(f->rdelay,1));
    //result -= f->b1 * getVarDelay(f->rdelay,1);
    //printf("result is %f\n",result);
    //printf("-%f\n",f->b2 * getVarDelay(f->rdelay,2));
    //result -= f->b2 * getVarDelay(f->rdelay,2);
    //printf("result is %f\n",result);
    result = f->a0 * in
         + f->a1 * getVarDelay(f->ldelay,1)
         + f->a2 * getVarDelay(f->ldelay,2)
         - f->b1 * getVarDelay(f->rdelay,1)
         - f->b2 * getVarDelay(f->rdelay,2)
         ;
    //printf("result is %f\n",result);
    return (int) result;
    }

int
updateBiquad(biquad *f,int in)
    {
    int out = getBiquad(f,in);
    //printf("in is %d\n",in);
    //printf("out is %d\n",out);
    //printf("pushing %d to ldelay\n",in);
    updateDelay(f->ldelay,in);
    //printf("pushing %d to rdelay\n",out);
    updateDelay(f->rdelay,out);
    //getchar();
    return out;
    }

/********** modified biquad filter *****************/

modbiquad *
newModBiquad(double a0,double a1,double a2,double b1,double b2,double wet,double dry)
    {
    modbiquad *f = New(sizeof(modbiquad));

    f->bq = newBiquad(a0,a1,a2,b1,b2);
    f->wet = wet;
    f->dry = dry;
    return f;
    }

int
getModBiquad(modbiquad *f,int in)
    {
    double result;
    //printf("getting %d from ldelay\n",getVarDelay(f->ldelay,1));
    //printf("getting %d from ldelay\n",getVarDelay(f->ldelay,2));
    //printf("getting %d from rdelay\n",getVarDelay(f->rdelay,1));
    //printf("getting %d from rdelay\n",getVarDelay(f->rdelay,2));
    //result = f->a0 * in;
    //printf("result is %f\n",result);
    //printf("%f\n",f->a1 * getVarDelay(f->ldelay,1));
    //result += f->a1 * getVarDelay(f->ldelay,1);
    //printf("result is %f\n",result);
    //printf("%f\n",f->a2 * getVarDelay(f->ldelay,2));
    //result += f->a2 * getVarDelay(f->ldelay,2);
    //printf("result is %f\n",result);
    //printf("-%f\n",f->b1 * getVarDelay(f->rdelay,1));
    //result -= f->b1 * getVarDelay(f->rdelay,1);
    //printf("result is %f\n",result);
    //printf("-%f\n",f->b2 * getVarDelay(f->rdelay,2));
    //result -= f->b2 * getVarDelay(f->rdelay,2);
    //printf("result is %f\n",result);
    result = f->dry * in + f->wet * getBiquad(f->bq,in);
    //printf("result is %f\n",result);
    return (int) result;
    }

int
setModBiquad(modbiquad *f,int in)
    {
    int out = getModBiquad(f,in);
    //printf("in is %d\n",in);
    //printf("out is %d\n",out);
    //printf("pushing %d to ldelay\n",in);
    updateBiquad(f->bq,in);
    //getchar();
    return out;
    }

/********** digital delay line *****************/

ddl *
newDDL(int length,double wet,double fb)
    {
    ddl *d = New(sizeof(ddl));
    d->fb = fb > 1? 1 : (fb < 0? 0: fb);
    d->wet = wet > 1? 1 : (wet < 0? 0: wet);
    d->dry = 1 - wet;
    d->length = length;
    d->sdelay = newDelay(length);
    return d;
    }

static int
getDDLPartial(ddl *d,int in)
    {
    return in + d->fb * getDelay(d->sdelay);
    }

int
getDDL(ddl *d,int in)
    {
    return d->dry * in + d->wet * getDDLPartial(d,in);
    }

int
updateDDL(ddl *d,int in)
    {
    int out = getDDL(d,in);
    int s = getDDLPartial(d,in);
    updateDelay(d->sdelay,s);
    return out;
    }

/********** low frequency oscillator *****************/

lfo *
newLFO(double freq)
    {
    lfo *o = New(sizeof(lfo));
    o->start = 0;
    o->freq = freq;
    o->step = 2 * M_PI / (SampleRate / freq);
    return o;
    }

double
getLFO(lfo *o)
    {
    return sin(o->start);
    }

double
updateLFO(lfo *o)
    {
    double out = getLFO(o);
    o->start += o->step;
    return out;
    }

/********** modulated delay effect filter *****************/

mde *
newMDE(double wet,double fb,int dsamples,double freq,int lo,int hi)
    {
    mde *m = New(sizeof(mde));
    m->dry = 1 - wet;
    m->wet = wet;
    m->fb = fb;
    m->sdelay = newDelay(dsamples);
    m->osc = newLFO(freq);
    m->lo = lo;
    m->hi = hi;
    return m;
    }

static double
getMDEOscillation(mde *m)
    {
    double f = getLFO(m->osc);
    return (m->lo + m->hi + f * (m->hi - m->lo)) / 2.0;
    }

static int
getMDEDelay(mde *m)
    {
    double d = getMDEOscillation(m);
    return getVarDelay(m->sdelay,d);
    }

int
getMDE(mde *m,int in)
    {
    return (int)(m->dry * in + m->wet * getMDEDelay(m));
    }

int
updateMDE(mde *m,int in)
    {
    int out = getMDE(m,in);
    int recirc = getMDEDelay(m);
    
    updateLFO(m->osc);
    updateDelay(m->sdelay,in + m->fb * recirc);

    return out;
    }

/********** comb filter *****************/

combf *
newCombF(int samplesDelayed,double fb)
    {
    combf *c = New(sizeof(combf));
    c->sdelay = newDelay(samplesDelayed);
    c->fb = fb;
    c->length = samplesDelayed;
    return c;
    }

int 
getCombF(combf *c,int in)
    {
    return in + c->fb * getDelay(c->sdelay);
    }

int
updateCombF(combf *c,int in)
    {
    int out = getCombF(c,in);
    updateDelay(c->sdelay,out);
    return out;
    }

/********** lowpass comb filter *****************/

lpc *
newLPC(int samplesDelayed,double gc,double glp)
    {
    lpc *f = New(sizeof(lpc));
    f->idelay = newDelay(samplesDelayed+1);
    f->odelay = newDelay(samplesDelayed);
    f->gc = gc;
    f->glp = glp;
    f->length = samplesDelayed;
    return f;
    }

int 
getLPC(lpc *f,int in)
    {
    return getVarDelay(f->idelay,f->length)
        - f->glp * getDelay(f->idelay)
        + f->glp * getVarDelay(f->odelay,1)
        + f->gc  * getDelay(f->odelay)
        ;
    }

int
updateLPC(lpc *f,int in)
    {
    int out = getLPC(f,in);
    updateDelay(f->idelay,in);
    updateDelay(f->odelay,out);
    return out;
    }

/********** delaying allpass filter *****************/

dap *
newDAP(int samplesDelayed,double g)
    {
    dap *f = New(sizeof(dap));
    f->xdelay = newDelay(samplesDelayed);
    f->ydelay = newDelay(samplesDelayed);
    f->g = g;
    f->length = samplesDelayed;
    return f;
    }

int 
getDAP(dap *f,int in)
    {
    return -(f->g * in)
        + getDelay(f->xdelay)
        + f->g  * getDelay(f->ydelay)
        ;
    }

int
updateDAP(dap *f,int in)
    {
    int out = getDAP(f,in);
    updateDelay(f->xdelay,in);
    updateDelay(f->ydelay,out);
    return out;
    }
