/*
 *  This is a arplay (RRA player based
 *  on ALSA layer). The source file was
 *  taken from ALSA library pcm test file
 *  and changed to support RRA files.
 *  
 *  modified by madhavRao
 *
 *  WARNING: barely tested
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <ctype.h>
#include <sched.h>
#include <errno.h>
#include <getopt.h>
#include <alsa/asoundlib.h>
#include <sys/time.h>
#include <math.h>
#include "/usr/local/include/songlib/util.h"
#include "/usr/local/include/songlib/rra.h"
#include "/usr/local/include/songlib/wave.h"

#define RING_BUFFER_LEN 8196

char *PROGRAM_NAME = "arplay";
char *PROGRAM_VERSION = "0.01";

static char *device = "plughw:0,0";                     /* playback device */
static snd_pcm_format_t format = SND_PCM_FORMAT_S16;    /* sample format */
static unsigned int rate = 44100;                       /* stream rate */
static unsigned int channels = 2;                       /* count of channels */
static unsigned int buffer_time = 1000000;              /* ring buffer length in us */
//static unsigned int period_time = 91000;              /* period time in us */
static unsigned int period_time = 1000000;              /* period time in us */
static int verbose = 0;                                 /* verbose flag */
static int resample = 1;                                /* enable alsa-lib resampling */
static snd_pcm_sframes_t buffer_size, period_size, sampleCounter;
static snd_output_t *output = NULL;
static int scaling_factor = 1;
static int shuffleResult = 0;
static int loopResult = 0;

/*
 * arplayHandler incorporates any user
 * generated applicable comments to
 * arplay module
 *
 */

static void 
arplayHandler(FILE *in)
    {
    int ch ;
    char s[512]="";
    int count;

    int first=1;
    int decimal=0;
    int floater=0;
    int exponent=0;
    count = 0;
    ch = fgetc(in);
    if(ch == 'a')
        {
        fprintf(stderr,"%c ",ch);
        ch = fgetc(in);
        while(!(isdigit(ch)))
            {
            fprintf(stderr,"%c ",ch);
            }
        //ungetc(ch,in);
        while(isdigit(ch)||(first && ch
        == '-') || (!decimal && ch ==
        '.') || (!exponent && (ch == 'E' || ch == 'e')))
            {
            first =0;
            if(ch == '.')
                {
                decimal = 1;
                floater = 1;
                }
            if(ch=='E' || ch == 'e')        
                {
                decimal = 1;
                floater = 1;
                exponent = 1;
                first = 1;
                }
            s[count++]=ch;          
            if(count >= sizeof(s)-1)
                {
                fprintf(stderr,"Count =%d\n",count);
                fprintf(stderr,"RRA FORMAT error\n");
                
                }
            ch = fgetc(in);
            }
        s[count]='\0';
            
        if(floater)
            {
            scaling_factor = (int)(atof(s));
            }
        else
            {
            scaling_factor=atoi(s);
            }
        }
    /*
     * Added on Jan-26-2010
     * lyrics comment handler added
     *
     */
    else if(ch == 'l')
        {
        ch = fgetc(in);
        while (ch != '\n')
            {
            s[count++] = ch;
            ch = fgetc(in);
            }
        s[count] = '\0';
        fprintf(stdout,"lyrics: %s\n",s);

        }
    else
        {
        fprintf(stderr,"This is not meant for arplay process\n");
        while((ch = fgetc(in)) && ch != EOF && ch != '\n')
            continue;
        }
    }

/*
 * generateRRA function prepares the
 * audio buffer with RRA data.
 *
 *
 */

