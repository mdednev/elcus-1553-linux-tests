#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef DOS
#include <conio.h>
#include <dos.h>
#include <alloc.h>
#endif
#if defined(WINDOWS) || defined(WINDOWSNT)
#include <conio.h>
#include <windows.h>
#endif
#ifdef QNX4
#include <conio.h>
#include <malloc.h>
#include <sys/console.h>
#include <fcntl.h>
#endif
#ifdef LINUX
#include <unistd.h>
#include <termios.h>
#endif

#ifdef DOS
/*#include "tmkll4.c"*/
#include "tmkinit.c"
#endif
#ifdef WINDOWS
#include "wdmtmkv2.cpp"
#endif
#ifdef WINDOWSNT
#include "ntmk.c"
#define WINDOWS
#endif
#ifdef LINUX
#include "ltmk.c"
#endif
#ifdef QNX4
#include "tmkll4.c"
#endif

#ifdef DOS
void wherexy(int *px, int *py)
{
 *px = wherex();
 *py = wherey();
}
#endif //DOS

#if defined(WINDOWS) || defined(LINUX)
#define HUGE
#define disable()
#define enable()
#define farmalloc malloc
#define farfree free
#define far
#endif //WINDOWSLINUX

#ifdef LINUX
struct termios saved_attr, cur_attr;
int peek_character = -1;

void reset_input_mode()
{
  tcsetattr(STDIN_FILENO, TCSANOW, &saved_attr);
}

void set_input_mode()
{
  /* Make sure stdin is a terminal. */
  if (!isatty(STDIN_FILENO))
  {
    printf("Not a terminal.\n");
    exit(0);
  }

  tcgetattr(STDIN_FILENO, &saved_attr);
  atexit(reset_input_mode);

  cur_attr = saved_attr;
  cur_attr.c_lflag &= ~(ICANON|ECHO); /* Clear ICANON and ECHO. */
  cur_attr.c_cc[VMIN] = 1;
  cur_attr.c_cc[VTIME] = 0;
//  tcsetattr(STDIN_FILENO, TCSAFLUSH, &cur_attr);
  tcsetattr(STDIN_FILENO, TCSANOW, &cur_attr);
}

int kbhit()
{
  unsigned char ch;
  int nread;

  if (peek_character != -1)
    return 1;
  cur_attr.c_cc[VMIN] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &cur_attr);
  nread = read(STDIN_FILENO, &ch, 1);
  cur_attr.c_cc[VMIN] = 1;
  tcsetattr(STDIN_FILENO, TCSANOW, &cur_attr);
  if (nread == 1)
  {
    peek_character = ch;
    return 1;
  }
  return 0;
}

int getch()
{
  char ch;

  if (peek_character != -1)
  {
    ch = peek_character;
    peek_character = -1;
    return ch;
  }
  read(STDIN_FILENO, &ch, 1);
  return ch;
}

void gotoxy(int x, int y)
{
  printf("\x1B[%d;%dH", y, x);
}

int wherex()
{
 int x, y;
 printf("\x1B[6n");
 scanf("\x1B[%d;%dR", &y, &x);
 return x;
}

int wherey()
{
 int x, y;
 printf("\x1B[6n");
 scanf("\x1B[%d;%dR", &y, &x);
 return y;
}

void wherexy(int *px, int *py)
{
 printf("\x1B[6n");
 scanf("\x1B[%d;%dR", py, px);
}
#define cprintf printf
#endif //LINUX

#ifdef QNX4
#define HUGE
#define farmalloc malloc
#define farfree free
#define disable _disable
#define enable _enable

struct _console_ctrl *cc;

void gotoxy(int x, int y)
{
 printf("\x1B=%c%c", y+0x1F, x+0x1F);
}

int wherex()
{
 int x;
 console_read(cc, 0, 0, NULL, 0, NULL, &x, NULL);
 return x+1;
}

int wherey()
{
 int y;
 console_read(cc, 0, 0, NULL, 0, &y, NULL, NULL);
 return y+1;
}

void wherexy(int *px, int *py)
{
 console_read(cc, 0, 0, NULL, 0, py, px, NULL);
 ++(*px);
 ++(*py);
}
#endif //QNX4

#ifdef WINDOWS
HANDLE hIn, hOut;

void gotoxy(int x, int y)
{
 COORD xy;

 xy.X = x-1;
 xy.Y = y-1;
 SetConsoleCursorPosition(hOut, xy);
}

int wherex()
{
 CONSOLE_SCREEN_BUFFER_INFO csbi;

 GetConsoleScreenBufferInfo(hOut, &csbi);
 return csbi.dwCursorPosition.X+1;
}

int wherey()
{
 CONSOLE_SCREEN_BUFFER_INFO csbi;

 GetConsoleScreenBufferInfo(hOut, &csbi);
 return csbi.dwCursorPosition.Y+1;
}

void wherexy(int *px, int *py)
{
 CONSOLE_SCREEN_BUFFER_INFO csbi;

 GetConsoleScreenBufferInfo(hOut, &csbi);
 *px = csbi.dwCursorPosition.X+1;
 *py = csbi.dwCursorPosition.Y+1;
}
#endif //WINDOWS

#ifndef HUGE
#define HUGE huge
#endif

#ifdef DOS
#define LPT_INT_EXC
#define LPT_ERR

//#ifdef LPT_INT_EXC
//#define LPT
//#endif
#endif //DOS

#define WAITWITHKBHIT

#define INTX

#define MAX_ERRLOG 10000L

#define RT_ADDR 10

#ifdef INTX
#define MY_BUS_A CX_BUS_A
#define MY_BUS_B CX_BUS_B
#define BUS_SHIFT 10
#else
#define MY_BUS_A BUS_A
#define MY_BUS_B BUS_B
#define BUS_SHIFT 15
#endif

#ifdef LINUX
int events;
TTmkEventData tmkEvD;
#endif //LINUX

#ifdef WINDOWS
HANDLE hEvent;
TTmkEventData tmkEvD;
#endif //WINDOWS

#ifdef QNX4
pid_t proxy, proxyMT;
TTmkProxyData TmkProxyData;
int msg[1];
#endif //QNX4

int fStop, fInt;

