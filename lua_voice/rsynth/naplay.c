#include <useconfig.h>
#include <sys/time.h>
#include <sys/stat.h>

#ifdef __GNUC__
#define NeedFunctionPrototypes 1
#define NeedNestedPrototypes 1
#endif

#include <audio/audiolib.h>
#include <audio/soundlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include "proto.h"
#include "getargs.h"
#include "hplay.h"

long samp_rate = 8000;
static int volume = 100;
static char *audioserver = NULL;
static AuServer *aud;


static void done PROTO((AuServer * aud, AuEventHandlerRec * handler, AuEvent * ev, AuPointer data));

static void
done(aud, handler, ev, data)
AuServer *aud;
AuEventHandlerRec *handler;
AuEvent *ev;
AuPointer data;
{
 switch (ev->auany.type)
  {
   case AuEventTypeElementNotify:
    {
     int *d = (int *) data;
     *d = (ev->auelementnotify.cur_state == AuStateStop);
     if (!*d || ev->auelementnotify.reason != AuReasonEOF)
      {
       fprintf(stderr, "curr_state=%d reason=%d\n",
               ev->auelementnotify.cur_state,
               ev->auelementnotify.reason);
      }
    }
    break;
   case AuEventTypeMonitorNotify:
    break;
   default:
    fprintf(stderr, "type=%d serial=%ld time=%ld id=%ld\n",
            ev->auany.type, ev->auany.serial, ev->auany.time, ev->auany.id);
    break;
  }
}

void
audio_play(n,data)
int n;
short *data;
{
 int endian = 1;
#define little_endian ((*((char *)&endian) == 1))
 int priv = 0;
 AuEvent ev;
 Sound s = SoundCreate(SoundFileFormatNone, little_endian ?
                       AuFormatLinearSigned16LSB : AuFormatLinearSigned16MSB,
                       1, samp_rate, n, "Chit chat");
 if (aud)
  {
#ifdef USE_ALL_ARGS
   AuStatus ret_status;
   AuFlowID flow = 0;
   int monitor = 0;
   int multiplier = 0;
   if (!AuSoundPlayFromData(aud, s, data, AuNone,
                            AuFixedPointFromFraction(volume, 100),
                            done, &priv,
                            &flow, &multiplier,
                            &monitor, &ret_status))
#else
   if (!AuSoundPlayFromData(aud, s, data, AuNone,
                            AuFixedPointFromFraction(volume, 100),
                            done, &priv,
                            NULL, NULL, NULL, NULL))
#endif
    perror("problems playing data");
   else
    {
     while (1)
      {
       AuNextEvent(aud, AuTrue, &ev);
       AuDispatchEvent(aud, &ev);
       if (priv)
        break;
      }
    }
  }
 SoundDestroy(s);
}

void
audio_term()
{
 if (aud)
  {
   AuFlush(aud);
   AuCloseServer(aud);
  }
}

int
audio_init(argc,argv)
int argc;
char *argv[];
{
 int rate_set = samp_rate;
 int vol = 0;

 argc = getargs("Nas",argc, argv,
                "r", "%d", &rate_set, "Sample rate Hz",
                "V", "%d", &vol,      "Volume 0 .. 1.0",
                "a", "", &audioserver,"Name of server",
                NULL);
 if (help_only)
  return argc;

 if ((aud = AuOpenServer(audioserver, 0, NULL, 0, NULL, NULL)) == (AuServer *) 0)
  perror(audioserver);

 if (rate_set && rate_set != samp_rate)
  samp_rate = rate_set;

 if (vol)
  volume = vol;

 return argc;
}