static void 
generateRRA(const snd_pcm_channel_area_t *areas, 
                          snd_pcm_uframes_t offset,
                          int count, double *_phase, RRA *h,FILE *in )
    {
    int amp, steps[channels], ires,  bps; 
    unsigned int chn, byte;
    unsigned int maxval; 
    double phase = *_phase, res;
    unsigned char *samples[channels], *tmp;

    bps = h->bitsPerSample/8;
    channels = h->channels;
    maxval = h->maxAmp;

    /* verify and prepare the contents of areas */
    for (chn = 0; chn < channels; chn++) 
        {
        if ((areas[chn].first % 8) != 0) 
            {
            fprintf(stderr,"areas[%i].first == %i, aborting...\n", chn, areas[chn].first);
            exit(EXIT_FAILURE);
            }
        samples[chn] = /*(signed short *)*/(((unsigned char *)areas[chn].addr) + (areas[chn].first / 8));
        if ((areas[chn].step % 8) != 0) 
            {
            fprintf(stderr,"areas[%i].step == %i, aborting...\n", chn, areas[chn].step);
            exit(EXIT_FAILURE);
            }
        steps[chn] = areas[chn].step / 8;
        samples[chn] += offset * steps[chn];
        }
        
    while (count-- > 0) 
        {
        res = amp; 
        ires = res;
        tmp = (unsigned char *)(&ires);

        for (chn = 0; chn < channels; chn++) 
            {
            amp = readRRAAmplitude(in,h->bitsPerSample,arplayHandler);
            if(scaling_factor>1) amp = amp/scaling_factor;
            res = amp;
            ires = res;
            tmp = (unsigned char *)(&ires);
            for (byte = 0; byte < (unsigned int)bps; byte++)
                *(samples[chn] + byte) = tmp[byte];
            samples[chn] += steps[chn];
            }
        }
    *_phase = phase;
    }

/*
 * code taken from the original alsa-lib
 * source (alsa-lib-1.0.20/test/pcm.c)
 *
 *
 *
 */

static int 
set_hwparams(snd_pcm_t *handle,
                        snd_pcm_hw_params_t *params,
                        snd_pcm_access_t access)
   {
    unsigned int rrate;
    snd_pcm_uframes_t size;
    int err, dir;

    /* choose all parameters */
    err = snd_pcm_hw_params_any(handle, params);
    if (err < 0) 
        {
        fprintf(stderr,"Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
        return err;
        }
    /* set hardware resampling */
    err = snd_pcm_hw_params_set_rate_resample(handle, params, resample);
    if (err < 0) 
        {
        fprintf(stderr,"Resampling setup failed for playback: %s\n", snd_strerror(err));
        return err;
        }
    /* set the interleaved read/write format */
    err = snd_pcm_hw_params_set_access(handle, params, access);
    if (err < 0) 
        {
        printf("Access type not available for playback: %s\n", snd_strerror(err));
        return err;
        }
    /* set the sample format */
    err = snd_pcm_hw_params_set_format(handle, params, format);
    if (err < 0) 
        {
        printf("Sample format not available for playback: %s\n", snd_strerror(err));
        return err;
        }
    /* set the count of channels */
    err = snd_pcm_hw_params_set_channels(handle, params, channels);
    if (err < 0) 
        {
        printf("Channels count (%i) not available for playbacks: %s\n", channels, snd_strerror(err));
        return err;
        }
    /* set the stream rate */
    rrate = rate;
    err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
    if (err < 0) 
        {
        printf("Rate %iHz not available for playback: %s\n", rate, snd_strerror(err));
        return err;
        }
    if (rrate != rate) 
        {
        fprintf(stderr,"Rate doesn't match (requested %iHz, get %iHz)\n", rate, err);
        return -EINVAL;
        }
    /* set the buffer time */
    err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, &dir);
    if (err < 0) 
        {
        fprintf(stderr,"Unable to set buffer time %i for playback: %s\n", buffer_time, snd_strerror(err));
        return err;
        }
    err = snd_pcm_hw_params_get_buffer_size(params, &size);
    if (err < 0) 
        {
        fprintf(stderr,"Unable to get buffer size for playback: %s\n", snd_strerror(err));
        return err;
        }
    buffer_size = size;
    /* set the period time */
    err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, &dir);
    if (err < 0) 
        {
        fprintf(stderr,"Unable to set period time %i for playback: %s\n", period_time, snd_strerror(err));
        return err;
        }

    err = snd_pcm_hw_params_get_period_size(params, &size, &dir);
    if (err < 0) 
        {
        fprintf(stderr,"Unable to get period size for playback: %s\n", snd_strerror(err));
        return err;
        }
    period_size = size;
    /* write the parameters to device */
    err = snd_pcm_hw_params(handle, params);
    if (err < 0) 
        {
        fprintf(stderr,"Unable to set hw params for playback: %s\n", snd_strerror(err));
        return err;
        }
    return 0;
    }

/*
 * code taken from the original alsa-lib
 * source (alsa-lib-1.0.20/test/pcm.c)
 *
 *
 *
 */