#ifdef LPT
unsigned lpt_data_port, lpt_status_port, lpt_ctrl_port;
int lpt_num = 1;
void init_lpt();
void set_lpt_data(unsigned char data);
#endif

#ifdef WAITWITHKBHIT
long dcWait;
#endif
volatile int nIntBC;
volatile int nIntRT;
volatile int nIntMT;
volatile unsigned long cMtInts = 0L;
volatile unsigned long cRtInts = 0L;
volatile unsigned long cRTErrInts = 0L;
volatile unsigned long cInts = 0L;
volatile unsigned long cGoodInts = 0L;
volatile unsigned long cBadInts = 0L;
volatile unsigned wRes;
unsigned short wResP;
unsigned long
  cDataBcRt = 0L, cDataBcRtBrcst = 0L,
  cDataRtBc = 0L,
  cDataRtRtBrcst = 0L,
  cCtrlCA = 0L, cCtrlCBrcst = 0L,
  cCtrlCDA = 0L, cCtrlCDBrcst = 0L,
  cCtrlCAD = 0L;
unsigned long cDataErrA = 0L;
unsigned long cDataErrB = 0L;

int fBus = 0, fBusP = 0;
int fBusA = 0;
int fBusB = 0;
int fBothBuses;
int fCmds;
int fBrcst;
int fInstr;
int fFlag;
int fArgErr;
int long nMsg, nMsgBeg, nMsgEnd;
unsigned short wSubAddr0, wSubAddrMask;
unsigned short wBcMaxBase, wMtMaxBase;
int fMtActive;

int nMaxErr;
int fTryMT;
int nbc, nrt, nmt, n, cn = 0;
int res = -1;

unsigned short wBase, wBaseP, wCtrlCode, wCtrlCodeP, wSubAddr, wSubAddrP, wLen, wLenP;
unsigned short wMtBase, wMtBaseP, wResMT;
unsigned short buf[64], bufP[64], buf1[64];

int i, fError;
int nCheckDir;
unsigned short wCheckMode;
unsigned short wChecked;

FILE *fileOut = NULL;

#ifdef LPT
void init_lpt()
/*
       Вычисление номеров портов по таблице данных BIOS .
*/
{
  lpt_data_port = peek(0x0040, 8 + (lpt_num - 1) * 2);
  lpt_status_port = lpt_data_port + 1;
  lpt_ctrl_port = lpt_data_port + 2;
}

void set_lpt_data(unsigned char data)
{
  outportb(lpt_data_port, data);
}
#endif

#ifdef INTX
void BCIntX(unsigned wBase, unsigned wResult)
#else
void BCInt(unsigned wResult, unsigned nothing1, unsigned nothing2)
#endif
{
  wRes = wResult;
  if (wRes == 0)
    ++cGoodInts;
  else
  {
    ++cBadInts;
#ifdef LPT_INT_EXC
    outportb(0x378, 0xff);
    outportb(0x378, 0);
//    set_lpt_data(0xff);
//    set_lpt_data(0);
#endif
  }
  ++cInts;
  nIntBC = 1;
#ifdef DOS
  bcrestore();
#endif //DOS
}

void MTIntSig(unsigned nothing)
{
  ++cMtInts;
  nIntMT = 1;
#ifdef DOS
  mtrestore();
#endif //DOS
}

void RTInt(unsigned nothing)
{
  ++cRtInts;
  nIntRT = 1;
#ifdef DOS
  rtrestore();
#endif //DOS
}

void RTIntErr(unsigned nothing)
{
  ++cRTErrInts;
  nIntRT = 1;
#ifdef DOS
  rtrestore();
#endif //DOS
}

void IncDataErrD(char *pszMsg, unsigned wData)
{
  fError = 1;
  if (fBusP == MY_BUS_A)
    ++cDataErrA;
  else
    ++cDataErrB;
  if ((cDataErrA + cDataErrB) > MAX_ERRLOG)
    return;
  fprintf(fileOut, "%ld: %04X:", cInts, wResP);
  switch (wCtrlCodeP)
  {
  case DATA_BC_RT:
    fprintf(fileOut, "BC->RT");
    break;
  case DATA_BC_RT_BRCST:
    fprintf(fileOut, "BC->allRT");
    break;
  case DATA_RT_BC:
    fprintf(fileOut, "RT->BC");
    break;
  case DATA_RT_RT_BRCST:
    fprintf(fileOut, "RT->allRT");
    break;
  case CTRL_C_A:
    fprintf(fileOut, "C-A");
    break;
  case CTRL_C_BRCST:
    fprintf(fileOut, "C->AllRT");
    break;
  case CTRL_CD_A:
    fprintf(fileOut, "CD-A");
    break;
  case CTRL_CD_BRCST:
    fprintf(fileOut, "CD->AllRT");
    break;
  case CTRL_C_AD:
    fprintf(fileOut, "C-AD");
    break;
  default:
    break;
  }
  fprintf(fileOut, "(%d): %s: %04X\n", wLenP, pszMsg, wData);
}

void IncDataErr()
{
  int iP = i;

#ifdef LPT_ERR
  outportb(0x378, 0xff);
  outportb(0x378, 0);
#endif
  fError = 1;
  if (fBusP == MY_BUS_A)
    ++cDataErrA;
  else
    ++cDataErrB;
  if ((cDataErrA + cDataErrB) > MAX_ERRLOG)
    return;
  fprintf(fileOut, "%ld: %04X:", cInts, wResP);
  switch (wCheckMode)
  {
  case 0:
    fprintf(fileOut, "bc: ");
    break;
  case 1:
    fprintf(fileOut, "rt: ");
    break;
  case 2:
    fprintf(fileOut, "mt: ");
    break;
  default:
    fprintf(fileOut, "??: ");
    break;
  }
  switch (wCtrlCodeP)
  {
  case DATA_BC_RT:
    if (wCheckMode == 1)
      iP = i + 1; 
    fprintf(fileOut, "BC->RT(%d)", wLenP);
    break;
  case DATA_BC_RT_BRCST:
    if (wCheckMode == 1)
      iP = i + 1;
    fprintf(fileOut, "BC->allRT(%d)", wLenP);
    break;
  case DATA_RT_BC:
    if (wCheckMode == 1)
      iP = i + 2;
    fprintf(fileOut, "RT->BC(%d)", wLenP);
    break;
  case DATA_RT_RT_BRCST:
    if (wCheckMode == 1)
      iP = i + 3;
    fprintf(fileOut, "RT->allRT(%d)", wLenP);
    break;
  case CTRL_C_A:
    fprintf(fileOut, "C-A");
    break;
  case CTRL_C_BRCST:
    fprintf(fileOut, "C->AllRT");
    break;
  case CTRL_CD_A:
    if (wCheckMode == 1)
      iP = i + 1;
    fprintf(fileOut, "CD-A");
    break;
  case CTRL_CD_BRCST:
    if (wCheckMode == 1)
      iP = i + 1;
    fprintf(fileOut, "CD->AllRT");
    break;
  case CTRL_C_AD:
    fprintf(fileOut, "C-AD");
    break;
  default:
    fprintf(fileOut, "???");
    break;
  }
  fprintf(fileOut, ": %d: %04X %04X\n", i, bufP[iP], buf1[i]);
}

