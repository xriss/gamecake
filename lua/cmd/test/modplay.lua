

--[[
/*

  TinyMOD
  Written by Tammo "kb" Hinrichs in 2007
  This source code is hereby placed into the public domain. Use, distribute, 
  modify, misappropriate and generally abuse it as you wish. Giving credits
  would be nice of course.

  This player includes an Amiga Paula chip "emulation" that faithfully recreates
  how it sounds when a sample is resampled using a master clock of 3.5 MHz. Yes,
  rendering at this rate and downsampling to the usual 48KHz takes quite a bit
  of CPU. Feel free to replace this part with some conventional mixing routines
  if authenticity isn't your goal and you really need Protracker MOD support
  for any other reason...

  The code should be pretty portable, all OS/platform dependent stuff is
  at the top. Code for testing is at the bottom.

  You'll need some kind of sound output that calls back the player providing
  it a stereo interleaved single float buffer to write into (0dB=1.0).

  Changelog:
  
  2007-12-07:
  * fixed 40x and 4x0 vibrato effects (jogeir - tiny tunes)
  * fixed pattern loop (olof gustafsson - pinball illusions)
  * fixed fine volslide down (olof gustafsson - pinball illusions)
  * included some external header files
  * cleanups

  2007-12-06: first "release". Note to self: Don't post stuff on pouet.net when drunk.

*/


//------------------------------------------------------------------------------
// system dependent stuff starts here

#define _CRT_SECURE_NO_DEPRECATE
#include <math.h>
#include <string.h>
#pragma intrinsic (memset, sqrt, sin, cos, atan, pow)

typedef int               sInt;
typedef unsigned int      sUInt;

typedef sInt              sBool;
typedef char              sChar;

typedef signed   char     sS8;
typedef signed   short    sS16;
typedef signed   long     sS32;
typedef signed   __int64  sS64;

typedef unsigned char     sU8;
typedef unsigned short    sU16;
typedef unsigned long     sU32;
typedef unsigned __int64  sU64;

typedef float             sF32;
typedef double            sF64;

inline void sZeroMem(void *dest, sInt size) { memset(dest,0,size); } 
inline sF32 sFSqrt(sF32 x) { return sqrtf(x); }
inline sF32 sFSin(sF32 x) { return sinf(x); }
inline sF32 sFCos(sF32 x) { return cosf(x); }
inline sF32 sFAtan(sF32 x) { return atanf(x); }
inline sF32 sFPow(sF32 b, sF32 e) { return powf(b,e); }

inline void sSwapEndian(sU16 &v) { v=((v&0xff)<<8)|(v>>8); }


// system dependent stuff ends here
//------------------------------------------------------------------------------

const sF32 sFPi=4*sFAtan(1);

template<typename T> inline T sMin(const T a, const T b) { return (a<b)?a:b;  }
template<typename T> inline T sMax(const T a, const T b) { return (a>b)?a:b;  }
template<typename T> inline T sClamp(const T x, const T min, const T max) { return sMax(min,sMin(max,x)); }
template<typename T> T sSqr(T v) { return v*v; }
template<typename T> T sLerp(T a, T b, sF32 f) { return a+f*(b-a); }
template<typename T> T sAbs(T x) { return abs(x); }

inline sF32 sFSinc(sF32 x) { return x?sFSin(x)/x:1; }
inline sF32 sFHamming(sF32 x) { return (x>-1 && x<1)?sSqr(sFCos(x*sFPi/2)):0;}

union sIntFlt { sU32 U32; sF32 F32; };

const sInt PAULARATE=3740000; // approx. pal timing
const sInt OUTRATE=48000;     // approx. pal timing
const sInt OUTFPS=50;         // approx. pal timing

//------------------------------------------------------------------------------

class Paula
{
public:

  static const sInt FIR_WIDTH=512;
  sF32 FIRMem[2*FIR_WIDTH+1];

  struct Voice
  {
  private:
    sInt Pos;
    sInt PWMCnt, DivCnt;
    sIntFlt Cur;

  public:
    sS8* Sample;
    sInt SampleLen;
    sInt LoopLen;
    sInt Period; // 124 .. 65535
    sInt Volume; // 0 .. 64

    Voice() : Period(65535), Volume(0), Sample(0), Pos(0), PWMCnt(0), DivCnt(0), LoopLen(1) { Cur.F32=0; }
    void Render(sF32 *buffer, sInt samples)
    {
      if (!Sample) return;

      sU8 *smp=(sU8*)Sample;
      for (sInt i=0; i<samples; i++)
      {
        if (!DivCnt)
        {
          // todo: use a fake d/a table for this
          Cur.U32=((smp[Pos]^0x80)<<15)|0x40000000;
          Cur.F32-=3.0f;
          if (++Pos==SampleLen) Pos-=LoopLen;
          DivCnt=Period;
        }
        if (PWMCnt<Volume) buffer[i]+=Cur.F32;
        PWMCnt=(PWMCnt+1)&0x3f;
        DivCnt--;
      }
    }

    void Trigger(sS8 *smp,sInt sl, sInt ll, sInt offs=0)
    {
      Sample=smp;
      SampleLen=sl;
      LoopLen=ll;
      Pos=sMin(offs,SampleLen-1);
    }
  };

  Voice V[4];

  // rendering in paula freq
  static const sInt RBSIZE = 4096;
  sF32 RingBuf[2*RBSIZE];
  sInt WritePos;
  sInt ReadPos;
  sF32 ReadFrac;

  void CalcFrag(sF32 *out, sInt samples)
  {
    sZeroMem(out,sizeof(sF32)*samples);
    sZeroMem(out+RBSIZE,sizeof(sF32)*samples);
    for (sInt i=0; i<4; i++)
    {
      if (i==1 || i==2)
        V[i].Render(out+RBSIZE,samples);
      else
        V[i].Render(out,samples);
    }
  }

  void Calc()
  {
    sInt RealReadPos=ReadPos-FIR_WIDTH-1;
    sInt samples=(RealReadPos-WritePos)&(RBSIZE-1);

    sInt todo=sMin(samples,RBSIZE-WritePos);
    CalcFrag(RingBuf+WritePos,todo);
    if (todo<samples)
    {
      WritePos=0;
      todo=samples-todo;
      CalcFrag(RingBuf,todo);
    }
    WritePos+=todo;
  };

  sF32 MasterVolume;
  sF32 MasterSeparation;

  // rendering in output freq
  void Render(sF32 *outbuf, sInt samples)
  {
    const sF32 step=sF32(PAULARATE)/sF32(OUTRATE);
    const sF32 pan=0.5f+0.5f*MasterSeparation;
    const sF32 vm0=MasterVolume*sFSqrt(pan);
    const sF32 vm1=MasterVolume*sFSqrt(1-pan);

    for (sInt s=0; s<samples; s++)
    {
      sInt ReadEnd=ReadPos+FIR_WIDTH+1;
      if (WritePos<ReadPos) ReadEnd-=RBSIZE;
      if (ReadEnd>WritePos) Calc();
      sF32 outl0=0, outl1=0;
      sF32 outr0=0, outr1=0;

      // this needs optimization. SSE would come to mind.
      sInt offs=(ReadPos-FIR_WIDTH-1)&(RBSIZE-1);
      sF32 vl=RingBuf[offs];
      sF32 vr=RingBuf[offs+RBSIZE];
      for (sInt i=1; i<2*FIR_WIDTH-1; i++)
      {
        sF32 w=FIRMem[i];
        outl0+=vl*w;
        outr0+=vr*w;
        offs=(offs+1)&(RBSIZE-1);
        vl=RingBuf[offs];
        vr=RingBuf[offs+RBSIZE];
        outl1+=vl*w;
        outr1+=vr*w;
      }
      sF32 outl=sLerp(outl0,outl1,ReadFrac);
      sF32 outr=sLerp(outr0,outr1,ReadFrac);
      *outbuf++=vm0*outl+vm1*outr;
      *outbuf++=vm1*outl+vm0*outr;

      ReadFrac+=step;
      sInt rfi=sInt(ReadFrac);
      ReadPos=(ReadPos+rfi)&(RBSIZE-1);
      ReadFrac-=rfi;
    }
  }

  Paula()
  {
    // make FIR table
    sF32 *FIRTable=FIRMem+FIR_WIDTH;
    sF32 yscale=sF32(OUTRATE)/sF32(PAULARATE);
    sF32 xscale=sFPi*yscale;
    for (sInt i=-FIR_WIDTH; i<=FIR_WIDTH; i++)
      FIRTable[i]=yscale*sFSinc(sF32(i)*xscale)*sFHamming(sF32(i)/sF32(FIR_WIDTH-1));

    sZeroMem(RingBuf,sizeof(RingBuf));
    ReadPos=0;
    ReadFrac=0;
    WritePos=FIR_WIDTH;

    MasterVolume=0.66f;
    MasterSeparation=0.5f;
    //FltBuf=0;
  }
};

//------------------------------------------------------------------------------

class ModPlayer
{

  Paula *P;

  static sInt BasePTable[5*12+1];
  static sInt PTable[16][60];
  static sInt VibTable[3][15][64];

  struct Sample
  {
    char Name[22];
    sU16 Length;
    sS8  Finetune;
    sU8  Volume;
    sU16 LoopStart;
    sU16 LoopLen;

    void Prepare()
    {
      sSwapEndian(Length);
      sSwapEndian(LoopStart);
      sSwapEndian(LoopLen);
      Finetune&=0x0f;
      if (Finetune>=8) Finetune-=16;
    }
  };

  struct Pattern
  {
    struct Event
    {
      sInt Sample;
      sInt Note;
      sInt FX;
      sInt FXParm;
    } Events[64][4];

    Pattern() { sZeroMem(this,sizeof(Pattern)); }

    void Load(sU8 *ptr)
    {
      for (sInt row=0; row<64; row++) for (sInt ch=0; ch<4; ch++)
      {
        Event &e=Events[row][ch];
        e.Sample = (ptr[0]&0xf0)|(ptr[2]>>4);
        e.FX     = ptr[2]&0x0f;
        e.FXParm = ptr[3];

        e.Note=0;        
        sInt period = (sInt(ptr[0]&0x0f)<<8)|ptr[1];
        sInt bestd = sAbs(period-BasePTable[0]);
        if (period) for (sInt i=1; i<=60; i++)
        {
          sInt d=sAbs(period-BasePTable[i]);
          if (d<bestd)
          {
            bestd=d;
            e.Note=i;
          }
        }

        ptr+=4;
      }
    }
  };

  Sample *Samples;
  sS8    *SData[32];
  sInt   SampleCount;
  sInt   ChannelCount;

  sU8    PatternList[128];
  sInt   PositionCount;
  sInt   PatternCount;

  Pattern Patterns[128];

  struct Chan
  {
    sInt Note;
    sInt Period;
    sInt Sample;
    sInt FineTune;
    sInt Volume;
    sInt FXBuf[16];
    sInt FXBuf14[16];
    sInt LoopStart;
    sInt LoopCount;
    sInt RetrigCount;
    sInt VibWave;
    sInt VibRetr;
    sInt VibPos;
    sInt VibAmpl;
    sInt VibSpeed;
    sInt TremWave;
    sInt TremRetr;
    sInt TremPos;
    sInt TremAmpl;
    sInt TremSpeed;

    Chan() { sZeroMem(this,sizeof(Chan)); }
    
    sInt GetPeriod(sInt offs=0, sInt fineoffs=0) 
    { 
      sInt ft=FineTune+fineoffs;
      while (ft>7) { offs++; ft-=16; }
      while (ft<-8) { offs--; ft+=16; }
      return Note?(PTable[ft&0x0f][sClamp(Note+offs-1,0,59)]):0; 
    }
    void SetPeriod(sInt offs=0, sInt fineoffs=0) { if (Note) Period=GetPeriod(offs,fineoffs); }

  } Chans[4];

  sInt Speed;
  sInt TickRate;
  sInt TRCounter;

  sInt CurTick;
  sInt CurRow;
  sInt CurPos;
  sInt Delay;

  void CalcTickRate(sInt bpm)
  {
    TickRate=(125*OUTRATE)/(bpm*OUTFPS);
  }

  void TrigNote(sInt ch, const Pattern::Event &e)
  {
    Chan &c=Chans[ch];
    Paula::Voice &v=P->V[ch];
    const Sample &s=Samples[c.Sample];
    sInt offset=0;

    if (e.FX==9) offset=c.FXBuf[9]<<8;
    if (e.FX!=3 && e.FX!=5)
    {
      c.SetPeriod();
      if (s.LoopLen>1)
        v.Trigger(SData[c.Sample],2*(s.LoopStart+s.LoopLen),2*s.LoopLen,offset);
      else
        v.Trigger(SData[c.Sample],v.SampleLen=2*s.Length,1,offset);
      if (!c.VibRetr) c.VibPos=0;
      if (!c.TremRetr) c.TremPos=0;
    }
    
  }

  void Reset()
  {
    CalcTickRate(125);
    Speed=6;
    TRCounter=0;
    CurTick=0;
    CurRow=0;
    CurPos=0;
    Delay=0;
  }


  void Tick()
  {
    const Pattern &p=Patterns[ PatternList[CurPos] ];
    const Pattern::Event *re=p.Events[CurRow];
    for (sInt ch=0; ch<4; ch++)
    {
      const Pattern::Event &e=re[ch];
      Paula::Voice &v=P->V[ch];
      Chan &c=Chans[ch];
      const sInt fxpl=e.FXParm&0x0f;
      sInt TremVol=0;
      if (!CurTick)
      {
        if (e.Sample)
        {
          c.Sample=e.Sample;
          c.FineTune=Samples[c.Sample].Finetune;
          c.Volume=Samples[c.Sample].Volume;
        }

        if (e.FXParm)
          c.FXBuf[e.FX]=e.FXParm;
        
        if (e.Note && (e.FX!=14 || ((e.FXParm>>4)!=13)))
        {
          c.Note=e.Note;
          TrigNote(ch,e);
        }

        switch (e.FX)
        {
        case 4: // vibrato
        case 6:
          if (c.FXBuf[4]&0x0f) c.VibAmpl=c.FXBuf[4]&0x0f;
          if (c.FXBuf[4]&0xf0) c.VibSpeed=c.FXBuf[4]>>4;
          c.SetPeriod(0,VibTable[c.VibWave][(c.VibAmpl)-1][c.VibPos]);
          break;
        case 7: // tremolo
          if (c.FXBuf[7]&0x0f) c.TremAmpl=c.FXBuf[7]&0x0f;
          if (c.FXBuf[7]&0xf0) c.TremSpeed=c.FXBuf[7]>>4;
          TremVol=VibTable[c.TremWave][(c.TremAmpl)-1][c.TremPos];
          break;
        case 12: // set vol
          c.Volume=sClamp(e.FXParm,0,64);
          break;
        case 14: // special
          if (fxpl) c.FXBuf14[e.FXParm>>4]=fxpl;
          switch (e.FXParm>>4)
          {
          case 0: // set filter
            break;
          case 1: // fineslide up
            c.Period=sMax(113,c.Period-c.FXBuf14[1]);
            break;
          case 2: // slide down
            c.Period=sMin(856,c.Period+c.FXBuf14[2]);
            break;
          case 3: // set glissando sucks!
            break;
          case 4: // set vib waveform
            c.VibWave=fxpl&3;
            if (c.VibWave==3) c.VibWave=0;
            c.VibRetr=fxpl&4;
            break;
          case 5: // set finetune
            c.FineTune=fxpl;
            if (c.FineTune>=8) c.FineTune-=16;
            break;
          case 7:  // set tremolo 
            c.TremWave=fxpl&3;
            if (c.TremWave==3) c.TremWave=0;
            c.TremRetr=fxpl&4;
            break;
          case 9: // retrigger
            if (c.FXBuf14[9] && !e.Note)
              TrigNote(ch,e);
            c.RetrigCount=0;
            break;
          case 10: // fine volslide up
            c.Volume=sMin(c.Volume+c.FXBuf14[10],64);
            break;
          case 11: // fine volslide down;
            c.Volume=sMax(c.Volume-c.FXBuf14[11],0);
            break;
          case 14: // delay pattern
            Delay=c.FXBuf14[14];
            break;
          case 15: // invert loop (WTF)
            break;            
          }
          break;
        case 15: // set speed
          if (e.FXParm)
            if (e.FXParm<=32)
              Speed=e.FXParm;
            else
              CalcTickRate(e.FXParm);
          break;
        }


      }
      else
      {
        switch (e.FX)
        {
        case 0: // arpeggio
          if (e.FXParm)
          {
            sInt no=0;
            switch (CurTick%3)
            {
            case 1: no=e.FXParm>>4; break;
            case 2: no=e.FXParm&0x0f; break;
            }
            c.SetPeriod(no);
          }
          break;
        case 1: // slide up
          c.Period=sMax(113,c.Period-c.FXBuf[1]);
          break;
        case 2: // slide down
          c.Period=sMin(856,c.Period+c.FXBuf[2]);
          break;
        case 5: // slide plus volslide
          if (c.FXBuf[5]&0xf0)
            c.Volume=sMin(c.Volume+(c.FXBuf[5]>>4),0x40);
          else
            c.Volume=sMax(c.Volume-(c.FXBuf[5]&0x0f),0);
          // no break!
        case 3: // slide to note
          {
            sInt np=c.GetPeriod();
            if (c.Period>np)
              c.Period=sMax(c.Period-c.FXBuf[3],np);
            else if (c.Period<np)
              c.Period=sMin(c.Period+c.FXBuf[3],np);
          }
          break;
        case 6: // vibrato plus volslide
          if (c.FXBuf[6]&0xf0)
            c.Volume=sMin(c.Volume+(c.FXBuf[6]>>4),0x40);
          else
            c.Volume=sMax(c.Volume-(c.FXBuf[6]&0x0f),0);
          // no break!
        case 4: // vibrato ???
          c.SetPeriod(0,VibTable[c.VibWave][c.VibAmpl-1][c.VibPos]);
          c.VibPos=(c.VibPos+c.VibSpeed)&0x3f;
          break;
        case 7: // tremolo ???
          TremVol=VibTable[c.TremWave][c.TremAmpl-1][c.TremPos];
          c.TremPos=(c.TremPos+c.TremSpeed)&0x3f;
          break;
        case 10: // volslide
          if (c.FXBuf[10]&0xf0)
            c.Volume=sMin(c.Volume+(c.FXBuf[10]>>4),0x40);
          else
            c.Volume=sMax(c.Volume-(c.FXBuf[10]&0x0f),0);
          break;
        case 11: // pos jump
          if (CurTick==Speed-1)
          {
            CurRow=-1;
            CurPos=e.FXParm;
          }
          break;
        case 13: // pattern break
          if (CurTick==Speed-1)
          {
            CurPos++;
            CurRow=(10*(e.FXParm>>4)+(e.FXParm&0x0f))-1;
          }
          break;
        case 14: // special
          switch (e.FXParm>>4)
          {
          case 6: // loop pattern
            if (!fxpl) // loop start
              c.LoopStart=CurRow;
            else if (CurTick==Speed-1)
            {
              if (c.LoopCount<fxpl)
              {
                CurRow=c.LoopStart-1;
                c.LoopCount++;
              }
              else
                c.LoopCount=0;
            }
            break;
          case 9: // retrigger
            if (++c.RetrigCount == c.FXBuf14[9])
            {
              c.RetrigCount=0;
              TrigNote(ch,e);
            }
            break;
          case 12: // cut
            if (CurTick==c.FXBuf14[12])
              c.Volume=0;
            break;
          case 13: // delay
            if (CurTick==c.FXBuf14[13])
              TrigNote(ch,e);
            break;
          }
          break;       
        }
      }

      v.Volume=sClamp(c.Volume+TremVol,0,64);
      v.Period=c.Period;
    }

    CurTick++;
    if (CurTick>=Speed*(Delay+1))
    {
      CurTick=0;
      CurRow++;
      Delay=0;
    }
    if (CurRow>=64)
    {
      CurRow=0;
      CurPos++;
    }
    if (CurPos>=PositionCount)
      CurPos=0;
  };

public:

  char Name[21];

  ModPlayer(Paula *p, sU8 *moddata) : P(p)
  {
    // calc ptable
    for (sInt ft=0; ft<16; ft++)
    {
      sInt rft= -((ft>=8)?ft-16:ft);
      sF32 fac=sFPow(2.0f,sF32(rft)/(12.0f*16.0f));
      for (sInt i=0; i<60; i++)
        PTable[ft][i]=sInt(sF32(BasePTable[i])*fac+0.5f);
    }

    // calc vibtable
    for (sInt ampl=0; ampl<15; ampl++)
    {
      sF32 scale=ampl+1.5f;
      sF32 shift=0;
      for (sInt x=0; x<64; x++)
      {
        VibTable[0][ampl][x]=sInt(scale*sFSin(x*sFPi/32.0f)+shift);
        VibTable[1][ampl][x]=sInt(scale*((63-x)/31.5f-1.0f)+shift);
        VibTable[2][ampl][x]=sInt(scale*((x<32)?1:-1)+shift);
      }
    }

    // "load" the mod
    memcpy(Name,moddata,20); Name[20]=0; moddata+=20;

    SampleCount=16;
    ChannelCount=4;
    Samples=(Sample*)(moddata-sizeof(Sample)); moddata+=15*sizeof(Sample);
    sU32 &tag=*(sU32*)(moddata+130+16*sizeof(Sample));
    switch (tag)
    {
    case '.K.M': case '4TLF': case '!K!M':
      SampleCount=32;
      break;
    }
    if (SampleCount>16)
      moddata+=(SampleCount-16)*sizeof(Sample);
    for (sInt i=1; i<SampleCount; i++) Samples[i].Prepare();

    PositionCount=*moddata; moddata+=2; // + skip unused byte
    memcpy(PatternList,moddata,128); moddata+=128;
    if (SampleCount>15) moddata+=4; // skip tag

    PatternCount=0;
    for (sInt i=0; i<128; i++)
      PatternCount=sClamp(PatternCount,PatternList[i]+1,128);

    for (sInt i=0; i<PatternCount; i++)
    {
      Patterns[i].Load(moddata);
      moddata+=1024;
    }

    sZeroMem(SData,sizeof(SData));
    for (sInt i=1; i<SampleCount; i++)
    {
      SData[i]=(sS8*)moddata;
      moddata+=2*Samples[i].Length;
    }

    Reset();
  }

  sU32 Render(sF32 *buf, sU32 len)
  {
    while (len)
    {
      sInt todo=sMin<sInt>(len,TRCounter);
      if (todo)
      {
        P->Render(buf,todo);
        buf+=2*todo;
        len-=todo;
        TRCounter-=todo;
      }
      else
      {
        Tick();
        TRCounter=TickRate;
      }
    }
    return 1;
  }
  
  static sU32 __stdcall RenderProxy(void *parm, sF32 *buf, sU32 len)
  { 
    return ((ModPlayer*)parm)->Render(buf,len); 
  }

};

sInt ModPlayer::BasePTable[61]=
{
  0, 1712,1616,1525,1440,1357,1281,1209,1141,1077,1017, 961, 907,
   856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
   428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
   214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113,
   107, 101,  95,  90,  85,  80,  76,  71,  67,  64,  60,  57,
};

sInt ModPlayer::PTable[16][60];
sInt ModPlayer::VibTable[3][15][64];

//------------------------------------------------------------------------------
// ok, let's test it:

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include "dsio.h" // good luck.

static sU8 mod[4*1024*1024];

int __cdecl main(int argc, const char **argv)
{
  if (argc<2) 
  {
    MessageBox(0,"Usage: tinymod <mod name>","TinyMOD",MB_OK);
    return 1;
  }
  HANDLE fh=CreateFile(argv[1],GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
  if (fh==INVALID_HANDLE_VALUE) 
  {
    MessageBox(0,"couldn't open file!","TinyMOD",MB_OK);
    return 1;
  }
  sInt size=GetFileSize(fh,0);
  DWORD read;
  ReadFile(fh,mod,size,&read,0);
  CloseHandle(fh);

  Paula P;
  ModPlayer player(&P,mod);
  dsInit(player.RenderProxy,&player,GetForegroundWindow());
  MessageBox(0,player.Name,"TinyMOD",MB_OK);
  dsClose();
  
  return 0;
}

]]

local wpack=require("wetgenes.pack")
local wstr=require("wetgenes.string")
local bit=require("bit")


function load_xm(name)
	local mod={}
	
	local xm=assert(io.open(name,"rb")):read("*a")

--	local dbg=function()end
	local dbg=print

	local off=0

dbg("mod length","=",#xm)

	local prehead=xm:sub(1,17)

dbg("prehead =",prehead)
assert(prehead,"Extended module: ")

	local off=17

	local head_fmt={
		20,"name",
		"u8","ID",
		20,"tracker_name",
		"u16","tracker_version",
		"u32","size",
		"u16","length",
		"u16","restart",
		"u16","numof_chanels",
		"u16","numof_patterns",
		"u16","numof_instruments",
		"u16","flags",
		"u16","tempo",
		"u16","bpm",
	}

	local head,_size=wpack.load(xm,head_fmt,off)
	off=off+_size
dbg(wstr.dump(head))
dbg("off","=",off)

	mod.head=head

	-- read anupto 256 byte array, which is the order of song patterns
	local song=wpack.load_array(xm,"u8",off,head.length)
	off=off+256 -- fixed size advance

	mod.song=song

dbg(wstr.dump(song))
dbg("off","=",off)


	local pattern_fmt={
		"u32","head_size",
		"u8","type",
		"u16","numof_rows",
		"u16","size",
	}

	local patterns={}
	mod.patterns=patterns
	for i=1,head.numof_patterns do

		local pattern,_size=wpack.load(xm,pattern_fmt,off)
		off=off+_size

		patterns[i]=pattern

		if pattern.size > 0 then
			local bytes=wpack.load_array(xm,"u8",off,pattern.size)
			local idx=1
			local function decomp()
	--print(idx,"/",pattern.size)
				local c=bytes[idx]
	--print(c,idx,"/",pattern.size)
				if bit.band(c,128)==128 then -- compressed
					idx=idx+1
				else
					c=255
				end
				local r={0,0,0,0,0}
				if bit.band(c,1)==1 then
					r[1]=bytes[idx]
					idx=idx+1
				end
				if bit.band(c,2)==2 then
					r[2]=bytes[idx]
					idx=idx+1
				end
				if bit.band(c,4)==4 then
					r[3]=bytes[idx]
					idx=idx+1
				end
				if bit.band(c,8)==8 then
					r[4]=bytes[idx]
					idx=idx+1
				end
				if bit.band(c,16)==16 then
					r[5]=bytes[idx]
					idx=idx+1
				end
				return r
			end
			
			pattern.tab={}
			for cs=1,head.numof_chanels do pattern.tab[cs]={} end
			for cs=1,head.numof_chanels do
				for rs=1,pattern.numof_rows do
	--print(cs.."/"..head.numof_chanels,rs.."/"..pattern.numof_rows,idx.."/"..pattern.size)
					if idx < pattern.size then
						pattern.tab[cs][rs]=decomp()
					else
						pattern.tab[cs][rs]={0,0,0,0,0} --no more data so just pad
					end
				end
			end
			
		end
		
		off=off+pattern.size
	--dbg(wstr.dump(pattern))
	dbg("off","=",off)



	end


	local instrument_fmt={
		"u32","head_size",
		22,"name",
		"u8","type",
		"u16","numof_samples",
		"u32","sample_head_size",
	}

	local sample_fmt={
		"u32","length",
		"u32","loop_start",
		"u32","loop_length",
		"u8","volume",
		"s8","finetune",
		"u8","type",
		"u8","pan",
		"s8","tune",
		"u8","reserved",
		22,"name",
	}

	local instruments={}
	mod.instruments=instruments
	
	for i=1,head.numof_instruments do

		local instrument,_size=wpack.load(xm,instrument_fmt,off)
		off=off+instrument.head_size

		instruments[i]=instrument

dbg(wstr.dump(instrument))
dbg("off","=",off,_size)

		instrument.samples={}
		for i=1,instrument.numof_samples do
			local sample,_size=wpack.load(xm,sample_fmt,off)
			off=off+instrument.sample_head_size
			
			local _size=sample.length

dbg(wstr.dump(sample))
dbg("off","=",off,_size)

			if sample.length>0 then
				if bit.band(sample.type,0x10)~=0 then -- a 16 bit sample
				
		--			_size=_size*2 -- double its size
					
					sample.data=wpack.load_array(xm,"s16",off,sample.length)
					local t=0
					for i,v in ipairs(sample.data) do
						t=sample.data[i]+t -- handle delta "compression"
						sample.data[i]=0x8000+t -- convert from signed
					end
				else -- 8bit
					sample.data=wpack.load_array(xm,"s8",off,sample.length)
					local t=0
					for i,v in ipairs(sample.data) do
						t=sample.data[i]+t -- handle delta "compression"
						sample.data[i]=0x8000+(t*256) -- scale up and convert from signed
					end
				end
				sample.data=wpack.save_array(sample.data,"u16",0,sample.length)
			end
sample.data=nil

	-- need to convert the sample to a standard 16bit
			
			


			instrument.samples[i]=sample
			
			off=off+_size
		end
		
	--do return end


	end
	
	return mod
end

print("loading ".."dat/mods/Neverending_Story.xm")
local mod=load_xm("dat/mods/Neverending_Story.xm")

--print(wstr.dump(mod))