static int 
set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams)
    {
    int err;
    /* get the current swparams */
    err = snd_pcm_sw_params_current(handle, swparams);
    if (err < 0) 
        {
        fprintf(stderr,"Unable to determine current swparams for playback: %s\n", snd_strerror(err));
        return err;
        }
    /* start the transfer when the buffer is almost full: */
    /* (buffer_size / avail_min) * avail_min */
    err = snd_pcm_sw_params_set_start_threshold(handle, swparams, (buffer_size / period_size) * period_size);
    if (err < 0) 
        {
        fprintf(stderr,"Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
        return err;
        }
    /* allow the transfer when at least period_size samples can be processed */
    err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_size);
    if (err < 0) 
        {
        fprintf(stderr,"Unable to set avail min for playback: %s\n", snd_strerror(err));
        return err;
        }
    /* align all transfers to 1 sample */
    err = snd_pcm_sw_params_set_xfer_align(handle, swparams, 1);
    if (err < 0) 
        {
        fprintf(stderr,"Unable to set transfer align for playback: %s\n", snd_strerror(err));
        return err;
        }
    /* write the parameters to the playback device */
    err = snd_pcm_sw_params(handle, swparams);
    if (err < 0) 
        {
        fprintf(stderr,"Unable to set sw params for playback: %s\n", snd_strerror(err));
        return err;
        }
    return 0;
    }

/*
 *   Underrun and suspend recovery
 */
 