int main(int argc, char *argv[])
{
#ifdef WINDOWS
  hIn = GetStdHandle(STD_INPUT_HANDLE);
  hOut = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
#ifdef LINUX
  set_input_mode();
#endif //LINUX
#ifdef QNX4
  if ((cc = console_open(fileno(stdout), O_RDWR)) == NULL)
  {
    printf("console_open error\n");
    exit(0);
  }
  strcpy(szDevName,"/dev/tmk");
#endif //QNX4
  if ((fileOut = fopen("errlog","wt")) == NULL)
  {
    printf("Can't open errlog\n");
    goto stop;
  }
  fCmds = 1;
  fBrcst = 1;
  fInstr = 1;
  fFlag = 1;
  fBothBuses = 1;
  fArgErr = 0;
  fBusA = fBusB = 0;
  fMtActive = 0;
  nMsgBeg = 0L;
  nMsgEnd = -1L;
  nbc = 0;
  nrt = 1;
  nmt = 2;
  fTryMT = 0;
  nMaxErr = 0;
  for (i = 1; i < argc; ++i)
  {
    switch (argv[i][0])
    {
    case 'a':
    case 'A':
      fBothBuses = 0;
      fBusA = 1;
      break;
    case 'b':
    case 'B':
      fBothBuses = 0;
      fBusB = 1;
      break;
    case 'c':
    case 'C':
      switch (argv[i][1])
      {
      case '+':
        fCmds = 1;
        break;
      case '-':
        fCmds = 0;
        break;
      default:
        fArgErr = 1;
        break;
      }
      break;
    case 'g':
    case 'G':
      switch (argv[i][1])
      {
      case '+':
        fBrcst = 1;
        break;
      case '-':
        fBrcst = 0;
        break;
      default:
        fArgErr = 1;
        break;
      }
      break;
    case 'i':
    case 'I':
      switch (argv[i][1])
      {
      case '+':
        fInstr = 1;
        break;
      case '-':
        fInstr = 0;
        break;
      default:
        fArgErr = 1;
        break;
      }
      break;
    case 'f':
    case 'F':
      switch (argv[i][1])
      {
      case '+':
        fFlag = 1;
        break;
      case '-':
        fFlag = 0;
        break;
      default:
        fArgErr = 1;
        break;
      }
      break;
    case 'm':
    case 'M':
      if (sscanf(&argv[i][1], "%ld-%ld", &nMsgBeg, &nMsgEnd) != 2)
        fArgErr = 1;
      break;
    case 'e':
    case 'E':
      if (sscanf(&argv[i][1], "%d", &nMaxErr) != 1)
        fArgErr = 1;
      break;
    default:
      if (sscanf(argv[i], "%d", &n) != 0)
      {
        switch (cn)
        {
        case 0:
          nbc = n;
          ++cn;
          fTryMT = 0;
          break;
        case 1:
          nrt = n;
          ++cn;
          break;
        case 2:
          nmt = n;
          ++cn;
          fTryMT = 1;
          break;
        default:
          fArgErr = 1;
        }
      }
      else
        fArgErr = 1;
      break;
    }
  }
  if (fInstr)
  {
    wSubAddr0 = 0x10;
    wSubAddrMask = 0x0F;
  }
  else
  {
    wSubAddr0 = 0x00;
    wSubAddrMask = 0x1F;
  }
  if (fArgErr)
  {
    printf("Usage: randbcrt [D D [D]] [a] [b] [c+-] [g+-] [i+-] [f+-] [eD] [mD-D]\n"
           "Default: randbcrt 0 1 a b c+ g+ i+\n");
    goto stopn;
  }
  if (fBothBuses)
    fBusA = fBusB = 1;
#ifdef DOS 
  if (TmkInit("tmk.cfg"))
  {
    printf("TmkInit error\n");
    goto stop;
  }
#endif //DOS
#ifdef WINDOWS
  hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (!hEvent)
  {
    printf("CreateEvent() failed!\n");
    goto stop;
  }
#endif //WINDOWS
#if defined(WINDOWS) || defined(LINUX)
  if (TmkOpen())
  {
    printf("TmkOpen error\n");
    goto stop;
  }
  if (tmkconfig(nbc))
  {
    printf("tmkconfig(%d) error for BC\n", nbc);
    goto stop;
  }
  if (tmkconfig(nrt))
  {
    printf("tmkconfig(%d) error for RT\n", nrt);
    goto stop;
  }
  if (fTryMT && tmkconfig(nmt))
  {
    printf("tmkconfig(%d) error for MT\n", nmt);
    goto stop;
  }
#endif //WINDOWSLINUX
#ifdef QNX4
  proxyMT = qnx_proxy_attach(0, 0, 0, -1);
  if (proxyMT == -1)
  {
    printf("qnx_proxy_attach error\n");
    goto stop;
  }
  if (TmkInit("tmk.cfg"))
  {
    printf("TmkInit error\n");
    goto stop;
  }
#endif //QNX4

  if (tmkselect(nbc))
  {
    printf("tmkselect(%d) error for BC\n", nbc);
    goto stop;
  }
#ifdef QNX4
  if (tmk_lock())
  {
   printf("tmk_lock error for BC\n");
   goto stop;
  }
  if (tmkdefmode(BC_MODE))
  {
   printf("tmkdefmode(BC_MODE) error\n");
   goto stop;
  }
#endif //QNX4
  if (bcreset())
  {
    printf("bcreset error\n");
    goto stop;
  }

  if (tmkselect(nrt))
  {
    printf("tmkselect(%d) error for RT\n", nrt);
    goto stop;
  }
#ifdef QNX4
  if (tmk_lock())
  {
   printf("tmk_lock error for RT\n");
   goto stop;
  }
  if (tmkdefmode(RT_MODE))
  {
   printf("tmkdefmode(RT_MODE) error\n");
   goto stop;
  }
#endif //QNX4
  if (rtreset())
  {
    printf("rtreset error\n");
    goto stop;
  }

  if (fTryMT)
  {
    if (tmkselect(nmt))
    {
      printf("tmkselect(%d) error for MT\n", nmt);
      goto stop;
    }
#ifdef QNX4
    if (tmk_lock())
    {
     printf("tmk_lock error for MT\n");
     goto stop;
    }
    if (tmkdefmode(RT_MODE))
    {
     printf("tmkdefmode(MT_MODE) error\n");
     goto stop;
    }
#endif //QNX4
    if (mtreset())
    {
      printf("mtreset error\n");
      goto stop;
    }
  }

  printf("randbcrt v1.03.");

  tmkselect(nbc);
#ifdef INTX
  printf("(bcstartx).\n");
#ifdef DOS
  bcdefintx(BCIntX);
#endif //DOS
#else
  printf("(bcstart).\n");
#ifdef DOS
  bcdefintnorm(BCInt);
  bcdefintexc(BCInt);
#endif //DOS
#endif
#ifdef WINDOWS
  tmkdefevent(hEvent, TRUE);
#endif //WINDOWS
#ifdef QNX4
  tmkdefproxy(proxyMT);
  tmkproxymode(TMK_PROXY_BUFFERED);
#endif //QNX4
  wBcMaxBase = bcgetmaxbase();
  printf("BC:%d RT:%d  Buses:", nbc, nrt);
  if (fBusA)
    printf("A");
  if (fBusB)
    printf("B");
  printf("  BC bases:%d\n", wBcMaxBase + 1);
  printf("RT busy flags: %c\n", (fFlag)?'+':'-');
  printf("Mode commands: %c\n", (fCmds)?'+':'-');
  printf("Broadcast commands: %c\n", (fBrcst)?'+':'-');
  printf("Instr. bit: %c\n", (fInstr)?'+':'-');
  printf("Max errors: %d\n", nMaxErr);
  tmkselect(nrt);
#ifdef DOS
  rtdefintcmd(RTInt);
  rtdefintdata(RTInt);
  rtdefinterr(RTIntErr);
#endif
#ifdef WINDOWS
  tmkdefevent(hEvent, TRUE);
#endif //WINDOWS
#ifdef QNX4
  tmkdefproxy(proxyMT);
  tmkproxymode(TMK_PROXY_BUFFERED);
#endif //QNX4
  rtdefaddress(RT_ADDR);
  rtdefirqmode(rtgetirqmode() & ~RT_DATA_BL);
  for (wSubAddr = 0x10; wSubAddr <= 0x1E; ++wSubAddr)
  {
    rtdefsubaddr(RT_RECEIVE, wSubAddr);
    rtclrflag();
    rtdefsubaddr(RT_TRANSMIT, wSubAddr);
    rtclrflag();
  }
  if (fFlag)
    rtdefmode(rtgetmode() | RT_FLAG_MODE);
  else
    rtdefmode(rtgetmode() &~ RT_FLAG_MODE);
  rtputcmddata(CMD_TRANSMIT_VECTOR_WORD, 0xA05F);
  if (fTryMT)
  {
    tmkselect(nmt);
    wMtMaxBase = mtgetmaxbase();
    for (wMtBase = 0; wMtBase < wMtMaxBase; ++wMtBase)
    {
      mtdefbase(wMtBase);
      mtdeflink((wMtBase + 1), CX_CONT | CX_NOINT | CX_SIG);
    }
    mtdefbase(wMtBase);
    mtdeflink(0, CX_CONT | CX_NOINT | CX_SIG);
#ifdef DOS
    mtdefintsig(MTIntSig);
#endif //DOS
#ifdef WINDOWS
    tmkdefevent(hEvent, TRUE);
#endif //WINDOWS
#ifdef QNX4
    tmkdefproxy(proxyMT);
    tmkproxymode(TMK_PROXY_BUFFERED);
#endif //QNX4
    mtstartx(0, CX_CONT | CX_NOINT | CX_NOSIG);
    fMtActive = 1;
    printf("MT:%d  MT bases:%d\n\n", nmt, wMtMaxBase + 1);
  }
  srand(1);
  nIntBC = 1;
  nIntRT = 1;
  nIntMT = 1;
  wBase = 0;
  wMtBase = 0;
  wCtrlCodeP = 0xFFFF;
  nMsg = 0L;
  while (!kbhit())
  {
    tmkselect(nbc);
    if (wBase > wBcMaxBase)
      wBase = 0;
    bcdefbase(wBase);
    wCtrlCode = rand() & 0xF;
    wLen = 0;
    switch (wCtrlCode)
    {
    case DATA_BC_RT:
      wSubAddr = (rand() & wSubAddrMask) + wSubAddr0;
      if (wSubAddr == 0x1F || wSubAddr == 0x00 || wSubAddr == wSubAddrP)
        continue;
      ++cDataBcRt;
      wLen = rand() & 0x1F;
      if (wLen == 0)
        wLen = 32;
      buf[0] = CW(RT_ADDR, RT_RECEIVE, wSubAddr, wLen);
      for (i = 0; i < wLen + 2; ++i) // sw + extra word
        buf[i+1] = rand() + rand();
      bcputblk(0, buf, wLen + 3);
      buf[wLen+1] = CW(RT_ADDR, 0, 0, 0); // status
      if (fMtActive)
      {
        tmkselect(nmt);
        mtdefbase(wMtBase);
        mtputw(wLen+2, buf[wLen+2]);
      }
      break;
    case DATA_BC_RT_BRCST:
      if (!fBrcst)
        continue;
      wSubAddr = (rand() & wSubAddrMask) + wSubAddr0;
      if (wSubAddr == 0x1F || wSubAddr == 0x00 || wSubAddr == wSubAddrP)
        continue;
      ++cDataBcRtBrcst;
      wLen = rand() & 0x1F;
      if (wLen == 0)
        wLen = 32;
      buf[0] = CW(31, RT_RECEIVE, wSubAddr, wLen);
      for (i = 0; i < wLen + 2; ++i) // + 2 extra words
        buf[i+1] = rand() + rand();
      bcputblk(0, buf, wLen + 3);
      if (fMtActive)
      {
        tmkselect(nmt);
        mtdefbase(wMtBase);
        mtputw(wLen+1, buf[wLen+1]);
        mtputw(wLen+2, buf[wLen+2]);
      }
      break;
    case DATA_RT_BC:
      wSubAddr = (rand() & wSubAddrMask) + wSubAddr0;
      if (wSubAddr == 0x1F || wSubAddr == 0x00 || wSubAddr == wSubAddrP)
        continue;
      ++cDataRtBc;
      wLen = rand() & 0x1F;
      if (wLen == 0)
        wLen = 32;
      buf[0] = CW(RT_ADDR, RT_TRANSMIT, wSubAddr, wLen);
      bcputw(0, buf[0]);
      buf[1] = CW(RT_ADDR,  0, 0, 0);
      buf[wLen+2] = rand() + rand(); // + extra word
      bcputw(wLen+2, buf[wLen+2]);
      tmkselect(nrt);
      for (i = 0; i < wLen; ++i)
        buf[i+2] = rand() + rand();
      rtdefsubaddr(RT_TRANSMIT, wSubAddr);
      rtputblk(0, buf + 2, wLen);
      rtsetflag();
      if (fMtActive)
      {
        tmkselect(nmt);
        mtdefbase(wMtBase);
        mtputw(wLen+2, buf[wLen+2]);
      }
      break;
    case DATA_RT_RT_BRCST:
      wSubAddr = (rand() & wSubAddrMask) + wSubAddr0;
      if (wSubAddr == 0x1F || wSubAddr == 0x00 || wSubAddr == wSubAddrP)
        continue;
      ++cDataRtRtBrcst;
      wLen = rand() & 0x1F;
      if (wLen == 0)
        wLen = 32;
      buf[0] = CW(31, RT_RECEIVE, wSubAddr, wLen);
      bcputw(0, buf[0]);
      buf[1] = CW(RT_ADDR, RT_TRANSMIT, wSubAddr, wLen);
      bcputw(1, buf[1]);
      buf[2] = CW(RT_ADDR, 0, 0, 0);
      buf[wLen+3] = rand() + rand(); // + 2 extra words
      bcputw(wLen+3, buf[wLen+3]);
      buf[wLen+4] = rand() + rand(); 
      bcputw(wLen+4, buf[wLen+4]);
      tmkselect(nrt);
      for (i = 0; i < wLen; ++i)
        buf[i+3] = rand() + rand();
      rtdefsubaddr(RT_TRANSMIT, wSubAddr);
      rtputblk(0, buf + 3, wLen);
      rtsetflag();
      if (fMtActive)
      {
        tmkselect(nmt);
        mtdefbase(wMtBase);
        mtputw(wLen+3, buf[wLen+3]);
        mtputw(wLen+4, buf[wLen+4]);
      }
      break;
    case CTRL_C_A:
      if (!fCmds)
        continue;
      ++cCtrlCA;
      buf[0] = CWM(RT_ADDR, CMD_SYNCHRONIZE);
      bcputw(0, buf[0]);
      buf[1] = rand() + rand(); // sw + extra word
      bcputw(1, buf[1]);
      buf[1] = CW(RT_ADDR, 0, 0, 0);
      buf[2] = rand() + rand();
      bcputw(2, buf[2]);
      if (fMtActive)
      {
        tmkselect(nmt);
        mtdefbase(wMtBase);
        mtputw(2, buf[2]);
      }
      break;
    case CTRL_C_BRCST:
      if (!fCmds)
        continue;
      if (!fBrcst)
        continue;
      ++cCtrlCBrcst;
      buf[0] = CWM(31, CMD_SYNCHRONIZE);
      bcputw(0, buf[0]);
      buf[1] = rand() + rand(); // + 2 extra words
      bcputw(1, buf[1]);
      buf[2] = rand() + rand();
      bcputw(2, buf[2]);
      if (fMtActive)
      {
        tmkselect(nmt);
        mtdefbase(wMtBase);
        mtputw(1, buf[1]);
        mtputw(2, buf[2]);
      }
      break;
    case CTRL_CD_A:
      if (!fCmds)
        continue;
      if (wCtrlCodeP == CTRL_CD_A || wCtrlCodeP == CTRL_CD_BRCST)
        continue;
      ++cCtrlCDA;
      buf[0] = CWM(RT_ADDR, CMD_SYNCHRONIZE_WITH_DATA_WORD);
      bcputw(0, buf[0]);
      buf[1] = rand() + rand();
      bcputw(1, buf[1]);
      buf[2] = rand() + rand(); // sw + extra word
      bcputw(2, buf[2]);
      buf[2] = CW(RT_ADDR, 0, 0, 0);
      buf[3] = rand() + rand();
      bcputw(3, buf[3]);
      if (fMtActive)
      {
        tmkselect(nmt);
        mtdefbase(wMtBase);
        mtputw(3, buf[3]);
      }
      break;
    case CTRL_CD_BRCST:
      if (!fCmds)
        continue;
      if (!fBrcst)
        continue;
      if (wCtrlCodeP == CTRL_CD_A || wCtrlCodeP == CTRL_CD_BRCST)
        continue;
      ++cCtrlCDBrcst;
      buf[0] = CWM(31, CMD_SYNCHRONIZE_WITH_DATA_WORD);
      bcputw(0, buf[0]);
      buf[1] = rand() + rand();
      bcputw(1, buf[1]);
      buf[2] = rand() + rand(); // + 2 extra words
      bcputw(2, buf[2]);
      buf[3] = rand() + rand();
      bcputw(3, buf[3]);
      if (fMtActive)
      {
        tmkselect(nmt);
        mtdefbase(wMtBase);
        mtputw(2, buf[2]);
        mtputw(3, buf[3]);
      }
      break;
    case CTRL_C_AD:
      if (!fCmds)
        continue;
      if (wCtrlCodeP == CTRL_C_AD)
        continue;
      ++cCtrlCAD;
      buf[0] = CWM(RT_ADDR, CMD_TRANSMIT_VECTOR_WORD);
      bcputw(0, buf[0]);
      buf[1] = CW(RT_ADDR, 0, 0, 0);
      buf[2] = rand() + rand();
      buf[3] = rand() + rand(); // + extra word
      bcputw(3, buf[3]);
      tmkselect(nrt);
      rtputcmddata(CMD_TRANSMIT_VECTOR_WORD, buf[2]);
      if (fMtActive)
      {
        tmkselect(nmt);
        mtdefbase(wMtBase);
        mtputw(3, buf[3]);
      }
      break;
    default:
      continue;
    }

    wChecked = 0;
    wCheckMode = rand() & 3;
    nCheckDir = rand() & 1;
    if (nCheckDir == 0)
      nCheckDir = -1;

    fStop = 0;
    fInt = 0;
//    tmkselect(nbc);
#ifdef DOS
#ifdef WAITWITHKBHIT
    dcWait = 0L;
#endif
    while (!nIntBC)
#ifdef WAITWITHKBHIT
    {
      if (dcWait == 100000L)
      {
        if (kbhit())
//         && getch() == 27)
          break;
        else
          dcWait = 0L;
      }
      else
        ++dcWait;
    }
#else
    ;
#endif
#endif //DOS
#if defined(WINDOWS) || defined(LINUX) || defined(QNX4)
    while (nIntBC == 0 || nIntRT == 0 || (fMtActive && nIntMT == 0))
    {
#ifdef WAITWITHKBHIT
      if (kbhit() && getch() == 27)
      {
        fStop = 1;
        break;
      }
#endif
      fInt = 0;
#ifdef QNX4
      proxy = Creceive(0, &msg, 0);
//      if (proxy != (pid_t)(-1))
      if (proxy == proxyMT)
        fInt = 1;
/*
      else
      {
        if (kbhit())
        {
          getch();
          fStop = 1;
        }
      }
*/
#endif //QNX4
#ifdef LINUX
      events = tmkwaitevents((1<<nbc)|(1<<nrt)|(fMtActive<<nmt), 1000);
      if (events == 0)
      {
        printf("\ninterrupt error! %d %d %d\n", nIntBC, nIntRT, nIntMT);
        fStop = 1;
      }
      else if (events < 0)
      {
        printf("\ninterrupt cancel!\n");
        fStop = 1;
      }
      else
        fInt = 1;
#endif //LINUX
#ifdef WINDOWS
      switch (WaitForSingleObject(hEvent, 1000))
      {
      case WAIT_OBJECT_0:
        ResetEvent(hEvent);
        fInt = 1;
        break;
      case WAIT_TIMEOUT:
        printf("\ninterrupt error! %d %d %d\n", nIntBC, nIntRT, nIntMT);
        fStop = 1;
        break;
      default:
        printf("\ninterrupt cancel!\n");
        fStop = 1;
        break;
      }
#endif //WINDOWS
      if (fStop)
        break;
      if (!fInt)
        continue;

      if (fMtActive && nIntMT == 0)
      {
        tmkselect(nmt);
//      do {
          tmkgetevd(&tmkEvD);
          switch (nIntMT = tmkEvD.nInt)
          {
          case 4:
            MTIntSig(0);
            break;
          }
//      } while (tmkEvD.nInt != 0);
      }

      if (nIntRT == 0)
      {
        tmkselect(nrt);
//      do {
          tmkgetevd(&tmkEvD);
          switch (nIntRT = tmkEvD.nInt)
          {
          case 1:
          case 3:
            RTInt(0);
            break;
          case 2:
            RTIntErr(0);
            break;
          }
//      } while (tmkEvD.nInt != 0);
      }

      if (nIntBC == 0)
      {
        tmkselect(nbc);
//      do {
          tmkgetevd(&tmkEvD);
          switch (nIntBC = tmkEvD.nInt)
          {
#ifdef INTX
          case 3:
            BCIntX(tmkEvD.bcx.wBase, tmkEvD.bcx.wResultX);
            break;
#else
          case 1:
          case 2:
            BCInt(tmkEvD.bc.wResult, 0, 0);
            break;
#endif
          }
//      } while (tmkEvD.nInt != 0);
      }
    }
#endif //WINDOWSLINUX
    if (fStop)
      break;

    tmkselect(nbc);
    nIntBC = 0;
    nIntRT = 0;
    nIntMT = 0;
    wResP = wRes;
    fBusP = fBus;
    fBus = rand() & MY_BUS_B;
    if (!fBusA)
      fBus = MY_BUS_B;
    if (!fBusB)
      fBus = MY_BUS_A;
    if (nMsg >= nMsgBeg && (nMsgEnd < 0 || nMsg <= nMsgEnd))
    {
#ifdef INTX
      bcstartx(wBase, wCtrlCode | fBus | CX_STOP | CX_NOSIG);
#else
      bcdefbus(fBus);
      bcstart(wBase, wCtrlCode);
#endif
    }
    else
    {
      wCtrlCodeP = 0xFFFF;
      nIntBC = 1;
    }

    switch (wCtrlCodeP)
    {
    case DATA_BC_RT:
    case DATA_BC_RT_BRCST:
      while (wChecked != 7)
      {
        fError = 0;
        switch (wCheckMode)
        {
        case 0:
          tmkselect(nbc);
          bcdefbase(wBaseP);
          bcgetblk(0, buf1, wLenP + 3);
          for (i = 0; i < wLenP + 3; ++i)
            if (bufP[i] != buf1[i])
              IncDataErr();
          if (fError)
          {
            bcgetblk(0, buf1, wLenP + 3);
            for (i = 0; i < wLenP + 3; ++i)
              if (bufP[i] != buf1[i])
                IncDataErr();
          }
          wChecked |= 1;
          break;
        case 1:
          tmkselect(nrt);
          rtdefsubaddr(RT_RECEIVE, wSubAddrP);
          rtgetblk(0, buf1, wLenP);
          rtclrflag();
          for (i = 0; i < wLenP; ++i)
            if (bufP[i+1] != buf1[i])
              IncDataErr();
          if (fError)
          {
            rtgetblk(0, buf1, wLenP);
            rtclrflag();
            for (i = 0; i < wLenP; ++i)
              if (bufP[i+1] != buf1[i])
                IncDataErr();
          }
          wChecked |= 2;
          break;
        case 2:
          if (fMtActive)
          {
            tmkselect(nmt);
            mtdefbase(wMtBaseP);
            wResMT = mtgetsw();
            mtgetblk(0, buf1, wLenP + 3);
            for (i = 0; i < wLenP + 3; ++i)
              if (bufP[i] != buf1[i])
                IncDataErr();
            if (fError)
            {
              mtgetblk(0, buf1, wLenP + 3);
              for (i = 0; i < wLenP + 3; ++i)
                if (bufP[i] != buf1[i])
                  IncDataErr();
            }
          }
          wChecked |= 4;
          break;
        }
        wCheckMode = (wCheckMode + nCheckDir) & 3;
      }
      break;
    case DATA_RT_BC:
      while (wChecked != 7)
      {
        fError = 0;
        switch (wCheckMode)
        {
        case 0:
          tmkselect(nbc);
          bcdefbase(wBaseP);
          bcgetblk(0, buf1, wLenP + 3);
          for (i = 0; i < wLenP + 3; ++i)
            if (bufP[i] != buf1[i])
              IncDataErr();
          if (fError)
          {
            bcgetblk(0, buf1, wLenP + 3);
            for (i = 0; i < wLenP + 3; ++i)
              if (bufP[i] != buf1[i])
                IncDataErr();
          }
          wChecked |= 1;
          break;
        case 1:
          tmkselect(nrt);
          rtdefsubaddr(RT_TRANSMIT, wSubAddrP);
          rtgetblk(0, buf1, wLenP);
          rtclrflag();
          for (i = 0; i < wLenP; ++i)
            if (bufP[i+2] != buf1[i])
              IncDataErr();
          if (fError)
          {
            rtgetblk(0, buf1, wLenP);
            rtclrflag();
            for (i = 0; i < wLenP; ++i)
              if (bufP[i+2] != buf1[i])
                IncDataErr();
          }
          wChecked |= 2;
          break;
        case 2:
          if (fMtActive)
          {
            tmkselect(nmt);
            mtdefbase(wMtBaseP);
            wResMT = mtgetsw();
            mtgetblk(0, buf1, wLenP + 3);
            for (i = 0; i < wLenP + 3; ++i)
              if (bufP[i] != buf1[i])
                IncDataErr();
            if (fError)
            {
              mtgetblk(0, buf1, wLenP + 3);
              for (i = 0; i < wLenP + 3; ++i)
                if (bufP[i] != buf1[i])
                  IncDataErr();
            }
          }
          wChecked |= 4;
          break;
        }
        wCheckMode = (wCheckMode + nCheckDir) & 3;
      }
      break;
    case DATA_RT_RT_BRCST:
      while (wChecked != 7)
      {
        fError = 0;
        switch (wCheckMode)
        {
        case 0:
          tmkselect(nbc);
          bcdefbase(wBaseP);
          bcgetblk(0, buf1, wLenP + 5);
          for (i = 0; i < wLenP + 5; ++i)
            if (bufP[i] != buf1[i])
              IncDataErr();
          if (fError)
          {
            bcgetblk(0, buf1, wLenP + 5);
            for (i = 0; i < wLenP + 5; ++i)
              if (bufP[i] != buf1[i])
                IncDataErr();
          }
          wChecked |= 1;
          break;
        case 1:
          tmkselect(nrt);
          rtdefsubaddr(RT_TRANSMIT, wSubAddrP);
          rtgetblk(0, buf1, wLenP);
          rtclrflag();
          for (i = 0; i < wLenP; ++i)
            if (bufP[i+3] != buf1[i])
              IncDataErr();
          if (fError)
          {
            rtgetblk(0, buf1, wLenP);
            rtclrflag();
            for (i = 0; i < wLenP; ++i)
              if (bufP[i+3] != buf1[i])
                IncDataErr();
          }
          wChecked |= 2;
          break;
        case 2:
          if (fMtActive)
          {
            tmkselect(nmt);
            mtdefbase(wMtBaseP);
            wResMT = mtgetsw();
            mtgetblk(0, buf1, wLenP + 5);
            for (i = 0; i < wLenP + 5; ++i)
              if (bufP[i] != buf1[i])
                IncDataErr();
            if (fError)
            {
              mtgetblk(0, buf1, wLenP + 5);
              for (i = 0; i < wLenP + 5; ++i)
                if (bufP[i] != buf1[i])
                  IncDataErr();
            }
          }
          wChecked |= 4;
          break;
        }
        wCheckMode = (wCheckMode + nCheckDir) & 3;
      }
      break;
    case CTRL_C_A:
    case CTRL_C_BRCST:
      while (wChecked != 7)
      {
        fError = 0;
        switch (wCheckMode)
        {
        case 0:
          tmkselect(nbc);
          bcdefbase(wBaseP);
          bcgetblk(0, buf1, 3);
          for (i = 0; i < 3; ++i)
            if (bufP[i] != buf1[i])
              IncDataErr();
          if (fError)
          {
            bcgetblk(0, buf1, 3);
            for (i = 0; i < 3; ++i)
              if (bufP[i] != buf1[i])
                IncDataErr();
          }
          wChecked |= 1;
          break;
        case 1:
          wChecked |= 2;
          break;
        case 2:
          if (fMtActive)
          {
            tmkselect(nmt);
            mtdefbase(wMtBaseP);
            wResMT = mtgetsw();
            mtgetblk(0, buf1, 3);
            for (i = 0; i < 3; ++i)
              if (bufP[i] != buf1[i])
                IncDataErr();
            if (fError)
            {
              mtgetblk(0, buf1, 3);
              for (i = 0; i < 3; ++i)
                if (bufP[i] != buf1[i])
                  IncDataErr();
            }
          }
          wChecked |= 4;
          break;
        }
        wCheckMode = (wCheckMode + nCheckDir) & 3;
      }
      break;
    case CTRL_CD_A:
    case CTRL_CD_BRCST:
      while (wChecked != 7)
      {
        fError = 0;
        switch (wCheckMode)
        {
        case 0:
          tmkselect(nbc);
          bcdefbase(wBaseP);
          bcgetblk(0, buf1, 4);
          for (i = 0; i < 4; ++i)
            if (bufP[i] != buf1[i])
              IncDataErr();
          if (fError)
          {
            bcgetblk(0, buf1, 4);
            for (i = 0; i < 4; ++i)
              if (bufP[i] != buf1[i])
                IncDataErr();
          }
          wChecked |= 1;
          break;
        case 1:
          tmkselect(nrt);
          buf1[0] = rtgetcmddata(CMD_SYNCHRONIZE_WITH_DATA_WORD);
          i = 0;
          if (bufP[1] != buf1[0])
            IncDataErr();
          if (fError)
          {
            buf1[0] = rtgetcmddata(CMD_SYNCHRONIZE_WITH_DATA_WORD);
            i = 0;
            if (bufP[1] != buf1[0])
              IncDataErr();
          }
          wChecked |= 2;
          break;
        case 2:
          if (fMtActive)
          {
            tmkselect(nmt);
            mtdefbase(wMtBaseP);
            wResMT = mtgetsw();
            mtgetblk(0, buf1, 4);
            for (i = 0; i < 4; ++i)
              if (bufP[i] != buf1[i])
                IncDataErr();
            if (fError)
            {
              mtgetblk(0, buf1, 4);
              for (i = 0; i < 4; ++i)
                if (bufP[i] != buf1[i])
                  IncDataErr();
            }
          }
          wChecked |= 4;
          break;
        }
        wCheckMode = (wCheckMode + nCheckDir) & 3;
      }
      break;
    case CTRL_C_AD:
      while (wChecked != 7)
      {
        fError = 0;
        switch (wCheckMode)
        {
        case 0:
          tmkselect(nbc);
          bcdefbase(wBaseP);
          bcgetblk(0, buf1, 4);
          for (i = 0; i < 4; ++i)
            if (bufP[i] != buf1[i])
              IncDataErr();
          if (fError)
          {
            bcgetblk(0, buf1, 4);
            for (i = 0; i < 4; ++i)
              if (bufP[i] != buf1[i])
                IncDataErr();
          }
          wChecked |= 1;
          break;
        case 1:
          wChecked |= 2;
          break;
        case 2:
          if (fMtActive)
          {
            tmkselect(nmt);
            mtdefbase(wMtBaseP);
            wResMT = mtgetsw();
            mtgetblk(0, buf1, 4);
            for (i = 0; i < 4; ++i)
              if (bufP[i] != buf1[i])
                IncDataErr();
            if (fError)
            {
              mtgetblk(0, buf1, 4);
              for (i = 0; i < 4; ++i)
                if (bufP[i] != buf1[i])
                  IncDataErr();
            }
          }
          wChecked |= 4;
          break;
        }
        wCheckMode = (wCheckMode + nCheckDir) & 3;
      }
      break;
    default:
      break;
    }

    if (fMtActive)
    {
      if (wCtrlCodeP != 0xFFFF)
      {
        if ((wResMT & 0xFC3F) != ((fBusP<<BUS_SHIFT) | (wCtrlCodeP<<10)))
        {
          fprintf(fileOut, "Error wResMT=%04X; %d; ", wResMT, wMtBaseP);
          wCheckMode = 2;
          i = 0;
          IncDataErr();
        }
      }
      wMtBaseP = wMtBase;
      ++wMtBase;
      if (wMtBase > wMtMaxBase)
        wMtBase = 0;
    }

    wCtrlCodeP = wCtrlCode;
    wBaseP = wBase;
    wSubAddrP = wSubAddr;
    wLenP = wLen;
    for (i = 0; i < 37; ++i)
      bufP[i] = buf[i];
    
    if (nMsgEnd >= 0L && nMsg > nMsgEnd)
      break;
    ++nMsg;
    ++wBase;

    if (cInts%1000 == 0)
    {
      gotoxy(1, wherey());
      cprintf("Ints BC: %ld (-%ld), RT: %ld (-%ld). Errors: %ld, (%ld, %ld)", cInts, cBadInts, cRtInts, cRTErrInts, cDataErrA + cDataErrB, cDataErrA, cDataErrB);
      if (fMtActive)
      {
        gotoxy(1, wherey()-1);
        cprintf("Ints MT: %ld\n\r", cMtInts);
      }
    }
    if (nMaxErr && (int)(cDataErrA+cDataErrB+cBadInts+cRTErrInts) > nMaxErr)
      break;
  }
  while (kbhit())
    getch();
  if (fMtActive)
  {
    cprintf("\n\rInts MT: %ld", cMtInts);
  }
  cprintf("\n\rInts BC: %ld (-%ld), RT: %ld (-%ld). Errors: %ld, (%ld, %ld)\n", cInts, cBadInts, cRtInts, cRTErrInts, cDataErrA + cDataErrB, cDataErrA, cDataErrB);
  if (cBadInts == 0L && cRTErrInts == 0L && cDataErrA == 0L && cDataErrB == 0L)
    res = 0;
  stop:
  tmkselect(nbc);
  bcreset();
  tmkselect(nrt);
  bcreset();
  if (fMtActive)
  {
    tmkselect(nmt);
    bcreset();
  }
  tmkdone(ALL_TMKS);
#if defined(WINDOWS) || defined(LINUX)
  TmkClose();
#endif //WINDOWSLINUX
#ifdef WINDOWS
  if (hEvent)
    CloseHandle(hEvent);
#endif //WINDOWS
#ifdef QNX4
  if (proxyMT != (pid_t)(-1))
    qnx_proxy_detach(proxyMT);
  console_close(cc);
#endif //QNX4
  stopn:
  if (fileOut != NULL)
    fclose(fileOut);
  return res;
}