static int 
xrun_recovery(snd_pcm_t *handle, int err)
    {
    if (err == -EPIPE) 
        {       /* under-run */
        err = snd_pcm_prepare(handle);
        if (err < 0)
            fprintf(stderr,"Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
        return 0;
        } 
    else if (err == -ESTRPIPE) 
        {
        while ((err = snd_pcm_resume(handle)) == -EAGAIN)
            sleep(1);   /* wait until the suspend flag is released */
        if (err < 0) 
            {
            err = snd_pcm_prepare(handle);
            if (err < 0)
                fprintf(stderr,"Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
            }
        return 0;
        }
    return err;
    }

/*
 * code taken from the original alsa-lib
 * source (alsa-lib-1.0.20/test/pcm.c)
 *
 *
 * Transfer method - write only 
 *
 */


static int 
write_loop(snd_pcm_t *handle,
                      signed short *samples,
                      snd_pcm_channel_area_t *areas, FILE *in, RRA *h)
    {
    double phase = 0;
    signed short *ptr;
    int err, cptr; 

    sampleCounter = h->samples;

    do
        {
        generateRRA(areas, 0, period_size, &phase,h,in);
        ptr = samples;
        cptr = period_size;
        while (cptr > 0) 
            {
            err = snd_pcm_writei(handle, ptr, cptr);
            if (err == -EAGAIN)
                {
                continue;
                }
            if (err < 0) 
                {
                if (xrun_recovery(handle, err) < 0) 
                    {
                    fprintf(stderr,"Write error: %s\n", snd_strerror(err));
                    exit(EXIT_FAILURE);
                    }
                break;  /* skip one period */
                }
            ptr += err * channels;
            cptr -= err;
            }

        sampleCounter -= period_size;    
        if(sampleCounter < RING_BUFFER_LEN && sampleCounter > 0)
            period_size = sampleCounter;
        }
    //while(sampleCounter > 0);
    while(!feof(in));

    return 0;
    }

/*
 * code taken from the original alsa-lib
 * source (alsa-lib-1.0.20/test/pcm.c)
 *
 *
 * Transfer method - write and wait for room in buffer using poll
 *
 *
 */

static int 
wait_for_poll(snd_pcm_t *handle, struct pollfd *ufds, unsigned int count)
{
unsigned short revents;
while (1) 
    {
    poll(ufds, count, -1);
    snd_pcm_poll_descriptors_revents(handle, ufds, count, &revents);
    if (revents & POLLERR)
        return -EIO;
    if (revents & POLLOUT)
        return 0;
    }

    return 0;
}



static int 
write_and_poll_loop(snd_pcm_t *handle,
                               signed short *samples,
                               snd_pcm_channel_area_t *areas, FILE *in, RRA *h)
    {
    struct pollfd *ufds;
    double phase = 0;
    signed short *ptr;
    int err, count, cptr, init; 

    sampleCounter = h->samples;

    count = snd_pcm_poll_descriptors_count (handle);
    if (count <= 0) 
        {
        //fprintf(stderr,"Invalid poll descriptors count\n");
        return count;
        }

    ufds = malloc(sizeof(struct pollfd) * count);
    if (ufds == NULL) 
        {
        //fprintf(stderr,"No enough memory\n");
        return -ENOMEM;
        }
    if ((err = snd_pcm_poll_descriptors(handle, ufds, count)) < 0) 
        {
        //fprintf(stderr,"Unable to obtain poll descriptors for playback: %s\n", snd_strerror(err));
        return err;
        }
    init = 1;

    do  
        {
        if (!init) 
            {
            err = wait_for_poll(handle, ufds, count);
            if (err < 0) 
                {
                if (snd_pcm_state(handle) == SND_PCM_STATE_XRUN ||
                    snd_pcm_state(handle) == SND_PCM_STATE_SUSPENDED) 
                    {
                    err = snd_pcm_state(handle) == SND_PCM_STATE_XRUN ? -EPIPE : -ESTRPIPE;
                    if (xrun_recovery(handle, err) < 0) 
                        {
                        //fprintf(stderr,"Write error: %s\n", snd_strerror(err));
                        exit(EXIT_FAILURE);
                        }
                    init = 1;
                    } 
                else 
                    {
                    //fprintf(stderr,"Wait for poll failed\n");
                    return err;
                    }
                }
            }

        generateRRA(areas, 0, period_size, &phase,h,in);
        ptr = samples;
        cptr = period_size;

        while (cptr > 0) 
            {
            err = snd_pcm_writei(handle, ptr, cptr);
            if (err < 0) 
                {
                if (xrun_recovery(handle, err) < 0) 
                    {
                    //fprintf(stderr,"Write error: %s\n", snd_strerror(err));
                    exit(EXIT_FAILURE);
                    }
                init = 1;
                break;  /* skip one period */
                }
            if (snd_pcm_state(handle) == SND_PCM_STATE_RUNNING)
                init = 0;

            ptr += err * channels;
            cptr -= err;

            if (cptr == 0)
                    break;

            /* it is possible, that the initial buffer cannot store */
            /* all data from the last period, so wait awhile */
            err = wait_for_poll(handle, ufds, count);
            if (err < 0) 
                {
                if (snd_pcm_state(handle) == SND_PCM_STATE_XRUN ||
                snd_pcm_state(handle) == SND_PCM_STATE_SUSPENDED) 
                    {
                    err = snd_pcm_state(handle) == SND_PCM_STATE_XRUN ? -EPIPE : -ESTRPIPE;
                    if (xrun_recovery(handle, err) < 0) 
                        {
                        //fprintf(stderr,"Write error: %s\n", snd_strerror(err));
                        exit(EXIT_FAILURE);
                        }
                    init = 1;
                    } 
                else 
                    {
                    //fprintf(stderr,"Wait for poll failed\n");
                    return err;
                    }
                }
            }

        sampleCounter -= period_size; 
        if(sampleCounter < RING_BUFFER_LEN && sampleCounter > 0)
            period_size = sampleCounter;
        }
    //while(sampleCounter > 0);
    while(!feof(in));

    return 0;
    }

/*
 * code taken from the original alsa-lib
 * source (alsa-lib-1.0.20/test/pcm.c)
 *
 *
 *
 * Transfer method - asynchronous notification
 */

struct async_private_data 
    {
    signed short *samples;
    snd_pcm_channel_area_t *areas;
    double phase;
    RRA *h;
    FILE *in;
    };

static void 
async_callback(snd_async_handler_t *ahandler)
    {
    snd_pcm_t *handle = snd_async_handler_get_pcm(ahandler);
    struct async_private_data *data = //
    snd_async_handler_get_callback_private(ahandler);
    signed short *samples = data->samples;
    snd_pcm_channel_area_t *areas = data->areas;
    snd_pcm_sframes_t avail;
    int err;

    avail = snd_pcm_avail_update(handle);
    while (avail >= period_size) 
        {
        generateRRA(areas, 0, period_size, &data->phase,data->h,data->in);

        err = snd_pcm_writei(handle, samples, period_size);
        if (err < 0) 
            {
            fprintf(stderr,"Initial write error: %s\n", snd_strerror(err));
            exit(EXIT_FAILURE);
            }
        if (err != period_size) 
            {
            fprintf(stderr,"Initial write error: written %i expected %li\n", err, period_size);
            exit(EXIT_FAILURE);
            }
        avail = snd_pcm_avail_update(handle);

        }
    }

static int 
async_loop(snd_pcm_t *handle,
                      signed short *samples,
                      snd_pcm_channel_area_t *areas, FILE *in, RRA *h)
    {
    struct async_private_data data;
    snd_async_handler_t *ahandler;
    int err, count;


    data.samples = samples;
    data.areas = areas;
    data.phase = 0;
    data.h = h;
    data.in = in;

    err = snd_async_add_pcm_handler(&ahandler, handle, async_callback, &data);
    if (err < 0) 
        {
        fprintf(stderr,"Unable to register async handler\n");
        exit(EXIT_FAILURE);
        }

    for (count = 0; count < 20; count++) 
        {

        err = snd_pcm_writei(handle, samples, period_size);
        if (err < 0) 
            {
            fprintf(stderr,"Initial write error: %s\n", snd_strerror(err));
            exit(EXIT_FAILURE);
            }
        if (err != period_size) 
            {
            fprintf(stderr,"Initial write error: written %i expected %li\n", err, period_size);
            exit(EXIT_FAILURE);
            }           
        //err = snd_pcm_writei(handle, samples, period_size);
        if (err < 0) 
            {
            fprintf(stderr,"Initial write error: %s\n", snd_strerror(err));
            exit(EXIT_FAILURE);
            }
        if (err != period_size) 
            {
            fprintf(stderr,"Initial write error: written %i expected %li\n", err, period_size);
            exit(EXIT_FAILURE);
            }           

        }
    err = snd_pcm_start(handle);
    if (err < 0) 
        {
        fprintf(stderr,"Start error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
        }

    /* because all other work is done in the signal handler,
   suspend the process */
    while (1) 
        {
        sleep(1);
        }
    }
  
/*
 * code taken from the original alsa-lib
 * source (alsa-lib-1.0.20/test/pcm.c)
 *
 *
 *
 *   Transfer method - asynchronous notification + direct write
 */

static void 
async_direct_callback(snd_async_handler_t *ahandler)
    {
    snd_pcm_t *handle = snd_async_handler_get_pcm(ahandler);
    struct async_private_data *data =  
    snd_async_handler_get_callback_private(ahandler);
    const snd_pcm_channel_area_t *my_areas;
    snd_pcm_uframes_t offset, frames, size;
    snd_pcm_sframes_t avail, commitres;
    snd_pcm_state_t state;
    int first = 0, err;

    while (1)
        {
        state = snd_pcm_state(handle);
        if (state == SND_PCM_STATE_XRUN) 
            {
            err = xrun_recovery(handle, -EPIPE);
            if (err < 0) 
                {
                fprintf(stderr,"XRUN recovery failed: %s\n", snd_strerror(err));
                exit(EXIT_FAILURE);
                }
            first = 1;
            } 
        else if (state == SND_PCM_STATE_SUSPENDED) 
            {
            err = xrun_recovery(handle, -ESTRPIPE);
            if (err < 0) 
                {
                fprintf(stderr,"SUSPEND recovery failed: %s\n", snd_strerror(err));
                exit(EXIT_FAILURE);
                }
            }
        avail = snd_pcm_avail_update(handle);
        if (avail < 0) 
            {
            err = xrun_recovery(handle, avail);
            if (err < 0) 
                {
                fprintf(stderr,"avail update failed: %s\n", snd_strerror(err));
                exit(EXIT_FAILURE);
                }
            first = 1;
            continue;
            }
        if (avail < period_size) 
            {
            if (first) 
                {
                first = 0;
                err = snd_pcm_start(handle);
                if (err < 0) 
                    {
                    fprintf(stderr,"Start error: %s\n", snd_strerror(err));
                    exit(EXIT_FAILURE);
                    }
                } 
            else 
                {
                break;
                }
            continue;
            }
        size = period_size;
        while (size > 0) 
            {
            frames = size;
            err = snd_pcm_mmap_begin(handle, &my_areas, &offset, &frames);
            if (err < 0) 
                {
                if ((err = xrun_recovery(handle, err)) < 0) 
                    {
                    fprintf(stderr,"MMAP begin avail error: %s\n", snd_strerror(err));
                    exit(EXIT_FAILURE);
                    }
                first = 1;
                }
            generateRRA(my_areas, offset, frames, &data->phase,data->h,data->in);
            commitres = snd_pcm_mmap_commit(handle, offset, frames);
            if (commitres < 0 || (snd_pcm_uframes_t)commitres != frames) 
                {
                if ((err = xrun_recovery(handle, commitres >= 0 ? -EPIPE : commitres)) < 0) 
                    {
                    fprintf(stderr,"MMAP commit error: %s\n", snd_strerror(err));
                    exit(EXIT_FAILURE);
                    }
                first = 1;
                }
            size -= frames;
            }
        }
    }

static int 
async_direct_loop(snd_pcm_t *handle,
         signed short *samples ATTRIBUTE_UNUSED,
         snd_pcm_channel_area_t *areas ATTRIBUTE_UNUSED, FILE *in, RRA *h)
    {
    struct async_private_data data;
    snd_async_handler_t *ahandler;
    const snd_pcm_channel_area_t *my_areas;
    snd_pcm_uframes_t offset, frames, size, sampleCounter;
    snd_pcm_sframes_t commitres;
    int err, count;
    data.samples = NULL;        /* we do not require the global sample area for direct write */
    data.areas = NULL;  /* we do not require the global areas for direct write */
    data.phase = 0;
    data.h = h;
    data.in = in;

    sampleCounter = h->samples;
    err = snd_async_add_pcm_handler(&ahandler, handle, async_direct_callback, &data);
    if (err < 0) 
        {
        printf("Unable to register async handler\n");
        exit(EXIT_FAILURE);
        }
    for (count = 0; count < 2; count++) 
        {
        size = period_size;
        while (size > 0) 
            {
            sampleCounter -= size;
            frames = size;
            err = snd_pcm_mmap_begin(handle, &my_areas, &offset, &frames);
            if (err < 0) 
                {
                if ((err = xrun_recovery(handle, err)) < 0) 
                    {
                    printf("MMAP begin avail error: %s\n", snd_strerror(err));
                    exit(EXIT_FAILURE);
                    }
                }
            commitres = snd_pcm_mmap_commit(handle, offset, frames);
            if (commitres < 0 || (snd_pcm_uframes_t)commitres != frames) 
                {
                if ((err = xrun_recovery(handle, commitres >= 0 ? -EPIPE : commitres)) < 0) 
                    {
                    printf("MMAP commit error: %s\n", snd_strerror(err));
                    exit(EXIT_FAILURE);
                    }
                }
            size -= frames;
            }
        }
    err = snd_pcm_start(handle);
    if (err < 0) 
        {
        printf("Start error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
        }

    /* because all other work is done in the signal handler,
   suspend1the process */
   //while (sampleCounter) 
   while (1) 
        {
        //fprintf(stderr,"sleeping\n");
        sleep(1);
        }
    return 0;       
    }

/*
 * code taken from the original alsa-lib
 * source (alsa-lib-1.0.20/test/pcm.c)
 *
 *
 *
 *   Transfer method - direct write only
 */

static int 
direct_loop(snd_pcm_t *handle,
                       signed short *samples ATTRIBUTE_UNUSED,
                       snd_pcm_channel_area_t *areas ATTRIBUTE_UNUSED, FILE *in, RRA *h)
    {
    double phase = 0;
    const snd_pcm_channel_area_t *my_areas;
    snd_pcm_uframes_t offset, frames, size;
    snd_pcm_sframes_t avail, commitres;
    snd_pcm_state_t state;
    int err, first = 1;

    sampleCounter = h->samples;

    //while (sampleCounter >= 0) 
    while (!feof(in)) 
        {
            {
            state = snd_pcm_state(handle);
            if (state == SND_PCM_STATE_XRUN) 
                {
                err = xrun_recovery(handle, -EPIPE);
                if (err < 0) 
                    {
                    printf("XRUN recovery failed: %s\n", snd_strerror(err));
                    return err;
                    }
                first = 1;
                } 
            else if (state == SND_PCM_STATE_SUSPENDED) 
                {
                err = xrun_recovery(handle, -ESTRPIPE);
                if (err < 0) 
                    {
                    printf("SUSPEND recovery failed: %s\n", snd_strerror(err));
                    return err;
                    }
                }
            avail = snd_pcm_avail_update(handle);
            if (avail < 0) 
                {
                err = xrun_recovery(handle, avail);
                if (err < 0) 
                    {
                    printf("avail update failed: %s\n", snd_strerror(err));
                    return err;
                    }
                first = 1;
                continue;
                }
            if (avail < period_size) 
                {
                if (first) 
                    {
                    first = 0;
                    err = snd_pcm_start(handle);
                    if (err < 0) 
                        {
                        printf("Start error: %s\n", snd_strerror(err));
                        exit(EXIT_FAILURE);
                        }
                    } 
                else 
                    {
                    err = snd_pcm_wait(handle, -1);
                    if (err < 0) 
                        {
                        if ((err = xrun_recovery(handle, err)) < 0) 
                            {
                            printf("snd_pcm_wait error: %s\n", snd_strerror(err));
                            exit(EXIT_FAILURE);
                            }
                        first = 1;
                        }
                    }
                continue;
                }
            size = period_size;
            while (size > 0) 
                {
                frames = size;
                err = snd_pcm_mmap_begin(handle, &my_areas, &offset, &frames);
                if (err < 0) 
                    {
                    if ((err = xrun_recovery(handle, err)) < 0) 
                        {
                        printf("MMAP begin avail error: %s\n", snd_strerror(err));
                        exit(EXIT_FAILURE);
                        }
                    first = 1;
                    }
                generateRRA(my_areas,offset,frames,&phase,h,in);        
                commitres = snd_pcm_mmap_commit(handle, offset, frames);
                if (commitres < 0 || (snd_pcm_uframes_t)commitres != frames) 
                    {
                    if ((err = xrun_recovery(handle, commitres >= 0 ? -EPIPE : commitres)) < 0) 
                        {
                        printf("MMAP commit error: %s\n", snd_strerror(err));
                        exit(EXIT_FAILURE);
                        }
                    first = 1;
                    }
                    size -= frames;
                }
            }

        sampleCounter -= period_size;    
        if(sampleCounter < RING_BUFFER_LEN && sampleCounter > 0)
          period_size = sampleCounter;
        }
    
    return 0;
    }


/*
 * code taken from the original alsa-lib
 * source (alsa-lib-1.0.20/test/pcm.c)
 *
 *
 *
 * Transfer method - direct write only using mmap_write functions
 */

static int 
direct_write_loop(snd_pcm_t *handle,
                             signed short *samples,
                             snd_pcm_channel_area_t *areas, FILE *in, RRA *h)
    {
    double phase = 0;
    signed short *ptr;
    int err, cptr;
    sampleCounter = h->samples;

    do
        {
        generateRRA(areas, 0, period_size, &phase,h,in);
        ptr = samples;
        cptr = period_size;
        while (cptr > 0) 
            {
            err = snd_pcm_mmap_writei(handle, ptr, cptr);
            if (err == -EAGAIN)
                continue;
            if (err < 0) 
                {
                if (xrun_recovery(handle, err) < 0) 
                    {
                    printf("Write error: %s\n", snd_strerror(err));
                    exit(EXIT_FAILURE);
                    }
                break;  /* skip one period */
                }
            ptr += err * channels;
            cptr -= err;
            }

        sampleCounter -= period_size;    
        if(sampleCounter < RING_BUFFER_LEN && sampleCounter > 0)
          period_size = sampleCounter;
            
        }
    //while(sampleCounter > 0);
    while(!feof(in) > 0);

    return 0;       
    } 
/*
 *
 */

struct transfer_method 
    {
    const char *name;
    snd_pcm_access_t access;
    int (*transfer_loop)(snd_pcm_t *handle,
    signed short *samples,
    snd_pcm_channel_area_t *areas, FILE
    *in, RRA *h);
    };

static struct transfer_method transfer_methods[] = {
        { "write", SND_PCM_ACCESS_RW_INTERLEAVED, write_loop },
        { "write_and_poll", SND_PCM_ACCESS_RW_INTERLEAVED, write_and_poll_loop },
        { "async", SND_PCM_ACCESS_RW_INTERLEAVED, async_loop },
        { "async_direct", SND_PCM_ACCESS_MMAP_INTERLEAVED, async_direct_loop },
        { "direct_interleaved", SND_PCM_ACCESS_MMAP_INTERLEAVED, direct_loop },
        { "direct_noninterleaved", SND_PCM_ACCESS_MMAP_NONINTERLEAVED, direct_loop },
        { "direct_write", SND_PCM_ACCESS_MMAP_INTERLEAVED, direct_write_loop },
        { NULL, SND_PCM_ACCESS_RW_INTERLEAVED, NULL }
};



static void 
detectShuffleOrLoop(char **
arguments, int count) 
    {
    char *token = NULL;
    int i = 0;

    do
        {
        token = arguments[i]; 

        if(!strcmp(token,"shuffle"))
            shuffleResult = 1;

        if(!strcmp(token,"loop"))           
            {
            loopResult = 1;
            fprintf(stderr,"loop detected \n");
            }

        i++;    
        }
    while(arguments[i]!=NULL);      
    }

static void 
shuffleTracks(char **listFiles, int *itemNo, int
totalItems)
    {

    do
        *itemNo = (rand() % (totalItems)) ;
    while(!strcmp(listFiles[*itemNo],"loop") //
    ||  !strcmp(listFiles[*itemNo],"shuffle") );                
    }

static void
loopTracks(char **listFiles,int *itemNo, int
totalItems)
    {
    do
        {
        if(totalItems-1 == *itemNo)
            *itemNo = 0;
        else        
            (*itemNo) = *itemNo + 1;
        
        fprintf(stderr,"itemNo = %d\n",*itemNo);
        }
    while(!strcmp(listFiles[*itemNo],"loop") //
    ||  !strcmp(listFiles[*itemNo],"shuffle"));         
    }

int 
main(int argc, char *argv[])
    {
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *hwparams;
    snd_pcm_sw_params_t *swparams;
    int err, method = 0, track = 0, totalTracks = argc - 1;
    signed short *samples;
    unsigned int chn;
    snd_pcm_channel_area_t *areas;
    RRA *h;
    FILE *in;
    char **playlist = &argv[1];

    snd_pcm_hw_params_alloca(&hwparams);
    snd_pcm_sw_params_alloca(&swparams);

    /* loop or shuffle in argument list */
    if(playlist[track] != NULL)
        detectShuffleOrLoop(playlist,
        totalTracks);

    do
        {

        if(argc == 1)
            in = stdin;
        else
            in  = OpenFile(playlist[track],"r");

        /* initializing an RRA structure */
        h = newRRAHeader();

        readRRAHeader(in,h,0);

        /* Update the /dev/pcm parameters,
         * according to the RRA file
         *
         *
         */

        format = (h->bitsPerSample == 8?
        SND_PCM_FORMAT_U8:
        SND_PCM_FORMAT_S16);
        rate = h->sampleRate;
        channels = h->channels;
            
        err = snd_output_stdio_attach(&output, stdout, 0);
        if (err < 0) 
            {
            fprintf(stderr,"Output failed: %s\n", snd_strerror(err));
            return 0;
            }

        fprintf(stderr,"Using transfer method: %s\n", transfer_methods[method].name);
        if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) 
            {
            fprintf(stderr,"Playback open error: %s\n", snd_strerror(err));
            return 0;
            }
        if ((err = set_hwparams(handle, hwparams, transfer_methods[method].access)) < 0) 
            {
            fprintf(stderr,"Setting of hwparams failed: %s\n", snd_strerror(err));
            exit(EXIT_FAILURE);
            }
        if ((err = set_swparams(handle, swparams)) < 0) 
            {
            fprintf(stderr,"Setting of swparams failed: %s\n", snd_strerror(err));
            exit(EXIT_FAILURE);
            }
        if (verbose > 0)
            snd_pcm_dump(handle, output);

        samples = malloc((period_size * channels * snd_pcm_format_width(format)) / 8);
        if (samples == NULL) 
            {
            fprintf(stderr,"No enough memory\n");
            exit(EXIT_FAILURE);
            }
        areas = calloc(channels, sizeof(snd_pcm_channel_area_t));
        if (areas == NULL) 
            {
            fprintf(stderr,"No enough memory\n");
            exit(EXIT_FAILURE);
            }
        for (chn = 0; chn < channels; chn++) 
            {
            areas[chn].addr = samples;
            areas[chn].first = chn * snd_pcm_format_width(format);
            areas[chn].step = channels * snd_pcm_format_width(format);
            }

        err = transfer_methods[method].transfer_loop(handle, samples, areas,in,h);
        if (err < 0)
            fprintf(stderr,"Transfer failed: %s\n", snd_strerror(err));

        fclose(in);    

        if(shuffleResult) shuffleTracks(playlist,&track,totalTracks);
        else if(loopResult) loopTracks(playlist,&track, totalTracks);

        free(areas);
        free(samples);
        snd_pcm_close(handle);
        }
    while((shuffleResult && in != stdin) //
    || (loopResult && in != stdin));


    return 0;
    }
