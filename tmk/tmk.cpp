/****************************************************************************/
/*      TMK.CPP. (c) ELCUS, 1994,2002.                                      */
/*      Uses TMKLL4 library.                                                */
/****************************************************************************/

#ifdef ELCUS
//#define LPT_AT
#define LPT_SYN
#define LPT_INT_EXC
#endif //def ELCUS

#ifdef LPT_AT
#define LPT
#endif
#ifdef LPT_SYN
#define LPT
#endif
#ifdef LPT_INT_EXC
#define LPT
#endif

#define TMK_CONFIGURATION_TABLE

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#ifdef _TMK1553B_DOS
#include <conio.h>
#include <bios.h>
#include <dos.h>
#include <process.h>
#include <io.h>
#endif
#ifdef _TMK1553B_LINUX
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#endif

#include "tmkinit.c"
#ifdef _TMK1553B_DOS
#include "tmktest.c"
#endif

#ifdef _TMK1553B_LINUX
#define inportb(port) inb(port)
#define outportb(port, data) outb(port, data)
#define inport(port) inw(port)
#define outport(port, data) outw(port, data)
#define _INT16 short
#define _UINT16 unsigned short
#define getch() ESC_peek
#endif
#ifdef _TMK1553B_DOS
#define _INT16 int
#define _UINT16 unsigned
#endif

#ifdef _TMK1553B_LINUX
#define WAITDLY 8
#endif
#ifdef _TMK1553B_DOS
#define WAITDLY 2
#endif

#define TMKCPP_MAX_BASE 1023
#define TMKCPP_MAX_NUM 3

#define CR 0x0D                 /* возврат каретки */
#define LF 0x0A
#define CTRL_C 0x03
#define ESCAPE 0x1B
#define BACKSPACE 0x08
#define TAB 0x09
/* расширенные коды ASCII */
#define F01 0x3B
#define F02 0x3C
#define F03 0x3D
#define F04 0x3E
#define F05 0x3F
#define F06 0x40
#define F07 0x41
#define F08 0x42
#define F09 0x43
#define F10 0x44
#define PGUP 0x49
#define PGDN 0x51
#define HOME 0x47
#define END 0x4F
#define LEFT 0x4B
#define RIGHT 0x4D
#define UP 0x48
#define DOWN 0x50
#define GPLUSW 0x4E2B
#define GPLUS 0x4E

#define BC_BUF_SIZE 64                  /* размер буфера */
#define BC_DUMP_SIZE 36                 /* количество выводимых в дампе слов */
#define CC_SIZE 4                       /* количество битов в коде управления */
#define TIME_OUT 0x0300                 /* время ожидания прерывания от ТМК */

#define DUMP_WIDE 10                    /* количество слов в строке */

#define RT_BUF_SIZE 32                  /* размер буфера */
#define RT_DUMP_SIZE 32                 /* количество выводимых в дампе слов */
#define SW_SIZE 5                       /* количество битов в ответном слове */
#define ZONE_MASK 0x0020                /* маска для работы с битом приема/передачи */
#define RT_MASK 0x0080                  /* маска установки режима ОУ */

/* общие функции */
#ifdef LPT
_UINT16 lpt_data_port, lpt_status_port, lpt_ctrl_port;
_INT16 lpt_num = 1;                         /* номер используемого LPT */
void init_lpt();                         /* определение адресов портов */
void set_lpt_data(unsigned char data);   /* вывод данных в порт LPT */
#endif
_INT16 nTest;
void TmkInit0();
void in_cmd(char *str);                  /* прием командной строки */
_INT16 in_key();                            /* ввод символа с проверкой прерывания */
void check_int();                        /* обнаружение прерывания и оповещение */
void make_func(_UINT16 key);            /* обработка функциональных клавиш */
void make_ctrl(_INT16 key);                 /* обработка управляющих кодов */
void make_cmd(char *str);                /* обработка командной строки */
void make_quit(const char *str);               /* поддержка выхода из отладчика */
void select_win(char *str);              /* выбор окна */
void select_tmk(char *str);              /* выбор платы для окна */
void mode_bc(const char *str);                 /* установка режима КК */
void mode_rt(const char *str);                 /* установка режима ОУ */
//void mode_mt(char *str);                 /* установка режима МТ */
void comment(const char *str);                 /* вывод коментария */
void kbd_pause(const char *str);               /* программируемая пауза */
void fields_chg(const char *str);              /* управление выделением полей */
#ifdef _TMK1553B_DOS
void interrupt prtscr_brk(...);          /* обработчик прерывания PrtScr */
#endif
void cmds_open(char *str);               /* открытие командного файла */
void cmds_open2(char *str);              /* выполнение командного файла  в цикле */
void exe_open(char *str);                /* запуск другой программы */
void cmd_reset(const char *str);               /* сброс контроллера */
void buf_fill(char *str);                /* заполнение буфера константой */
void buf_clear(const char *str);               /* очистка буфера */
void buf_edit(const char *str);                /* редактирование буферной памяти */
_INT16 mark_set(const char *str, _INT16 *mark);      /* ввод текущего слова для редактирования */
_INT16 end_ctrl(const char *str);                 /* контроль конца строки */
void msg_out(const char *msg);                 /* вывод строки сообщения */
void all_screen();                       /* вывод всего экрана */
void out_func_str();                     /* назначение функциональных клавиш */
void out_mode();                         /* вывод режима работы */
void tmk_outpw(char *str);
#ifdef TMK_DAC
void tmk_outdac(char *str);
#endif
void set_io_delay(char *str);
inline void out_port(_UINT16 wPort);    /* вывод порта платы */
inline void out_irq(_UINT16 wIrq);      /* вывод номера прерывания платы */
void out_type();                         /* вывод типа платы */
void out_fname();                        /* вывод имени последнего файла данных */
void out_inn();                          /* вывод номера прерывания */
void out_buffer();                       /* вывод дампа буфера */
void bin_unpack(_INT16 x, _INT16 y, _INT16 adr, _INT16 data);/* двоичная развертка */
void out_dump(_UINT16 *data);           /* вывод дампа */
void out_bin(_INT16 data, _INT16 num_digits);  /* вывод в двоичной форме */
void select_window(_INT16 nwin);
void set_max_err(char *str);
void set_stat_len(char *str);            /* задание циклов накопления статистики
                                            в автоматических тестах */

/* функции для КК */
void bc_fill_test();
void bc_help(const char *str);                 /* вывод подсказки */
void bc_clear_ram();                     /* обнуление БЗУ КК */
_INT16 bc_ram_test(const char *str);              /* тестирование БЗУ КК */
void bc_buf_rd(char *str);               /* чтение в буфер из файла */
void bc_buf_wr(char *str);               /* запись буфера в файл */
void sym(char *str);                     /* симметрирование */
void statistic_out(const char *str);           /* вывод статистики канала */
void statistic_err_out();                /* вывод статистики ошибок */
void statistic_bad_out(const char *str);       /* вывод статистики сбоев канала */
void statistic_clear(const char *str);         /* очистка переменных статистики */
_INT16 start_loop(const char *str);               /* запуск в цикле */
_INT16 start_loop_file(const char *str);          /* запуск в цикле */
_INT16 start_loop_n(unsigned long counter); /* запуск в цикле */
_INT16 start_1(const char *str);                  /* однократный запуск */
_INT16 start();                             /* запуск */
void set_base(char *str);                /* установка базы */
void set_count(char *str);               /* установка циклов повторения команды */
void set_pause(char *str);               /* установка паузы */
void set_ctrl_code(char *str);           /* команда установки кода управления */
void set_number(char *str);              /* команда установки номера канала */
void out_maxerrs();
void out_number();                       /* вывод номера канала */
void out_base();                         /* вывод базы */
void out_count();                        /* вывод значения счетчика повторений */
void out_glcount();                      /* вывод значения счетчика повторений */
void out_pause();                        /* вывод значения паузы */
void out_ctrl_code();                    /* вывод кода управления */
void bc_out_sw();                        /* вывод регистра слова состояния */
void bc_out_ram(_INT16 base);               /* вывод дампа страницы БЗУ */
_INT16 correct_cc(_INT16 ctrl_code);           /* проверка корректности упр. кода */

/* функции для ОУ */
void rt_fill_test();
void block_data_irq(const char *str);          /* блокировка прерывания данных */
void unblock_data_irq(const char *str);        /* разблокировка прерывания данных */
void rt_clear_ram();                     /* обнуление БЗУ ОУ */
_INT16 rt_ram_test(const char *str);              /* тестирование БЗУ ОУ */
void rt_help(const char *str);                 /* вывод подсказки */
void rt_buf_rd(char *str);               /* чтение в буфер из файла */
void rt_buf_wr(char *str);               /* запись буфера в файл */
void set_flag(char *str);                /* установка флагов */
void rd_sw_loop(char *str);              /* чтение слова состояния в цикле */
void wr_mr_loop(char *str);              /* циклическая запись в РРЖ */
_INT16 set_subadr(char *str);               /* ввод подадреса */
void set_spage(char *str);               /* установка текущей страницы БЗУ */
void set_cpage(char *str);               /* установка текущей страницы МК */
void set_page(const char *str);                /* установка текущей страницы */
void set_at(char *str);                  /* установка адреса терминала */
_INT16 wr_at(_INT16 at);                       /* запись адреса терминала в ОУ */
void set_subadr_r(char *str);            /* установка подадареса, зона приема */
void set_subadr_t(char *str);            /* установка подадареса, зона приема */
void set_status_word(const char *str);         /* команда установки ответного слова (ОС) */
void set_sw_bits(char *str, _INT16 mask);    /* функции установки отдельных битов ОС */
void read_sw(const char *str);                 /* чтение слова состояния */
void read_sp(char *str);                 /* чтение слова состояния */
void clear_sw_bits(char *str, _INT16 mask);  /* функции очистки отдельных битов ОС */
void wr_status_word();                   /* запись битов ОС */
void out_rt_mode();
void out_page();                         /* вывод номера страницы МК */
void out_subadr();                       /* вывод базы */
void out_at();                           /* вывод адреса терминала */
void out_status_word();                  /* вывод битов ОС */
void rt_out_sw();                        /* вывод слова состояния */
void rt_out_sp();                        /* вывод слова состояния */
void rt_out_ram(_INT16 dir, _INT16 subadr);     /* вывод дампа подадреса БЗУ */

const _INT16 BC_DUMP_STRINGS = BC_DUMP_SIZE / DUMP_WIDE + 1;
const _INT16 RT_DUMP_STRINGS = RT_DUMP_SIZE / DUMP_WIDE + 1;
const _INT16 BDUMP_X = 10, BDUMP_Y = 6;
_INT16 DUMP_STRINGS;
_INT16 DUMP_SIZE;

_INT16 fGoto = 0;
char szGoto[16];
char cmd_str[80];               /* буфер введенной команды */
char old_cmd[80];               /* буфер старой командной строки */
char ex_fname[80];              /* буфер циклически выполняемого ком. файла */
long cmd_count = 1;             /* счетчик повторения команды */
long cur_count;                 /* текущий счетчик цикла повторения */
_INT16 str_ptr = 0;                /* указатель в буфере ввода с консоли */
_INT16 fCmds_f = 0;

volatile _UINT16 bc_sw = 0xFFFF; /* слово состояния обмена */
volatile _UINT16 bc_aw1 = 0xFFFF; /* первое ответное слово, если bc_sw != 0 */
volatile _UINT16 bc_aw2 = 0xFFFF; /* второе ответное слово, если bc_sw != 0 */
_INT16 base;                       /* база в БЗУ */
_INT16 bus_num;                    /* номер канала */
_UINT16 tmkMaxN = TMKCPP_MAX_NUM;
_UINT16 tmkMaxBase[TMKCPP_MAX_NUM + 1];
unsigned char ctrl_code[TMKCPP_MAX_NUM + 1][TMKCPP_MAX_BASE + 1];
/* код управления для базы */
_UINT16 base_link[TMKCPP_MAX_NUM + 1][TMKCPP_MAX_BASE + 1];
/* link на базу в цепочке */
_UINT16 opause = 0;            /* определяет время между стартами в мкс */
_UINT16 _pause_ = 0;               /* определяет время между стартами в периодах
                                   таймера */

volatile _INT16 prtscr_flag = 0;   /* флаг нажатия PrtScr */
volatile _INT16 int_num[4] = {0, 0, 0, 0};/* номер прерывания */
/* переменные статистики: */
#define BADLEN 100
#define ERRLEN 40
_UINT16 at_bad_mode[ERRLEN];
_UINT16 at_bad_base[ERRLEN];
_UINT16 at_bad_sa[ERRLEN];
_UINT16 at_bad_num[ERRLEN];
_UINT16 at_bad_good[ERRLEN];
_UINT16 at_bad_bad[ERRLEN];
volatile _UINT16 bad_sws[BADLEN];
volatile _UINT16 bad_aws[BADLEN];
volatile unsigned long bad_sts[BADLEN];
volatile unsigned long good_starts = 0; /* количество нормальных */
volatile unsigned long bad_starts = 0; /* и аварийных завершений обмена в МК, */
unsigned long to_errors = 0;         /* стартов не завершившихся прерыванием */
unsigned long data_err = 0;
unsigned long channel_err = 0;
unsigned long dwMaxErr = 0;
_INT16 fStatErrStop = 0;

_UINT16 bcbuffer[2][64];
_UINT16 rtbuffer[2][32];
_UINT16 buffer[64];            /* буфер ПЭВМ */
_UINT16 vbuffer[64];           /* дополнительный буфер */
_INT16 buf_adr = 0;                /* база в буфере */
_INT16 buf_mark = 0;               /* номер слова в буфере выводимого в BIN */
_INT16 in_ed_word = 3;             /* определяет текущую тетраду при редактировании */
_INT16 buf_size;                   /* текущий размер буфера */
_INT16 fields = 1;                   /* режим вывода полей управляющего слова
                                   1 - поля выводятся, 0 - нет */

volatile _UINT16 rt_sp = 0;
volatile _UINT16 rt_sw = 0xFFFF; /* слово состояния */
volatile _UINT16 rt_cmd = 0;  /* команда ОУ */
_INT16 fLock = 0;
_INT16 dir;                       /* направление передачи подадреса */
_INT16 subadr;                    /* подадрес в БЗУ */
_INT16 terminal_adr;              /* адрес ОУ в канале */
_INT16 bram_page;                 /* текущая страница БЗУ со стороны ПЭВМ */
_INT16 cram_page;                 /* текущая страница БЗУ со стороны канала */

_INT16 nwin;                      /* текущее выводимое окно */
_INT16 nmainwin = 0;              /* текущее рабочее окно */
const _INT16 nMaxWin = 1;
_INT16 fInt = 0;
char chLU, chRU, chLD, chRD, chH, chV;

struct TW {
	_INT16 nX;
	_INT16 nY;
	_INT16 nDX;
	_INT16 nDY;
	_INT16 nTMK;
	_INT16 nType;
	_INT16 nMode;
	_INT16 fVisible;
	_INT16 at_avail;
	char data_fname[13];
};

TW Window[nMaxWin + 1] = { { 1, 1, 80, 11, 0, -1, (_INT16)UNDEFINED_MODE, 0, 0, "" },
	{ 1, 12, 80, 11, 0, -1, (_INT16)UNDEFINED_MODE, 0, 0, "" }
};

#ifdef _TMK1553B_DOS
void interrupt(*oldvect3)(...); /* старое значение вектора PrtScr */
#endif

FILE *in_file;                  /* файл ввода данных в буфер */
FILE *out_file;                 /* файл вывода данных из буфер */
FILE *cmd_file;                 /* файл ввода команд */

char szIDataFName[80] = "rtbc.dat";
char szODataFName[80] = "bcrt.dat";

char buf_str[100];              /* буфер для работы с файлом ввода */
_INT16 fSkipCmds = 0;              /* флаг прер. выполнения командного файла */
_INT16 enter_mode = 0;               /* режим ввода команд: 0 - с консоли,
                                                       1 - из файла */
/* таблица корректности упр. кодов (1) */
char ccs[16] = {1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0};
/* указатели на сообщения об ошибках */
#ifdef ENG
const char *inp_err = "Input error";
const char *base_err = "Wrong base number";
const char *bus_num_err = "Wrong bus number";
const char *bad_ctrl_code = "Wrong control code";
char cc_no_def[] = "Control code XXXX not defined";
const char *mark_err = "Wrong word address";
const char *undef_cmd = "Command doesn't exist";
const char *const_err = "Wrong constant";
const char *cr_err = "File creation error";
const char *open_err = "File open error";
const char *format_err = "File format error";
const char *bc_reset_ok = "Bus Controller reset OK";
const char *rt_reset_ok = "Remote Terminal reset OK";
const char *subadr_err = "Wrong subaddress";
const char *bad_status_word = "Error in status word";
const char *no_int = "Time-out exit";
const char *int_ok = "Interrupt";
const char *kbd_break = "Aborted by user";
#else
char *inp_err = "Ошибка ввода";
char *base_err = "Неверное значение базы";
char *bus_num_err = "Неправильный номер канала";
char *bad_ctrl_code = "Ошибка в коде управления";
char cc_no_def[] = "Код управления XXXX не определен";
char *mark_err = "Некорректный адрес слова";
char *undef_cmd = "Введена несуществующая команда";
char *const_err = "Неверное значение константы";
char *cr_err = "Ошибка создания файла";
char *open_err = "Ошибка открытия файла";
char *format_err = "Ошибка формата входного файла";
char *bc_reset_ok = "Сброс контроллера канала";
char *rt_reset_ok = "Сброс оконечного устройства";
char *subadr_err = "Неверное значение подадреса";
char *bad_status_word = "Ошибка в ответном слове";
char *no_int = "Выход по тайм-ауту";
char *int_ok = "Прерывание";
char *kbd_break = "Цикл прерван";
#endif

#define TIMER_CTRL_PORT 0x43       /* порт управления таймером */
#define TIMER0_PORT 0x40           /* порт данных таймера 0 */
#define _30_MCSEC 72               /* задержка 30 мкс */
#define _660_MCSEC 1580            /* задержка 660 мкс */
#define _5000_MCSEC 11932          /* задержка 5 мс */
#define TEST_RTFL 1
#define TEST_MASK_RTFL 2
#define TEST_UNMASK_RTFL 3
#define TEST_SSFL 4
#define TEST_BUSY 5
#define TEST_SREQ 6
#define TEST_DNBA_OFF 7
#define TEST_DNBA_ON 8
#define TEST_RESET 9
#define TEST_SYNC 10
#define TEST_GET_VECTOR 11
#define TEST_GET_SELFTEST 12
#define TEST_SYNC_DATA 13
#define TEST_AT 14
#define TEST_BUSY1 15
#define TEST_NOBUSY1 16
#define TEST_COUNTER 17
#define TEST_RT_RT_BRCST 18
#define TEST_COUNTER_RT_BC 19
#define TEST_COUNTER_BC_RT 20
#define TEST_DNBA 21
#define TEST_BRCST_CMD 22
#define TEST_BRCST_BIT 23
#define TEST_ERROR 24
#define TEST_SYNC_DATA1 25
#define TEST_SYNC_DATA_BRCST 26
#define TEST_NO_RT_15 27
#define TEST_RT_15 28
#define TEST_NO_RT_0A 29
#define TEST_RT_0A 30
#define TEST_OTHER_AT 31
#define TEST_BLOCK_TR 32
#define TEST_UNBLOCK_TR 33
#define TEST_BLOCKED_TR 34
#define TEST_UNBLOCKED_TR 35
#define TEST_RT_DATA_IRQ 36
#define TEST_RT_DATA_IRQ_BL 37
#define TEST_RTFL_NI 38
#define TEST_SSFL_NI 39
#define TEST_BUSY_NI 40
#define TEST_SREQ_NI 41

#define DNB 0x400                  /* Динамическое управление */
#define SYNC 0x401                 /* Синхронизация */
#define GET_SW 0x402               /* Передать ответное слово */
#define START_SELFTEST 0x403       /* Начать самоконтроль */
#define BLOCK_TRANSMIT 0x404       /* Блокировать передатчик */
#define UNBLOCK_TRANSMIT 0x405     /* Разблокировать передатчик */
#define MASK_RTFL 0x406            /* Подавить бит флага терминала */
#define UNMASK_RTFL 0x407          /* Отменить подавление бита флага терминала */
#define RESET 0x408                /* Установить исходное состояние ОУ */
#define GET_VECTOR 0x410           /* Передать векторное слово */
#define SYNC_DATA 0x011            /* Синхронизация с ИС */
#define GET_CMD 0x412              /* Передать последнее командное слово */
#define GET_SELFTEST 0x413         /* Передать слово встроенного контроля */
#define BLOCK_SEL_TRANSMIT 0x04    /* Блокировать выбранный передатчик */
#define UNBLOCK_SEL_TRANSMIT 0x015 /* Разблокировать выбранный передатчик */

_INT16 statistic_bc_rt(_INT16 st_base, _INT16 st_subadr, _INT16 st_data);
_INT16 statistic_rt_bc(_INT16 st_subadr, _INT16 st_base, _INT16 st_data);
void avt_bc(char *str);         /* поддержка проверки КК */
void avt_rt(char *str);         /* поддержка проверки ОУ */
void avt_ctrl(const char *str);       /* управление выводом на экран */
_INT16 cmp_ram_buf(_INT16 offset, _INT16 voffset, _INT16 size);

const char *err_msg;
_INT16 bcwin, rtwin;
_INT16 bcnum, rtnum;
_INT16 rtadr, rtadr1, rtadr2;
_UINT16 ci_field;
_INT16 hb_mode = 1;
_INT16 subadr1, subadr2, subadr0, dir0, len;
_INT16 avt_out = 1;
_INT16 avt_ok;
_INT16 avt_err;
_INT16 fATArBlk = 1;
_INT16 fLPTSyn = 0;
_INT16 fReserv = 0;
_INT16 fSimpleRT = 0;
_INT16 fSimpleA = 0;
_INT16 fBcRt = 1;

_UINT16 nMainBus = BUS_A, nReservBus = BUS_B;

long GL_COUNTER = 500;                      /* счетчик стартов */
long GL_COUNTER2 = GL_COUNTER * 2;         /* счетчик стартов 2 */
long GL_COUNTER3 = GL_COUNTER * 4;         /* счетчик стартов 3 */

_INT16 avtomat = 0;                  /* флаг работы в автоматическом режиме:
                                0 - выключен, 1 - включен */

unsigned long err_level = 0;   /* количество предельно допустимых ошибок */

#ifdef _TMK1553B_LINUX
#define cprintf printf
#define cputs printf
#define putch putchar

#define BLACK          0x1E
#define RED            0x1F
#define GREEN          0x20
#define BROWN          0x21
#define BLUE           0x22
#define VIOLET         0x23
#define CYAN           0x24
#define LIGHTGRAY      0x25

#define DARKGRAY       0x5E
#define LIGHTRED       0x5F
#define LIGHTGREEN     0x60
#define YELLOW         0x61
#define LIGHTBLUE      0x62
#define LIGHTVIOLET    0x63
#define LIGHTCYAN      0x64
#define WHITE          0x65

#define ERAO_MASK S_ERAO_MASK
#define MEO_MASK S_MEO_MASK
#define EM_MASK S_EM_MASK
#define EBC_MASK S_EBC_MASK
#define ELN_MASK S_ELN_MASK
#define IB_MASK S_IB_MASK
#define TO_MASK S_TO_MASK

_INT16 bckcolor = BLACK + 10;
unsigned char ESC_peek = 0;

struct termios saved_attr, cur_attr;
//struct timespec biostime1;
struct timeval biostime1;
int peek_character = -1;
_INT16 WinXSav, WinYSav;

_INT16 WinXMin = 1,
       WinXMax = 80,
       WinYMin = 1,
       WinYMax = 25;

void reset_input_mode()
{
	tcsetattr(STDIN_FILENO, TCSANOW, &saved_attr);
}

void set_input_mode()
{
	/* Make sure stdin is a terminal. */
	if (!isatty(STDIN_FILENO)) {
		printf("Not a terminal.\n");
		exit(0);
	}

	tcgetattr(STDIN_FILENO, &saved_attr);
	atexit(reset_input_mode);

	cur_attr = saved_attr;
	cur_attr.c_lflag &= ~(ICANON | ECHO); /* Clear ICANON and ECHO. */
	cur_attr.c_cc[VMIN] = 1;
	cur_attr.c_cc[VTIME] = 0;
//  tcsetattr(STDIN_FILENO, TCSAFLUSH, &cur_attr);
	tcsetattr(STDIN_FILENO, TCSANOW, &cur_attr);
}

void set_output_mode()
{
	printf("\x1B(U");
// printf("\x1B~");
}

void reset_output_mode()
{
	printf("\x1B(B");
// printf("\x1B}");
}

void gotoxy(_INT16 x, _INT16 y)
{
	WinXSav = x;
	WinYSav = y;
	printf("\x1B[%d;%dH", WinYMin - 1 + y, WinXMin - 1 + x);
	fflush(stdout);
}

_INT16 wherex()
{
	_INT16 x, y;
	char inp[15];

	tcflush(STDIN_FILENO, TCIFLUSH);
	printf("\x1B[6n");
	fflush(stdout);
	read(STDIN_FILENO, inp, 15);
	sscanf(inp, "\x1B[%hd;%hdR", &y, &x);
	x &= 0x7F;
	return x;
}

_INT16 wherey()
{
	_INT16 x, y;
	char inp[15];

	tcflush(STDIN_FILENO, TCIFLUSH);
	printf("\x1B[6n");
	fflush(stdout);
	read(STDIN_FILENO, inp, 15);
	sscanf(inp, "\x1B[%hd;%hdR", &y, &x);
	y &= 0x7F;
	return y;
}

void window(_INT16 xmin, _INT16 ymin, _INT16 xmax, _INT16 ymax)
{
	WinXMin = xmin,
	WinXMax = xmax,
	WinYMin = ymin,
	WinYMax = ymax;
}

void clrscr(void)
{
	_INT16 x, y;

	for (y = WinYMin; y <= WinYMax; y++) {
		//gotoxy(1,y-WinYMin+1);
		printf("\x1B[%d;%dH", y, 1);
		for (x = WinXMin; x <= WinXMax; x++) {
			putchar(' ');
		}
	}
	gotoxy(1, 1);
}

void clrscr_all(void)
{
	putchar(0x0C);
	fflush(stdout);
}

void textcolor(_INT16 color)
{
	char intens = (color >> 6) & 1;

	color &= 0x3F;
	printf("\x1B[%d;%d;%dm", intens, bckcolor, color);
}

void textbackground(_INT16 color)
{
	bckcolor = color + 10;
	fflush(stdout);
}

_INT16 kbhit()
{
	unsigned char ch;
	int nread;

	cur_attr.c_cc[VMIN] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &cur_attr);
	nread = read(STDIN_FILENO, &ch, 1);
	cur_attr.c_cc[VMIN] = 1;
	tcsetattr(STDIN_FILENO, TCSANOW, &cur_attr);
	if (nread == 1) {
		if (ch == 0x1B) {
			ESC_peek = ESCAPE;
		}
		return 1;
	}
	return 0;
}

unsigned long biostime(int dummy1, long dummy2)
{
	/* struct timespec biostime2;

	 clock_gettime(CLOCK_REALTIME, &biostime2);
	 return ((biostime2.tv_sec-biostime1.tv_sec)*1000L+
	         biostime2.tv_nsec/1000000L-biostime1.tv_nsec/1000000L)/55L;*/
	struct timeval biostime2;

	gettimeofday(&biostime2, NULL);
	return ((biostime2.tv_sec - biostime1.tv_sec) * 1000L +
	        biostime2.tv_usec / 1000L - biostime1.tv_usec / 1000L) / 55L;
}

void clreol(void)
{
	_INT16 x, xsav, y;

	xsav = x = wherex();
	y = wherey();

	for (; x <= WinXMax; x++) {
		putchar(' ');
	}
	printf("\x1B[%d;%dH", y, xsav);
}

void bioskey_f12()
{
	unsigned char inp[5];
	int nread;

	cur_attr.c_cc[VMIN] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &cur_attr);
	nread = read(STDIN_FILENO, inp, 5);
	cur_attr.c_cc[VMIN] = 1;
	tcsetattr(STDIN_FILENO, TCSANOW, &cur_attr);

	if (nread == 5 && inp[0] == 0x1B && inp[1] == 0x5B && inp[2] == 0x32 &&
	                inp[3] == 0x34 && inp[4] == 0x7E) {
		prtscr_flag = 1;
	}
}

unsigned short bioskey(int arg)
{
	unsigned short key;
	unsigned char inp[5];
	int nread;

	ESC_peek = 0;

	tcflush(STDIN_FILENO, TCIFLUSH);
	nread = read(STDIN_FILENO, inp, 5);

	key = inp[0];

	if ((key >= '0' && key <= '9') || (key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z')) {
		goto gotkey;
	}

	switch (key) {
	case 0x1B:
		if (nread == 1) {
			key = ESCAPE;
			goto gotkey;
		} else if (nread == 3 && inp[1] == 0x5B) {
			switch (inp[2]) {
			case 0x41://UP
				key = UP << 8;
				goto gotkey;
			case 0x42://DOWN
				key = DOWN << 8;
				goto gotkey;
			case 0x43://RIGHT
				key = RIGHT << 8;
				goto gotkey;
			case 0x44://LEFT
				key = LEFT << 8;
				goto gotkey;
			}//arrows
		} else if (nread == 4 && inp[1] == 0x5B && inp[3] == 0x7E) {
			switch (inp[2]) {
			case 0x31://HOME
				key = HOME << 8;
				goto gotkey;
			case 0x34://END
				key = END << 8;
				goto gotkey;
			case 0x35://PGUP
				key = PGUP << 8;
				goto gotkey;
			case 0x36://PGDN
				key = PGDN << 8;
				goto gotkey;
			}
		} else if (nread == 4 && inp[1] == 0x5B && inp[2] == 0x5B) {
			switch (inp[3]) {
			case 0x41://F1
				key = F01 << 8;
				goto gotkey;
			case 0x42://F2
				key = F02 << 8;
				goto gotkey;
			case 0x43://F3
				key = F03 << 8;
				goto gotkey;
			case 0x44://F4
				key = F04 << 8;
				goto gotkey;
			case 0x45://F5
				key = F05 << 8;
				goto gotkey;
			}
		} else if (nread == 5 && inp[1] == 0x5B && inp[2] == 0x31 && inp[4] == 0x7E) {
			switch (inp[3]) {
			case 0x37://F6
				key = F06 << 8;
				goto gotkey;
			case 0x38://F7
				key = F07 << 8;
				goto gotkey;
			case 0x39://F8
				key = F08 << 8;
				goto gotkey;
			}
		} else if (nread == 5 && inp[1] == 0x5B && inp[2] == 0x32 && inp[4] == 0x7E) {
			switch (inp[3]) {
			case 0x30://F9
				key = F09 << 8;
				goto gotkey;
			case 0x31://F10
				key = F10 << 8;
				goto gotkey;
			case 0x33://F11
				key = GPLUS << 8;
				goto gotkey;
			}
		}
		break;
	case 0x09:
		key = TAB;
		goto gotkey;
	case 0x0A:
		key = CR;
		goto gotkey;
	case 0x7F:
		key = BACKSPACE;
		goto gotkey;
	case 0x2B:
//   key = GPLUSW;
		key = '+';
		goto gotkey;
	case ' ':
	case '<':
	case '>':
	case '/':
	case '?':
	case '\'':
	case '\"':
	case '#':
	case '(':
	case ')':
	case '-':
	case '*':
	case '$':
	case ';':
	case ':':
	case '!':
	case '=':
	case '@':
	case '~':
	case '_':
	case '`':
	case '%':
	case '^':
	case '&':
	case '[':
	case ']':
	case '{':
	case '}':
	case '.':
	case ',':
	case '\\':
	case '|':
		goto gotkey;
	}
	key = 0;
gotkey:
	return key;
}

void CheckTmkEvent(int fWaitTime)
{
	TTmkEventData tmkEvD;
	int nSaveTmk;
	int nTmk;
	int hTMK = 0;
	unsigned int dwEvent;
	dwEvent = tmkwaitevents(TmkEvents, fWaitTime) & TmkEvents;
	for (; dwEvent != 0; hTMK++, dwEvent = dwEvent >> 1) {
		if ((dwEvent & 0x01) == 0) {
			continue;
		}
		nSaveTmk = tmkselected();
		nTmk = hTMK;
		if (nSaveTmk != nTmk) {
			tmkselect(nTmk);
		}
		while (1) {
			tmkgetevd(&tmkEvD);
			if (tmkEvD.nInt == 0) {
				break;
			}
			int_num[nTmk] = tmkEvD.nInt;
			switch (tmkEvD.wMode) {
			case BC_MODE:
				bc_sw = tmkEvD.bc.wResult;
				if (tmkEvD.nInt == 1) {
					++good_starts;
				} else {
					bc_aw1 = tmkEvD.bc.wAW1;
					bc_aw2 = tmkEvD.bc.wAW2;
					if (bad_starts < (unsigned long)BADLEN) {
						bad_sts[(unsigned)bad_starts] = bad_starts + good_starts + 1L;
						bad_sws[(unsigned)bad_starts] = bc_sw;
						bad_aws[(unsigned)bad_starts] = bc_aw1;
					}
					++bad_starts;
				}
				break;
			case RT_MODE:
				if (tmkEvD.nInt == 1) {
					rt_cmd = tmkEvD.rt.wCmd;
					rt_sw = rtgetstate();
				} else if (tmkEvD.nInt == 2) {
					rt_sw = tmkEvD.rt.wStatus;
				} else {
					rt_sw = rtgetstate();
				}
				break;
			}
		}//while(1)
		if (nSaveTmk != nTmk) {
			tmkselect(nSaveTmk);
		}
	}//for all TMKs
//CheckInput();//if pressed F12
}
#endif

#ifdef _TMK1553B_DOS
_UINT16 inline get_timer()
{
	register t;

	_disable();
	outportb(TIMER_CTRL_PORT, 0x00);
	t = (_UINT16)inportb(TIMER0_PORT);
	t |= (_UINT16)inportb(TIMER0_PORT) << 8;
	_enable();
	return t;
}
#endif

_INT16 fBeep = 1;

void beep1()
{
	if (fBeep) {
		putch('\007');
	}
}

void beep(const char *str)
{
	int i, nBeeps;
	while (*str == '\t' || *str == ' ') {
		str++;
	}
	if (*str) {
		if (sscanf(str, "%u", &nBeeps) != 1) {
			msg_out(inp_err);
			return;
		}
	} else {
		nBeeps = 1;
	}
	for (i = 0; i < nBeeps; ++i) {
		putch('\007');
	}
}

#ifdef ELCUS
_UINT16 nATWords = 32;

void set_a_words(char *str)
{
	_UINT16 c;

#ifdef _TMK1553B_DOS
	if (sscanf(str, "%U", &c) != 1 || c > 32)
#endif
#ifdef _TMK1553B_LINUX
		if (sscanf(str, "%hu", &c) != 1 || c > 32)
#endif
		{
			msg_out(inp_err);
			return;
		}
	nATWords = c;
}
#endif //def ELCUS

#ifdef ELCUS
char szBC[] = "0";
char szRT[] = "0";
char szBus[] = "a";
char szInst[] = "i+";
char szCmds[] = "c+";
char szFlags[] = "f+";

_INT16 randbcrt(_INT16 bcnum, _INT16 rtnum, _INT16 fReserv, _INT16 hb_mode)
{
	int res;
	_UINT16 bus;
	_INT16 num;
#ifdef _TMK1553B_LINUX
	pid_t pid;
#endif

	num = tmkselected();
	tmkselect(bcnum);
	bus = bcgetbus();
	tmkdone(ALL_TMKS);

	window(1, 1, 80, 25);
	clrscr();
	szBC[0] = bcnum + '0';
	szRT[0] = rtnum + '0';
	szInst[1] = (hb_mode) ? '+' : '-';
	szCmds[1] = (fSimpleRT) ? '-' : '+';
	szFlags[1] = (fSimpleRT) ? '-' : '+';

#ifdef _TMK1553B_DOS
	if (fReserv) {
		res = spawnlp(P_WAIT, "randbcrt.exe", "randbcrt.exe", szBC, szRT, "m0-2000000", szInst, szCmds, szFlags, "e1", NULL);
	} else {
		szBus[0] = (bus == BUS_A) ? 'a' : 'b';
		res = spawnlp(P_WAIT, "randbcrt.exe", "randbcrt.exe", szBC, szRT, szBus, "m0-2000000", szInst, szCmds, szFlags, "e1", NULL);
	}
#endif
#ifdef _TMK1553B_LINUX
	pid = fork();
	if (pid == -1) {
		res = -1;
	} else if (pid) {
		wait(&res);
	} else {
		if (fReserv) {
			if (execl("randbcrt", "randbcrt", szBC, szRT, "m0-2000000", szInst, szCmds, szFlags, "e1", NULL) == -1) {
				res = -1;
			}
		} else {
			szBus[0] = (bus == BUS_A) ? 'a' : 'b';
			if (execl("randbcrt", "randbcrt", szBC, szRT, szBus, "m0-2000000", szInst, szCmds, szFlags, "e1", NULL) == -1) {
				res = -1;
			}
		}
	}
#endif

	TmkInit0();
//  num1 = tmkselected();
	select_window(bcwin);
//  tmkselect(bcnum);
	bus_num = bus;
	out_number();
	bcdefbus(bus);
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
//  tmkselect(num1);
	return res;
}
#endif //def ELCUS

_INT16 test_start(_INT16 test_mode)
{
	_INT16 result = 1;
// unsigned cmdw;
	unsigned long p;
	_INT16 fWaitRtIntr = 0;
	_INT16 fRtIntr = 0;

	err_msg = NULL;
	switch (test_mode) {
	case TEST_RTFL:
	case TEST_SSFL:
	case TEST_BUSY:
	case TEST_SREQ:
	case TEST_DNBA_OFF:
	case TEST_DNBA_ON:
// case TEST_RESET:
	case TEST_SYNC:
	case TEST_GET_VECTOR:
	case TEST_GET_SELFTEST:
	case TEST_SYNC_DATA:
	case TEST_RT_DATA_IRQ:
		fRtIntr = 1;
	case TEST_RT_DATA_IRQ_BL:
	case TEST_OTHER_AT:
		fWaitRtIntr = 1;
		break;
	}
#ifdef _TMK1553B_LINUX
	CheckTmkEvent(0);
#endif
	int_num[rtnum] = 0;
	if (fWaitRtIntr) {
		p = biostime(0, 0L);        /* ожидание прерывания 2 тика */
	}
	start_1("");
	if (fWaitRtIntr)
		while (int_num[rtnum] == 0 && (biostime(0, 0L) - p) < WAITDLY)
#ifdef _TMK1553B_LINUX
			CheckTmkEvent(0);
#endif
	;

	if (int_num[bcnum] == 0) {
#ifdef ENG
		err_msg = "No BC interrupt";
#else
		err_msg = "Нет прерывания КК";
#endif
		return 1;
	}
	if (fRtIntr && int_num[rtnum] == 0) {
#ifdef ENG
		err_msg = "No RT interrupt";
#else
		err_msg = "Нет прерывания ОУ";
#endif
		return 1;
	}
// cmdw = bcgetw(0);
	switch (test_mode) {
	case TEST_RTFL:
	case TEST_RTFL_NI:
		if (int_num[bcnum] == 2 &&
		                bc_aw1 == (_UINT16)((rtadr << 11) | RTFL_MASK) && 1)
//      rt_cmd == SYNC)
		{
			result = 0;
			good_starts++;
			bad_starts--;
		}
		break;
	case TEST_MASK_RTFL:
		if (int_num[bcnum] == 1 && 1)
//      rt_cmd == MASK_RTFL)
		{
			result = 0;
		}
		break;
	case TEST_UNMASK_RTFL:
		if (int_num[bcnum] == 2 &&
		                bc_aw1 == (_UINT16)((rtadr << 11) | RTFL_MASK) && 1)
//      rt_cmd == UNMASK_RTFL)
		{
			result = 0;
			good_starts++;
			bad_starts--;
		}
		break;
	case TEST_SSFL:
	case TEST_SSFL_NI:
		if (int_num[bcnum] == 2 &&
		                bc_aw1 == (_UINT16)((rtadr << 11) | SSFL_MASK) && 1)
//      rt_cmd == SYNC)
		{
			result = 0;
			good_starts++;
			bad_starts--;
		}
		break;
	case TEST_BUSY:
	case TEST_BUSY_NI:
		if (int_num[bcnum] == 2 &&
		                bc_aw1 == (_UINT16)((rtadr << 11) | BUSY_MASK) && 1)
//      rt_cmd == SYNC)
		{
			result = 0;
			good_starts++;
			bad_starts--;
		}
		break;
	case TEST_SREQ:
	case TEST_SREQ_NI:
		if (int_num[bcnum] == 2 &&
		                bc_aw1 == (_UINT16)((rtadr << 11) | SREQ_MASK) && 1)
//      rt_cmd == SYNC)
		{
			result = 0;
			good_starts++;
			bad_starts--;
		}
		break;
	case TEST_DNBA_OFF:
		if (fSimpleRT && fSimpleA) {
			if (int_num[bcnum] == 2 &&
			                bc_aw1 == (_UINT16)((rtadr << 11) | ERROR_MASK) &&
			                bad_starts == 1001) {
				result = 0;
				good_starts++;
				bad_starts -= 1001;
			}
			break;
		}
		if (int_num[bcnum] == 1 &&
		                rt_cmd == DNB) {
			result = 0;
		}
		break;
	case TEST_DNBA_ON:
		if (int_num[bcnum] == 2 &&
		                bc_aw1 == (_UINT16)((rtadr << 11) | DNBA_MASK) &&
		                rt_cmd == DNB) {
			result = 0;
			good_starts++;
			bad_starts--;
		}
		break;
	case TEST_RESET:
		if (int_num[bcnum] == 1 && 1)
//      rt_cmd == RESET)
		{
			result = 0;
		}
		break;
	case TEST_SYNC:
		if (int_num[bcnum] == 1 &&
		                rt_cmd == SYNC) {
			result = 0;
		}
		break;
	case TEST_GET_VECTOR:
		if (int_num[bcnum] == 1 &&
		                bcgetw(2) == 0xFACE &&
		                rt_cmd == GET_VECTOR) {
			result = 0;
		}
		break;
	case TEST_GET_SELFTEST:
		if (int_num[bcnum] == 1 &&
		                (bcgetw(2) == 0xBABA || (fSimpleRT && bcgetw(2) == 0xFACE)) &&
		                rt_cmd == GET_SELFTEST) {
			result = 0;
		}
		break;
	case TEST_SYNC_DATA:
		if (int_num[bcnum] == 1 &&
		                rt_cmd == SYNC_DATA) {
			tmkselect(rtnum);
			if (rtgetcmddata(SYNC_DATA) == 0xA5A5 || (fSimpleRT && rtgetcmddata(0x0000) == 0xA5A5)) {
				result = 0;
			}
			tmkselect(bcnum);
		}
		break;
	case TEST_OTHER_AT:
		if (int_num[bcnum] == 2 &&
		                bc_sw == TO_MASK) {
			result = 0;
			good_starts++;
			bad_starts--;
		}
		break;
	case TEST_AT:
		if (int_num[bcnum] == 1 &&
		                bcgetw(1) == (rtadr << 11)) {
			result = 0;
		}
		break;
	case TEST_BUSY1:
		if (int_num[bcnum] == 2 &&
		                ((dir == RT_RECEIVE && bc_sw == IB_MASK) ||
		                 (dir == RT_TRANSMIT && bc_sw == (IB_MASK | TO_MASK))) &&
		                bc_aw1 == (_UINT16)((rtadr << 11) | BUSY_MASK)) {
			result = 0;
			good_starts++;
			bad_starts--;
		}
		break;
	case TEST_NOBUSY1:
		if (int_num[bcnum] == 1) {
			result = 0;
		}
		break;
	case TEST_COUNTER:
		if (int_num[bcnum] == 1) {
			result = 0;
		}
		break;
	case TEST_COUNTER_RT_BC:
	case TEST_COUNTER_BC_RT:
		if (int_num[bcnum] == 1 &&
		                bc_sw == 0) {
			result = 0;
		}
		break;
	case TEST_RT_RT_BRCST:
		if (int_num[bcnum] == 1 &&
		                !cmp_ram_buf(0, 3, 32)) {
			result = 0;
		}
		break;
	case TEST_DNBA:
		if (int_num[bcnum] == 2 &&
		                bc_sw == IB_MASK &&
		                bc_aw1 == (_UINT16)((rtadr << 11) | DNBA_MASK)) {
			result = 0;
			good_starts++;
			bad_starts--;
		}
		break;
	case TEST_BRCST_CMD:
		if (int_num[bcnum] == 1 &&
		                bc_sw == 0) {
			result = 0;
		}
		break;
	case TEST_BRCST_BIT:
		if (int_num[bcnum] == 2 &&
		                bc_sw == IB_MASK &&
		                bc_aw1 == (_UINT16)((rtadr << 11) | BRCST_MASK)) {
			result = 0;
			good_starts++;
			bad_starts--;
		}
		break;
	case TEST_ERROR:
		if (int_num[bcnum] == 2 &&
		                bc_sw == IB_MASK &&
		                bc_aw1 == (_UINT16)((rtadr << 11) | ERROR_MASK)) {
			result = 0;
			good_starts++;
			bad_starts--;
		}
		break;
	case TEST_SYNC_DATA1:
		if (int_num[bcnum] == 1 &&
		                bc_sw == 0) {
			result = 0;
		}
		break;
	case TEST_SYNC_DATA_BRCST:
		if (int_num[bcnum] == 1 &&
		                bc_sw == 0) {
			result = 0;
		}
		break;
	case TEST_NO_RT_15:
		if (int_num[bcnum] == 2 &&
		                bc_sw == TO_MASK) {
			result = 0;
			good_starts++;
			bad_starts--;
		}
		break;
	case TEST_RT_15:
		if (int_num[bcnum] == 1 &&
		                bc_sw == 0) {
			result = 0;
		}
		break;
	case TEST_NO_RT_0A:
		if (int_num[bcnum] == 2 &&
		                bc_sw == TO_MASK) {
			result = 0;
			good_starts++;
			bad_starts--;
		}
		break;
	case TEST_RT_0A:
		if (int_num[bcnum] == 1 &&
		                bc_sw == 0) {
			result = 0;
		}
		break;
	case TEST_BLOCK_TR:
	case TEST_UNBLOCK_TR:
		if (int_num[bcnum] == 1 &&
		                bc_sw == 0) {
			result = 0;
		}
		break;
	case TEST_BLOCKED_TR:
		if (int_num[bcnum] == 2 &&
		                bc_sw == TO_MASK) {
			result = 0;
			good_starts++;
			bad_starts--;
		}
		break;
	case TEST_UNBLOCKED_TR:
		if (int_num[bcnum] == 1 &&
		                bc_sw == 0) {
			result = 0;
		}
		break;
	case TEST_RT_DATA_IRQ:
		if (int_num[bcnum] == 1 &&
		                int_num[rtnum] == 3) {
			result = 0;
		}
		break;
	case TEST_RT_DATA_IRQ_BL:
		if (int_num[bcnum] == 1 &&
		                int_num[rtnum] == 0) {
			result = 0;
		}
		break;
	}
// test_err = !result;
	return result;
}

void make_test1(_INT16 start)
{
	_UINT16 data;
	_INT16 k;

	data = 0;
	for (k = start; k <= start + 31; ++k) {
		buffer[k] = data;
		switch (data) {
		case 0x0000:
			data = 0x0001;
			break;
		case 0x8000:
			data = 0xAAAA;
			break;
		case 0xAAAA:
			data = 0x5555;
			break;
		case 0x5555:
			data = 0xAAAA;
			break;
		default:
			data <<= 1;
		}
	}
}

/************************************************************************/
/*                      Начало секции для КК                            */
/*                      Bus Controller Tests                            */
/************************************************************************/

void avt_bc(char *str)                  /* поддержка проверки КК */
{
	_INT16 k, maxbase, err;
	unsigned long times, nBusSwapTime;

	while (*str == '\t' || *str == ' ') {
		str++;
	}
	if (*str) {
#ifdef _TMK1553B_DOS
		if (sscanf(str, "%U", &times) != 1)
#endif
#ifdef _TMK1553B_LINUX
			if (sscanf(str, "%lu", &times) != 1)
#endif
			{
				msg_out(inp_err);
				return;
			}
	} else {
		times = 1;
	}
	if (Window[0].nMode == BC_MODE) {
		bcwin = 0;
	} else if (Window[1].nMode == BC_MODE) {
		bcwin = 1;
	} else {
#ifdef ENG
		msg_out("BC not defined");
#else
		msg_out("Не задан КК");
#endif
		return;
	}
	if (Window[0].nMode == RT_MODE) {
		rtwin = 0;
	} else if (Window[1].nMode == RT_MODE) {
		rtwin = 1;
	} else {
#ifdef ENG
		msg_out("RT not defined");
#else
		msg_out("Не задано ОУ");
#endif
		return;
	}
	bcnum = Window[bcwin].nTMK;
	rtnum = Window[rtwin].nTMK;
	avt_ctrl("1");
	avt_ok = 0;
	select_window(bcwin);
	if (fReserv) {
		nReservBus = ((nMainBus = bcgetbus()) == BUS_A) ? BUS_B : BUS_A;
		bus_num = nMainBus;
		out_number();
		nBusSwapTime = times + 1;
		times <<= 1;
	}
	select_window(rtwin);
	wr_at(rtadr = 0x0A);
	out_at();
	set_status_word("0"); /* сбросить все биты ОС */
#ifdef ENG
	comment("Bus Controller automatic test.");
#else
	comment("Автоматический тест контроллера канала.");
#endif
	select_window(bcwin);
	statistic_clear("");
	if (hb_mode) {
		subadr1 = 0x10;
		subadr2 = 0x1E;
		ci_field = 0x1F << 5;
	} else {
		subadr1 = 0x01;
		subadr2 = 0x1E;
		ci_field = 0;
	}
	if (bc_ram_test("")) {
#ifdef ENG
		err_msg = "RAM test didn't pass";
#else
		err_msg = "Не прошел тест БЗУ";
#endif
		goto fin;
	}
	do {
		select_window(bcwin);
		maxbase = tmkMaxBase[tmkselected()];
		buf_clear("");
		srand(1);
		subadr = subadr1;
		for (base = 0; base <= maxbase; base++) {
			bcdefbase(base);
			ctrl_code[bcnum][base] = DATA_BC_RT;
			buffer[0] = (rtadr << 11) | (RT_RECEIVE) | (subadr << 5);
			buffer[1] = base | (base << 8);
			for (k = 2; k <= 32; ++k) {
				buffer[k] = rand() + rand();
			}
			bcputblk(0, buffer, 33);
			if (++subadr > subadr2) {
				subadr = subadr1;
			}
		}
		subadr = subadr1;
		srand(1);
		err = 0;
		for (base = 0; base <= maxbase; base++) {
			select_window(bcwin);
			if (start_1("")) {
#ifdef ENG
				err_msg = "Transmitting of test array from BC to RT didn't pass";
#else
				err_msg = "Не прошла передача тестового массива из КК в ОУ";
#endif
				goto fin;
			}
			select_window(rtwin);
			rtdefsubaddr(RT_RECEIVE, subadr);
			rtgetblk(0, buffer, 32);
			if (buffer[0] != (_UINT16)(base | (base << 8))) {
				err = 1;
				++data_err;
				break;
			}
			for (k = 1; k <= 31; ++k)
				if (buffer[k] != (_UINT16)(rand() + rand())) {
					err = 1;
					++data_err;
					break;
				}
			if (err) {
				break;
			}
			if (++subadr > subadr2) {
				subadr = subadr1;
			}
		}
		if (err) {
#ifdef ENG
			err_msg = "Test array transmitted to RT with errors";
#else
			err_msg = "Тестовый массив передан в ОУ с ошибками";
#endif
			goto fin;
		}
		select_window(bcwin);
		bc_clear_ram();
		srand(1);
		subadr = subadr1;
		for (base = 0; base <= maxbase; base++) {
			select_window(rtwin);
			rtdefsubaddr(RT_TRANSMIT, subadr);
			buffer[0] = base | (base << 8);
			for (k = 1; k <= 31; ++k) {
				buffer[k] = rand() + rand();
			}
			rtputblk(0, buffer, 32);
			select_window(bcwin);
			bcdefbase(base);
			ctrl_code[bcnum][base] = DATA_RT_BC;
			bcputw(0, (rtadr << 11) | RT_TRANSMIT | (subadr << 5));
			if (start_1("")) {
#ifdef ENG
				err_msg = "Receiving of test array from RT to BC didn't pass";
#else
				err_msg = "Не прошел прием тестового массива из ОУ в КК";
#endif
				goto fin;
			}
			if (++subadr > subadr2) {
				subadr = subadr1;
			}
		}
		srand(1);
		select_window(bcwin);
		for (base = 0; base <= maxbase; base++) {
			bcdefbase(base);
			bcgetblk(2, buffer, 32);
			if (buffer[0] != (_UINT16)(base | (base << 8))) {
				err = 1;
				++data_err;
				break;
			}
			for (k = 1; k <= 31; ++k)
				if (buffer[k] != (_UINT16)(rand() + rand())) {
					err = 1;
					++data_err;
					break;
				}
			if (err) {
				break;
			}
			if (++subadr > subadr2) {
				subadr = subadr1;
			}
		}
		if (err) {
#ifdef ENG
			err_msg = "Test array received from RT with errors";
#else
			err_msg = "Тестовый массив принят из ОУ с ошибками";
#endif
			goto fin;
		}
		/* Тестирование счетчика ИС */
		select_window(rtwin);
		rtdefsubaddr(dir = RT_TRANSMIT, subadr = 0x10);
		for (k = 0; k <= 31; ++k) {
			buffer[k] = k + 1;
		}
		rtputblk(0, buffer, 32);
		select_window(bcwin);
		bcdefbase(1);
		bcputblk(1, buffer, 32);
		bcdefbase(base = 0);
		ctrl_code[bcnum][base] = DATA_RT_BC;
		for (len = 1; len <= 32; len++) {
			bcputw(0, (rtadr << 11) | RT_TRANSMIT | (subadr << 5) | (len & 0x1F));
			if (test_start(TEST_COUNTER_RT_BC)) {
				if (err_msg == NULL)
#ifdef ENG
					err_msg = "Data counter test (RT->BC message) didn't pass";
#else
					err_msg = "Не прошел тест счетчика ИС в режиме ОУ->КК";
#endif
				goto fin;
			}
		}
		bcdefbase(base = 1);
		ctrl_code[bcnum][base] = DATA_BC_RT;
		for (len = 1; len <= 32; len++) {
			bcputw(0, (rtadr << 11) | RT_RECEIVE | (subadr << 5) | (len & 0x1F));
			if (test_start(TEST_COUNTER_BC_RT)) {
				if (err_msg == NULL)
#ifdef ENG
					err_msg = "Data counter test (BC->RT message) didn't pass";
#else
					err_msg = "Не прошел тест счетчика ИС в режиме КК->ОУ";
#endif
				goto fin;
			}
		}
		/* Формат ОУ-ОУ (групповой режим), подадрес - 17 */
		make_test1(3);
		select_window(rtwin);
		rtdefsubaddr(dir = RT_TRANSMIT, subadr = 0x17);
		rtputblk(0, buffer + 3, 32);
		select_window(bcwin);
		bcdefbase(base = 2);
		ctrl_code[bcnum][base] = DATA_RT_RT_BRCST;
		bcputw(0, (0x1F << 11) | RT_RECEIVE | (subadr << 5));
		bcputw(1, (rtadr << 11) | RT_TRANSMIT | (subadr << 5));
		if (start_loop_n(GL_COUNTER)) {
#ifdef ENG
			err_msg = "Broadcast RT->RT message test didn't pass";
#else
			err_msg = "Не прошел тест формата ОУ->ОУ в групповом режиме";
#endif
			goto fin;
		}
		select_window(rtwin);
		if (cmp_ram_buf(3, 0, 32)) {
#ifdef ENG
			err_msg = "Test array in RT->RT message received with errors";
#else
			err_msg = "Тестовый массив в формате ОУ->ОУ в групповом режиме принят с ошибками";
#endif
			goto fin;
		}
		/* Проверка битов ответного слова */
		select_window(bcwin);
		base = 3;
		bcdefbase(base);
		buf_clear("");
		select_window(rtwin);
		rtsetanswbits(RTFL);
		out_status_word();
#ifdef _TMK1553B_LINUX
		fflush(stdout);
#endif
		select_window(bcwin);
		ctrl_code[bcnum][base] = CTRL_C_A;
		bcputw(0, (rtadr << 11) | ci_field | UNBLOCK_TRANSMIT);
		if (test_start(TEST_RTFL_NI)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "Status with 'Terminal Flag' bit test didn't pass";
#else
				err_msg = "Не прошел тест приема ОС с битом 'Неисправность терминала'";
#endif
			goto fin;
		}
		select_window(rtwin);
		rtclranswbits(RTFL);
		rtsetanswbits(SSFL);
		select_window(bcwin);
		if (test_start(TEST_SSFL_NI)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "Status with 'Subsystem Flag' bit test didn't pass";
#else
				err_msg = "Не прошел тест приема ОС с битом 'Неисправность подсистемы'";
#endif
			goto fin;
		}
		select_window(rtwin);
		rtclranswbits(SSFL);
		rtsetanswbits(BUSY);
		select_window(bcwin);
		if (test_start(TEST_BUSY_NI)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "Status with 'Subsystem Busy' bit test didn't pass";
#else
				err_msg = "Не прошел тест приема ОС с битом 'Подсистема занята'";
#endif
			goto fin;
		}
		select_window(rtwin);
		rtclranswbits(BUSY);
		rtsetanswbits(SREQ);
		select_window(bcwin);
		if (test_start(TEST_SREQ_NI)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "Status with 'Service Request' bit test didn't pass";
#else
				err_msg = "Не прошел тест приема ОС с битом 'Запрос обслуживания'";
#endif
			goto fin;
		}
		select_window(rtwin);
		rtclranswbits(SREQ);
		if (!fSimpleRT) {
			rtsetanswbits(DNBA);
			select_window(bcwin);
			bcputw(0, (rtadr << 11) | ci_field | DNB);
			if (test_start(TEST_DNBA)) {
				if (err_msg == NULL)
#ifdef ENG
					err_msg = "Status with 'Dynamic Bus Control Acceptance' bit test didn't pass";
#else
					err_msg = "Не прошел тест приема ОС с битом 'Принято управление каналом'";
#endif
				goto fin;
			}
			select_window(rtwin);
			rtclranswbits(DNBA);
		}
		out_status_word();
#ifdef _TMK1553B_LINUX
		fflush(stdout);
#endif
		/* сброс ОУ в исходное состояние */
		select_window(bcwin);
		ctrl_code[bcnum][base] = CTRL_C_BRCST;
		bcputw(0, (0x1F << 11) | ci_field | RESET);
		if (test_start(TEST_BRCST_CMD)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "'Broadcast Reset Remote Terminal' test didn't pass";
#else
				err_msg = "Не прошел тест 'Уст.исходное состояние ОУ, групповой режим'";
#endif
			goto fin;
		}
		/* Проверка бита групповой команды */
		ctrl_code[bcnum][base] = CTRL_C_A;
		bcputw(0, (rtadr << 11) | ci_field | GET_SW);
		if (test_start(TEST_BRCST_BIT)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "Status with 'Broadcast Command Received' bit test didn't pass";
#else
				err_msg = "Не прошел тест приема ОС с битом 'Принята групповая команда'";
#endif
			goto fin;
		}
		/* Проверка бита ошибки в сообщении */
		bcputw(0, ~RT_DIR_MASK & ((rtadr << 11) | ci_field | SYNC));
		if (test_start(TEST_ERROR)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "Status with 'Message Error' bit test didn't pass";
#else
				err_msg = "Не прошел тест приема ОС с битом 'Ошибка сообщения'";
#endif
			goto fin;
		}
		/* Синхронизация со словом AAAA */
		ctrl_code[bcnum][base] = CTRL_CD_A;
		bcputw(0, (rtadr << 11) | ci_field | SYNC_DATA);
		bcputw(1, 0xAAAA);
		if (test_start(TEST_SYNC_DATA1)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "'Synchronize with Data AAAAh' test didn't pass";
#else
				err_msg = "Не прошел тест 'Синхронизация со словом AAAAh'";
#endif
			goto fin;
		}
		ctrl_code[bcnum][base] = CTRL_CD_BRCST;
		bcputw(0, (0x1F << 11) | ci_field | SYNC_DATA);
		bcputw(1, 0x5555);
		if (test_start(TEST_SYNC_DATA_BRCST)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "'Broadcast Synchronize with Data 5555h' test didn't pass";
#else
				err_msg = "Не прошел тест 'Синхронизация со словом 5555h, групповой режим'";
#endif
			goto fin;
		}
		ctrl_code[bcnum][base] = CTRL_C_A;
		bcputw(0, (rtadr << 11) | ci_field | RESET);
		if (test_start(TEST_BRCST_CMD)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "Status with 'Broadcast Command Received' bit test didn't pass";
#else
				err_msg = "Не прошел тест приема ОС с битом 'Принята групповая команда'";
#endif
			goto fin;
		}
		all_screen();
		if (Window[rtwin].at_avail) {
			rtadr1 = 0x15;
			rtadr2 = 0x0A;
		} else {
#ifdef LPT_AT
			rtadr1 = 0x15;
			rtadr2 = 0x0A;
#else
			rtadr1 = (rtadr == 0) ? 0x1E : rtadr ^ 0x1F;
			rtadr2 = rtadr;
#endif
		}
		/* Обращениe к несуществующему ОУ c адресом 0x15 */
		ctrl_code[bcnum][base] = CTRL_C_A;
		bcputw(0, (rtadr1 << 11) | ci_field | GET_SW);
		if (test_start(TEST_NO_RT_15)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "Missing RT with address 15h (10101) access test didn't pass";
#else
				err_msg = "Не прошел тест обращения к несуществующему ОУ с адресом 15h (10101)";
#endif
			goto fin;
		}
#ifndef LPT_AT
		if (Window[rtwin].at_avail)
#endif
		{
			select_window(rtwin);
			wr_at(rtadr = rtadr1);
			select_window(bcwin);
			/* Обращение к ОУ с адресом 0x15 */
			if (test_start(TEST_RT_15)) {
				if (err_msg == NULL)
#ifdef ENG
					err_msg = "RT with address 15h (10101) access test didn't pass";
#else
					err_msg = "Не прошел тест обращения к ОУ с адресом 15h (10101)";
#endif
				goto fin;
			}
			/* Обращениe к несуществующему ОУ c адресом 0x0A */
			bcputw(0, (rtadr2 << 11) | ci_field | GET_SW);
			if (test_start(TEST_NO_RT_0A)) {
				if (err_msg == NULL)
#ifdef ENG
					err_msg = "Missing RT with address 0Ah (01010) access test didn't pass";
#else
					err_msg = "Не прошел тест обращения к несуществующему ОУ с адресом 0Ah (01010)";
#endif
				goto fin;
			}
		}
		select_window(rtwin);
		wr_at(rtadr = rtadr2);
		select_window(bcwin);
		/* Обращение к ОУ с адресом 0x0A */
		bcputw(0, (rtadr2 << 11) | ci_field | GET_SW);
		if (test_start(TEST_RT_0A)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "RT with address 0Ah (01010) access test didn't pass";
#else
				err_msg = "Не прошел тест обращения к ОУ с адресом 0Ah (01010)";
#endif
			goto fin;
		}
#ifdef _TMK1553B_LINUX
		CheckTmkEvent(0);
#endif
		int_num[rtnum] = 0;
		select_window(rtwin);

//  rtdefirqmode(RT_DATA_BL); //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

		wr_at(rtadr);
//  select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_rt_bc(0x10, 0, 0xAAAA)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_rt_bc(0x12, 2, 0x0000)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_rt_bc(0x14, 4, 0x5555)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_rt_bc(0x16, 6, 0xFFFF)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_rt_bc(0x18, 8, 0x8000)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_rt_bc(0x1A, 10, 0x7FFF)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_bc_rt(12, 0x10, 0xAAAA)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_bc_rt(14, 0x12, 0x0000)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_bc_rt(16, 0x14, 0x5555)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_bc_rt(18, 0x16, 0xFFFF)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_bc_rt(20, 0x18, 0x8000)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_bc_rt(22, 0x1A, 0x7FFF)) {
			goto bad_statistic;
		}
		if (channel_err > 0L) {
			goto bad_statistic;
		}
		select_window(bcwin);
		if (fReserv && (times == nBusSwapTime || times == 1)) {
			_UINT16 nTempBus;
			bcdefbus(nReservBus);
			nTempBus = nMainBus;
			nMainBus = nReservBus;
			nReservBus = nTempBus;
			bus_num = nMainBus;
			out_number();
#ifdef _TMK1553B_LINUX
			fflush(stdout);
#endif
		}
	} while (--times);
#ifdef ELCUS
	if (fBcRt && *str) {
		if (randbcrt(bcnum, rtnum, fReserv, hb_mode)) {
#ifdef ENG
			err_msg = "Randbcrt test didn't pass";
#else
			err_msg = "Не прошел тест Randbcrt";
#endif
			goto fin;
		}
	}
#endif //def ELCUS
	/* Конец теста */
	avt_ok = 1;
#ifdef ENG
	err_msg = "Bus Controller test completed";
#else
	err_msg = "Тест контроллера канала завершен";
#endif
	goto fin;
bad_statistic:
#ifdef ENG
	err_msg = "'Statistics' test didn't pass";
#else
	err_msg = "Не прошел тест 'Накопление статистики'";
#endif
fin:
	if (prtscr_flag) {
		prtscr_flag = 0;
#ifdef ENG
		err_msg = "Test aborted on PrtScr";
#else
		err_msg = "Тест прерван по PrtScr";
#endif
	}
	check_int();
	select_window(nmainwin = bcwin);
	all_screen();
	select_window(rtwin);
	beep1();
	if (avt_ok) {
		textcolor(GREEN);
	} else {
		textcolor(LIGHTRED);
	}
	comment(err_msg);
	textcolor(LIGHTGRAY);
	select_window(nmainwin = bcwin);
	statistic_out("");
	if (channel_err > 0L) {
		statistic_err_out();
	}
	avt_ctrl("0");
	avt_err = !avt_ok;
}

/************************************************************************/
/*                      Начало секции для ОУ                            */
/*                      Remote Terminal Tests                           */
/************************************************************************/

_INT16 test_rt_busy()
{
	_INT16 test_result;

	test_result = 1;
	rtdefsubaddr(dir = RT_TRANSMIT, subadr = 0x11);
	select_window(bcwin);
	bcdefbase(base);
	ctrl_code[bcnum][base] = DATA_RT_BC;
	bcputw(0, (rtadr << 11) | RT_TRANSMIT | (subadr << 5));
#ifdef _TMK1553B_LINUX
	CheckTmkEvent(0);
#endif
	int_num[bcnum] = 0;
	bcstart(base, ctrl_code[bcnum][base]);
	tmkselect(rtnum);
	do {
		if (rtbusy()) {
			test_result = 0;
		}
#ifdef _TMK1553B_LINUX
		CheckTmkEvent(0);
#endif
	} while (!int_num[bcnum]);
	select_window(rtwin);
	return test_result;
}

void avt_rt(char *str)                   /* поддержка проверки ОУ */
{
	_INT16 maxpage, k, page;
	unsigned long times, nBusSwapTime;

	while (*str == '\t' || *str == ' ') {
		str++;
	}
	if (*str) {
#ifdef _TMK1553B_DOS
		if (sscanf(str, "%U", &times) != 1)
#endif
#ifdef _TMK1553B_LINUX
			if (sscanf(str, "%lu", &times) != 1)
#endif
			{
				msg_out(inp_err);
				return;
			}
	} else {
		times = 1;
	}
	if (Window[0].nMode == RT_MODE) {
		rtwin = 0;
	} else if (Window[1].nMode == RT_MODE) {
		rtwin = 1;
	} else {
#ifdef ENG
		msg_out("RT not defined");
#else
		msg_out("Не задано ОУ");
#endif
		return;
	}
	if (Window[0].nMode == BC_MODE) {
		bcwin = 0;
	} else if (Window[1].nMode == BC_MODE) {
		bcwin = 1;
	} else {
#ifdef ENG
		msg_out("BC not defined");
#else
		msg_out("Не задан КК");
#endif
		return;
	}
	bcnum = Window[bcwin].nTMK;
	rtnum = Window[rtwin].nTMK;
	statistic_clear("");
	avt_ctrl("1");
	avt_ok = 0;
	select_window(bcwin);
	if (fReserv) {
		nReservBus = ((nMainBus = bcgetbus()) == BUS_A) ? BUS_B : BUS_A;
		bus_num = nMainBus;
		out_number();
#ifdef _TMK1553B_LINUX
		fflush(stdout);
#endif
		nBusSwapTime = times + 1;
		times <<= 1;
	}
	select_window(rtwin);
	if (rt_ram_test("")) {
#ifdef ENG
		err_msg = "RAM test didn't pass";
#else
		err_msg = "Не прошел тест БЗУ";
#endif
		goto fin;
	}
#ifdef ENG
	comment("Remote Terminal automatic test.");
#else
	comment("Автоматический тест оконечного устройства.");
#endif
	wr_at(rtadr = 0x0A);
	out_at();
	set_status_word("0"); /* сбросить все биты ОС */
	maxpage = rtgetmaxpage();
	if (hb_mode) {
		subadr1 = 0x10;
		subadr2 = 0x1E;
		ci_field = CI_MASK;
	} else {
		subadr1 = 0x01;
		subadr2 = 0x1E;
		ci_field = 0;
	}
	do {
		select_window(rtwin);
		srand(1);
		for (page = 0; page <= maxpage; page++) {
			rtdefpage(page);
			for (subadr = subadr1; subadr <= subadr2; subadr++) {
				rtdefsubaddr(RT_TRANSMIT, subadr);
				buffer[0] = (page << 14) | (subadr << 8) | (page << 6) | subadr | 0x2020;
				for (k = 1; k <= 31; ++k) {
					buffer[k] = rand() + rand();
				}
				rtputblk(0, buffer, 32);
			}
		}
		srand(1);
		for (page = 0; page <= maxpage; page++) {
			select_window(rtwin);
			rtdefpage(page);
			select_window(bcwin);
			bcdefbase(base = 0);
			ctrl_code[bcnum][base] = DATA_RT_BC;
			for (subadr = subadr1; subadr <= subadr2; subadr++) {
				bcputw(0, (rtadr << 11) | RT_TRANSMIT | (subadr << 5));
				if (start_1("")) {
#ifdef ENG
					err_msg = "Transmitting of test array from RT to BC didn't pass";
#else
					err_msg = "Не прошла передача тестового массива из ОУ в КК";
#endif
					goto fin;
				}
				vbuffer[0] = (page << 14) | (subadr << 8) | (page << 6) | subadr | 0x2020;
				for (k = 1; k <= 31; ++k) {
					vbuffer[k] = rand() + rand();
				}
				if (cmp_ram_buf(2, -1, 32)) {
#ifdef ENG
					err_msg = "Test array transmitted from RT with errors";
#else
					err_msg = "Тестовый массив передан из ОУ с ошибками";
#endif
					goto fin;
				}
			}
		}
		for (page = 0; page <= maxpage; page++) {
			select_window(rtwin);
			rtdefpage(page);
			select_window(bcwin);
			bcdefbase(base = 1);
			ctrl_code[bcnum][base] = DATA_BC_RT;
			for (subadr = subadr1; subadr <= subadr2; subadr++) {
				buffer[0] = (rtadr << 11) | RT_RECEIVE | (subadr << 5);
				buffer[1] = (page << 14) | (subadr << 8) | (page << 6) | subadr;
				for (k = 2; k <= 32; ++k) {
					buffer[k] = rand() + rand();
				}
				bcputblk(0, buffer, 33);
				if (start_1("")) {
#ifdef ENG
					err_msg = "Receiving of test array from BC to RT didn't pass";
#else
					err_msg = "Не прошел прием тестового массива в ОУ из КК";
#endif
					goto fin;
				}
			}
		}
		srand(1);
		select_window(rtwin);
		for (page = 0; page <= maxpage; page++) {
			rtdefpage(page);
			for (subadr = subadr1; subadr <= subadr2; subadr++) {
				buffer[0] = (page << 14) | (subadr << 8) | (page << 6) | subadr | 0x2020;
				for (k = 1; k <= 31; ++k) {
					buffer[k] = rand() + rand();
				}
				rtdefsubaddr(RT_TRANSMIT, subadr);
				if (cmp_ram_buf(0, 0, 32)) {
#ifdef ENG
					err_msg = "Data in Tx subaddress has changed while receiving test array";
#else
					err_msg = "При приеме тестового массива в ОУ изменились данные в подадресе передачи";
#endif
					goto fin;
				}
			}
		}
		for (page = 0; page <= maxpage; page++) {
			rtdefpage(page);
			for (subadr = subadr1; subadr <= subadr2; subadr++) {
				buffer[0] = (page << 14) | (subadr << 8) | (page << 6) | subadr;
				for (k = 1; k <= 31; ++k) {
					buffer[k] = rand() + rand();
				}
				rtdefsubaddr(RT_RECEIVE, subadr);
				if (cmp_ram_buf(0, 0, 32)) {
#ifdef ENG
					err_msg = "Test array received to RT with errors";
#else
					err_msg = "Тестовый массив принят в ОУ с ошибками";
#endif
					goto fin;
				}
			}
		}
		set_page("0");
		/* проверка битов ответного слова */
		rtsetanswbits(RTFL); /* TF */
		select_window(bcwin);
		ctrl_code[bcnum][base] = CTRL_C_A;
		bcputw(0, (rtadr << 11) | ci_field | SYNC);
		if (test_start(TEST_RTFL)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "Status bit 'Terminal Flag' didn't set";
#else
				err_msg = "Не установился в ОС бит 'Неисправность терминала'";
#endif
			goto fin;
		}
		bcputw(0, (rtadr << 11) | ci_field | MASK_RTFL);
		if (test_start(TEST_MASK_RTFL)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "'Inhibit Terminal Flag Bit' command didn't pass";
#else
				err_msg = "Не прошла команда 'Подавить бит терминала'";
#endif
			goto fin;
		}
		bcputw(0, (rtadr << 11) | ci_field | UNMASK_RTFL);
		if (test_start(TEST_UNMASK_RTFL)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "'Override Inhibit Terminal Flag Bit' command didn't pass";
#else
				err_msg = "Не прошла команда 'Отменить подавление бита терминала'";
#endif
			goto fin;
		}
		select_window(rtwin);
		rtclranswbits(RTFL); /* TF */
		rtsetanswbits(SSFL); /* SF */
		select_window(bcwin);
		bcputw(0, (rtadr << 11) | ci_field | SYNC);
		if (test_start(TEST_SSFL)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "Status bit 'Subsystem Flag' didn't set";
#else
				err_msg = "Не установился в ОС бит 'Неисправность подсистемы'";
#endif
			goto fin;
		}
		select_window(rtwin);
		rtclranswbits(SSFL); /* SF */
		rtsetanswbits(BUSY); /* BS */
		select_window(bcwin);
		bcputw(0, (rtadr << 11) | ci_field | SYNC);
		if (test_start(TEST_BUSY)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "Status bit 'Subsystem Busy' didn't set";
#else
				err_msg = "Не установился в ОС бит 'Подсистема занята'";
#endif
			goto fin;
		}
		select_window(rtwin);
		rtclranswbits(BUSY); /* BS */
		rtsetanswbits(SREQ); /* SR */
		select_window(bcwin);
		bcputw(0, (rtadr << 11) | ci_field | SYNC);
		if (test_start(TEST_SREQ)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "Status bit 'Service Request' didn't set";
#else
				err_msg = "Не установился в ОС бит 'Запрос обслуживания подсистемы'";
#endif
			goto fin;
		}
		select_window(rtwin);
		rtclranswbits(SREQ); /* SR */
		select_window(bcwin);
		bcputw(0, (rtadr << 11) | ci_field | DNB);
		if (test_start(TEST_DNBA_OFF)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "Status bit 'Dynamic Bus Control Acceptance' set while RT disallowed it";
#else
				err_msg = "Установился в ОС бит 'Принято управление каналом' без разрешения ОУ";
#endif
			goto fin;
		}
		if (!fSimpleRT) {
			select_window(rtwin);
			rtsetanswbits(DNBA); /* DN */
			select_window(bcwin);
			bcputw(0, (rtadr << 11) | ci_field | DNB);
			if (test_start(TEST_DNBA_ON)) {
				if (err_msg == NULL)
#ifdef ENG
					err_msg = "Status bit 'Dynamic Bus Control Acceptance' didn't set";
#else
					err_msg = "Не установился в ОС бит 'Принято управление каналом'";
#endif
				goto fin;
			}
			select_window(rtwin);
			rtclranswbits(DNBA); /* Сброс DN */
		}
		select_window(bcwin);
		bcputw(0, (rtadr << 11) | ci_field | RESET);
		if (test_start(TEST_RESET)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "Status bit 'Dynamic Bus Control Acceptance' didn't reset on 'Reset RT' cmd";
#else
				err_msg = "Не сбросился в ОС бит 'Принято управление каналом' после команды 'Сброс ОУ'";
#endif
			goto fin;
		}
		if (fReserv) {
			bcputw(0, (rtadr << 11) | ci_field | BLOCK_TRANSMIT);
			if (test_start(TEST_BLOCK_TR)) {
				if (err_msg == NULL)
#ifdef ENG
					err_msg = "'Transmitter shutdown' command1 didn't pass";
#else
					err_msg = "Не прошла команда1 блокировки передатчика";
#endif
				goto fin;
			}
			bcdefbus(nReservBus);
			ctrl_code[bcnum][base] = DATA_RT_BC;
			bcputw(0, (rtadr << 11) | RT_TRANSMIT | (0x10 << 5) | 1);
			if (test_start(TEST_BLOCKED_TR)) {
				if (err_msg == NULL)
#ifdef ENG
					err_msg = "Disabled transmitter test1 didn't pass";
#else
					err_msg = "Не прошел тест1 заблокированного передатчика";
#endif
				goto fin;
			}
			ctrl_code[bcnum][base] = CTRL_C_A;
			bcputw(0, (rtadr << 11) | ci_field | RESET);
			if (test_start(TEST_RESET)) {
				if (err_msg == NULL)
#ifdef ENG
					err_msg = "RT reset on disabled bus didn't pass";
#else
					err_msg = "Не прошел сброс ОУ по заблокированной ЛПИ";
#endif
				goto fin;
			}
			ctrl_code[bcnum][base] = DATA_RT_BC;
			bcputw(0, (rtadr << 11) | RT_TRANSMIT | (0x10 << 5) | 1);
			if (test_start(TEST_UNBLOCKED_TR)) {
				if (err_msg == NULL)
#ifdef ENG
					err_msg = "Enabled transmitter test1 didn't pass";
#else
					err_msg = "Не прошел тест1 разблокированного передатчика";
#endif
				goto fin;
			}
			bcdefbus(nMainBus);
			ctrl_code[bcnum][base] = CTRL_C_A;
			bcputw(0, (rtadr << 11) | ci_field | BLOCK_TRANSMIT);
			if (test_start(TEST_BLOCK_TR)) {
				if (err_msg == NULL)
#ifdef ENG
					err_msg = "'Transmitter shutdown' command2 didn't pass";
#else
					err_msg = "Не прошла команда2 блокировки передатчика";
#endif
				goto fin;
			}
			bcdefbus(nReservBus);
			ctrl_code[bcnum][base] = DATA_RT_BC;
			bcputw(0, (rtadr << 11) | RT_TRANSMIT | (0x10 << 5) | 1);
			if (test_start(TEST_BLOCKED_TR)) {
				if (err_msg == NULL)
#ifdef ENG
					err_msg = "Disabled transmitter test2 didn't pass";
#else
					err_msg = "Не прошел тест2 заблокированного передатчика";
#endif
				goto fin;
			}
			bcdefbus(nMainBus);
			ctrl_code[bcnum][base] = CTRL_C_A;
			bcputw(0, (rtadr << 11) | ci_field | UNBLOCK_TRANSMIT);
			if (test_start(TEST_UNBLOCK_TR)) {
				if (err_msg == NULL)
#ifdef ENG
					err_msg = "'Override Transmitter shutdown' command2 didn't pass";
#else
					err_msg = "Не прошла команда2 разблокировки передатчика";
#endif
				goto fin;
			}
			bcdefbus(nReservBus);
			ctrl_code[bcnum][base] = DATA_RT_BC;
			bcputw(0, (rtadr << 11) | RT_TRANSMIT | (0x10 << 5) | 1);
			if (test_start(TEST_UNBLOCKED_TR)) {
				if (err_msg == NULL)
#ifdef ENG
					err_msg = "Enabled transmitter test2 didn't pass";
#else
					err_msg = "Не прошел тест2 разблокированного передатчика";
#endif
				goto fin;
			}
			bcdefbus(nMainBus);
		}
		ctrl_code[bcnum][base] = CTRL_C_A;
		bcputw(0, (rtadr << 11) | ci_field | SYNC);
		if (test_start(TEST_SYNC)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "'Synchronize' command didn't pass";
#else
				err_msg = "Не прошла команда 'Синхронизация'";
#endif
			goto fin;
		}
		select_window(rtwin);
		rtputcmddata(GET_VECTOR, 0xFACE);
		rtputcmddata(GET_SELFTEST, 0xBABA);
		if (fSimpleRT) {
			rtputcmddata(0x0400, 0xFACE);
		}
		select_window(bcwin);
		ctrl_code[bcnum][base] = CTRL_C_AD;
		bcputw(0, (rtadr << 11) | ci_field | GET_VECTOR);
		if (test_start(TEST_GET_VECTOR)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "'Transmit Vector Word' command didn't pass";
#else
				err_msg = "Не прошла команда 'Дать векторное слово'";
#endif
			goto fin;
		}
		bcputw(0, (rtadr << 11) | ci_field | GET_SELFTEST);
		if (test_start(TEST_GET_SELFTEST)) {
#ifdef ENG
			err_msg = "'Transmit BIT Word' command didn't pass";
#else
			err_msg = "Не прошла команда 'Дать слово встроенного контроля'";
#endif
			goto fin;
		}
		/* Синхронизация со словом A5A5 */
		ctrl_code[bcnum][base] = CTRL_CD_A;
		bcputw(0, (rtadr << 11) | ci_field | SYNC_DATA);
		bcputw(1, 0xA5A5);
		if (test_start(TEST_SYNC_DATA)) {
			if (err_msg == NULL)
#ifdef ENG
				err_msg = "'Synchronize with Data A5A5h' command didn't pass";
#else
				err_msg = "Не прошла команда 'Синхронизация со словом A5A5h'";
#endif
			goto fin;
		}
		/* Проверка регистра адреса терминала */
		ctrl_code[bcnum][base] = CTRL_C_A;
		for (rtadr1 = 1; rtadr1 != 32; rtadr1 <<= 1) {
			if (rtadr == rtadr1) {
				continue;
			}
			bcputw(0, (rtadr1 << 11) | ci_field | SYNC);
			if (test_start(TEST_OTHER_AT)) {
				if (err_msg == NULL)
#ifdef ENG
					err_msg = "Other address RT access test didn't pass";
#else
					err_msg = "Не прошел тест 'Обращение по `чужому` адресу ОУ'";
#endif
				goto fin;
			}
		}
#ifndef LPT_AT
		if (Window[rtwin].at_avail)
#endif
		{
			for (rtadr = 1; rtadr != 32; rtadr <<= 1) {
				select_window(rtwin);
				wr_at(rtadr);
				select_window(bcwin);
				ctrl_code[bcnum][base] = CTRL_C_A;
				bcputw(0, (rtadr << 11) | ci_field | SYNC);
				if (test_start(TEST_AT)) {
					if (err_msg == NULL)
#ifdef ENG
						err_msg = "RT address test didn't pass";
#else
						err_msg = "Не прошел тест адреса ОУ";
#endif
					goto fin;
				}
			}
		}
		select_window(rtwin);
		wr_at(rtadr = 0x0A);
		avt_out = 0;
		if (!fSimpleRT) {
			/* Проверка арбитра доступа к БЗУ */
			for (subadr0 = subadr1; subadr0 <= subadr2; subadr0++) {
				rtdefsubaddr(RT_TRANSMIT, subadr0);
				rtputw(0, 0x0000);
			}
			for (dir = RT_RECEIVE; ; dir = RT_TRANSMIT) {
				for (subadr = subadr1; ;) {
					for (subadr0 = subadr1; subadr0 <= subadr2; subadr0++) {
						rtdefsubaddr(RT_RECEIVE, subadr0);
						rtputw(0, 0x0000);
					}
					rtlock(dir, subadr);
					fLock = 1;
					out_subadr();
#ifdef _TMK1553B_LINUX
					fflush(stdout);
#endif
					if (rtbusy()) {
#ifdef ENG
						err_msg = "Subaddress access disable bit didn't reset";
#else
						err_msg = "Не сбрасывается бит запрета доступа к подадресу ОУ";
#endif
						goto fin;
					}
					select_window(bcwin);
					for (dir0 = RT_RECEIVE; ; dir0 = RT_TRANSMIT) {
						ctrl_code[bcnum][base] = (dir0 == RT_TRANSMIT) ? DATA_RT_BC : DATA_BC_RT;
						for (subadr0 = subadr1; subadr0 <= subadr2; subadr0++) {
							bcputw(0, (rtadr << 11) | dir0 | (subadr0 << 5) | 1);
							bcputw(1, 0xFFFF);
							bcputw(2, 0xFFFF);
							if (dir0 == dir && subadr0 == subadr) {
								if (test_start(TEST_BUSY1)) {
									if (err_msg == NULL)
#ifdef ENG
										err_msg = "'BC access to locked subaddress' RAM arbiter test didn't pass";
#else
										err_msg = "Не прошел тест арбитра БЗУ 'Обращение КК в блокированный подадрес'";
#endif
									goto fin;
								}
							} else {
								if (test_start(TEST_NOBUSY1)) {
									if (err_msg == NULL)
#ifdef ENG
										err_msg = "'BC access to unlocked subaddress' RAM arbiter test didn't pass";
#else
										err_msg = "Не прошел тест арбитра БЗУ 'Обращение КК в неблокированный подадрес'";
#endif
									goto fin;
								}
							}
						}
						if (dir0 == RT_TRANSMIT) {
							break;
						}
					}
					select_window(rtwin);
					if (dir == RT_RECEIVE)
						if (rtgetw(0) != 0x0000) {
#ifdef ENG
							err_msg = "'BC access to locked subaddress' RAM arbiter test didn't pass";
#else
							err_msg = "Не прошел тест арбитра БЗУ 'Обращение КК в блокированный подадрес'";
#endif
							goto fin;
						}
					rtunlock();
					fLock = 0;
					select_window(bcwin);
					dir0 = dir;
					subadr0 = subadr;
					ctrl_code[bcnum][base] = (dir0 == RT_TRANSMIT) ? DATA_RT_BC : DATA_BC_RT;
					bcputw(0, (rtadr << 11) | dir0 | (subadr0 << 5) | 1);
					bcputw(1, 0xFFFF);
					bcputw(2, 0xFFFF);
					if (test_start(TEST_NOBUSY1)) {
						if (err_msg == NULL)
#ifdef ENG
							err_msg = "'BC access to unlocked subaddress' RAM arbiter test didn't pass";
#else
							err_msg = "Не прошел тест арбитра БЗУ 'Обращение КК в разблокированный подадрес'";
#endif
						goto fin;
					}
					select_window(rtwin);
					for (subadr0 = subadr1; subadr0 <= subadr2; subadr0++) {
						rtdefsubaddr(RT_RECEIVE, subadr0);
						if (rtgetw(0) != 0xFFFF) {
#ifdef ENG
							err_msg = "'BC access to unlocked subaddress' RAM arbiter test didn't pass";
#else
							err_msg = "Не прошел тест арбитра БЗУ 'Обращение КК в разблокированный подадрес'";
#endif
							goto fin;
						}
					}
					if (subadr == 0x18) {
						break;
					}
					if (subadr == 0x10) {
						subadr = 0x11;
					} else {
						subadr = ((subadr & 0x0F) << 1) | (subadr & 0x10);
					}
				}
				if (dir == RT_TRANSMIT) {
					break;
				}
			}
			/* Проверка запрета доступа к БЗУ при обмене с КК */
			if (test_rt_busy()) {
#ifdef ENG
				err_msg = "Subaddress access disable bit didn't set during BC access";
#else
				err_msg = "Не устанавливается бит запрета доступа к подадресу при обмене с КК";
#endif
				goto fin;
			}
		}
		avt_out = 1;
		/* Тестирование счетчика ИС */
		select_window(rtwin);
		rtdefsubaddr(dir = RT_TRANSMIT, subadr = 0x10);
		for (k = 0; k <= 31; ++k) {
			buffer[k] = k + 1;
		}
		rtputblk(0, buffer, 32);
		select_window(bcwin);
		ctrl_code[bcnum][base] = DATA_RT_BC;
		for (len = 1; len <= 32; len++) {
			bcputw(0, (rtadr << 11) | RT_TRANSMIT | (subadr << 5) | (len & 0x1F));
			if (test_start(TEST_COUNTER)) {
				if (err_msg == NULL)
#ifdef ENG
					err_msg = "Data counter test didn't pass";
#else
					err_msg = "Не прошел тест счетчика ИС";
#endif
				goto fin;
			}
		}
		select_window(rtwin);
		if (rtdefirqmode(RT_GENER1_BL | RT_GENER2_BL) == 0) {
#ifdef _TMK1553B_LINUX
			CheckTmkEvent(0);
#endif
			int_num[rtnum] = 0;
			select_window(bcwin);
			ctrl_code[bcnum][base] = DATA_BC_RT;
			bcputw(0, (rtadr << 11) | RT_RECEIVE | (subadr << 5));
			if (test_start(TEST_RT_DATA_IRQ)) {
				if (err_msg == NULL)
#ifdef ENG
					err_msg = "RT data receiving interrupt test didn't pass";
#else
					err_msg = "Не прошел тест прерывания ОУ при приеме данных";
#endif
				goto fin;
			}
			select_window(rtwin);
			rtdefirqmode(RT_GENER1_BL | RT_GENER2_BL | RT_DATA_BL);
#ifdef _TMK1553B_LINUX
			CheckTmkEvent(0);
#endif
			int_num[rtnum] = 0;
			select_window(bcwin);
			if (test_start(TEST_RT_DATA_IRQ_BL)) {
				if (err_msg == NULL)
#ifdef ENG
					err_msg = "RT data receiving interrupt mask test didn't pass";
#else
					err_msg = "Не прошел тест блокировки прерывания ОУ при приеме данных";
#endif
				goto fin;
			}
			select_window(rtwin);
			rtdefirqmode(RT_GENER1_BL | RT_GENER2_BL);
#ifdef _TMK1553B_LINUX
			CheckTmkEvent(0);
#endif
			int_num[rtnum] = 0;
			select_window(bcwin);
			ctrl_code[bcnum][base] = DATA_RT_BC;
			bcputw(0, (rtadr << 11) | RT_TRANSMIT | (subadr << 5));
			if (test_start(TEST_RT_DATA_IRQ)) {
				if (err_msg == NULL)
#ifdef ENG
					err_msg = "RT data transmitting interrupt test didn't pass";
#else
					err_msg = "Не прошел тест прерывания ОУ при передаче данных";
#endif
				goto fin;
			}
			select_window(rtwin);
			rtdefirqmode(RT_GENER1_BL | RT_GENER2_BL | RT_DATA_BL);
#ifdef _TMK1553B_LINUX
			CheckTmkEvent(0);
#endif
			int_num[rtnum] = 0;
			select_window(bcwin);
			if (test_start(TEST_RT_DATA_IRQ_BL)) {
				if (err_msg == NULL)
#ifdef ENG
					err_msg = "RT data transmitting interrupt mask test didn't pass";
#else
					err_msg = "Не прошел тест блокировки прерывания ОУ при прередаче данных";
#endif
				goto fin;
			}
		}
		/* Тест гр.ОУ-ОУ */
		select_window(rtwin);
		rtdefsubaddr(dir = RT_TRANSMIT, subadr = 0x10);
		make_test1(0);
		rtputblk(0, buffer, 32);
		select_window(bcwin);
		bcdefbase(base = 3);
		ctrl_code[bcnum][base] = DATA_RT_RT_BRCST;
		bcputw(0, (0x1F << 11) | RT_RECEIVE | (subadr << 5));
		bcputw(1, (rtadr << 11) | RT_TRANSMIT | (subadr << 5));
		if (start_loop_n(GL_COUNTER)) {
#ifdef ENG
			err_msg = "Broadcast RT->RT message test didn't pass";
#else
			err_msg = "Не прошел тест формата ОУ->ОУ в групповом режиме";
#endif
			goto fin;
		}
		select_window(rtwin);
		rtgetblk(0, buffer, 32);
		select_window(bcwin);
		if (cmp_ram_buf(0, 3, 32)) {
#ifdef ENG
			err_msg = "Test array in RT->RT message transmitted with errors";
#else
			err_msg = "Тестовый массив в формате ОУ->ОУ в групповом режиме передан с ошибкой";
#endif
			goto fin;
		}

#ifdef _TMK1553B_LINUX
		CheckTmkEvent(0);
#endif
		int_num[rtnum] = 0;
		select_window(rtwin);

//  rtdefirqmode(RT_DATA_BL); //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

		wr_at(rtadr);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_rt_bc(0x10, 0, 0xAAAA)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_rt_bc(0x12, 2, 0x0000)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_rt_bc(0x14, 4, 0x5555)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_rt_bc(0x16, 6, 0xFFFF)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_rt_bc(0x18, 8, 0x8000)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_rt_bc(0x1A, 10, 0x7FFF)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_bc_rt(12, 0x10, 0xAAAA)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_bc_rt(14, 0x12, 0x0000)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_bc_rt(16, 0x14, 0x5555)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_bc_rt(18, 0x16, 0xFFFF)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_bc_rt(20, 0x18, 0x8000)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_bc_rt(22, 0x1A, 0x7FFF)) {
			goto bad_statistic;
		}
		if (channel_err > 0L) {
			goto bad_statistic;
		}
		select_window(bcwin);
		if (fReserv && (times == nBusSwapTime || times == 1)) {
			_UINT16 nTempBus;
			bcdefbus(nReservBus);
			nTempBus = nMainBus;
			nMainBus = nReservBus;
			nReservBus = nTempBus;
			bus_num = nMainBus;
			out_number();
#ifdef _TMK1553B_LINUX
			fflush(stdout);
#endif
		}
	} while (--times);
#ifdef ELCUS
	if (fBcRt && *str) {
		if (randbcrt(bcnum, rtnum, fReserv, hb_mode)) {
#ifdef ENG
			err_msg = "Randbcrt test didn't pass";
#else
			err_msg = "Не прошел тест Randbcrt";
#endif
			goto fin;
		}
	}
#endif //def ELCUS
	/* Конец теста */
	avt_ok = 1;
#ifdef ENG
	err_msg = "Remote Terminal test completed";
#else
	err_msg = "Тест оконечного устройства завершен";
#endif
	goto fin;
bad_statistic:
#ifdef ENG
	err_msg = "'Statistics' test didn't pass";
#else
	err_msg = "Не прошел тест 'Накопление статистики'";
#endif
fin:
	if (prtscr_flag) {
		prtscr_flag = 0;
#ifdef ENG
		err_msg = "Test aborted on PrtScr";
#else
		err_msg = "Тест прерван по PrtScr";
#endif
	}
	select_window(nmainwin = rtwin);
	check_int();
	all_screen();
	beep1();
	if (avt_ok) {
		textcolor(GREEN);
	} else {
		textcolor(LIGHTRED);
	}
	comment(err_msg);
	textcolor(LIGHTGRAY);
	select_window(bcwin);
	statistic_out("");
	select_window(nmainwin = rtwin);
	if (channel_err > 0L) {
		statistic_err_out();
	}
	avt_ctrl("0");
	avt_err = !avt_ok;
}

_INT16 statistic_bc_rt(_INT16 st_base, _INT16 st_subadr, _INT16 st_data)
{
	long cnt;
	long ltime1;
	_INT16 k;
	_INT16 st_bcsubadr = st_subadr;
	_INT16 buf_base = 0, buf_subadr = 0;

	select_window(bcwin);
	gotoxy(1, 11);
	textcolor(CYAN);
#ifdef ENG
	cputs("Statistics accumulation. Press PrtScr to abort...");
#else
	cputs("Накопление статистики. Нажмите PrtScr для выхода...");
#endif
	textcolor(LIGHTGRAY);
	base = st_base;
	out_base();
	ctrl_code[bcnum][st_base] = DATA_BC_RT;
	ctrl_code[bcnum][st_base ^ 1] = DATA_BC_RT;
	out_ctrl_code();
	for (k = 1; k <= 32; ++k) {
		buffer[k] = st_data;
	}
	for (k = 0; k <= 32; ++k) {
		bcbuffer[0][k] = (st_base << 8) + k;
		bcbuffer[1][k] = ((st_base ^ 1) << 8) + k;
		if (k >= 32) {
			continue;
		}
		rtbuffer[0][k] = (st_subadr << 8) + k;
		rtbuffer[1][k] = ((st_subadr ^ 1) << 8) + k;
	}
	bcdefbase(st_base);
	buffer[0] = (rtadr) << 11 | RT_RECEIVE | (st_bcsubadr << 5);
	bcputblk(0, buffer, 33);
	out_buffer();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	for (cnt = 0; cnt <= GL_COUNTER; ++cnt) {
#ifdef _TMK1553B_LINUX
		bioskey_f12();
#endif
		if (prtscr_flag) {
			cnt = GL_COUNTER;
		}
		if (cnt < GL_COUNTER) {
			tmkselect(bcnum);
#ifdef _TMK1553B_LINUX
			CheckTmkEvent(0);
#endif
			int_num[bcnum] = 0;
			bcstart(st_base, DATA_BC_RT);
			ltime1 = biostime(0, 0L);
		}
		st_base ^= 1;
		buf_base ^= 1;
		st_subadr ^= 1;
		buf_subadr ^= 1;
		if (cnt < GL_COUNTER) {
			bcdefbase(st_base);
			buffer[0] = (rtadr) << 11 | RT_RECEIVE | (st_subadr << 5);
			bcputblk(0, bcbuffer[buf_base], 33);
			bcputblk(0, buffer, 33);
		}
//  else
//   --good_starts;
		if (cnt > 0) {
			tmkselect(rtnum);
			rtdefsubaddr(RT_RECEIVE, st_subadr);
			rtgetblk(0, vbuffer, 32);
			rtputblk(0, rtbuffer[buf_subadr], 32);
			for (k = 0; k <= 31; ++k) {
				if (vbuffer[k] != (_UINT16)st_data) {
#ifdef LPT_SYN
					if (fLPTSyn) {
						set_lpt_data(0xFF);
						set_lpt_data(0);
					}
#endif
					if (channel_err < (unsigned long)ERRLEN) {
						at_bad_mode[(unsigned)channel_err] = DATA_BC_RT;
						at_bad_base[(unsigned)channel_err] = st_base ^ 1;
						at_bad_sa[(unsigned)channel_err] = st_subadr;
						at_bad_num[(unsigned)channel_err] = k;
						at_bad_good[(unsigned)channel_err] = st_data;
						at_bad_bad[(unsigned)channel_err] = vbuffer[k];
					}
					++channel_err;
					if (fStatErrStop) {
						cnt = GL_COUNTER;
					}
				}
			}
			if (cnt == 1) {
				select_window(rtwin);
				out_subadr();
				out_buffer();
				select_window(bcwin);
#ifdef _TMK1553B_LINUX
				fflush(stdout);
#endif
			}
//   st_subadr ^= 1;
		}
#ifdef _TMK1553B_LINUX
		CheckTmkEvent(0);
#endif
		while (biostime(0, 0L) - ltime1 < WAITDLY) {
#ifdef _TMK1553B_LINUX
			CheckTmkEvent(0);
#endif
			if (int_num[bcnum]) {
				break;
			}
		}
		if (!int_num[bcnum] && cnt < GL_COUNTER) {
			++to_errors;
		}
	}
	gotoxy(1, 11);
	if (nwin == nmainwin) {
		textcolor(WHITE);
		cputs("════════════════════════════════════════════════════════════════════════════");
	} else {
		cputs("────────────────────────────────────────────────────────────────────────────");
	}
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	textcolor(LIGHTGRAY);
	statistic_out("");
	return (bad_starts > 0 || int_num[rtnum] || prtscr_flag || (dwMaxErr && channel_err >= dwMaxErr));
}

_INT16 statistic_rt_bc(_INT16 st_subadr, _INT16 st_base, _INT16 st_data)
{
	long cnt;
	long ltime1;
	_INT16 k;
	_INT16 buf_base = 0, buf_subadr = 0;

	select_window(rtwin);
	for (k = 0; k <= 31; ++k) {
		vbuffer[k] = st_data;
	}
	for (k = 0; k <= 33; ++k) {
		bcbuffer[0][k] = (st_base << 8) + k;
		bcbuffer[1][k] = ((st_base ^ 1) << 8) + k;
		if (k >= 32) {
			continue;
		}
		rtbuffer[0][k] = (st_subadr << 8) + k;
		rtbuffer[1][k] = ((st_subadr ^ 1) << 8) + k;
	}
	dir = RT_TRANSMIT;
	subadr = st_subadr;
	out_subadr();
	rtdefsubaddr(RT_TRANSMIT, st_subadr);
	rtputblk(0, vbuffer, 32);
	out_buffer();
	select_window(bcwin);
	gotoxy(1, 11);
	textcolor(CYAN);
#ifdef ENG
	cputs("Statistics accumulation. Press PrtScr to abort...");
#else
	cputs("Накопление статистики. Нажмите PrtScr для выхода...");
#endif
	textcolor(LIGHTGRAY);
	base = st_base;
	out_base();
	ctrl_code[bcnum][st_base] = DATA_RT_BC;
	ctrl_code[bcnum][st_base ^ 1] = DATA_RT_BC;
	out_ctrl_code();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	bcdefbase(st_base);
	bcputw(0, (rtadr) << 11 | RT_TRANSMIT | (st_subadr << 5));
	bcbuffer[0][0] = (rtadr) << 11 | RT_TRANSMIT | (st_subadr << 5);
	bcbuffer[1][0] = (rtadr) << 11 | RT_TRANSMIT | ((st_subadr ^ 1) << 5);
	for (cnt = 0; cnt <= GL_COUNTER; ++cnt) {
#ifdef _TMK1553B_LINUX
		bioskey_f12();
#endif
		if (prtscr_flag) {
			cnt = GL_COUNTER;
		}
		if (cnt < GL_COUNTER) {
#ifdef _TMK1553B_LINUX
			CheckTmkEvent(0);
#endif
			int_num[bcnum] = 0;
			bcstart(st_base, DATA_RT_BC);
			ltime1 = biostime(0, 0L);
		}
//  else
//   --good_starts;
		st_base ^= 1;
		buf_base ^= 1;
		bcdefbase(st_base);
		if (cnt > 0) {
			bcgetblk(2, buffer, 32);
			if (cnt == 1) {
				out_buffer();
			}
#ifdef _TMK1553B_LINUX
			fflush(stdout);
#endif
		}
		bcputblk(0, bcbuffer[buf_base], 34);
		tmkselect(rtnum);
		st_subadr ^= 1;
		buf_subadr ^= 1;
		rtdefsubaddr(RT_TRANSMIT, st_subadr);
		rtputblk(0, rtbuffer[buf_subadr], 32);
		rtputblk(0, vbuffer, 32);
		tmkselect(bcnum);
		if (cnt > 0) {
			for (k = 0; k <= 31; ++k) {
				if (buffer[k] != (_UINT16)st_data) {
#ifdef LPT_SYN
					if (fLPTSyn) {
						set_lpt_data(0xFF);
						set_lpt_data(0);
					}
#endif
					if (channel_err < (unsigned long)ERRLEN) {
						at_bad_mode[(unsigned)channel_err] = DATA_RT_BC;
						at_bad_base[(unsigned)channel_err] = st_base;
						at_bad_sa[(unsigned)channel_err] = st_subadr;
						at_bad_num[(unsigned)channel_err] = k;
						at_bad_good[(unsigned)channel_err] = st_data;
						at_bad_bad[(unsigned)channel_err] = buffer[k];
					}
					++channel_err;
					if (fStatErrStop) {
						cnt = GL_COUNTER;
					}
				}
			}
		}
#ifdef _TMK1553B_LINUX
		CheckTmkEvent(0);
#endif
		while (biostime(0, 0L) - ltime1 < WAITDLY) {
#ifdef _TMK1553B_LINUX
			CheckTmkEvent(0);
#endif
			if (int_num[bcnum]) {
				break;
			}
		}
		if (!int_num[bcnum] && cnt < GL_COUNTER) {
			++to_errors;
		}
	}
	gotoxy(1, 11);
	if (nwin == nmainwin) {
		textcolor(WHITE);
		cputs("════════════════════════════════════════════════════════════════════════════");
	} else {
		cputs("────────────────────────────────────────────────────────────────────────────");
	}
	textcolor(LIGHTGRAY);
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	statistic_out("");
	return (bad_starts > 0 || int_num[rtnum] || prtscr_flag || (dwMaxErr && channel_err >= dwMaxErr));
}

void avt_stat(char *str)                  /* Тест "Накопление статистики" */
{
	unsigned long times, nBusSwapTime;

	while (*str == '\t' || *str == ' ') {
		str++;
	}
	if (*str) {
#ifdef _TMK1553B_DOS
		if (sscanf(str, "%U", &times) != 1)
#endif
#ifdef _TMK1553B_LINUX
			if (sscanf(str, "%lu", &times) != 1)
#endif
			{
				msg_out(inp_err);
				return;
			}
	} else {
		times = 1;
	}
	if (Window[0].nMode == BC_MODE) {
		bcwin = 0;
	} else if (Window[1].nMode == BC_MODE) {
		bcwin = 1;
	} else {
#ifdef ENG
		msg_out("BC not defined");
#else
		msg_out("Не задан КК");
#endif
		return;
	}
	if (Window[0].nMode == RT_MODE) {
		rtwin = 0;
	} else if (Window[1].nMode == RT_MODE) {
		rtwin = 1;
	} else {
#ifdef ENG
		msg_out("RT not defined");
#else
		msg_out("Не задано ОУ");
#endif
		return;
	}
	bcnum = Window[bcwin].nTMK;
	rtnum = Window[rtwin].nTMK;
	avt_ctrl("1");
	avt_ok = 0;
	select_window(bcwin);
	if (fReserv) {
		nReservBus = ((nMainBus = bcgetbus()) == BUS_A) ? BUS_B : BUS_A;
		bus_num = nMainBus;
		out_number();
		nBusSwapTime = times + 1;
		times <<= 1;
	}
	select_window(rtwin);
	wr_at(rtadr = 0x0A);
	out_at();
	set_status_word("0"); /* сбросить все биты ОС */
#ifdef ENG
	comment("'Statistics' automatic test.");
#else
	comment("Автоматический тест 'Накопление статистики'.");
#endif
	select_window(bcwin);
	statistic_clear("");
	if (hb_mode) {
		subadr1 = 0x10;
		subadr2 = 0x1E;
		ci_field = 0x1F << 5;
	} else {
		subadr1 = 0x01;
		subadr2 = 0x1E;
		ci_field = 0;
	}
	do {
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_rt_bc(0x10, 0, 0xAAAA)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_rt_bc(0x12, 2, 0x0000)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_rt_bc(0x14, 4, 0x5555)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_rt_bc(0x16, 6, 0xFFFF)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_rt_bc(0x18, 8, 0x8000)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_rt_bc(0x1A, 10, 0x7FFF)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_bc_rt(12, 0x10, 0xAAAA)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_bc_rt(14, 0x12, 0x0000)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_bc_rt(16, 0x14, 0x5555)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_bc_rt(18, 0x16, 0xFFFF)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_bc_rt(20, 0x18, 0x8000)) {
			goto bad_statistic;
		}
		select_window(rtwin);
		rt_fill_test();
		select_window(bcwin);
		bc_fill_test();
		if (statistic_bc_rt(22, 0x1A, 0x7FFF)) {
			goto bad_statistic;
		}
		if (channel_err > 0L) {
			goto bad_statistic;
		}
		select_window(bcwin);
		if (fReserv && (times == nBusSwapTime || times == 1)) {
			_UINT16 nTempBus;
			bcdefbus(nReservBus);
			nTempBus = nMainBus;
			nMainBus = nReservBus;
			nReservBus = nTempBus;
			bus_num = nMainBus;
			out_number();
#ifdef _TMK1553B_LINUX
			fflush(stdout);
#endif
		}
	} while (--times);
	/* Конец теста */
	avt_ok = 1;
#ifdef ENG
	err_msg = "'Statistics' test completed";
#else
	err_msg = "Тест 'Накопление статистики' завершен";
#endif
	goto fin;
bad_statistic:
#ifdef ENG
	err_msg = "'Statistics' test didn't pass";
#else
	err_msg = "Не прошел тест 'Накопление статистики'";
#endif
fin:
	if (prtscr_flag) {
		prtscr_flag = 0;
#ifdef ENG
		err_msg = "Test aborted on PrtScr";
#else
		err_msg = "Тест прерван по PrtScr";
#endif
	}
	check_int();
	select_window(nmainwin = bcwin);
	all_screen();
	select_window(rtwin);
	beep1();
	if (avt_ok) {
		textcolor(GREEN);
	} else {
		textcolor(LIGHTRED);
	}
	comment(err_msg);
	textcolor(LIGHTGRAY);
	select_window(nmainwin = bcwin);
	statistic_out("");
	if (channel_err > 0L) {
		statistic_err_out();
	}
	avt_ctrl("0");
	avt_err = !avt_ok;
}

void avt_flags(char *str)            /* поддержка проверки флагов ОУ */
{
	_INT16 page;
	_UINT16 tsubadr;

	if (end_ctrl(str)) {
		return;
	}
	if (Window[0].nMode == RT_MODE) {
		rtwin = 0;
	} else if (Window[1].nMode == RT_MODE) {
		rtwin = 1;
	} else {
#ifdef ENG
		msg_out("RT not defined");
#else
		msg_out("Не задано ОУ");
#endif
		return;
	}
	if (Window[0].nMode == BC_MODE) {
		bcwin = 0;
	} else if (Window[1].nMode == BC_MODE) {
		bcwin = 1;
	} else {
#ifdef ENG
		msg_out("BC not defined");
#else
		msg_out("Не задан КК");
#endif
		return;
	}
	bcnum = Window[bcwin].nTMK;
	rtnum = Window[rtwin].nTMK;
	statistic_clear("");
	avt_ctrl("1");
	avt_ok = 0;
	select_window(rtwin);
	rtreset();
// rt_clear_ram();
#ifdef ENG
	comment("Remote Terminal flag mode automatic test.");
#else
	comment("Автоматический тест флагового режима оконечного устройства.");
#endif
	wr_at(rtadr = 0x0A);
	out_at();
	set_status_word("0"); /* сбросить все биты ОС */
	rtdefmode(rtgetmode() | RT_FLAG_MODE);
	rtdefpage(page = 0);
	if (hb_mode) {
		subadr1 = 0x10;
		subadr2 = 0x1E;
		ci_field = CI_MASK;
	} else {
		subadr1 = 0x01;
		subadr2 = 0x1E;
		ci_field = 0;
	}
	for (subadr = subadr1; subadr <= subadr2; subadr++) {
		rtdefsubaddr(RT_TRANSMIT, subadr);
		buffer[0] = (page << 14) | (subadr << 8) | (page << 6) | subadr | 0x2020;
		rtputblk(0, buffer, 1);
		rtsetflag();
	}
	select_window(bcwin);
	bcreset();
	bc_clear_ram();
	bcdefbase(base = 0);
	ctrl_code[bcnum][base] = DATA_RT_BC;
	for (subadr = subadr1; subadr <= subadr2; subadr++) {
		bcputw(0, (rtadr << 11) | RT_TRANSMIT | (subadr << 5) | 1);
		start_1("");
		if (int_num[bcnum] != 1) {
#ifdef ENG
			err_msg = "RT->BC:Transmitting from subaddress with set flag didn't pass";
#else
			err_msg = "ОУ->КК:Не прошла передача из подадреса с установленным флагом";
#endif
			goto fin;
		}
		select_window(rtwin);
		for (tsubadr = subadr1; tsubadr < subadr; tsubadr++) {
			if ((rtgetflag(RT_TRANSMIT, tsubadr)&RT_FLAG) != 0) {
#ifdef ENG
				err_msg = "RT->BC:Earlier transmitted subaddress flag set";
#else
				err_msg = "ОУ->КК:Установился флаг для ранее переданного подадреса";
#endif
				goto fin;
			}
		}
		if ((rtgetflag(RT_TRANSMIT, subadr)&RT_FLAG) != 0) {
#ifdef ENG
			err_msg = "RT->BC:Transmitted subaddress flag didn't reset";
#else
			err_msg = "ОУ->КК:Не сбросился флаг для переданного подадреса";
#endif
			goto fin;
		}
		for (tsubadr = subadr + 1; tsubadr < subadr2; tsubadr++) {
			if ((rtgetflag(RT_TRANSMIT, tsubadr)&RT_FLAG) == 0) {
#ifdef ENG
				err_msg = "RT->BC:Not yet transmitted subaddress flag reset";
#else
				err_msg = "ОУ->КК:Сбросился флаг для еще не переданного подадреса";
#endif
				goto fin;
			}
		}
		select_window(bcwin);
	}
	bcdefbase(base = 0);
	ctrl_code[bcnum][base] = DATA_RT_BC;
	for (subadr = subadr1; subadr <= subadr2; subadr++) {
		bcputw(0, (rtadr << 11) | RT_TRANSMIT | (subadr << 5) | 1);
		start_1("");
		if (int_num[bcnum] != 2 || bc_sw != (TO_MASK | IB_MASK)) {
#ifdef ENG
			err_msg = "RT->BC:Transmitting from subaddress with reset flag test didn't pass";
#else
			err_msg = "ОУ->КК:Не прошел тест передачи из подадреса со сброшенным флагом";
#endif
			goto fin;
		}
		select_window(rtwin);
		for (tsubadr = subadr1; tsubadr <= subadr2; tsubadr++) {
			if ((rtgetflag(RT_TRANSMIT, tsubadr)&RT_FLAG) != 0) {
#ifdef ENG
				err_msg = "RT->BC:Earlier transmitted subaddress flag set";
#else
				err_msg = "ОУ->КК:Установился флаг для ранее переданного подадреса";
#endif
				goto fin;
			}
		}
		select_window(bcwin);
	}
	select_window(rtwin);
	rt_clear_ram();
	for (subadr = subadr1; subadr <= subadr2; subadr++) {
		rtdefsubaddr(RT_RECEIVE, subadr);
		rtclrflag();
	}
	select_window(bcwin);
	bcdefbase(base = 1);
	ctrl_code[bcnum][base] = DATA_BC_RT;
	for (subadr = subadr1; subadr <= subadr2; subadr++) {
		buffer[0] = (rtadr << 11) | RT_RECEIVE | (subadr << 5) | 1;
		buffer[1] = (page << 14) | (subadr << 8) | (page << 6) | subadr;
		bcputblk(0, buffer, 2);
		start_1("");
		if (int_num[bcnum] != 1) {
#ifdef ENG
			err_msg = "BC->RT:Receiving to subaddress with reset flag didn't pass";
#else
			err_msg = "КК->ОУ:Не прошел прием в подадрес со сброшенным флагом";
#endif
			goto fin;
		}
		select_window(rtwin);
		for (tsubadr = subadr1; tsubadr < subadr; tsubadr++) {
			if ((rtgetflag(RT_RECEIVE, tsubadr)&RT_FLAG) == 0) {
#ifdef ENG
				err_msg = "BC->RT:Earlier received subaddress flag reset";
#else
				err_msg = "КК->ОУ:Сбросился флаг для ранее принятого подадреса";
#endif
				goto fin;
			}
		}
		if ((rtgetflag(RT_RECEIVE, subadr)&RT_FLAG) == 0) {
#ifdef ENG
			err_msg = "BC->RT:Received subaddress flag didn't set";
#else
			err_msg = "КК->ОУ:Не установился флаг для принятого подадреса";
#endif
			goto fin;
		}
		if (rtgetw(0) != ((page << 14) | (subadr << 8) | (page << 6) | subadr)) {
#ifdef ENG
			err_msg = "BC->RT:Data in Rx subaddress didn't change";
#else
			err_msg = "КК->ОУ:Данные в подадресе приема не изменились";
#endif
			goto fin;
		}
		for (tsubadr = subadr + 1; tsubadr < subadr2; tsubadr++) {
			if ((rtgetflag(RT_RECEIVE, tsubadr)&RT_FLAG) != 0) {
#ifdef ENG
				err_msg = "BC->RT:Not yet received subaddress flag set";
#else
				err_msg = "КК->ОУ:Установился флаг для еще не принятого подадреса";
#endif
				goto fin;
			}
		}
		select_window(bcwin);
	}
	ctrl_code[bcnum][base] = DATA_BC_RT;
	for (subadr = subadr1; subadr <= subadr2; subadr++) {
		buffer[0] = (rtadr << 11) | RT_RECEIVE | (subadr << 5) | 1;
		buffer[1] = 0xFFFF;
		bcputblk(0, buffer, 2);
		start_1("");
		if (int_num[bcnum] != 2 || bc_sw != IB_MASK) {
#ifdef ENG
			err_msg = "BC->RT:Receiving to subaddress with set flag test didn't pass";
#else
			err_msg = "КК->ОУ:Не прошел тест приема в подадрес с установленным флагом";
#endif
			goto fin;
		}
		select_window(rtwin);
		rtdefsubaddr(RT_RECEIVE, subadr);
		if (rtgetw(0) == 0xFFFF) {
#ifdef ENG
			err_msg = "BC->RT:Data were written in Rx subaddress with set flag";
#else
			err_msg = "КК->ОУ:При приеме в подадрес с установленным флагом записались данные";
#endif
			goto fin;
		}
		for (tsubadr = subadr1; tsubadr <= subadr2; tsubadr++) {
			if ((rtgetflag(RT_RECEIVE, tsubadr)&RT_FLAG) == 0) {
#ifdef ENG
				err_msg = "BC->RT:Earlier received subaddress flag reset";
#else
				err_msg = "КК->ОУ:Сбросился флаг для ранее принятого подадреса";
#endif
				goto fin;
			}
		}
		select_window(bcwin);
	}
	/* Конец теста */
	avt_ok = 1;
#ifdef ENG
	err_msg = "Remote Terminal flag mode test completed";
#else
	err_msg = "Тест флагового режима оконечного устройства завершен";
#endif
fin:
	/*
	 if (prtscr_flag)
	 {
	  prtscr_flag = 0;
	  err_msg = "Тест прерван по PrtScr";
	 }
	*/
	select_window(rtwin);
	rtdefmode(rtgetmode() & (~RT_FLAG_MODE));
	check_int();
	all_screen();
	beep1();
	if (avt_ok) {
		textcolor(GREEN);
	} else {
		textcolor(LIGHTRED);
	}
	comment(err_msg);
	textcolor(LIGHTGRAY);
	select_window(bcwin);
	statistic_out("");
	select_window(nmainwin = rtwin);
	avt_ctrl("0");
	avt_err = !avt_ok;
}

#ifdef ELCUS
void avt_va996(char *str)        /* поддержка проверки приемопередатчика */
{
	_INT16 i;
	long ltime1, lmaxtime;
	unsigned minutes;

// if (end_ctrl(str)) return;
	while (*str == '\t' || *str == ' ') {
		str++;
	}
	if (*str) {
#ifdef _TMK1553B_DOS
		if (sscanf(str, "%U", &minutes) != 1)
#endif
#ifdef _TMK1553B_LINUX
			if (sscanf(str, "%u", &minutes) != 1)
#endif
			{
				msg_out(inp_err);
				return;
			}
	} else {
		minutes = 1;
	}
	lmaxtime = 182L * 6L * (long)minutes;
	if (Window[0].nMode == BC_MODE) {
		bcwin = 0;
	} else if (Window[1].nMode == BC_MODE) {
		bcwin = 1;
	} else {
#ifdef ENG
		msg_out("BC not defined");
#else
		msg_out("Не задан КК");
#endif
		return;
	}
	if (Window[0].nMode == RT_MODE) {
		rtwin = 0;
	} else if (Window[1].nMode == RT_MODE) {
		rtwin = 1;
	} else {
#ifdef ENG
		msg_out("RT not defined");
#else
		msg_out("Не задано ОУ");
#endif
		return;
	}
	bcnum = Window[bcwin].nTMK;
	rtnum = Window[rtwin].nTMK;
	avt_ctrl("1");
	avt_ok = 0;
	select_window(rtwin);
	wr_at(rtadr = 0x0A);
	out_at();
	set_status_word("0"); /* сбросить все биты ОС */
	select_window(rtwin);
#ifdef ENG
	comment("Transceiver automatic test.");
#else
	comment("Автоматический тест приемопередатчика.");
#endif
	select_window(bcwin);
	statistic_clear("");
	if (hb_mode) {
		subadr1 = 0x10;
		subadr2 = 0x1E;
		ci_field = 0x1F << 5;
	} else {
		subadr1 = 0x01;
		subadr2 = 0x1E;
		ci_field = 0;
	}
	/* уст. кода управления */
	/* занесение констант в базы */
	set_base("0");
	buf_fill("1, 32, AAAA");
	ctrl_code[tmkselected()][base] = DATA_BC_RT;
	bcputw(0, (rtadr << 11) | RT_RECEIVE | (0x11 << 5));
	set_base("1");
	buf_fill("1, 32, 0000");
	ctrl_code[tmkselected()][base] = DATA_BC_RT;
	bcputw(0, (rtadr << 11) | RT_RECEIVE | (0x11 << 5));
	set_base("2");
	buf_fill("1, 32, 5555");
	ctrl_code[tmkselected()][base] = DATA_BC_RT;
	bcputw(0, (rtadr << 11) | RT_RECEIVE | (0x11 << 5));
	set_base("3");
	buf_fill("1, 32, FFFF");
	ctrl_code[tmkselected()][base] = DATA_BC_RT;
	bcputw(0, (rtadr << 11) | RT_RECEIVE | (0x11 << 5));
	set_base("4");
	buf_fill("1, 32, 8000");
	ctrl_code[tmkselected()][base] = DATA_BC_RT;
	bcputw(0, (rtadr << 11) | RT_RECEIVE | (0x11 << 5));
	set_base("5");
	buf_fill("1, 32, 7FFF");
	ctrl_code[tmkselected()][base] = DATA_BC_RT;
	bcputw(0, (rtadr << 11) | RT_RECEIVE | (0x11 << 5));
	ltime1 = biostime(0, 0L);
	prtscr_flag = 0;
	while ((biostime(0, 0L) - ltime1) < (unsigned long)lmaxtime && !prtscr_flag && bad_starts < 100L) {
		for (i = 0; i < GL_COUNTER && !prtscr_flag;) {
			for (base = 0; base <= 5; base++) {
#ifdef _TMK1553B_LINUX
				CheckTmkEvent(0);
#endif
				int_num[bcnum] = 0;
				bcstart(base, ctrl_code[tmkselected()][base]);
				while (!int_num[bcnum] && !prtscr_flag) {
#ifdef _TMK1553B_LINUX
					CheckTmkEvent(0);
					bioskey_f12();
#endif
				}
				i++;
			}
#ifdef _TMK1553B_LINUX
			bioskey_f12();
#endif
		}
		statistic_out("");
	}
	set_base("0");
	out_base();
	out_ctrl_code();
	bc_out_ram(base);
	/* Конец теста */
	if (bad_starts >= 100L) {
#ifdef ENG
		err_msg = "Transceiver test didn't pass";
#else
		err_msg = "Тест приемопередатчика не прошел";
#endif
	} else if (prtscr_flag) {
#ifdef ENG
		err_msg = "Test aborted on PrtScr";
#else
		err_msg = "Тест прерван по PrtScr";
#endif
	} else {
		avt_ok = 1;
#ifdef ENG
		err_msg = "Transceiver test completed";
#else
		err_msg = "Тест приемопередатчика завершен";
#endif
	}
	check_int();
	select_window(bcwin);
	all_screen();
	select_window(rtwin);
	beep1();
	if (avt_ok) {
		textcolor(GREEN);
	} else {
		textcolor(LIGHTRED);
	}
	comment(err_msg);
	textcolor(LIGHTGRAY);
	select_window(nmainwin = bcwin);
	statistic_out("");
	avt_ctrl("0");
	avt_err = !avt_ok;
}

void avt_arb_rt_r(char *str)  /* поддержка проверки арбитра ОУ (чтение) */
{
	_INT16 i;
	long starts = 0;
	_INT16 mode = 0;

	if (end_ctrl(str)) {
		while (*str == '\t' || *str == ' ') {
			str++;
		}
#ifdef _TMK1553B_DOS
		if (sscanf(str, "%d", &mode) != 1)
#endif
#ifdef _TMK1553B_LINUX
			if (sscanf(str, "%hd", &mode) != 1)
#endif
			{
				msg_out(mark_err);
				return;
			}
	}
	if (Window[0].nMode == BC_MODE) {
		bcwin = 0;
	} else if (Window[1].nMode == BC_MODE) {
		bcwin = 1;
	} else {
#ifdef ENG
		msg_out("BC not defined");
#else
		msg_out("Не задан КК");
#endif
		return;
	}
	if (Window[0].nMode == RT_MODE) {
		rtwin = 0;
	} else if (Window[1].nMode == RT_MODE) {
		rtwin = 1;
	} else {
#ifdef ENG
		msg_out("RT not defined");
#else
		msg_out("Не задано ОУ");
#endif
		return;
	}
	bcnum = Window[bcwin].nTMK;
	rtnum = Window[rtwin].nTMK;
	avt_ctrl("1");
	avt_ok = 0;
	select_window(rtwin);
	wr_at(rtadr = 0x0A);
	out_at();
	set_status_word("0"); /* сбросить все биты ОС */
	select_window(rtwin);
	set_subadr_r("10");
#ifdef ENG
	comment("RT arbiter automatic test (reading).");
#else
	comment("Автоматический тест арбитра ОУ (чтение).");
#endif
	select_window(bcwin);
	statistic_clear("");
	if (hb_mode) {
		subadr1 = 0x10;
		subadr2 = 0x1E;
		ci_field = 0x1F << 5;
	} else {
		subadr1 = 0x01;
		subadr2 = 0x1E;
		ci_field = 0;
	}
	/* уст. кода управления */
	/* занесение констант в базы */
	set_base("1");
	buf_fill("1, 32, FFFF");
	ctrl_code[tmkselected()][base] = DATA_BC_RT;
	bcputw(0, (rtadr << 11) | RT_RECEIVE | (0x11 << 5) | (nATWords & 0x1F));
#ifdef _TMK1553B_LINUX
	CheckTmkEvent(0);
#endif
	int_num[bcnum] = 0;
	tmkselect(rtnum);
	for (i = 0; i < 32; i++) {
		buffer[i] = (i + 1) + ((i + 1) << 8);
	}
	rtdefsubaddr(RT_RECEIVE, 0x10);
	rtputblk(0, buffer, 32);
	tmkselect(bcnum);
	prtscr_flag = 0;
	while (!prtscr_flag && (dwMaxErr == 0L || bad_starts < dwMaxErr)) {
#ifdef _TMK1553B_LINUX
		CheckTmkEvent(0);
#endif
		int_num[bcnum] = 0;
		bcstart(1, ctrl_code[tmkselected()][1]);
		tmkselect(rtnum);
#ifdef _TMK1553B_LINUX
		CheckTmkEvent(0);
#endif
		while (!int_num[bcnum] && !prtscr_flag) {
			if (fATArBlk) {
				rtgetblk(0, vbuffer, nATWords);
			}
			for (i = 0; i < nATWords; i++) {
				if (!fATArBlk) {
					vbuffer[i] = rtgetw(i);
				}
				if (vbuffer[i] != buffer[i] && !mode) {
					if (channel_err < (unsigned long)ERRLEN) {
						at_bad_mode[(unsigned)channel_err] = DATA_BC_RT;
						at_bad_base[(unsigned)channel_err] = 1;
						at_bad_sa[(unsigned)channel_err] = 0x10;
						at_bad_num[(unsigned)channel_err] = i;
						at_bad_good[(unsigned)channel_err] = buffer[i];
						at_bad_bad[(unsigned)channel_err] = vbuffer[i];
					}
					channel_err++;
#ifdef LPT_SYN
					if (fLPTSyn) {
						set_lpt_data(0xFF);
						set_lpt_data(0);
					}
#endif
				}
			}
#ifdef _TMK1553B_LINUX
			bioskey_f12();
			CheckTmkEvent(0);
#endif
		}
		tmkselect(bcnum);
		if (!fLPTSyn && ((++starts) % 100 == 0)) {
			statistic_out("");
		}
#ifdef _TMK1553B_LINUX
		bioskey_f12();
#endif
	}
	set_base("1");
	out_base();
	out_ctrl_code();
	bc_out_ram(base);
	/* Конец теста */
	if (bad_starts > 0L || channel_err > 0L) {
#ifdef ENG
		err_msg = "RT arbiter test didn't pass";
#else
		err_msg = "Тест арбитра ОУ не прошел";
#endif
	} else if (prtscr_flag) {
#ifdef ENG
		err_msg = "Test aborted on PrtScr";
#else
		err_msg = "Тест прерван по PrtScr";
#endif
	} else {
		avt_ok = 1;
#ifdef ENG
		err_msg = "RT arbiter test completed";
#else
		err_msg = "Тест арбитра ОУ завершен";
#endif
	}
	check_int();
	select_window(bcwin);
	all_screen();
	select_window(rtwin);
	beep1();
	if (avt_ok) {
		textcolor(GREEN);
	} else {
		textcolor(LIGHTRED);
	}
	comment(err_msg);
	textcolor(LIGHTGRAY);
	select_window(nmainwin = bcwin);
	statistic_out("");
	if (channel_err > 0L) {
		statistic_err_out();
	}
	avt_ctrl("0");
	avt_err = !avt_ok;
}

void avt_arb_rt_w(char *str)  /* поддержка проверки арбитра ОУ (запись) */
{
	_INT16 i;
	long starts = 0;
	_INT16 mode = 0;
	_UINT16 subadrmax;
	_UINT16 wLastCWBase;

	if (end_ctrl(str)) {
		while (*str == '\t' || *str == ' ') {
			str++;
		}
#ifdef _TMK1553B_DOS
		if (sscanf(str, "%d", &mode) != 1)
#endif
#ifdef _TMK1553B_LINUX
			if (sscanf(str, "%hd", &mode) != 1)
#endif
			{
				msg_out(mark_err);
				return;
			}
	}
	if (Window[0].nMode == BC_MODE) {
		bcwin = 0;
	} else if (Window[1].nMode == BC_MODE) {
		bcwin = 1;
	} else {
#ifdef ENG
		msg_out("BC not defined");
#else
		msg_out("Не задан КК");
#endif
		return;
	}
	if (Window[0].nMode == RT_MODE) {
		rtwin = 0;
	} else if (Window[1].nMode == RT_MODE) {
		rtwin = 1;
	} else {
#ifdef ENG
		msg_out("RT not defined");
#else
		msg_out("Не задано ОУ");
#endif
		return;
	}
	bcnum = Window[bcwin].nTMK;
	rtnum = Window[rtwin].nTMK;
	avt_ctrl("1");
	avt_ok = 0;
	select_window(rtwin);
	wr_at(rtadr = 0x0A);
	out_at();
	set_status_word("0"); /* сбросить все биты ОС */
	select_window(rtwin);
	set_subadr_r("10");
#ifdef ENG
	comment("RT arbiter automatic test (writing).");
#else
	comment("Автоматический тест арбитра ОУ (запись).");
#endif
	select_window(bcwin);
	statistic_clear("");
	subadr1 = 0x10;
	subadrmax = subadr2 = 0x1E;
	ci_field = 0x1F << 5;
	/* уст. кода управления */
	/* занесение констант в базы */
	set_base("1");
	buf_fill("1, 32, FFFF");
	ctrl_code[bcnum][base] = DATA_BC_RT;
	bcputw(0, (rtadr << 11) | RT_RECEIVE | (0x10 << 5) | (nATWords & 0x1F));
	wLastCWBase = 2;
#ifdef _TMK1553B_LINUX
	CheckTmkEvent(0);
#endif
	int_num[bcnum] = 0;
	for (i = 0; i < 32; i++) {
		buffer[i] = (i + 1) + ((i + 1) << 8);
		vbuffer[i] = 0;
	}
	prtscr_flag = 0;
	while (!prtscr_flag && (dwMaxErr == 0L || bad_starts < dwMaxErr)) {
		tmkselect(rtnum);
		for (subadr = subadr1; subadr <= subadrmax; subadr++) {
			rtdefsubaddr(RT_TRANSMIT, subadr);
			rtputblk(0, vbuffer, 32);
		}
		tmkselect(bcnum);
#ifdef _TMK1553B_LINUX
		CheckTmkEvent(0);
#endif
		int_num[bcnum] = 0;
		bcstart(1, ctrl_code[bcnum][1]);
		tmkselect(rtnum);
		subadr = subadr1;
#ifdef _TMK1553B_LINUX
		CheckTmkEvent(0);
#endif
		while (!int_num[bcnum] && subadr <= subadr2) {
			rtdefsubaddr(RT_TRANSMIT, subadr);
			rtputblk(0, buffer, 32);
			subadrmax = subadr;
			subadr++;
#ifdef _TMK1553B_LINUX
			CheckTmkEvent(0);
#endif
		}
		if (int_num[bcnum] == 2) {
			tmkselect(bcnum);
			if (wLastCWBase < tmkMaxBase[tmkselected()]) {
				bcdefbase(wLastCWBase);
				bcputw(0, CWM(rtadr, CMD_TRANSMIT_LAST_COMMAND_WORD));
				bcputw(1, 0xFFFF);
				bcputw(2, 0xFFFF);
				int_num[bcnum] = 0;
				bcstart(wLastCWBase, CTRL_C_AD);
				while (!int_num[bcnum]) {
#ifdef _TMK1553B_LINUX
					CheckTmkEvent(0);
#endif
				}
				++wLastCWBase;
				bcdefbase(wLastCWBase);
				bcputw(0, CWM(rtadr, CMD_TRANSMIT_STATUS_WORD));
				bcputw(1, 0xFFFF);
				int_num[bcnum] = 0;
				bcstart(wLastCWBase, CTRL_C_A);
				while (!int_num[bcnum]) {
#ifdef _TMK1553B_LINUX
					CheckTmkEvent(0);
#endif
				}
				++wLastCWBase;
			}
			tmkselect(rtnum);
		}
		for (subadr = subadr1; subadr <= subadrmax; subadr++) {
			rtdefsubaddr(RT_TRANSMIT, subadr);
			rtgetblk(0, buffer + 32, nATWords);
			for (i = 0; i < nATWords; i++) {
				if (buffer[i + 32] != buffer[i] && !mode) {
					if (channel_err < (unsigned long)ERRLEN) {
						at_bad_mode[(unsigned)channel_err] = DATA_BC_RT;
						at_bad_base[(unsigned)channel_err] = 1;
						at_bad_sa[(unsigned)channel_err] = subadr | RT_TRANSMIT;
						at_bad_num[(unsigned)channel_err] = i;
						at_bad_good[(unsigned)channel_err] = buffer[i];
						at_bad_bad[(unsigned)channel_err] = buffer[i + 32];
					}
					channel_err++;
#ifdef LPT_SYN
					if (fLPTSyn) {
						set_lpt_data(0xFF);
						set_lpt_data(0);
					}
#endif
				}
			}
		}
		tmkselect(bcnum);
		if (!fLPTSyn && ((++starts) % 100 == 0)) {
			statistic_out("");
		}
#ifdef _TMK1553B_LINUX
		bioskey_f12();
#endif
	}
	set_base("1");
	out_base();
	out_ctrl_code();
	bc_out_ram(base);
	/* Конец теста */
	if (bad_starts > 0L || channel_err > 0L) {
#ifdef ENG
		err_msg = "RT arbiter test didn't pass";
#else
		err_msg = "Тест арбитра ОУ не прошел";
#endif
	} else if (prtscr_flag) {
#ifdef ENG
		err_msg = "Test aborted on PrtScr";
#else
		err_msg = "Тест прерван по PrtScr";
#endif
	} else {
		avt_ok = 1;
#ifdef ENG
		err_msg = "RT arbiter test completed";
#else
		err_msg = "Тест арбитра ОУ завершен";
#endif
	}
	check_int();
	select_window(bcwin);
	all_screen();
	select_window(rtwin);
	beep1();
	if (avt_ok) {
		textcolor(GREEN);
	} else {
		textcolor(LIGHTRED);
	}
	comment(err_msg);
	textcolor(LIGHTGRAY);
	select_window(nmainwin = bcwin);
	statistic_out("");
	if (channel_err > 0L) {
		statistic_err_out();
	}
	avt_ctrl("0");
	avt_err = !avt_ok;
}

void avt_arb_rt_f(char *str)  /* поддержка проверки арбитра ОУ (запись/чтение) */
{
	_INT16 i;
	long starts = 0;
	_INT16 mode = 0;
	_UINT16 subadrmax;

	if (end_ctrl(str)) {
		while (*str == '\t' || *str == ' ') {
			str++;
		}
#ifdef _TMK1553B_DOS
		if (sscanf(str, "%d", &mode) != 1)
#endif
#ifdef _TMK1553B_LINUX
			if (sscanf(str, "%hd", &mode) != 1)
#endif
			{
				msg_out(mark_err);
				return;
			}
	}
	if (Window[0].nMode == BC_MODE) {
		bcwin = 0;
	} else if (Window[1].nMode == BC_MODE) {
		bcwin = 1;
	} else {
#ifdef ENG
		msg_out("BC not defined");
#else
		msg_out("Не задан КК");
#endif
		return;
	}
	if (Window[0].nMode == RT_MODE) {
		rtwin = 0;
	} else if (Window[1].nMode == RT_MODE) {
		rtwin = 1;
	} else {
#ifdef ENG
		msg_out("RT not defined");
#else
		msg_out("Не задано ОУ");
#endif
		return;
	}
	bcnum = Window[bcwin].nTMK;
	rtnum = Window[rtwin].nTMK;
	avt_ctrl("1");
	avt_ok = 0;
	select_window(rtwin);
	wr_at(rtadr = 0x0A);
	out_at();
	set_status_word("0"); /* сбросить все биты ОС */
	select_window(rtwin);
	set_subadr_r("10");
#ifdef ENG
	comment("RT arbiter automatic test (writing/reading).");
#else
	comment("Автоматический тест арбитра ОУ (запись/чтение).");
#endif
	select_window(bcwin);
	statistic_clear("");
	subadr1 = 0x10;
	subadrmax = subadr2 = 0x1E;
	ci_field = 0x1F << 5;
	/* уст. кода управления */
	/* занесение констант в базы */
	set_base("1");
	buf_fill("1, 32, FFFF");
	ctrl_code[bcnum][base] = DATA_BC_RT;
	bcputw(0, (rtadr << 11) | RT_RECEIVE | (0x10 << 5) | (nATWords & 0x1F));
	int_num[bcnum] = 0;
	for (i = 0; i < 32; i++) {
		buffer[i] = (i + 1) + ((i + 1) << 8);
		vbuffer[i] = 0;
	}
	prtscr_flag = 0;
	while (!prtscr_flag && (dwMaxErr == 0L || bad_starts < dwMaxErr)) {
		tmkselect(rtnum);
		for (subadr = subadr1; subadr <= subadrmax; subadr++) {
			rtdefsubaddr(RT_TRANSMIT, subadr);
			rtputblk(0, vbuffer, 32);
		}
		tmkselect(bcnum);
#ifdef _TMK1553B_LINUX
		CheckTmkEvent(0);
#endif
		int_num[bcnum] = 0;
		bcstart(1, ctrl_code[bcnum][1]);
		tmkselect(rtnum);
		subadr = subadr1;
#ifdef _TMK1553B_LINUX
		CheckTmkEvent(0);
#endif
		while (!int_num[bcnum] && subadr <= subadr2) {
			rtdefsubaddr(RT_TRANSMIT, subadr);
			for (i = 0; i < 32; i++) {
				rtputw(i, buffer[i]);
				if (rtgetw(i) != buffer[i] && !mode) {
					if (channel_err < (unsigned long)ERRLEN) {
						at_bad_mode[(unsigned)channel_err] = DATA_BC_RT;
						at_bad_base[(unsigned)channel_err] = 1;
						at_bad_sa[(unsigned)channel_err] = subadr | RT_TRANSMIT;
						at_bad_num[(unsigned)channel_err] = i;
						at_bad_good[(unsigned)channel_err] = buffer[i];
						at_bad_bad[(unsigned)channel_err] = buffer[i + 32];
					}
					channel_err++;
#ifdef LPT_SYN
					if (fLPTSyn) {
						set_lpt_data(0xFF);
						set_lpt_data(0);
					}
#endif
				}
			}
			subadrmax = subadr;
			subadr++;
#ifdef _TMK1553B_LINUX
			CheckTmkEvent(0);
#endif
		}
		tmkselect(bcnum);
		if (!fLPTSyn && ((++starts) % 100 == 0)) {
			statistic_out("");
		}
#ifdef _TMK1553B_LINUX
		bioskey_f12();
#endif
	}
	set_base("1");
	out_base();
	out_ctrl_code();
	bc_out_ram(base);
	/* Конец теста */
	if (bad_starts > 0L || channel_err > 0L) {
#ifdef ENG
		err_msg = "RT arbiter test didn't pass";
#else
		err_msg = "Тест арбитра ОУ не прошел";
#endif
	} else if (prtscr_flag) {
#ifdef ENG
		err_msg = "Test aborted on PrtScr";
#else
		err_msg = "Тест прерван по PrtScr";
#endif
	} else {
		avt_ok = 1;
#ifdef ENG
		err_msg = "RT arbiter test completed";
#else
		err_msg = "Тест арбитра ОУ завершен";
#endif
	}
	check_int();
	select_window(bcwin);
	all_screen();
	select_window(rtwin);
	beep1();
	if (avt_ok) {
		textcolor(GREEN);
	} else {
		textcolor(LIGHTRED);
	}
	comment(err_msg);
	textcolor(LIGHTGRAY);
	select_window(nmainwin = bcwin);
	statistic_out("");
	if (channel_err > 0L) {
		statistic_err_out();
	}
	avt_ctrl("0");
	avt_err = !avt_ok;
}

void avt_ou400(char *str)  /* поддержка проверки простого ОУ */
{
	_INT16 k, fLen32;
	long starts = 0, startnum;
	_UINT16 nWords, rtaddr = 10;
	_UINT16 waitaw, setaw;
	_UINT16 savebus;

	if (end_ctrl(str)) {
		while (*str == '\t' || *str == ' ') {
			str++;
		}
		if (sscanf(str, "%ld", &starts) != 1) {
			msg_out(mark_err);
			return;
		}
	}
	avt_ok = 0;
	bcnum = tmkselected();
	savebus = bcgetbus();
	startnum = 0;
	prtscr_flag = 0;
	bcdefbase(base = 0);
	ctrl_code[bcnum][base] = CTRL_CD_A;
	bcputw(0, 0x53F1);
	bcputw(1, 0x0010);
	start_1("");
	statistic_clear("");
	start_1("");
	if (bad_starts > 0L || to_errors > 0L) {
		goto ret;
	}
	avt_ctrl("1");
	statistic_clear("");
	fLen32 = 1;
	do {
		if (fReserv) {
			bcdefbus(rand() & BUS_B);
		}
		bcdefbase(base = 0);
		ctrl_code[bcnum][base] = CTRL_CD_A;
		bcputw(0, 0x53F1);
		setaw = rand() & 0xF;
		bcputw(1, setaw);
		start();
		waitaw = 0x5000;
		if (setaw & 1) {
			waitaw |= RTFL_MASK;
		}
		if (setaw & 2) {
			waitaw |= SSFL_MASK;
		}
		if (setaw & 4) {
			waitaw |= BUSY_MASK;
		}
		if (setaw & 8) {
			waitaw |= SREQ_MASK;
		}
		bcdefbase(base = 1);
		ctrl_code[bcnum][base] = CTRL_C_A;
		bcputw(0, 0x57E8);
		start();
		if (waitaw != 0x5000) {
			if (int_num[bcnum] == 2 && bc_sw == IB_MASK && bc_aw1 == waitaw) {
				--bad_starts;
				++good_starts;
			} else {
				if (int_num[bcnum] != 2) {
					++bad_starts;
					--good_starts;
				}
			}
		}
		bcdefbase(base = 2);
		ctrl_code[bcnum][base] = CTRL_CD_A;
		bcputw(0, 0x53F1);
		bcputw(1, 0x0010);
		start();
		if (int_num[bcnum] == 2 && bc_sw == IB_MASK && bc_aw1 == waitaw) {
			--bad_starts;
			++good_starts;
		}
		if (bad_starts > 0L) {
			break;
		}

		bcdefbase(base = 3);
		ctrl_code[bcnum][base] = CTRL_C_A;
		bcputw(0, 0x53E1); //illegal
		start();
		if (int_num[bcnum] == 2 && bc_sw == IB_MASK && bc_aw1 == 0x5400) {
			--bad_starts;
			++good_starts;
		} else {
			if (int_num[bcnum] != 2) {
				++bad_starts;
				--good_starts;
			}
			break;
		}

		bcdefbase(base = 4);
		ctrl_code[bcnum][base] = CTRL_C_A;
		bcputw(0, 0x57E2); //tos
		start();
		if (int_num[bcnum] == 2 && bc_sw == IB_MASK && bc_aw1 == 0x5400) {
			--bad_starts;
			++good_starts;
		} else {
			if (int_num[bcnum] != 2) {
				++bad_starts;
				--good_starts;
			}
			break;
		}

		bcdefbase(base = 5);
		ctrl_code[bcnum][base] = CTRL_C_AD;
		bcputw(0, 0x57F2); //tlc
		start();
		if (int_num[bcnum] == 2 && bc_sw == IB_MASK && bc_aw1 == 0x5400 && bcgetw(2) == 0x57E2) {
			--bad_starts;
			++good_starts;
		} else {
			if (int_num[bcnum] != 2) {
				++bad_starts;
				--good_starts;
			}
			break;
		}

		bcdefbase(base = 6);
		ctrl_code[bcnum][base] = CTRL_CD_A;
		bcputw(0, 0xFBF1); //syn d
		bcputw(1, 0x0010);
		start();
		if (int_num[bcnum] == 2 && bc_sw == TO_MASK) {
			--bad_starts;
			++good_starts;
		} else {
			if (int_num[bcnum] != 2) {
				++bad_starts;
				--good_starts;
			}
			break;
		}

		bcdefbase(base = 7);
		ctrl_code[bcnum][base] = CTRL_C_A;
		bcputw(0, 0x57E2); //tos
		start();
		if (int_num[bcnum] == 2 && bc_sw == IB_MASK && bc_aw1 == 0x5010) {
			--bad_starts;
			++good_starts;
		} else {
			if (int_num[bcnum] != 2) {
				++bad_starts;
				--good_starts;
			}
			break;
		}

		for (base = 0; base < 0xF; ++base) {
			if (fLen32) {
				nWords = 32;
			} else {
				nWords = (rand() & 31) + 1;
			}
			buffer[0] = CW(rtaddr, RT_RECEIVE, 0x10 + base, nWords);
			for (k = 1; k <= 32; ++k) {
				buffer[k] = rand() + rand();
			}
			bcdefbase(base);
			bcputblk(0, buffer, nWords + 1);
			while (int_num[bcnum] == 0) {
#ifdef _TMK1553B_LINUX
				CheckTmkEvent(0);
#endif
			}
			int_num[bcnum] = 0;
			bcstart(base, DATA_BC_RT);
		}
		while (int_num[bcnum] == 0) {
#ifdef _TMK1553B_LINUX
			CheckTmkEvent(0);
#endif
		}
		for (base = 0x10; base < 0x1F; ++ base) {
			bcdefbase(base - 0x10);
			bcgetblk(0, vbuffer, 33);
			if ((nWords = (vbuffer[0] & 31)) == 0) {
				nWords = 32;
			}
			bcdefbase(base);
			bcputw(0, CW(rtaddr, RT_TRANSMIT, base, nWords));
			int_num[bcnum] = 0;
			bcstart(base, DATA_RT_BC);
			while (int_num[bcnum] == 0) {
#ifdef _TMK1553B_LINUX
				CheckTmkEvent(0);
#endif
			}
			bcgetblk(2, buffer, nWords);
			for (k = 0; k < nWords; ++k) {
				if (buffer[k] != vbuffer[k + 1]) {
#ifdef LPT_SYN
					if (fLPTSyn) {
						set_lpt_data(0xFF);
						set_lpt_data(0);
					}
#endif
					if (channel_err < (unsigned long)ERRLEN) {
						at_bad_mode[(unsigned)channel_err] = DATA_RT_BC;
						at_bad_base[(unsigned)channel_err] = base;
						at_bad_sa[(unsigned)channel_err] = base;
						at_bad_num[(unsigned)channel_err] = k;
						at_bad_good[(unsigned)channel_err] = vbuffer[k + 1];
						at_bad_bad[(unsigned)channel_err] = buffer[k];
					}
					++channel_err;
				}
			}
		}
		fLen32 = !fLen32;
		++startnum;
		if (startnum % 10 == 0) {
			statistic_out("");
			bcdefbase(base = 0);
			ctrl_code[bcnum][base] = CTRL_CD_A;
			bcputw(0, 0x53F1);
			bcputw(1, 0x0010);
			start();
			ctrl_code[bcnum][base] = DATA_BC_RT;
			bcputw(0, 0x5222);
			bcputw(1, 0x0F5A);
			bcputw(1, 0xF0A5);
			start();
		}
#ifdef _TMK1553B_LINUX
		bioskey_f12();
		CheckTmkEvent(0);
#endif
		if (prtscr_flag || bad_starts > 0L || channel_err > 0L) {
			break;
		}
	} while (!starts || startnum < starts);
//? ctrl_code[bcnum][base] = DATA_RT_BC;

ret:

	if (fReserv) {
		bcdefbus(savebus);
	}
	out_base();
	out_ctrl_code();
	bc_out_ram(base);

	/* Конец теста */
	if (bad_starts > 0L || to_errors > 0L || channel_err > 0L) {
#ifdef ENG
		err_msg = "RT test didn't pass";
#else
		err_msg = "Тест ОУ не прошел";
#endif
	}
	/*
	 else if (prtscr_flag)
	 {
	   err_msg = "Тест прерван по 'PrtScr'";
	 }
	*/
	else {
		avt_ok = 1;
#ifdef ENG
		err_msg = "RT test completed";
#else
		err_msg = "Тест ОУ завершен";
#endif
	}
	check_int();
	/*
	 all_screen();
	*/
	beep1();
	if (channel_err > 0L) {
		statistic_err_out();
	}
	avt_ctrl("0");
	if (avt_ok) {
		textcolor(GREEN);
	} else {
		textcolor(LIGHTRED);
	}
	comment(err_msg);
	textcolor(LIGHTGRAY);
	statistic_out("");
	avt_err = !avt_ok;
}
#endif //def ELCUS

_INT16 cmp_ram_buf(_INT16 offset, _INT16 voffset, _INT16 size)
/* сравнение буфера и ram_window: 0 - все ОК, 1 - ошибка
offset - смещение, size - размер сравниваемых массивов */
{
	_INT16 i;

	if (voffset >= 0) {
		switch (tmkgetmode()) {
		case BC_MODE:
			bcgetblk(0, vbuffer, size + voffset);
			break;
		case RT_MODE:
			rtgetblk(0, vbuffer, size + voffset);
		}
	} else {
		voffset = 0;
	}
	for (i = 0; i < size; i++)
		if (buffer[i + offset] != vbuffer[i + voffset]) {
			return 1;
		}
	return 0;
}

void avt_ctrl(const char *str)                /* управление выводом на экран */
{
	_UINT16 a;

	if (sscanf(str, "%hu", &a) != 1) {
		msg_out(inp_err);
		return;
	}
	if (a != 0 && a != 1) {
#ifdef ENG
		msg_out("Wrong variable value");
#else
		msg_out("Неверное значение переменной");
#endif
		return;
	}
	avtomat = a;
}

#ifdef LPT
void init_lpt()
/*
       Вычисление номеров портов по таблице данных BIOS .
*/
{
#ifdef _TMK1553B_DOS
	lpt_data_port = peek(0x0040, 8 + (lpt_num - 1) * 2); /* адреса портов LPT */
#endif
#ifdef _TMK1553B_LINUX
// unsigned char *bios_mem;

	/* bios_mem = mmap(0, 256, PROT_READ, MAP_SHARED | MAP_PHYS, NOFD, 0x400);
	 if(bios_mem != MAP_FAILED)
	 {
	   lpt_data_port = *((unsigned short *)(bios_mem + 8));
	   munmap(bios_mem, 256);
	 }
	 else*/
	lpt_data_port = 0x378;//default
	iopl(3);
#endif
	lpt_status_port = lpt_data_port + 1;
	lpt_ctrl_port = lpt_data_port + 2;
}

void set_lpt_data(unsigned char data)
{
	outportb(lpt_data_port, data);
}
#endif

void buf_fill(char *str)   /* заполнение буфера константой */
{
	_UINT16 start, end, len, constant, i;

	while (*str == '\t' || *str == ' ') {
		str++;
	}
#ifdef _TMK1553B_DOS
	if (sscanf(str, "%u", &start) != 1)
#endif
#ifdef _TMK1553B_LINUX
		if (sscanf(str, "%hu", &start) != 1)
#endif
		{
			msg_out(mark_err);
			return;
		}
	if (start > buf_size - 1) {
		msg_out(mark_err);
		return;
	}
	while (*str != ' ' && *str != ',') {
		str++;
	}
	while (*str == ' ' || *str == ',') {
		str++;
	}
#ifdef _TMK1553B_DOS
	if (sscanf(str, "%u", &end) != 1)
#endif
#ifdef _TMK1553B_LINUX
		if (sscanf(str, "%hu", &end) != 1)
#endif
		{
			msg_out(mark_err);
			return;
		}
	if (end < start || end > buf_size - 1) {
		msg_out(mark_err);
		return;
	}
	while (*str != ' ' && *str != ',') {
		str++;
	}
	while (*str == ' ' || *str == ',') {
		str++;
	}
#ifdef _TMK1553B_DOS
	if (sscanf(str, "%x", &constant) != 1)
#endif
#ifdef _TMK1553B_LINUX
		if (sscanf(str, "%hx", &constant) != 1)
#endif
		{
			msg_out(const_err);
			return;
		}
	len = end - start + 1;
	for (i = start; i <= end; i++) {
		buffer[i] = constant;
	}
	out_buffer();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	switch (Window[nwin].nMode) {
	case BC_MODE:
		bcdefbase(base);
		bcputblk(start, buffer + start, len);
		break;
	case RT_MODE:
		rtdefsubaddr(dir, subadr);
		rtputblk(start, buffer + start, len);
	}
}

void buf_clear(const char *str)  /* очистка буфера */
{
	_INT16 i;

	if (end_ctrl(str)) {
		return;
	}
	for (i = 64; i-- != 0; buffer[i] = 0);
	buf_mark = 0;
	in_ed_word = 3;
	out_buffer();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	switch (Window[nwin].nMode) {
	case BC_MODE:
		bcdefbase(base);
		bcputblk(0, buffer, 64);
		break;
	case RT_MODE:
		rtdefsubaddr(dir, subadr);
		rtputblk(0, buffer, 32);
	}
}

_INT16 end_ctrl(const char *str)
/* контроль конца строки(для команд без аргументов):
   1 - в строке еще что-то значащее есть,
   0 - в строке пусто                 */
{
	while (*str == '\t' || *str == ' ') {
		str++;
	}
	if (*str) {
		msg_out(undef_cmd);
		return 1;
	} else {
		return 0;
	}
}

void clear_dump()
{
	_INT16 y, maxy;

	maxy = (avtomat) ? BDUMP_Y + 3 : BDUMP_Y + 4;
	for (y = BDUMP_Y - 1; y <= maxy; y++) {
		gotoxy(1, y);
		clreol();
	}
}

void buf_edit(const char *str)                   /* редактирование буферной памяти */
{
	static _INT16 in_word = 3;  /* номер текущей тетрады для редактирования */
	static _INT16 edit_mode = 0; /* текущий режим редактирования: 0 - HEX, 1 - BIN */
	static _INT16 bit_num = 15; /* текущий бит при редактировании */
	_UINT16 key, tetrada, bit;
	_INT16 tmkmode;

	switch (tmkmode = Window[nwin].nMode) {
	case BC_MODE:
		bc_out_ram(base);
		break;
	case RT_MODE:
		rt_out_ram(dir, subadr);
	}
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	while (*str == '\t' || *str == ' ') {
		str++;
	}
	if (!mark_set(str, &buf_mark)) {
		do {
			if (edit_mode) {
				bin_unpack(62, BDUMP_Y, buf_mark, buffer[buf_mark]);
				gotoxy(BDUMP_X - 1 + 5 * (buf_mark % DUMP_WIDE + 1) - 3, BDUMP_Y + buf_mark / DUMP_WIDE);
				cprintf("%04X", buffer[buf_mark]);
				gotoxy(61 + (16 - bit_num), BDUMP_Y + 1);
			} else {
				bin_unpack(62, BDUMP_Y, buf_mark, buffer[buf_mark]);
				gotoxy(BDUMP_X - 1 + 5 * (buf_mark % DUMP_WIDE + 1) - in_word, BDUMP_Y + buf_mark / DUMP_WIDE);
			}
			key = bioskey(0);
			if ((key & 0x00FF) && key != GPLUSW) {
				switch (key = toupper(key & 0x00FF)) {   /* обычные и символы управления */
				case 'Q':
				case ESCAPE:
					key = 'Q';
					break;
				case TAB:
					if (edit_mode) {
						edit_mode = 0;
						in_word = bit_num / 4;
					} else {
						edit_mode = 1;
						bit_num = in_word * 4 + 3;
					}
					break;
				case CR:
					edit_mode = 0;
					if (buf_mark != DUMP_SIZE - 1) {
						++buf_mark;
					} else {
						buf_mark = 0;
					}
					break;
				case ' ':
					if (edit_mode) {
						break;
					}
					if (in_word == 0) {
						if (buf_mark != DUMP_SIZE - 1) {
							in_word = 3;
							buf_mark++;
						}
					} else {
						in_word--;
					}
					break;
				case BACKSPACE:
					if (edit_mode) {
						break;
					}
					if (in_word == 3) {
						if (buf_mark != 0) {
							in_word = 0;
							buf_mark--;
						}
					} else {
						in_word++;
					}
					break;
				default:
					if (edit_mode) {
						if (key != '0' && key != '1') {
							break;
						}
						bit = key - '0';
						buffer[buf_mark] &= ~(0x0001 << bit_num);
						buffer[buf_mark] |= (bit << bit_num);
						if (bit_num != 0) {
							bit_num--;
						}
						break;
					}
					if (key >= '0' && key <= '9') {
						tetrada = key - '0';
					} else if (key >= 'A' && key <= 'F') {
						tetrada = key - 'A' + 10;
					} else {
						break;
					}
					putch(key);
					buffer[buf_mark] &= ~(0x000F << in_word * 4);
					buffer[buf_mark] |= (tetrada << in_word * 4);
					if (in_word == 0) {
						if (buf_mark != DUMP_SIZE - 1) {
							in_word = 3;
							buf_mark++;
						}
					} else {
						in_word--;
					}
					break;
				}
			} else {
				switch (key >> 8) {                        /* функциональные клавиши */
				case F01:
					clear_dump();

#ifdef ENG
					gotoxy(1, BDUMP_Y);
					cputs("  \x1E     SPACE - tetrada right, \x11--(BACKSPACE) - tetrada left, ");
					gotoxy(1, BDUMP_Y + 1);
					cputs("\x11   \x10 - go to words, HOME - buffer begin, END - buffer end, ");
					gotoxy(1, BDUMP_Y + 2);
					cputs("  \x1F     TAB - HEX/BIN mode, Q(F10) - finish editing.");
					gotoxy(1, BDUMP_Y + 3);
					switch (tmkmode) {
					case BC_MODE:
						cputs("        PGUP, PGDN - change base.");
						break;
					case RT_MODE:
						cputs("        PGUP, PGDN - change subaddress, GRAY PLUS - Tx/Rx.");
					}
#else
					gotoxy(1, BDUMP_Y);
					cputs("  \x1E     ПРОБЕЛ - на тетраду вправо, \x11--(BACKSPACE) - на тетраду влево, ");
					gotoxy(1, BDUMP_Y + 1);
					cputs("\x11   \x10 - перемещение по словам, HOME - в начало буфера, END - в конец буфера, ");
					gotoxy(1, BDUMP_Y + 2);
					cputs("  \x1F     TAB - HEX/BIN режим, Q(F10) - завершить редактирование.");
					gotoxy(1, BDUMP_Y + 3);
					switch (tmkmode) {
					case BC_MODE:
						cputs("        PGUP, PGDN - изменение базы.");
						break;
					case RT_MODE:
						cputs("        PGUP, PGDN - изменение подадреса, СЕРЫЙ ПЛЮС - Прием/Передача.");
					}
#endif
#ifdef _TMK1553B_LINUX
					fflush(stdout);
#endif
#ifdef _TMK1553B_DOS
					while (!kbhit());
#endif
					bioskey(0);
					clear_dump();
					out_buffer();
#ifdef _TMK1553B_LINUX
					fflush(stdout);
#endif
					break;
				case LEFT:
					if (edit_mode) {
						if (bit_num != 15) {
							bit_num++;
						}
						break;
					}
					if (buf_mark != 0) {
						--buf_mark;
					} else {
						buf_mark = DUMP_SIZE - 1;
					}
					in_word = 3;
					break;
				case RIGHT:
					if (edit_mode) {
						if (bit_num != 0) {
							bit_num--;
						}
						break;
					}
					if (buf_mark != DUMP_SIZE - 1) {
						++buf_mark;
					} else {
						buf_mark = 0;
					}
					in_word = 3;
					break;
				case UP:
					if (edit_mode) {
						if (buf_mark != 0) {
							--buf_mark;
						} else {
							buf_mark = DUMP_SIZE - 1;
						}
						break;
					}
					if (buf_mark - DUMP_WIDE >= 0) {
						buf_mark -= DUMP_WIDE;
					} else {
						buf_mark += (DUMP_WIDE * (DUMP_STRINGS - 1));
						if (buf_mark > DUMP_SIZE - 1) {
							buf_mark -= DUMP_WIDE;
						}
					}
					in_word = 3;
					break;
				case DOWN:
					if (edit_mode) {
						if (buf_mark != DUMP_SIZE - 1) {
							++buf_mark;
						} else {
							buf_mark = 0;
						}
						break;
					}
					if (buf_mark + DUMP_WIDE <= DUMP_SIZE - 1) {
						buf_mark += DUMP_WIDE;
					} else {
						buf_mark -= (DUMP_WIDE * (DUMP_STRINGS - 1));
						if (buf_mark < 0) {
							buf_mark += DUMP_WIDE;
						}
					}
					in_word = 3;
					break;
				case HOME:
					if (edit_mode) {
						bit_num = 15;
						break;
					}
					buf_mark = 0;
					in_word = 3;
					break;
				case END:
					if (edit_mode) {
						bit_num = 0;
						break;
					}
					buf_mark = DUMP_SIZE - 1;
					in_word = 3;
					break;
				case GPLUS:
					if (tmkmode == RT_MODE) {
						rtputblk(0, buffer, 32);
						dir = (dir == RT_TRANSMIT) ? RT_RECEIVE : RT_TRANSMIT;
						rtdefsubaddr(dir, subadr);
						rtgetblk(0, buffer, 32);
						out_subadr();
						out_buffer();
#ifdef _TMK1553B_LINUX
						fflush(stdout);
#endif
					}
					break;
				case PGUP:
					switch (tmkmode) {
					case BC_MODE:
						if (base > 0) {
							bcputblk(0, buffer, 36);
							bcdefbase(--base);
							bcgetblk(0, buffer, 36);
							out_base();
							out_ctrl_code();
							out_buffer();
#ifdef _TMK1553B_LINUX
							fflush(stdout);
#endif
						}
						break;
					case RT_MODE:
						if (subadr > 0) {
							rtputblk(0, buffer, 32);
							rtdefsubaddr(dir, --subadr);
							rtgetblk(0, buffer, 32);
							out_subadr();
							out_buffer();
#ifdef _TMK1553B_LINUX
							fflush(stdout);
#endif
						}
					}
					in_word = 3;
					break;
				case PGDN:
					switch (tmkmode) {
					case BC_MODE:
						if (base < tmkMaxBase[tmkselected()]) {
							bcputblk(0, buffer, 36);
							bcdefbase(++base);
							bcgetblk(0, buffer, 36);
							out_base();
							out_ctrl_code();
							out_buffer();
#ifdef _TMK1553B_LINUX
							fflush(stdout);
#endif
						}
						break;
					case RT_MODE:
						if (subadr < 31) {
							rtputblk(0, buffer, 32);
							rtdefsubaddr(dir, ++subadr);
							rtgetblk(0, buffer, 32);
							out_subadr();
							out_buffer();
#ifdef _TMK1553B_LINUX
							fflush(stdout);
#endif
						}
					}
					in_word = 3;
					break;
				case F10:
					key = 'Q';
					break;
				}
				continue;
			}
		} while (key != 'Q');
	}
	switch (tmkmode) {
	case BC_MODE:
		bcputblk(0, buffer, 36);
		break;
	case RT_MODE:
		rtputblk(0, buffer, 32);
	}
}

_INT16 mark_set(const char *str, _INT16 *mark)
/* ввод номера текущего слова для редактирования
1 - ошибка, 0 - все в порядке
*/
{
	_UINT16 i, constant;

	if (*str == '\0') {
		return 0;
	}
#ifdef _TMK1553B_DOS
	if ((sscanf(str, "%u", &i)) != 1)
#endif
#ifdef _TMK1553B_LINUX
		if ((sscanf(str, "%hu", &i)) != 1)
#endif
		{
			msg_out(inp_err);
			return 1;
		}
	if (i > DUMP_SIZE - 1) {
		msg_out(mark_err);
		return 1;
	}
	*mark = i;
	while (*str != ' ' && *str != ',' && *str != '\0') {
		str++;
	}
	if (!*str) {
		return 0;
	}
	while (*str == ' ' || *str == ',') {
		str++;
	}
#ifdef _TMK1553B_DOS
	if (sscanf(str, "%x", &constant) != 1)
#endif
#ifdef _TMK1553B_LINUX
		if (sscanf(str, "%hx", &constant) != 1)
#endif
		{
			msg_out(const_err);
			return 1;
		}
	buffer[*(mark)] = constant;
	out_buffer();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	return 1;
}

void out_func_str()   /* Вывод подсказки о назначении функциональных клавиш */
{
	_INT16 i;

#ifdef ENG
	static const char *bc_func_str[10] = {  " Help ", /* F01 */
	                                  "  RAM ",  /* F02 */
	                                  "Set RT",  /* F03 */
	                                  "Window",  /* F04 */
	                                  " LoopF",  /* F05 */
	                                  " Start",  /* F06 */
	                                  " Loop ",  /* F07 */
	                                  " Reset",  /* F08 */
	                                  "Fields",  /* F09 */
	                                  "Exit"    /* F10 */
	                               };
	static const char *rt_func_str[10] = {  " Help ", /* F01 */
	                                  "  RAM ",  /* F02 */
	                                  "Set BC",  /* F03 */
	                                  "Window",  /* F04 */
	                                  "      ",  /* F05 */
	                                  "  RSW ",  /* F06 */
	                                  "  RM  ",  /* F07 */
	                                  " Reset",  /* F08 */
	                                  "Fields",  /* F09 */
	                                  "Exit"    /* F10 */
	                               };
#else
	static char *bc_func_str[10] = {  "Помощь", /* F01 */
	                                  "  БЗУ ",  /* F02 */
	                                  "Уст.ОУ",  /* F03 */
	                                  " Окно ",  /* F04 */
	                                  " ЦиклФ",  /* F05 */
	                                  " Старт",  /* F06 */
	                                  " Цикл ",  /* F07 */
	                                  " Сброс",  /* F08 */
	                                  " Поля ",  /* F09 */
	                                  "Вых."    /* F10 */
	                               };
	static char *rt_func_str[10] = {  "Помощь", /* F01 */
	                                  "  БЗУ ",  /* F02 */
	                                  "Уст.КК",  /* F03 */
	                                  " Окно ",  /* F04 */
	                                  "      ",  /* F05 */
	                                  "  РСС ",  /* F06 */
	                                  "  РРЖ ",  /* F07 */
	                                  " Сброс",  /* F08 */
	                                  " Поля ",  /* F09 */
	                                  "Вых."    /* F10 */
	                               };
#endif

	gotoxy(1, 25);
	for (i = 1; i <= 10; i++) {
		cprintf("%u", i);
		textbackground(LIGHTGRAY);
		textcolor(BLACK);
		switch (Window[nwin].nMode) {
		case BC_MODE:
			cprintf("%s", bc_func_str[i - 1]);
			break;
		case RT_MODE:
			cprintf("%s", rt_func_str[i - 1]);
			break;
		}
		textbackground(BLACK);
		textcolor(LIGHTGRAY);
		putch(' ');
	}
}

void hor_line(_INT16 maxi)
{
	register _INT16 i;
	for (i = 1; i <= maxi; i++) {
		putch(chH);
	}
}

void all_screen()                                   /* Вывод полного экрана */
{
	_INT16 savenwin, nDX, nDY, i;

	savenwin = nwin;
	clrscr();
	window(1, 1, 80, 25);
	out_func_str();
	for (nwin = 0; nwin <= nMaxWin; nwin++) {
		if (!Window[nwin].fVisible) {
			continue;
		}
		select_window(nwin);
		if (nwin == savenwin) {
			textcolor(WHITE);
			chLU = '╔';
			chRU = '╗';
			chLD = '╚';
			chRD = '╝';
			chV = '║';
			chH = '═';
		} else {
			textcolor(LIGHTGRAY);
			chLU = '┌';
			chRU = '┐';
			chLD = '└';
			chRD = '┘';
			chV = '│';
			chH = '─';
		}
		nDX = Window[nwin].nDX;
		nDY = Window[nwin].nDY;
		window(Window[nwin].nX, Window[nwin].nY,
		       Window[nwin].nX + nDX - 1,
		       Window[nwin].nY + nDY - 1);
		gotoxy(1, 1);
		putch(chLU);
		hor_line(nDX - 2);
		putch(chRU);
		for (i = 2; i < nDY; i++) {
			gotoxy(1, i);
			putch(chV);
			gotoxy(nDX, i);
			putch(chV);
		}
		gotoxy(1, nDY);
		putch(chLD);
		hor_line(nDX - 2);
		putch(chRD);
		window(Window[nwin].nX + 1, Window[nwin].nY,
		       Window[nwin].nX + nDX - 2,
		       Window[nwin].nY + nDY - 1);
		textcolor(LIGHTGRAY);
		out_mode();
		out_type();
		out_fname();
		out_inn();
		switch (Window[nwin].nMode) {
		case BC_MODE:
			out_maxerrs();
			out_count();
			out_glcount();
			out_number();
			out_base();
			out_ctrl_code();
			out_pause();
			bc_out_sw();
			bc_out_ram(base);
			break;
		case RT_MODE:
			out_subadr();
			out_at();
			out_page();
			out_rt_mode();
			out_status_word();
			rt_out_sw();
			rt_out_ram(dir, subadr);
			break;
		}
	}
	nwin = savenwin;
	select_window(nwin);
	window(Window[nwin].nX + 1, Window[nwin].nY,
	       Window[nwin].nX + Window[nwin].nDX - 2,
	       Window[nwin].nY + Window[nwin].nDY - 1);
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

void bc_out_ram(_INT16 base)
/* чтение и вывод дампа базы БЗУ */
{
	bcdefbase(base);
	bcgetblk(0, buffer, 36);
	out_buffer();
}

void rt_out_ram(_INT16 dir, _INT16 subadr)
/* чтение и вывод дампа подадреса БЗУ */
{
	if (fLock) {
		rtlock(dir, subadr);
	} else {
		rtdefsubaddr(dir, subadr);
	}
	rtgetblk(0, buffer, 32);
	out_buffer();
}

void out_buffer()      /* вывод дампа буфера */
{
	gotoxy(1, BDUMP_Y - 1);
#ifdef ENG
	cputs("     RAM");
#else
	cputs("     БЗУ");
#endif
	gotoxy(BDUMP_X, BDUMP_Y - 1);
	out_dump(buffer);
	bin_unpack(62, BDUMP_Y, buf_mark, buffer[buf_mark]);
}

void bin_unpack(_INT16 x, _INT16 y, _INT16 adr, _INT16 data)
/*
вывод адреса и содержимого(двоичная форма)
x, y - место расположения
*/
{
	gotoxy(x, y);
	cprintf("%02Xh:%02u: %04X", adr, adr, (_UINT16)data);
	gotoxy(x, y + 1);
	out_bin(data, 16);
	gotoxy(x, y + 2);
	cputs("FEDCBA9876543210");
}

void bc_out_sw()                    /* вывода слова состояния (bc_sw) */
{
	const char *pszRes;
	gotoxy(2, 4);
#ifdef ENG
	cputs("   Result: ");
#else
	cputs("Результат: ");
#endif
#ifdef _TMK1553B_LINUX
	CheckTmkEvent(0);
#endif
	if (int_num[Window[nwin].nTMK]) {
		out_bin(bc_sw, 7);
	} else {
		bc_sw = 0;
		cputs("XXXXXXX");
	}
#ifdef ENG
	if (bc_sw & ERAO_MASK) {
		pszRes = "RT address error           ";
	} else if (bc_sw & MEO_MASK) {
		pszRes = "Manchester code error      ";
	} else if (bc_sw & TO_MASK) {
		pszRes = "No Status word/Data word(s)";
	} else if (bc_sw & EM_MASK) {
		pszRes = "Internal device error      ";
	} else if (bc_sw & EBC_MASK) {
		pszRes = "Echocheck error            ";
	} else if (bc_sw & ELN_MASK) {
		pszRes = "Extra Data word(s)         ";
	} else {
		pszRes = "                           ";
	}
	if (bc_sw & IB_MASK) {
		cprintf(" Set bit.  %s |", pszRes);
	} else {
		cprintf(" %s           |", pszRes);
	}
#else
	if (bc_sw & ERAO_MASK) {
		pszRes = "Ошибка адреса ОУ           ";
	} else if (bc_sw & MEO_MASK) {
		pszRes = "Ошибка кода 'Манчестер-2'  ";
	} else if (bc_sw & TO_MASK) {
		pszRes = "Нет ОС или мало слов данных";
	} else if (bc_sw & EM_MASK) {
		pszRes = "Ошибка обмена в плате      ";
	} else if (bc_sw & EBC_MASK) {
		pszRes = "Ошибка эхоконтроля передачи";
	} else if (bc_sw & ELN_MASK) {
		pszRes = "Много слов данных          ";
	} else {
		pszRes = "                           ";
	}
	if (bc_sw & IB_MASK) {
		cprintf(" Бит в ОС. %s |", pszRes);
	} else {
		cprintf(" %s           |", pszRes);
	}
#endif
}

void out_inn()                        /* вывод номера прерывания (int_num) */
{
	gotoxy(65, 4);
#ifdef ENG
	cputs(" Interrupt: ");
#else
	cputs("Прерывание: ");
#endif
#ifdef _TMK1553B_LINUX
	CheckTmkEvent(0);
#endif
	if (int_num[Window[nwin].nTMK]) {
		cprintf("%u", int_num[Window[nwin].nTMK]);
	} else {
		putch('X');
	}
}

void out_maxerrs()
{
	gotoxy(64, 2);
	cputs("MaxErr:");
	if (dwMaxErr == 0L) {
		cputs(" ------");
	} else if (dwMaxErr <= 999999L) {
		cprintf(" %6ld", dwMaxErr);
	} else {
		cputs(">999999");
	}
}

void out_mode()     /* вывод текущего режима (mode) */
{
	gotoxy(2, 1);
	if (nwin == nmainwin) {
		textcolor(WHITE);
	} else {
		textcolor(LIGHTGRAY);
	}
	cprintf("%d%c", Window[nwin].nTMK, chH);
#ifdef ENG
	if (Window[nwin].nMode == BC_MODE) {
		cputs("BUS CONTROLLER");
	} else if (Window[nwin].nMode == RT_MODE) {
		cputs("REMOTE TERMINAL");
	}
#else
	if (Window[nwin].nMode == BC_MODE) {
		cputs("КОНТРОЛЛЕР КАНАЛА");
	} else if (Window[nwin].nMode == RT_MODE) {
		cputs("ОКОНЕЧНОЕ УСТРОЙСТВО");
	}
#endif
	textcolor(LIGHTGRAY);
}

inline void out_port(_UINT16 wPort)
{
	if (wPort && (_UINT16)~wPort != 0) {
		cprintf("%03Xh%c", wPort, chH);
	}
}

inline void out_irq(_UINT16 wIrq)
{
#ifdef _TMK1553B_DOS
	if (wIrq && (_UINT16)~wIrq)
#endif
#ifdef _TMK1553B_LINUX
		if (wIrq && (_UINT16)~wIrq != 0xFF00)
#endif
			cprintf("Irq %d%c", wIrq, chH);
}

#ifdef _TMK1553B_DOS
inline void out_iodelay()
{
	cprintf("IOD%3d%c", tmkiodelay(GET_IO_DELAY), chH);
}
#endif

inline void out_hw_ver()
{
	cprintf("Ver %3u%c", tmkgethwver(), chH);
}

void out_type()
{
	_INT16 nTMK = Window[nwin].nTMK;
	gotoxy(26, 1);
	if (nwin == nmainwin) {
		textcolor(WHITE);
	} else {
		textcolor(LIGHTGRAY);
	}
#ifdef _TMK1553B_DOS
	cprintf("%s%c", aTmkConfig[nTMK].achName, chH);
#endif
#ifdef _TMK1553B_LINUX
	cprintf("%s%c", aTmkConfig[nTMK].szName, chH);
#endif
	out_port(aTmkConfig[nTMK].wPorts1);
	out_port(aTmkConfig[nTMK].wPorts2);
	out_irq(aTmkConfig[nTMK].wIrq1);
	out_irq(aTmkConfig[nTMK].wIrq2);
	if (aTmkConfig[nTMK].nType < TMKX)
#ifdef _TMK1553B_DOS
		out_iodelay();
#endif
#ifdef _TMK1553B_LINUX
	cprintf("IOD%3d%c", aTmkConfig[nTMK].wIODelay, chH);
#endif
	else {
		out_hw_ver();
	}
	textcolor(LIGHTGRAY);
}

void out_count()   /* вывод значения счетчика повторений */
{
	gotoxy(47, 2);
#ifdef ENG
	if (cmd_count <= 9999L) {
		cprintf("Cycle: %04lu |", cmd_count);
	} else {
		cputs("Cycle:>9999 |");
	}
#else
	if (cmd_count <= 9999L) {
		cprintf("Циклы: %04lu |", cmd_count);
	} else {
		cputs("Циклы:>9999 |");
	}
#endif
}

void out_glcount()   /* вывод значения счетчика повторений */
{
	if (Window[nwin].nMode == BC_MODE) {
		gotoxy(41, 3);
		cprintf("L: %08lu |", GL_COUNTER);
	}
}

void out_fname()                      /* вывод имени последнего файла данных */
{
	gotoxy(61, 1);
#ifdef ENG
	cputs("File:");
#else
	cputs("Файл:");
#endif
	if (nwin == nmainwin) {
		textcolor(WHITE);
		cputs("═════════════");
		textcolor(LIGHTGRAY);
	} else {
		cputs("─────────────");
	}
	if (*Window[nwin].data_fname != '\0') {
		gotoxy(66, 1);
		cprintf("%s", Window[nwin].data_fname);
	}
}

void out_pause()
{
	gotoxy(28, 2);
#ifdef ENG
	cprintf("Pause: %5u us  |", opause);
#else
	cprintf("Пауза: %5u мкс |", opause);
#endif
}

void out_base()           /* вывод значения базы, используемого при запуске */
{
	gotoxy(2, 2);
#ifdef ENG
	cprintf("Base: %03X->%03X |", base, base_link[tmkselected()][base]);
#else
	cprintf("База: %03X->%03X |", base, base_link[tmkselected()][base]);
#endif
}

void out_number()                          /* вывод номера канала (bus_num) */
{
	gotoxy(19, 2);
#ifdef ENG
	cprintf("BUS: %1u |", bus_num + 1);
#else
	cprintf("ЛПИ: %1u |", bus_num + 1);
#endif
}

void out_ctrl_code()                    /* вывод кода управления (ctrl_code) */
{
	const char *pszFmt;
	_UINT16 ctrl_code2 = ctrl_code[tmkselected()][base];
	gotoxy(2, 3);
#ifdef ENG
	cputs("  Control Code: ");
#else
	cputs("Код управления: ");
#endif
	if (correct_cc(ctrl_code2)) {
		out_bin(ctrl_code2, CC_SIZE);
	} else {
		cputs("XXXX");
	}
	switch (ctrl_code2) {
#ifdef ENG
	case 0x0:
		pszFmt = "(BC->RT)       ";
		break;
	case 0x1:
		pszFmt = "(RT->BC)       ";
		break;
	case 0x2:
		pszFmt = "(RT->RT)       ";
		break;
	case 0x3:
		pszFmt = "(CW - SW)      ";
		break;
	case 0x4:
		pszFmt = "(CW+DW - SW)   ";
		break;
	case 0x5:
		pszFmt = "(CW - SW+DW)   ";
		break;
	case 0x8:
		pszFmt = "(BC->all RTs)  ";
		break;
	case 0xA:
		pszFmt = "(RT->all RTs)  ";
		break;
	case 0xB:
		pszFmt = "(CW,all RTs)   ";
		break;
	case 0xC:
		pszFmt = "(CW+DW,all RTs)";
		break;
	default:
		pszFmt = "(Not defined)  ";
		break;
#else
	case 0x0:
		pszFmt = "(КК->ОУ)       ";
		break;
	case 0x1:
		pszFmt = "(ОУ->КК)       ";
		break;
	case 0x2:
		pszFmt = "(ОУ->ОУ)       ";
		break;
	case 0x3:
		pszFmt = "(КС - ОС)      ";
		break;
	case 0x4:
		pszFmt = "(КС+ИС - ОС)   ";
		break;
	case 0x5:
		pszFmt = "(КС - ОС+ИС)   ";
		break;
	case 0x8:
		pszFmt = "(КК->всем ОУ)  ";
		break;
	case 0xA:
		pszFmt = "(ОУ->всем ОУ)  ";
		break;
	case 0xB:
		pszFmt = "(КС,всем ОУ)   ";
		break;
	case 0xC:
		pszFmt = "(КС+ИС,всем ОУ)";
		break;
	default:
		pszFmt = "(Не определен) ";
		break;
#endif
	}
	cprintf(" %s |", pszFmt);
}

_INT16 correct_cc(_INT16 ctrl_code)
/*
проверка корректности кода управления:
0 - несуществующий, 1 - нормальный.
*/
{
	return ccs[ctrl_code &= 0x000F];
}

void out_page()                               /* вывод номера страницы БЗУ */
{
	gotoxy(54, 3);
#ifdef ENG
	cprintf("   Page  Bus:%2u  PC:%2u", rtgetpagebus(), rtgetpagepc());
#else
	cprintf("Страница  МК:%2u  ПЭВМ:%2u", rtgetpagebus(), rtgetpagepc());
#endif
}

void out_rt_mode()
{
	gotoxy(52, 2);
#ifdef ENG
	cputs(" Mode: ");
#else
	cputs("Режим: ");
#endif
	if (rtgetmode() & RT_FLAG_MODE && tmkError == 0) {
		cputs("FLAG ");
	} else {
		cputs("     ");
	}
	if (hb_mode) {
		cputs("HBIT ");
	} else {
		cputs("     ");
	}
	if ((rtgetmode() & RT_BRCST_MODE) && tmkError == 0) {
		cputs("BCST ");
	} else {
		cputs("     ");
	}
	if ((rtgetirqmode() & RT_DATA_BL) && tmkError == 0) {
		cputs("DTBL");
	} else {
		cputs("    ");
	}
}

void out_at()                                   /* вывод адреса терминала */
{
	_INT16 at;
	gotoxy(29, 2);
	if ((at = rtgetaddress()) != -1)
// if (Window[nwin].at_avail)
	{
		terminal_adr = at;
	}
#ifndef LPT_AT
	else {
#ifdef ENG
		cputs("RT Address: UNKNOWN  |");
#else
		cputs("Адрес ОУ: НЕДОСТУПЕН |");
#endif
		return;
	}
#endif
#ifdef ENG
	cprintf("RT Addr.: %02u (", terminal_adr);
#else
	cprintf("Адрес ОУ: %02u (", terminal_adr);
#endif
	out_bin(terminal_adr, 5);
	cputs(") |");
}

void rt_out_sw()                          /* вывод слова состояния (rt_sw) */
{
	gotoxy(2, 4);
#ifdef ENG
	cputs("     State word: ");
#else
	cputs("Слово состояния: ");
#endif
	out_bin(rt_sw, 16);
	cputs(" |");
}

void rt_out_sp()
{
	gotoxy(35, 4);
	cprintf("%04X", rt_sp);
}

void out_subadr()                 /* вывод значения базы, используемого при запуске */
{
	gotoxy(2, 2);
#ifdef ENG
	cputs("Subaddr.: ");
	if (dir == RT_TRANSMIT) {
		cprintf("%02X (Transm.)  |", subadr);
	} else {
		cprintf("%02X (Recv.)    |", subadr);
	}
#else
	cputs("Подадрес: ");
	if (dir == RT_TRANSMIT) {
		cprintf("%02X (Передача) |", subadr);
	} else {
		cprintf("%02X (Прием)    |", subadr);
	}
#endif
}

void out_status_word()                  /* вывод битов ОС (status_word) */
{
	_UINT16 status_word;

	status_word = rtgetanswbits();
	gotoxy(2, 3);
#ifdef ENG
	cputs(" Status: ");
#else
	cputs("Биты ОС: ");
#endif
	out_bin(status_word, SW_SIZE);
	if (status_word & DNBA) {
		cputs(" (DN ");
	} else {
		cputs(" (-- ");
	}
	if (status_word & RTFL) {
		cputs("TF ");
	} else {
		cputs("-- ");
	}
	if (status_word & SSFL) {
		cputs("SF ");
	} else {
		cputs("-- ");
	}
	if (status_word & BUSY) {
		cputs("BS ");
	} else {
		cputs("-- ");
	}
	if (status_word & SREQ) {
		cputs("SR) |");
	} else {
		cputs("--) |");
	}
}

#ifdef _TMK1553B_DOS
void out_dump(_UINT16 *data)
/*
Вывод пословного дампа:
adr - значение адреса используемое при выводе
data - указатель на слово, с которого происходит вывод дампа
*/
{
	_INT16 i;                 /* счетчик */
	_INT16 offset;            /* текущее смещение в массиве отн. data */

	gotoxy(BDUMP_X, wherey());
	for (i = 0; i != DUMP_WIDE; cprintf("  %01X  ", i++));
	cputs("\x0D\x0A");
	for (offset = 0; offset != DUMP_SIZE;) {
		cprintf("     %02u:", offset);
		gotoxy(BDUMP_X, wherey());
		for (i = 0; (i != DUMP_WIDE) && (offset != DUMP_SIZE); i++, offset++) {
			cprintf(" %04X", *(data + offset));
		}
		cputs("\x0D\x0A");
	}
}
#endif
#ifdef _TMK1553B_LINUX
void out_dump(unsigned short *data)
{
	_INT16 i;
	_INT16 offset;
	_INT16 dump_y;

	gotoxy(BDUMP_X, BDUMP_Y - 1);
	for (i = 0; i != DUMP_WIDE; cprintf("  %01X  ", i++));
	dump_y = BDUMP_Y;
	for (offset = 0; offset != DUMP_SIZE;) {
		gotoxy(1, dump_y);
		cprintf("     %02u: ", offset);
		for (i = 0; (i != DUMP_WIDE) && (offset != DUMP_SIZE); i++, offset++) {
			cprintf(" %04X", *(data + offset));
		}
		dump_y++;
	}
}
#endif

void out_bin(_INT16 data, _INT16 num_digits)
/*
Вывод в двоичной форме:
data - выводимое значение
num_digits - число выводимых цифр, начиная справа
*/
{
	char string[16];
	_INT16 mask;
	_INT16 count;

	for (count = 0, mask = 1; count != 16; count++, mask <<= 1) {
		string[count] = (data & mask) ? '1' : '0';
	}
	if (fields && num_digits == 16) {
		count = num_digits;  /* вывод слова с выделением полей */
		textcolor(CYAN);
		for (; count != 11; putch(string[--count]));
		textcolor(LIGHTRED);
		putch(string[--count]);
		textcolor(YELLOW);
		for (; count != 5; putch(string[--count]));
		textcolor(GREEN);
		for (; count != 0; putch(string[--count]));
		textcolor(LIGHTGRAY);
	} else     /* вывод без выделения полей */
		for (count = num_digits; --count >= 0; putch(string[count]));
}

void exe_open(char *str)
{
	char *psz1;
	char *argv[10];
	_INT16 iarg;
	int nExeResult;
#ifdef _TMK1553B_LINUX
	pid_t pid;
#endif

	while (*str == ' ' || *str == '\t') {
		str++;
	}
	if (*str == '\0') {
		msg_out(format_err);
		return;
	}
	iarg = 0;
	do {
		while (*str == ' ' || *str == '\t') {
			str++;
		}
		if (*str != '\0') {
			argv[iarg] = new char[32];
			psz1 = argv[iarg];
			while (*str != ' ' && *str != '\t' && *str != '\0') {
				*psz1++ = *str++;
			}
			*psz1 = '\0';
			++iarg;
		}
	} while (*str != '\0');
	clrscr();
#ifdef _TMK1553B_DOS
	if (strstr(argv[0], ".BAT") != NULL || strstr(argv[0], ".bat") != NULL) {
		_INT16 i;
		argv[iarg++] = new char[32];
		argv[iarg++] = new char[32];
		for (i = iarg - 3; i >= 0; --i) {
			strcpy(argv[i + 2], argv[i]);
		}
		strcpy(argv[1], "/C");
		strcpy(argv[0], "command.com");
		argv[iarg] = NULL;
		nExeResult = spawnvp(P_WAIT, argv[0], argv);
	} else
#endif
	{
		argv[iarg] = NULL;
#ifdef _TMK1553B_DOS
		nExeResult = spawnv(P_WAIT, argv[0], argv);
#endif
#ifdef _TMK1553B_LINUX
		pid = fork();
		if (pid == -1) {
			nExeResult = -1;
		} else if (pid) {
			wait(&nExeResult);
		} else {
			if (execv(argv[0], argv) == -1) {
				nExeResult = -1;
			}
		}
#endif
	}
	do {
		delete argv[--iarg];
	} while (iarg > 0);
	all_screen();
	if (nExeResult < 0) {
		msg_out(open_err);
		return;
	}
}

void cmds_open(char *str)   /* открытие командного файла */
{
	while (*str == ' ' || *str == '\t') {
		str++;
	}
	if (enter_mode) {           /* если командный файл вызывается из командного */
		fclose(cmd_file);
		enter_mode = 0;
	}
	if ((cmd_file = fopen(str, "rt")) == NULL) {
		msg_out(open_err);
		return;
	}
	strcpy(ex_fname, str);
	enter_mode = 1;
}

void cmds_open_f(char *str)
{
	while (*str == ' ' || *str == '\t') {
		str++;
	}
	if (strchr(str, '.')) {
		msg_out(inp_err);
		return;
	}
	prtscr_flag = 0;
	gotoxy(2, 11);
	textcolor(CYAN);
#ifdef ENG
	cputs("Waiting for command file. Press PrtScr to abort...");
#else
	cputs("Ожидание командного файла. Нажмите PrtScr для выхода...");
#endif
	textcolor(LIGHTGRAY);
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif

	fCmds_f = 0;
	strcat(str, ".f");
	while ((access(str, 0)) != 0 && !prtscr_flag) {
#ifdef _TMK1553B_LINUX
		bioskey_f12();
#endif
	}
	if (!prtscr_flag) {
		*strchr(str, '.') = '\0';
		cmds_open(str);
		fCmds_f = 1;
	}
	gotoxy(2, 11);
	if (nwin == nmainwin) {
		textcolor(WHITE);
		cputs("════════════════════════════════════════════════════════════════════════════");
	} else {
		cputs("────────────────────────────────────────────────────────────────────────────");
	}
	textcolor(LIGHTGRAY);
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

void cmds_close_f(char *str)
{
	prtscr_flag = 0;
	gotoxy(2, 11);
	textcolor(CYAN);
#ifdef ENG
	cputs("Waiting for command file end. Press PrtScr to abort...");
#else
	cputs("Ожидание конца командного файла. Нажмите PrtScr для выхода...");
#endif
	textcolor(LIGHTGRAY);
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	strcat(str, ".ff");
	if ((cmd_file = fopen(str, "wt")) == NULL) {
		msg_out(open_err);
		return;
	}
	fclose(cmd_file);
	*(strchr(str, '.') + 2) = '\0';
// strcat(str, ".f");
	while ((access(str, 0)) == 0 && !prtscr_flag) {
#ifdef _TMK1553B_LINUX
		bioskey_f12();
#endif
	}
// if (!prtscr_flag)
	{
		strcat(str, "f");
		unlink(str);
		*strchr(str, '.') = '\0';
	}
	gotoxy(2, 11);
	if (nwin == nmainwin) {
		textcolor(WHITE);
		cputs("════════════════════════════════════════════════════════════════════════════");
	} else {
		cputs("────────────────────────────────────────────────────────────────────────────");
	}
	textcolor(LIGHTGRAY);
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

void bc_fill_test()
{
	_INT16 base, savebase, maxbase, i;
	_UINT16 zbuffer[64];

	savebase = bcgetbase();
	maxbase = tmkMaxBase[tmkselected()];
	for (base = 0; base <= maxbase; base++) {
		bcdefbase(base);
		for (i = 0; i <= 63; i++) {
			zbuffer[i] = (base << 8) + i;
		}
		bcputblk(0, zbuffer, 64);
	}
	bcdefbase(savebase);
}

void rt_fill_test()
{
	_INT16 page, subadr, savepage, savesubadr, maxpage, i;
	_UINT16 zbuffer[32];

	savepage = rtgetpage();
	savesubadr = rtgetsubaddr();
	maxpage = rtgetmaxpage();
	for (page = 0; page <= maxpage; page++) {
		rtdefpage(page);
		for (subadr = 0; subadr <= 31; subadr++) {
			rtdefsubaddr(RT_RECEIVE, subadr);
			for (i = 0; i <= 31; i++) {
				zbuffer[i] = (page << 11) + RT_RECEIVE + (subadr << 5) + i;
			}
			rtputblk(0, zbuffer, 32);
			rtdefsubaddr(RT_TRANSMIT, subadr);
			for (i = 0; i <= 31; i++) {
				zbuffer[i] = (page << 11) + RT_TRANSMIT + (subadr << 5) + i;
			}
			rtputblk(0, zbuffer, 32);
		}
	}
	rtdefpage(savepage);
	rtdefsubaddr(savesubadr, savesubadr);
}

void bc_buf_rd(char *str) /* чтение кода управления и буфера из файла */
{
	_INT16 ch;
	char *ptr;

	_INT16 ctrl_code2, offset;
	static _UINT16 buffer2[BC_BUF_SIZE];

	while (*str == ' ' || *str == '\t') {
		str++;
	}
	if ((in_file = fopen(str, "rt")) == NULL) {
		msg_out(open_err);
		return;
	}
	while (*str != '\0') {
		str++;
	}
	str--;
	for (; *str != ':' && *str != '\\' && *str != ' ' && *str != '\t'; str--);
	str++;
	strcpy(Window[nwin].data_fname, str);
	out_fname();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	fgets(buf_str, 80, in_file); /* пропустить пустую строку */
	/* чтение кода управления из файла */
	fgets(buf_str, 80, in_file); /* пропустить строку подсказки */
	if ((fgets(buf_str, 80, in_file)) == NULL) {
		msg_out(format_err);
		return;
	}
	str = buf_str;
	while (*str == '\t' || *str == ' ') {
		str++;
	}
	ptr = str;
	while ((ch = *(ptr++)) != ' ') {
		if (ch != '0' && ch != '1') {
			msg_out(bad_ctrl_code);
			return;
		}
	}
	--ptr;
	if (ptr - str > CC_SIZE) {
		msg_out(bad_ctrl_code);
		return;
	}
	ptr = str;
	for (ctrl_code2 = 0; *ptr != ' '; ptr++) {
		ctrl_code2 <<= 1;
		if (*ptr == '1') {
			ctrl_code2++;
		}
	}
	if (!correct_cc(ctrl_code2)) {
#ifdef ENG
		sprintf(cc_no_def, "Wrong control code");
#else
		sprintf(cc_no_def, "Неверный код управления");
#endif
		msg_out(cc_no_def);
		return;
	}
	/* Чтение в буфер из файла */
	fgets(buf_str, 80, in_file); /* пропустить строку подсказки */
	offset = 0;
	while (offset != BC_DUMP_SIZE) {
#ifdef _TMK1553B_DOS
		if (fscanf(in_file, "%x ", &buffer2[offset]) != 1)
#endif
#ifdef _TMK1553B_LINUX
			if (fscanf(in_file, "%hx ", &buffer2[offset]) != 1)
#endif
			{
				msg_out(format_err);
				return;
			}
		if ((++offset % DUMP_WIDE) == 0)
			while ((ch = fgetc(in_file)) != '\n')
				if (ch == EOF) {
					msg_out(format_err);
					return;
				}
	}
	ctrl_code[tmkselected()][base] = ctrl_code2;
	out_ctrl_code();
	for (offset = 0; offset != BC_DUMP_SIZE; offset++) {
		buffer[offset] = buffer2[offset];
	}
	bcputblk(0, buffer, 36);
	out_buffer();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	fclose(in_file);
}

void bc_buf_wr(char *str)   /* запись буфера и кода управления в файл */
{

	_INT16 i, offset;
	_UINT16 ctrl_code2 = ctrl_code[tmkselected()][base];

	while (*str == ' ' || *str == '\t') {
		str++;
	}
	if ((out_file = fopen(str, "wt")) == NULL) {
		msg_out(cr_err);
		return;
	}
	strcpy(Window[nwin].data_fname, str);
	out_fname();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	{
		_INT16 mask;
		_INT16 count;
		fprintf(out_file, "\n");
#ifdef ENG
		fprintf(out_file, "Control Code:\n");
#else
		fprintf(out_file, "Код управления:\n");
#endif
		if (correct_cc(ctrl_code2))
			for (count = 0, mask = 8; count != 4; count++, mask >>= 1) {
				fprintf(out_file, "%c", ((ctrl_code2 & mask) ? '1' : '0'));
			} else {
#ifdef ENG
			fprintf(out_file, "XXXX (Not defined)\n");
#else
			fprintf(out_file, "XXXX (Не определен)\n");
#endif
			goto out_data;
		}
		switch (ctrl_code2 & 0x0007) {
#ifdef ENG
		case 0x0:
			fprintf(out_file, " (BC->RT");
			break;
		case 0x1:
			fprintf(out_file, " (RT->BC");
			break;
		case 0x2:
			fprintf(out_file, " (RT->RT");
			break;
		case 0x3:
			fprintf(out_file, " (CW->SW");
			break;
		case 0x4:
			fprintf(out_file, " (CW+DW->SW");
			break;
		case 0x5:
			fprintf(out_file, " (CW->SW+DW");
			break;
#else
		case 0x0:
			fprintf(out_file, " (КК->ОУ");
			break;
		case 0x1:
			fprintf(out_file, " (ОУ->КК");
			break;
		case 0x2:
			fprintf(out_file, " (ОУ->ОУ");
			break;
		case 0x3:
			fprintf(out_file, " (КС->ОС");
			break;
		case 0x4:
			fprintf(out_file, " (КС+ИС->ОС");
			break;
		case 0x5:
			fprintf(out_file, " (КС->ОС+ИС");
			break;
#endif
		}
		if (ctrl_code2 & 0x0008)
#ifdef ENG
			fprintf(out_file, ", broadcast)\n");
#else
			fprintf(out_file, ", групповая посылка)\n");
#endif
		else {
			fprintf(out_file, ")\n");
		}
	}
out_data:
	;
#ifdef ENG
	fprintf(out_file, "Buffer:\n");
#else
	fprintf(out_file, "Буфер:\n");
#endif
	for (offset = 0; offset < BC_DUMP_SIZE;) {
		for (i = 0; i < DUMP_WIDE; i++, ++offset)
			if (offset < BC_DUMP_SIZE) {
				fprintf(out_file, "%04X ", buffer[offset]);
			} else {
				fprintf(out_file, "     ");
			}
		fprintf(out_file, "   : %02u", offset - DUMP_WIDE);
		fprintf(out_file, "\n");
	}
	for (i = 0; i != DUMP_WIDE;  fprintf(out_file, "  %01X  ", i++));
	fprintf(out_file, "\n");
	fclose(out_file);
}

void rt_buf_rd(char *str)                               /* чтение в буфер из файла */
{
	_INT16 ch;

	_INT16 offset;
	_UINT16 buffer2[RT_BUF_SIZE];

	while (*str == ' ' || *str == '\t') {
		str++;
	}
	if ((in_file = fopen(str, "rt")) == NULL) {
		msg_out(open_err);
		return;
	}
	strcpy(Window[nwin].data_fname, str);
	out_fname();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	fgets(buf_str, 80, in_file); /* пропустить строку подсказки */
	fgets(buf_str, 80, in_file); /* пропустить строку подсказки */
	offset = 0;
	while (offset != RT_DUMP_SIZE) {
#ifdef _TMK1553B_DOS
		if (fscanf(in_file, "%x ", &buffer2[offset]) != 1)
#endif
#ifdef _TMK1553B_LINUX
			if (fscanf(in_file, "%hx ", &buffer2[offset]) != 1)
#endif
			{
				msg_out(format_err);
				return;
			}
		if ((++offset % DUMP_WIDE) == 0)
			while ((ch = fgetc(in_file)) != '\n')
				if (ch == EOF) {
					msg_out(format_err);
					return;
				}
	}
	for (offset = 0; offset != RT_DUMP_SIZE; offset++) {
		buffer[offset] = buffer2[offset];
	}
	rtputblk(0, buffer, 32);
	out_buffer();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	fclose(in_file);
}

void rt_buf_wr(char *str)                  /* запись буфера и кода управления в файл */
{

	_INT16 i, offset;

	while (*str == ' ' || *str == '\t') {
		str++;
	}
	if ((out_file = fopen(str, "wt")) == NULL) {
		msg_out(cr_err);
		return;
	}
	strcpy(Window[nwin].data_fname, str);
	out_fname();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
#ifdef ENG
	fprintf(out_file, "\nBuffer:\n");
#else
	fprintf(out_file, "\nБуфер:\n");
#endif
	for (offset = 0; offset < RT_DUMP_SIZE;) {
		for (i = 0; i < DUMP_WIDE; i++, ++offset)
			if (offset < RT_DUMP_SIZE) {
				fprintf(out_file, "%04X ", buffer[offset]);
			} else {
				fprintf(out_file, "     ");
			}
		fprintf(out_file, "   : %02u", offset - DUMP_WIDE);
		fprintf(out_file, "\n");
	}
	for (i = 0; i != DUMP_WIDE;  fprintf(out_file, "  %01X  ", i++));
	fprintf(out_file, "\n");
	fclose(out_file);
}

void set_base(char *str)
/* ввод подадреса из командной строки: 1 - ошибка, 0 - все в порядке */
{
	_UINT16 i;

#ifdef _TMK1553B_DOS
	if (sscanf(str, "%x", &i) != 1)
#endif
#ifdef _TMK1553B_LINUX
		if (sscanf(str, "%hx", &i) != 1)
#endif
		{
			msg_out(inp_err);
			return;
		}
	if (i > tmkMaxBase[tmkselected()]) {
		msg_out(base_err);
		return;
	}
	base = i;
	out_base();
	out_ctrl_code();
	bc_out_ram(base);
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

void set_ctrl_code(char *str)           /* команда установки кода управления */
{
	char ch, *ptr;
	_INT16 ctrl_code2;

	while (*str == '\t' || *str == ' ') {
		str++;
	}
	ptr = str;
	while ((ch = *(ptr++)) != '\0')
		if (ch != '0' && ch != '1') {
			msg_out(bad_ctrl_code);
			return;
		}
	--ptr;
	if (ptr - str > CC_SIZE) {
		msg_out(bad_ctrl_code);
		return;
	}
	ptr = str;
	for (ctrl_code2 = 0; *ptr != '\0'; ptr++) {
		ctrl_code2 <<= 1;
		if (*ptr == '1') {
			ctrl_code2++;
		}
	}
	if (!correct_cc(ctrl_code2)) {
#ifdef ENG
		sprintf(cc_no_def, "Control code %s not defined", str);
#else
		sprintf(cc_no_def, "Код управления %s не определен", str);
#endif
		msg_out(cc_no_def);
		return;
	}
	ctrl_code[tmkselected()][base] = ctrl_code2;
	out_ctrl_code();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

void set_number(char *str)               /* команда установки номера канала */
{
	_UINT16 i;

#ifdef _TMK1553B_DOS
	if (sscanf(str, "%01x", &i) != 1)
#endif
#ifdef _TMK1553B_LINUX
		if (sscanf(str, "%01hx", &i) != 1)
#endif
		{
			msg_out(inp_err);
			return;
		}
	if (i > 2 || i == 0) {
		msg_out(bus_num_err);
		return;
	}
	bus_num = (i == 1) ? BUS_1 : BUS_2;
	if (bcdefbus(bus_num)) {
		bus_num = BUS_1;
		msg_out(bus_num_err);
		return;
	}
	out_number();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

void set_count(char *str)       /* установка циклов повторения команды */
{
	unsigned long c;

#ifdef _TMK1553B_DOS
	if (sscanf(str, "%U", &c) != 1)
#endif
#ifdef _TMK1553B_LINUX
		if (sscanf(str, "%lu", &c) != 1)
#endif
		{
			msg_out(inp_err);
			return;
		}
	cmd_count = c;
	out_count();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

void set_pause(char *str)            /* команда установки длины тайм-аута */
{
	_UINT16 p;

#ifdef _TMK1553B_DOS
	if (sscanf(str, "%U", &p) != 1)
#endif
#ifdef _TMK1553B_LINUX
		if (sscanf(str, "%hu", &p) != 1)
#endif
		{
			msg_out(inp_err);
			return;
		}
	if (p > 27461) {
#ifdef ENG
		msg_out("Maximum allowable pause - 27461 us.");
#else
		msg_out("Максимальное значение паузы - 27461 мкс.");
#endif
		return;
	}
	opause = p;
#ifdef _TMK1553B_DOS
	_pause_ = (_UINT16)((unsigned long)p * 2L * 11932L / 10000L);
#endif
#ifdef _TMK1553B_LINUX
	_pause_ = opause;
#endif
	out_pause();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

void set_cpage(char *str)                 /* установка текущей страницы МК */
{
	_UINT16 p;

#ifdef _TMK1553B_DOS
	if (sscanf(str, "%u", &p) != 1)
#endif
#ifdef _TMK1553B_LINUX
		if (sscanf(str, "%hu", &p) != 1)
#endif
		{
			msg_out(inp_err);
			return;
		}
	switch (rtdefpagebus(p)) {
	case RT_BAD_FUNC:
#ifdef ENG
		msg_out("Can not define separate page for bus channel");
#else
		msg_out("Невозможно задать отдельно страницу БЗУ для МК");
#endif
		break;
	case RT_BAD_PAGE:
#ifdef ENG
		msg_out("Wrong page number");
#else
		msg_out("Неверное значение страницы БЗУ");
#endif
		break;
	case 0:
		cram_page = p;
		out_page();
#ifdef _TMK1553B_LINUX
		fflush(stdout);
#endif
		break;
	}
}

void set_spage(char *str)                 /* установка текущей страницы БЗУ */
{
	_UINT16 p;

#ifdef _TMK1553B_DOS
	if (sscanf(str, "%u", &p) != 1)
#endif
#ifdef _TMK1553B_LINUX
		if (sscanf(str, "%hu", &p) != 1)
#endif
		{
			msg_out(inp_err);
			return;
		}
	switch (rtdefpagepc(p)) {
	case RT_BAD_FUNC:
#ifdef ENG
		msg_out("Can not define separate page for PC");
#else
		msg_out("Невозможно задать отдельно страницу БЗУ для ПЭВМ");
#endif
		break;
	case RT_BAD_PAGE:
#ifdef ENG
		msg_out("Wrong page number");
#else
		msg_out("Неверное значение страницы БЗУ");
#endif
		break;
	case 0:
		bram_page = p;
		out_page();
		rtdefsubaddr(dir, subadr);
		rt_out_ram(dir, subadr);
#ifdef _TMK1553B_LINUX
		fflush(stdout);
#endif
		break;
	}
}

void set_page(const char *str)       /* установка текущей страницы БЗУ */
{
	_UINT16 p;

#ifdef _TMK1553B_DOS
	if (sscanf(str, "%u", &p) != 1)
#endif
#ifdef _TMK1553B_LINUX
		if (sscanf(str, "%hu", &p) != 1)
#endif
		{
			msg_out(inp_err);
			return;
		}
	if (rtdefpage(p))
#ifdef ENG
		msg_out("Wrong page number");
#else
		msg_out("Неверное значение страницы БЗУ");
#endif
	else {
		cram_page = bram_page = p;
		out_page();
		rtdefsubaddr(dir, subadr);
		rt_out_ram(dir, subadr);
#ifdef _TMK1553B_LINUX
		fflush(stdout);
#endif
	}
}

_INT16 wr_at(_INT16 at)
{
	_INT16 res;

	switch (res = rtdefaddress(at)) {
	case RT_BAD_FUNC:
#ifdef LPT_AT
		set_lpt_data(at);
		rtdefaddress(at);
		terminal_adr = at;
		res = 0;
#endif
	case RT_BAD_ADDRESS:
		break;
	case 0:
		terminal_adr = at;
	}
	return res;
}

void set_at(char *str)                          /* установка адреса ОУ */
{
	_UINT16 p;

#ifdef _TMK1553B_DOS
	if (sscanf(str, "%u", &p) != 1)
#endif
#ifdef _TMK1553B_LINUX
		if (sscanf(str, "%hu", &p) != 1)
#endif
		{
			msg_out(inp_err);
			return;
		}
#ifndef LPT_AT
	if (Window[nwin].at_avail)
#endif
		if (wr_at(p))
#ifdef ENG
			msg_out("Wrong RT address");
#else
			msg_out("Неверное значение адреса ОУ");
#endif
		else {
			out_at();
		}
#ifndef LPT_AT
	else
#ifdef ENG
		msg_out("Can not define RT address program");
#else
		msg_out("Невозможно программно задать адрес ОУ");
#endif
#endif
}

void set_subadr_r(char *str)      /* установка подадареса, зона приема */
{
	if (set_subadr(str)) {
		return;
	}
	dir = RT_RECEIVE;
	rtdefsubaddr(dir, subadr);
	out_subadr();
	rt_out_ram(dir, subadr);
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

void set_subadr_t(char *str)      /* установка подадареса, зона передачи */
{
	if (set_subadr(str)) {
		return;
	}
	dir = RT_TRANSMIT;
	rtdefsubaddr(dir, subadr);
	out_subadr();
	rt_out_ram(dir, subadr);
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

_INT16 set_subadr(char *str)            /* ввод подадреса из командной строки */
{
	/* 1 - ошибка, 0 - все в порядке */
	_UINT16 i;

#ifdef _TMK1553B_DOS
	if (sscanf(str, "%x", &i) != 1)
#endif
#ifdef _TMK1553B_LINUX
		if (sscanf(str, "%hx", &i) != 1)
#endif
		{
			msg_out(inp_err);
			return 1;
		}
	if (i > 31) {
		msg_out(subadr_err);
		return 1;
	}
	subadr = i;
	return 0;
}

void set_sw_bits(char *str, _INT16 mask)                    /* установка битов ОС */
{
	if (end_ctrl(str)) {
		return;
	}
	rtsetanswbits(mask);
	out_status_word();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

void clear_sw_bits(char *str, _INT16 mask)                  /* сброс битов ОС */
{
	if (end_ctrl(str)) {
		return;
	}
	rtclranswbits(mask);
	out_status_word();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

void set_status_word(const char *str)         /* команда установки ответного слова */
{
	char ch;
	const char *ptr;
	_INT16 status_word;

	while (*str == '\t' || *str == ' ') {
		str++;
	}
	ptr = str;
	while ((ch = *(ptr++)) != '\0')
		if (ch != '0' && ch != '1') {
			msg_out(bad_status_word);
			return;
		}
	--ptr;
	if (ptr - str > SW_SIZE) {
		msg_out(bad_status_word);
		return;
	}
	ptr = str;
	for (status_word = 0; *ptr != '\0'; ptr++) {
		status_word <<= 1;
		if (*ptr == '1') {
			status_word++;
		}
	}
	rtclranswbits(~status_word);
	rtsetanswbits(status_word);
	out_status_word();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

void select_window(_INT16 nwindow)
{
	nwin = nwindow;
	/*
	 if (avtomat)
	  nmainwin = nwin;
	*/
	tmkselect(Window[nwin].nTMK);
	switch (Window[nwin].nMode) {
	case BC_MODE:
		if (!avtomat) {
			base = bcgetbase();
		}
		DUMP_STRINGS = BC_DUMP_STRINGS;
		DUMP_SIZE = BC_DUMP_SIZE;
		buf_size = BC_BUF_SIZE;
		break;
	case RT_MODE:
		if (!avtomat) {
			subadr = rtgetsubaddr();
			dir = subadr & RT_TRANSMIT;
			subadr &= ~RT_TRANSMIT;
		}
		DUMP_STRINGS = RT_DUMP_STRINGS;
		DUMP_SIZE = RT_DUMP_SIZE;
		buf_size = RT_BUF_SIZE;
	}
	if (avtomat || fInt)
		window(Window[nwin].nX + 1, Window[nwin].nY,
		       Window[nwin].nX + Window[nwin].nDX - 3,
		       Window[nwin].nY + Window[nwin].nDY - 1);
}

void _wait_(_UINT16 pause)
{
#ifdef _TMK1553B_DOS
	_UINT16 time1, dtime;

	time1 = get_timer();
	while (time1 - get_timer() < pause);
#endif
#ifdef _TMK1553B_LINUX
	struct timeval time1, time2;
	unsigned long d_usec;

	gettimeofday(&time1, NULL);
	do {
		gettimeofday(&time2, NULL);
		d_usec = (time2.tv_sec - time1.tv_sec) * 1000000L +
		         time2.tv_usec - time1.tv_usec;
	} while (d_usec < pause);
#endif
}

#ifdef _TMK1553B_DOS
void far BCIntNorm(_UINT16 nResult, _UINT16 nothing1, _UINT16 nothing2)
{
	int_num[tmkselected()] = 1;
	bc_sw = nResult;
	good_starts++;
	bcrestore();
	nothing1;
	nothing2;
}

void far BCIntExc(_UINT16 nResult, _UINT16 AW1, _UINT16 AW2)
{
	int_num[tmkselected()] = 2;
	bc_sw = nResult;
	bc_aw1 = AW1;
	bc_aw2 = AW2;
	if (bad_starts < (unsigned long)BADLEN) {
		bad_sts[(unsigned)bad_starts] = bad_starts + good_starts + 1L;
		bad_sws[(unsigned)bad_starts] = bc_sw;
		bad_aws[(unsigned)bad_starts] = bc_aw1;
	}
	bad_starts++;
#ifdef LPT_INT_EXC
	set_lpt_data(0xFF);
	set_lpt_data(0);
#endif
	bcrestore();
}

void far RTIntCmd(_UINT16 nCmd)
{
	int_num[tmkselected()] = 1;
	rt_cmd = nCmd;
	rt_sw = rtgetstate();
	rtrestore();
}

void far RTIntErr(_UINT16 nStatus)
{
	int_num[tmkselected()] = 2;
	rt_sw = nStatus;
	if ((rt_sw & (G1_MASK | G2_MASK)) != 0) {
		rtreset();
	}
	bad_starts += 1000L;
	rtrestore();
}

void far RTIntData(_UINT16 nStatus)
{
	int_num[tmkselected()] = 3;
	rt_sw = nStatus;
	rtrestore();
}
#endif

void reset_mode()
{
	_INT16 mode = tmkgetmode();
	*Window[nwin].data_fname = '\0';
	if (mode == BC_MODE) {
		Window[nwin].nMode = BC_MODE;
		bus_num = bcgetbus();
		base = bcgetbase();
		buf_size = BC_BUF_SIZE;
	} else if (mode == RT_MODE) {
		Window[nwin].nMode = RT_MODE;
		if (hb_mode) {
			rtdefmode(rtgetmode() | RT_HBIT_MODE);
		} else {
			rtdefmode(rtgetmode() & (~RT_HBIT_MODE));
		}
		rt_sw = rtgetstate();
		bram_page = cram_page = rtgetpage();
		subadr = rtgetsubaddr();
		dir = RT_TRANSMIT & subadr;
		subadr &= ~RT_TRANSMIT;
		Window[nwin].at_avail = (rtdefaddress(rtgetaddress())) ? 0 : 1;
		buf_size = RT_BUF_SIZE;
	}
}

void set_mode(_INT16 mode)
{
	*Window[nwin].data_fname = '\0';
	if (mode == BC_MODE) {
		bcreset();
		Window[nwin].nMode = BC_MODE;
#ifdef _TMK1553B_LINUX
		CheckTmkEvent(0);
#endif
		int_num[Window[nwin].nTMK] = 0;
		bcdefbus(bus_num = BUS_1);
		bcdefbase(base = 0);
		buf_size = BC_BUF_SIZE;
	} else if (mode == RT_MODE) {
		rtreset();
		Window[nwin].nMode = RT_MODE;
#ifdef _TMK1553B_LINUX
		CheckTmkEvent(0);
#endif
		int_num[Window[nwin].nTMK] = 0;
		if (hb_mode) {
			rtdefmode(rtgetmode() | RT_HBIT_MODE);
		} else {
			rtdefmode(rtgetmode() & (~RT_HBIT_MODE));
		}
		rt_sw = rtgetstate();
		rtdefpage(bram_page = cram_page = 0);
		rtdefsubaddr(dir = RT_RECEIVE, subadr = 1);
		Window[nwin].at_avail = (rtdefaddress(0)) ? 0 : 1;
		buf_size = RT_BUF_SIZE;
		wr_at(10);
	}
}

char szCfgName[16];
char szCmd[16];

void TmkInit0()
{
	_UINT16 iTMK;

	if (TmkInit(szCfgName)) {
#ifdef ENG
		printf("Configuration load error from the file %s\n", szCfgName);
#else
		printf("Ошибка загрузки конфигурации из файла %s\n", szCfgName);
#endif
		tmkdone(ALL_TMKS);
		exit(0);
	}
#ifdef LPT
	init_lpt();
#endif
	nwin = 0;
	if (tmkMaxN > tmkgetmaxn()) {
		tmkMaxN = tmkgetmaxn();
	}
	for (iTMK = 0; iTMK <= tmkMaxN; iTMK++) {
		tmkMaxBase[iTMK] = TMKCPP_MAX_BASE;
		if (tmkselect(iTMK)) {
			continue;
		}
		if (TMKCPP_MAX_BASE > bcgetmaxbase()) {
			tmkMaxBase[iTMK] = bcgetmaxbase();
		}
#ifdef _TMK1553B_DOS
		switch (aTmkConfig[iTMK].nType) {
		default:
			if (nTest <= 0) {
				break;
			}
		case RTMK:
		case RTMK1:
		case TMK400:
		case TMKMPC:
		case RTMK400:
			if (nTest < 0) {
				break;
			}
			if (TMK_TuneIODelay(0)) {
				while (kbhit()) {
					getch();
				}
#ifdef ENG
				printf("Initial TMK %d RAM test error.\n"
				       "Can not determine I/O delay for TMK %d.\n"
				       "Press any key to continue.\n",
				       iTMK, iTMK);
#else
				printf("Ошибка начального теста памяти TMK %d.\n"
				       "Невозможно подобрать задержку ввода-вывода для TMK %d.\n"
				       "Нажмите любую клавишу для продолжения.\n",
				       iTMK, iTMK);
#endif
				while (!kbhit());
				getch();
			}
			break;
		}
		bcdefintnorm(BCIntNorm);
		bcdefintexc(BCIntExc);
		rtdefintcmd(RTIntCmd);
		rtdefinterr(RTIntErr);
		rtdefintdata(RTIntData);
#endif
		if (nwin > nMaxWin) {
			continue;
		}
		if (nwin == bcwin) {
			set_mode(BC_MODE);
		} else {
			set_mode(RT_MODE);
		}
		Window[nwin].fVisible = 1;
		Window[nwin].nTMK = iTMK;
		Window[nwin++].nType = -1;
	}
	select_window(nmainwin);
	all_screen();
}

int main(int argc, char *argv[])
{
	_INT16 i;

#ifdef _TMK1553B_LINUX
	set_input_mode();
	set_output_mode();
	gettimeofday(&biostime1, NULL);
#endif

	strcpy(szCfgName, "tmk.cfg");
	*szCmd = '\0';
	nTest = 0;
#ifdef _TMK1553B_DOS
	for (i = 1; i <= argc; ++i)
#endif
#ifdef _TMK1553B_LINUX
		for (i = 1; i < argc; ++i)
#endif
		{
			switch (argv[i][0]) {
			case 'c':
			case 'C':
				strncpy(szCfgName, argv[i] + 1, 16);
				break;
			case 'r':
			case 'R':
				strncpy(szCmd, argv[i] + 1, 16);
				break;
			case 't':
			case 'T':
				switch (argv[i][1]) {
				case '+':
					nTest = 1;
					break;
				case '-':
					nTest = -1;
					break;
				}
				break;
			}
		}
#ifdef _TMK1553B_DOS
	textmode(C80);        /* параметры экрана */
#endif
	textbackground(BLACK);
	textcolor(LIGHTGRAY);
	clrscr();
#ifdef _TMK1553B_DOS
	_wscroll = 0;
	oldvect3 = getvect(0x05);      /* сохранить старое значение вектора */
	setvect(0x05, prtscr_brk);     /* установить новые значения вектора */
#endif

	bcwin = 0;
	TmkInit0();

	while (1) {
		/* ввод и выполнение команд */
		window(1, 1, 80, 25);
		if (*szCmd != '\0') {
			strcpy(cmd_str, szCmd);
			*szCmd = '\0';
		} else {
			in_cmd(cmd_str);
		}
		window(Window[nwin].nX + 1, Window[nwin].nY,
		       Window[nwin].nX + Window[nwin].nDX - 2,
		       Window[nwin].nY + Window[nwin].nDY - 1);
		make_cmd(cmd_str);
	}
	return 0;
}

void block_data_irq(const char *str)    /* блокировка прерывания данных */
{
	if (end_ctrl(str)) {
		return;
	}
	rtdefirqmode(rtgetirqmode() | RT_DATA_BL);
//  out_mode();
}

void unblock_data_irq(const char *str)    /* разблокировка прерывания данных */
{
	if (end_ctrl(str)) {
		return;
	}
	rtdefirqmode(rtgetirqmode() & (~RT_DATA_BL));
//  out_mode();
}

void set_hbit_mode(char *str)    /* команда управления режимом
                                    аппаратного бита */
{
	while (*str == '\t' || *str == ' ') {
		str++;
	}
	switch (*str) {
	case '0':
		rtdefmode(rtgetmode() & (~RT_HBIT_MODE));
		hb_mode = 0;
		break;
	case '1':
		rtdefmode(rtgetmode() | RT_HBIT_MODE);
		hb_mode = 1;
		break;
	}
	out_rt_mode();
}

void set_flag_mode(char *str)    /* команда управления режимом флагов */
{
	while (*str == '\t' || *str == ' ') {
		str++;
	}
	switch (*str) {
	case '0':
		rtdefmode(rtgetmode() & (~RT_FLAG_MODE));
		break;
	case '1':
		rtdefmode(rtgetmode() | RT_FLAG_MODE);
		break;
	}
//  out_mode();
}

void set_brcst_mode(char *str)    /* команда управления режимом
                                     групповых команд */
{
	while (*str == '\t' || *str == ' ') {
		str++;
	}
	switch (*str) {
	case '0':
		rtdefmode(rtgetmode() & (~RT_BRCST_MODE));
		break;
	case '1':
		rtdefmode(rtgetmode() | RT_BRCST_MODE);
		break;
	}
//  out_mode();
}

void make_cmd(char *str)
/* обработка командной строки, переход к выполнению команд */
{
	if (*cmd_str == '\0') {
		switch (Window[nwin].nMode) {
		case BC_MODE:
			out_base();
			out_ctrl_code();
			bc_out_ram(base);
#ifdef _TMK1553B_LINUX
			fflush(stdout);
#endif
			break;
		case RT_MODE:
			out_subadr();
			rt_out_ram(dir, subadr);
#ifdef _TMK1553B_LINUX
			fflush(stdout);
#endif
			break;
		}
	} else {
		strcpy(old_cmd, cmd_str);
	}
	while (*str == ' ' || *str == '\t') {
		str++;
	}
	if (*str == ':') {
		if (enter_mode && fGoto && !strcmp(str + 1, szGoto)) {
			fGoto = 0;
		}
	} else if (fGoto)
		;
	else if (!strcmp(str, "Q")) {
		make_quit("");
	} else if (!strncmp(str, "IOD ", 4)) {
		set_io_delay(str + 4);
	} else if (!strcmp(str, "I")) {
		cmd_reset("");
	} else if (!strcmp(str, "F")) {
		fields_chg("");
	} else if (!strcmp(str, "BE")) {
		buf_edit("");
	} else if (!strncmp(str, "BE ", 3)) {
		buf_edit(str + 3);
	} else if (!strcmp(str, "BC")) {
		buf_clear("");
	} else if (!strncmp(str, "BF ", 3)) {
		buf_fill(str + 3);
	} else if (!strncmp(str, "SL ", 3)) {
		set_stat_len(str + 3);
	} else if (!strncmp(str, "WIN ", 4)) {
		select_win(str + 4);
	} else if (!strncmp(str, "TMK ", 4)) {
		select_tmk(str + 4);
	} else if (!strcmp(str, "MBC")) {
		mode_bc("");
	} else if (!strcmp(str, "MRT")) {
		mode_rt("");
	}
// else if (!strcmp(str, "MMT"))
//  mode_mt("");
	else if (!strncmp(str, "AVT ", 4)) {
		avt_ctrl(str + 4);
	} else if (!strncmp(str, "ATBCSA", 6)) {
		fReserv = 0;
		fSimpleRT = 1;
		fSimpleA = 1;
		avt_bc(str + 6);
		fSimpleRT = 0;
	} else if (!strncmp(str, "RATBCSA", 7)) {
		fReserv = 1;
		fSimpleRT = 1;
		fSimpleA = 1;
		avt_bc(str + 7);
		fSimpleRT = 0;
	} else if (!strncmp(str, "ATRTSA", 6)) {
		fReserv = 0;
		fSimpleRT = 1;
		fSimpleA = 1;
		avt_rt(str + 6);
		fSimpleRT = 0;
	} else if (!strncmp(str, "RATRTSA", 7)) {
		fReserv = 1;
		fSimpleRT = 1;
		fSimpleA = 1;
		avt_rt(str + 7);
		fSimpleRT = 0;
	} else if (!strncmp(str, "ATBCS", 5)) {
		fReserv = 0;
		fSimpleRT = 1;
		fSimpleA = 0;
		avt_bc(str + 5);
		fSimpleRT = 0;
	} else if (!strncmp(str, "RATBCS", 6)) {
		fReserv = 1;
		fSimpleRT = 1;
		fSimpleA = 0;
		avt_bc(str + 6);
		fSimpleRT = 0;
	} else if (!strncmp(str, "ATRTS", 5)) {
		fReserv = 0;
		fSimpleRT = 1;
		fSimpleA = 0;
		avt_rt(str + 5);
		fSimpleRT = 0;
	} else if (!strncmp(str, "RATRTS", 6)) {
		fReserv = 1;
		fSimpleRT = 1;
		fSimpleA = 0;
		avt_rt(str + 6);
		fSimpleRT = 0;
	} else if (!strncmp(str, "ATBC", 4)) {
		fReserv = 0;
		avt_bc(str + 4);
	} else if (!strncmp(str, "RATBC", 5)) {
		fReserv = 1;
		avt_bc(str + 5);
	} else if (!strncmp(str, "ATRT", 4)) {
		fReserv = 0;
		avt_rt(str + 4);
	} else if (!strncmp(str, "RATRT", 5)) {
		fReserv = 1;
		avt_rt(str + 5);
	}
#ifdef ELCUS
	else if (!strncmp(str, "ATS1", 4)) {
		fReserv = 0;
		fStatErrStop = 1;
		avt_stat(str + 4);
		fStatErrStop = 0;
	} else if (!strncmp(str, "RATS1", 5)) {
		fReserv = 1;
		fStatErrStop = 1;
		avt_stat(str + 5);
		fStatErrStop = 0;
	}
#endif //def ELCUS
	else if (!strncmp(str, "ATS", 3)) {
		fReserv = 0;
		avt_stat(str + 3);
	} else if (!strncmp(str, "RATS", 4)) {
		fReserv = 1;
		avt_stat(str + 4);
	}
#ifdef ELCUS
	else if (!strncmp(str, "ATARR1", 6)) {
		fATArBlk = 0;
		avt_arb_rt_r(str + 6);
	} else if (!strncmp(str, "ATARR", 5)) {
		fATArBlk = 1;
		avt_arb_rt_r(str + 5);
	} else if (!strncmp(str, "ATARW", 5)) {
		avt_arb_rt_w(str + 5);
	} else if (!strncmp(str, "ATARF", 5)) {
		avt_arb_rt_f(str + 5);
	} else if (!strncmp(str, "BA", 2)) {
		avt_va996(str + 2);
	} else if (!strncmp(str, "WORDS ", 6)) {
		set_a_words(str + 6);
	}
#endif //def ELCUS
	else if (!strncmp(str, "ATFL", 4)) {
		avt_flags(str + 4);
	} else if (!strcmp(str, "STOP?")) {
		if (enter_mode && (avt_err || to_errors > 0L || bad_starts > 0L ||
		                   channel_err > 0L || data_err > 0L || (kbhit() && getch() == ESCAPE))) {
			fSkipCmds = 1;
		}
	} else if (!strcmp(str, "BCRT+")) {
		fBcRt = 1;
	} else if (!strcmp(str, "BCRT-")) {
		fBcRt = 0;
	} else if (!strcmp(str, "BEEP+")) {
		fBeep = 1;
	} else if (!strcmp(str, "BEEP-")) {
		fBeep = 0;
	} else if (!strcmp(str, "BEEP")) {
		beep("1");
	} else if (!strncmp(str, "BEEP ", 5)) {
		beep(str + 5);
	}
#ifdef LPT_SYN
	else if (!strcmp(str, "LP+")) {
		fLPTSyn = 1;
	} else if (!strcmp(str, "LP-")) {
		fLPTSyn = 0;
	}
#endif
#ifdef ELCUS
	else if (!strncmp(str, "OW ", 3)) {
		tmk_outpw(str + 3);
	}
#endif //def ELCUS
#ifdef TMK_DAC
	else if (!strncmp(str, "DAC ", 4)) {
		tmk_outdac(str + 4);
	}
#endif //TMK_DAC
	else if (!strcmp(str, "P")) {
		kbd_pause("");
	} else if (!strncmp(str, "P ", 2)) {
		kbd_pause(str + 1);
	} else if (!strncmp(str, ";", 1)) {
		comment(str + 1);
	} else if (!strncmp(str, "GOTO ", 5)) {
		if (enter_mode) {
			fseek(cmd_file, 0L, SEEK_SET);
			fGoto = 1;
			strcpy(szGoto, str + 5);
		}
	} else if (!strncmp(str, "#", 1))
		;
	else if (Window[nwin].nMode == BC_MODE) {
		if (!strncmp(str, "SB ", 3)) {
			set_base(str + 3);
		} else if (!strncmp(str, "ERR ", 4)) {
			set_max_err(str + 4);
		} else if (!strncmp(str, "SN ", 3)) {
			set_number(str + 3);
		} else if (!strncmp(str, "SP ", 3)) {
			set_pause(str + 3);
		} else if (!strncmp(str, "SC ", 3)) {
			set_ctrl_code(str + 3);
		} else if (!strncmp(str, "GLF", 3)) {
			start_loop_file(str + 3);
		} else if (!strncmp(str, "GL", 2)) {
			start_loop(str + 2);
		} else if (!strcmp(str, "G")) {
			start_1("");
		} else if (!strcmp(str, "STO")) {
			statistic_out("");
		} else if (!strcmp(str, "STBO")) {
			statistic_bad_out("");
		} else if (!strcmp(str, "STC")) {
			statistic_clear("");
		} else if (!strncmp(str, "R ", 2)) {
			bc_buf_rd(str + 2);
		} else if (!strncmp(str, "W ", 2)) {
			bc_buf_wr(str + 2);
		} else if (!strncmp(str, "R< ", 3) && cmd_count == 1) {
			cmds_open(str + 3);
		} else if (!strncmp(str, "R< ", 3) && cmd_count != 1) {
			cmds_open2(str + 3);
		} else if (!strncmp(str, "RF< ", 4)) {
			cmds_open_f(str + 3);
		} else if (!strncmp(str, "C ", 2)) {
			set_count(str + 2);
		} else if (!strncmp(str, "RE ", 3)) {
			exe_open(str + 3);
		} else if (!strcmp(str, "T")) {
			bc_ram_test("");
		} else if (!strcmp(str, "H")) {
			bc_help("");
		}
#ifdef ELCUS
		else if (!strncmp(str, "SYM", 3)) {
			sym(str + 3);
		} else if (!strncmp(str, "OU400", 5)) {
			fReserv = 0;
			avt_ou400(str + 5);
		} else if (!strncmp(str, "ROU400", 6)) {
			fReserv = 1;
			avt_ou400(str + 6);
		}
#endif //def ELCUS
		else if (*str != '\0') {
			msg_out(undef_cmd);
		}
	} else if (Window[nwin].nMode == RT_MODE) {
		if (!strncmp(str, "SW ", 3)) {
			set_status_word(str + 3);
		} else if (!strncmp(str, "HBIT ", 5)) {
			set_hbit_mode(str + 5);
			out_rt_mode();
#ifdef _TMK1553B_LINUX
			fflush(stdout);
#endif
		} else if (!strncmp(str, "FLAG ", 5)) {
			set_flag_mode(str + 5);
			out_rt_mode();
#ifdef _TMK1553B_LINUX
			fflush(stdout);
#endif
		} else if (!strncmp(str, "REJ ", 4)) {
			set_flag_mode(str + 4);
			out_rt_mode();
#ifdef _TMK1553B_LINUX
			fflush(stdout);
#endif
		} else if (!strncmp(str, "BCST ", 5)) {
			set_brcst_mode(str + 5);
			out_rt_mode();
#ifdef _TMK1553B_LINUX
			fflush(stdout);
#endif
		} else if (!strcmp(str, "DTBL 1")) {
			block_data_irq("");
			out_rt_mode();
#ifdef _TMK1553B_LINUX
			fflush(stdout);
#endif
		} else if (!strcmp(str, "DTBL 0")) {
			unblock_data_irq("");
			out_rt_mode();
#ifdef _TMK1553B_LINUX
			fflush(stdout);
#endif
		} else if (!strncmp(str, "SFL", 3)) {
			set_flag(str + 3);
		}
#ifdef ELCUS
		else if (!strcmp(str, "WRC")) {
			wr_mr_loop(str + 3);
		} else if (!strcmp(str, "RDC")) {
			rd_sw_loop(str + 3);
		}
#endif //def ELCUS
		else if (!strcmp(str, "SSR")) {
			set_sw_bits(str + 3, SREQ);
		} else if (!strcmp(str, "SBS")) {
			set_sw_bits(str + 3, BUSY);
		} else if (!strcmp(str, "SSF")) {
			set_sw_bits(str + 3, SSFL);
		} else if (!strcmp(str, "STF")) {
			set_sw_bits(str + 3, RTFL);
		} else if (!strcmp(str, "SDN")) {
			set_sw_bits(str + 3, DNBA);
		} else if (!strcmp(str, "CSR")) {
			clear_sw_bits(str + 3, SREQ);
		} else if (!strcmp(str, "CBS")) {
			clear_sw_bits(str + 3, BUSY);
		} else if (!strcmp(str, "CSF")) {
			clear_sw_bits(str + 3, SSFL);
		} else if (!strcmp(str, "CTF")) {
			clear_sw_bits(str + 3, RTFL);
		} else if (!strcmp(str, "CDN")) {
			clear_sw_bits(str + 3, DNBA);
		} else if (!strcmp(str, "SMR")) {
			rtlock(dir, subadr);
			fLock = 1;
		} else if (!strcmp(str, "CMR")) {
			rtunlock();
			fLock = 0;
		} else if (!strncmp(str, "SR ", 3)) {
			set_subadr_r(str + 3);
		} else if (!strncmp(str, "ST ", 3)) {
			set_subadr_t(str + 3);
		} else if (!strncmp(str, "SPG ", 4)) {
			set_spage(str + 4);
		} else if (!strncmp(str, "CPG ", 4)) {
			set_cpage(str + 4);
		} else if (!strncmp(str, "PG ", 3)) {
			set_page(str + 3);
		} else if (!strncmp(str, "SAT ", 4)) {
			set_at(str + 4);
		} else if (!strcmp(str, "RSW")) {
			read_sw("");
		} else if (!strncmp(str, "STP", 3)) {
			read_sp(str + 3);
		} else if (!strncmp(str, "R ", 2)) {
			rt_buf_rd(str + 2);
		} else if (!strncmp(str, "W ", 2)) {
			rt_buf_wr(str + 2);
		} else if (!strncmp(str, "R< ", 3)) {
			cmds_open(str + 3);
		} else if (!strncmp(str, "RF< ", 4)) {
			cmds_open_f(str + 3);
		} else if (!strcmp(str, "T")) {
			rt_ram_test("");
		} else if (!strcmp(str, "H")) {
			rt_help("");
		} else if (*str != '\0') {
			msg_out(undef_cmd);
		}
	}
}

void make_quit(const char *str)                   /* поддержка выхода из отладчика */
{
	if (end_ctrl(str)) {
		return;
	}
#ifdef _TMK1553B_DOS
	setvect(0x05, oldvect3);    /* восстановить вектор */
#endif
	tmkdone(ALL_TMKS);
	window(1, 1, 80, 25);
	clrscr();
#ifdef ENG
	cputs("Program has finished.");
#else
	cputs("Работа программы закончена.");
#endif
	exit(0);
}

void tmk_outpw(char *str)
{
	_UINT16 wAddr, wData;

	while (*str == '\t' || *str == ' ') {
		str++;
	}
#ifdef _TMK1553B_DOS
	if (*str == '\0' || sscanf(str, "%X", &wAddr) != 1)
#endif
#ifdef _TMK1553B_LINUX
		if (*str == '\0' || sscanf(str, "%hX", &wAddr) != 1)
#endif
		{
			msg_out(inp_err);
			return;
		}
	while (isxdigit(*str)) {
		str++;
	}
	while (*str == '\t' || *str == ' ') {
		str++;
	}
#ifdef _TMK1553B_DOS
	if (*str == '\0' || sscanf(str, "%X", &wData) != 1)
#endif
#ifdef _TMK1553B_LINUX
		if (*str == '\0' || sscanf(str, "%hX", &wData) != 1)
#endif
		{
			msg_out(inp_err);
			return;
		}
	outport(wAddr, wData);
}

#ifdef TMK_DAC
void tmk_outdac(char *str)
{
	_UINT16 wDac;

	while (*str == '\t' || *str == ' ') {
		str++;
	}
	if (*str == '\0' || sscanf(str, "%X", &wDac) != 1) {
		msg_out(inp_err);
		return;
	}
	tmkdefdac(wDac);
}
#endif //def TMK_DAC

void set_io_delay(char *str)
{
	_UINT16 nDelay;

	while (*str == '\t' || *str == ' ') {
		str++;
	}
#ifdef _TMK1553B_DOS
	if (*str == '\0' || sscanf(str, "%U", &nDelay) != 1)
#endif
#ifdef _TMK1553B_LINUX
		if (*str == '\0' || sscanf(str, "%hu", &nDelay) != 1)
#endif
		{
			msg_out(inp_err);
			return;
		}
#ifdef _TMK1553B_DOS
	tmkiodelay(nDelay);
#endif
	out_type();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

void set_max_err(char *str)
{
	unsigned long dwErrs;

	while (*str == '\t' || *str == ' ') {
		str++;
	}
#ifdef _TMK1553B_DOS
	if (*str == '\0' || sscanf(str, "%U", &dwErrs) != 1)
#endif
#ifdef _TMK1553B_LINUX
		if (*str == '\0' || sscanf(str, "%lu", &dwErrs) != 1)
#endif
		{
			msg_out(inp_err);
			return;
		}
	dwMaxErr = dwErrs;
	err_level = dwErrs;
	out_maxerrs();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

void select_win(char *str)
{
	_INT16 nwin;

	while (*str == '\t' || *str == ' ') {
		str++;
	}
#ifdef _TMK1553B_DOS
	if (*str == '\0' || sscanf(str, "%U", &nwin) != 1)
#endif
#ifdef _TMK1553B_LINUX
		if (*str == '\0' || sscanf(str, "%hu", &nwin) != 1)
#endif
		{
			msg_out(inp_err);
			return;
		}
	if (nwin > 1) {
#ifdef ENG
		msg_out("Window number range is from 0 to 1");
#else
		msg_out("Допустимый диапазон номеров окон от 0 до 1");
#endif
		return;
	}
	nmainwin = nwin;
// int_num[Window[0].nTMK] = 0;
// int_num[Window[1].nTMK] = 0;
	select_window(nmainwin);
	all_screen();
}

void mode_bc(const char *str)
{
	if (end_ctrl(str)) {
		return;
	}
#ifdef _TMK1553B_LINUX
	CheckTmkEvent(0);
#endif
	int_num[Window[nwin].nTMK] = 0;
	set_mode(BC_MODE);
	base = 0;
	bcdefbase(0);
	all_screen();
}

void mode_rt(const char *str)
{
	if (end_ctrl(str)) {
		return;
	}
	set_mode(RT_MODE);
	cram_page = bram_page = 0;
	rtdefpage(0);
	dir = RT_RECEIVE;
	subadr = 1;
	rtdefsubaddr(dir, subadr);
	all_screen();
}

void select_tmk(char *str)
{
	_INT16 ntmk;

	while (*str == '\t' || *str == ' ') {
		str++;
	}
#ifdef _TMK1553B_DOS
	if (*str == '\0' || sscanf(str, "%U", &ntmk) != 1)
#endif
#ifdef _TMK1553B_LINUX
		if (*str == '\0' || sscanf(str, "%hu", &ntmk) != 1)
#endif
		{
			msg_out(inp_err);
			return;
		}
	if (ntmk > 3) {
#ifdef ENG
		msg_out("Device number range is from 0 to 3");
#else
		msg_out("Допустимый диапазон номеров устройств от 0 до 3");
#endif
		return;
	}
	if (tmkselect(ntmk)) {
#ifdef ENG
		msg_out("Device with specified number has not defined in configuration file");
#else
		msg_out("Устройство с таким номером не задано в файле конфигурации");
#endif
		return;
	}
	if (ntmk == Window[!nmainwin].nTMK) {
//  set_mode(Window[!nmainwin].nMode);
#ifdef ENG
		msg_out("Can not define same board in both windows");
#else
		msg_out("Нельзя задать одну и ту же плату для обоих окон");
#endif
		return;
	}
	nwin = nmainwin;
	switch (tmkgetmode()) {
	case BC_MODE:
	case RT_MODE:
		reset_mode();
		break;
	case UNDEFINED_MODE:
		set_mode(BC_MODE);
		break;
	}
	Window[nmainwin].nTMK = ntmk;
	Window[nmainwin].nType = -1;
	select_window(nmainwin);
	all_screen();
}

void set_stat_len(char *str)
{
	unsigned long cnt;

	while (*str == '\t' || *str == ' ') {
		str++;
	}
	if (*str != '\0') {
#ifdef _TMK1553B_DOS
		if (sscanf(str, "%U", &cnt) != 1)
#endif
#ifdef _TMK1553B_LINUX
			if (sscanf(str, "%lu", &cnt) != 1)
#endif
			{
				msg_out(inp_err);
				return;
			}
		GL_COUNTER = cnt;
		GL_COUNTER2 = cnt * 2;
		GL_COUNTER3 = cnt * 4;
	}
	out_glcount();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

#ifdef ELCUS
void sym(char *str)                                      /* симметрирование */
{
	_INT16 cur_base;

	if (end_ctrl(str)) {
		return;
	}
	cur_base = base;
	/* уст. кода управления */
	/* занесение констант в базы */
	set_base("0");
	buf_fill("1, 32, AAAA");
	ctrl_code[tmkselected()][base] = DATA_BC_RT_BRCST;
	bcputw(0, (0x1F << 11) | RT_RECEIVE | (0x11 << 5));
	set_base("1");
	buf_fill("1, 32, 0000");
	ctrl_code[tmkselected()][base] = DATA_BC_RT_BRCST;
	bcputw(0, (0x1F << 11) | RT_RECEIVE | (0x11 << 5));
	set_base("2");
	buf_fill("1, 32, 5555");
	ctrl_code[tmkselected()][base] = DATA_BC_RT_BRCST;
	bcputw(0, (0x1F << 11) | RT_RECEIVE | (0x11 << 5));
	set_base("3");
	buf_fill("1, 32, FFFF");
	ctrl_code[tmkselected()][base] = DATA_BC_RT_BRCST;
	bcputw(0, (0x1F << 11) | RT_RECEIVE | (0x11 << 5));
	set_base("4");
	buf_fill("1, 32, 8000");
	ctrl_code[tmkselected()][base] = DATA_BC_RT_BRCST;
	bcputw(0, (0x1F << 11) | RT_RECEIVE | (0x11 << 5));
	set_base("5");
	buf_fill("1, 32, 7FFF");
	ctrl_code[tmkselected()][base] = DATA_BC_RT_BRCST;
	bcputw(0, (0x1F << 11) | RT_RECEIVE | (0x11 << 5));
	gotoxy(1, Window[nwin].nDY);
	textcolor(CYAN);
#ifdef ENG
	cprintf("Symmetry test. Press PrtScr to abort...");
#else
	cprintf("Поддержка симметрирования. Нажмите PrtScr для выхода...");
#endif
	textcolor(LIGHTGRAY);
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	while (!prtscr_flag) {
#ifdef _TMK1553B_LINUX
		bioskey_f12();
#endif
		base = 0;
		start();
		base = 1;
		start();
		base = 2;
		start();
		base = 3;
		start();
		base = 4;
		start();
		base = 5;
		start();
	}
	prtscr_flag = 0;
	textcolor(WHITE);
	gotoxy(1, Window[nwin].nDY);
	hor_line(Window[nwin].nDX);
	textcolor(LIGHTGRAY);
	base = cur_base;
	out_base();
	out_ctrl_code();
	bc_out_ram(base);
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}
#endif //def ELCUS

void bc_clear_ram()
{
	_INT16 k, maxbase;

	maxbase = tmkMaxBase[tmkselected()];
	for (k = 0; k <= 63; ++k) {
		buffer[k] = 0;
	}
	for (base = 0; base <= maxbase; base++) {
		bcdefbase(base);
		bcputblk(0, buffer, 64);
	}
	base = 0;
	bcdefbase(0);
	out_base();
	out_ctrl_code();
	out_buffer();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

void rt_clear_ram()
{
	_INT16 k, page, maxpage;

	maxpage = rtgetmaxpage();
	for (k = 0; k <= 31; ++k) {
		buffer[k] = 0;
	}
	for (page = 0; page <= maxpage; page++) {
		rtdefpage(page);
		for (subadr = 0; subadr <= 31; subadr++) {
			rtdefsubaddr(RT_RECEIVE, subadr);
			rtputblk(0, buffer, 32);
			rtdefsubaddr(RT_TRANSMIT, subadr);
			rtputblk(0, buffer, 32);
		}
	}
	cram_page = bram_page = 0;
	rtdefpage(0);
	dir = RT_RECEIVE;
	subadr = 1;
	rtdefsubaddr(dir, subadr);
	out_page();
	out_subadr();
	out_buffer();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

_INT16 bc_ram_test(const char *str)                         /* тестирование БЗУ */
{
	_INT16 k, error, bases;

	if (end_ctrl(str)) {
		return 0;
	}
	bases = tmkMaxBase[tmkselected()] + 1;
	clear_dump();
	gotoxy(1, BDUMP_Y - 3);
	hor_line(78);
	gotoxy(1, BDUMP_Y - 2);
	clreol();
	cputs("      0               1               2               3");
	gotoxy(1, BDUMP_Y - 1);
#ifdef ENG
	cputs("Base: 0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF");
#else
	cputs("База: 0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF");
#endif
	gotoxy(1, BDUMP_Y);
	cputs("  00:");
	gotoxy(1, BDUMP_Y + 1);
	cputs("  40:");
	gotoxy(1, BDUMP_Y + 2);
	cputs("  80:");
	gotoxy(1, BDUMP_Y + 3);
	cputs("  C0:");
	gotoxy(1, BDUMP_Y + 4);
#ifdef ENG
	cputs("      Memory map: \xB0, \xB1 - test passed, ");
	textcolor(RED);
	putch('B');
	textcolor(LIGHTGRAY);
	cputs(" - test failed.");
#else
	cputs("      Карта памяти: \xB0, \xB1 - тест прошел, ");
	textcolor(RED);
	putch('B');
	textcolor(LIGHTGRAY);
	cputs(" - сбойный участок.");
#endif
	srand(1);
	for (base = 0; base < bases; base++) {
		for (k = 0; k <= 63; ++k) {
			buffer[k] = (rand() + rand());
		}
		bcdefbase(base);
		bcputblk(0, buffer, 64);
		if (base % 64 == 0) {
			gotoxy(7, base / 64 + BDUMP_Y);
		}
		putch('.');
	}
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	srand(1);
	for (base = 0; base < bases; base++) {
		bcdefbase(base);
		bcgetblk(0, buffer, 64);
		error = 0;
		for (k = 0; k <= 63; ++k) {
			if (buffer[k] != (_UINT16)(rand() + rand())) {
				error = 1;
				++data_err;
			}
		}
		if (base % 64 == 0) {
			gotoxy(7, base / 64 + BDUMP_Y);
		}
		if (error) {
			textcolor(RED);
			putch('B');
			textcolor(LIGHTGRAY);
		} else {
			putch('\xB0');
		}
	}
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	if (!avtomat)
#ifdef ENG
		msg_out("Normal RAM test completed");
#else
		msg_out("Прямой тест БЗУ завершен");
#endif
// else
//  comment("Прямой тест БЗУ завершен");
	for (base = 0; base < bases; base++) {
		bcdefbase(base);
		bcgetblk(0, buffer, 64);
		for (k = 0; k <= 63; ++k) {
			buffer[k] = ~buffer[k];
		}
		bcputblk(0, buffer, 64);
		if (base % 64 == 0) {
			gotoxy(7, base / 64 + BDUMP_Y);
		}
		putch('.');
	}
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	srand(1);
	for (base = 0; base < bases; base++) {
		bcdefbase(base);
		bcgetblk(0, buffer, 64);
		error = 0;
		for (k = 0; k <= 63; ++k) {
			if (buffer[k] != (_UINT16)(~(rand() + rand()))) {
				error = 1;
				++data_err;
			}
		}
		if (base % 64 == 0) {
			gotoxy(7, base / 64 + BDUMP_Y);
		}
		if (error) {
			textcolor(RED);
			putch('B');
			textcolor(LIGHTGRAY);
		} else {
			putch('\xB1');
		}
	}
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	if (!avtomat)
#ifdef ENG
		msg_out("Inverse RAM test completed");
#else
		msg_out("Инверсный тест БЗУ завершен");
#endif
// else
//  comment("Инверсный тест БЗУ завершен");
	gotoxy(1, BDUMP_Y - 3);
	clreol();
	gotoxy(1, BDUMP_Y - 2);
	clreol();
	clear_dump();
// bc_clear_ram();
	base = 0;
	out_base();
	out_ctrl_code();
	bc_out_ram(base);
	bc_out_sw();
	out_inn();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	return (data_err > 0L);
}

_INT16 rt_ram_test(const char *str)                         /* тестирование БЗУ */
{
	_INT16 k, error, page, pages;

	if (end_ctrl(str)) {
		return 0;
	}
	pages = rtgetmaxpage() + 1;
	clear_dump();
	gotoxy(1, BDUMP_Y - 3);
	hor_line(78);
	gotoxy(1, BDUMP_Y - 2);
	clreol();
#ifdef ENG
	cputs("          0    RECEIVE    1    RECEIVE    0    TRANSMIT   1    TRANSMIT");
	gotoxy(1, BDUMP_Y - 1);
	cputs("Subaddr.: 0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF");
	gotoxy(1, BDUMP_Y);
	cputs("  Page 0:");
	gotoxy(1, BDUMP_Y + 1);
	cputs("  Page 1:");
	gotoxy(1, BDUMP_Y + 2);
	cputs("  Page 2:");
	gotoxy(1, BDUMP_Y + 3);
	cputs("  Page 3:");
	gotoxy(1, BDUMP_Y + 4);
	cputs("          Memory map: \xB0, \xB1 - test passed, ");
	textcolor(RED);
	putch('B');
	textcolor(LIGHTGRAY);
	cputs(" - test failed.");
#else
	cputs("          0     ПРИЕМ     1     ПРИЕМ     0    ПЕРЕДАЧА   1    ПЕРЕДАЧА");
	gotoxy(1, BDUMP_Y - 1);
	cputs("Подадрес: 0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF");
	gotoxy(1, BDUMP_Y);
	cputs("  Стр. 0:");
	gotoxy(1, BDUMP_Y + 1);
	cputs("  Стр. 1:");
	gotoxy(1, BDUMP_Y + 2);
	cputs("  Стр. 2:");
	gotoxy(1, BDUMP_Y + 3);
	cputs("  Стр. 3:");
	gotoxy(1, BDUMP_Y + 4);
	cputs("          Карта памяти: \xB0, \xB1 - тест прошел, ");
	textcolor(RED);
	putch('B');
	textcolor(LIGHTGRAY);
	cputs(" - сбойный участок.");
#endif
	srand(1);
	for (page = 0; page < pages; page++) {
		rtdefpage(page);
		gotoxy(11, page + BDUMP_Y);
		for (dir = RT_RECEIVE; ; dir = RT_TRANSMIT) {
			for (subadr = 0; subadr <= 31; subadr++) {
				for (k = 0; k <= 31; ++k) {
					buffer[k] = rand() + rand();
				}
				rtdefsubaddr(dir, subadr);
				rtputblk(0, buffer, 32);
				if (page <= 3) {
					putch('.');
				}
			}
#ifdef _TMK1553B_LINUX
			fflush(stdout);
#endif
			if (dir == RT_TRANSMIT) {
				break;
			}
		}
	}
	srand(1);
	for (page = 0; page < pages; page++) {
		rtdefpage(page);
		gotoxy(11, page + BDUMP_Y);
		for (dir = RT_RECEIVE; ; dir = RT_TRANSMIT) {
			for (subadr = 0; subadr <= 31; subadr++) {
				rtdefsubaddr(dir, subadr);
				rtgetblk(0, buffer, 32);
				error = 0;
				for (k = 0; k <= 31; ++k) {
					if (buffer[k] != (_UINT16)(rand() + rand())) {
						error = 1;
						++data_err;
					}
				}
				if (page <= 3) {
					if (error) {
						textcolor(RED);
						putch('B');
						textcolor(LIGHTGRAY);
					} else {
						putch('\xB0');
					}
				}
			}
#ifdef _TMK1553B_LINUX
			fflush(stdout);
#endif
			if (dir == RT_TRANSMIT) {
				break;
			}
		}
	}
	if (!avtomat)
#ifdef ENG
		msg_out("Normal RAM test completed");
#else
		msg_out("Прямой тест БЗУ завершен");
#endif
// else
//  comment("Прямой тест БЗУ завершен");
	for (page = 0; page < pages; page++) {
		rtdefpage(page);
		gotoxy(11, page + BDUMP_Y);
		for (dir = RT_RECEIVE; ; dir = RT_TRANSMIT) {
			for (subadr = 0; subadr <= 31; subadr++) {
				rtdefsubaddr(dir, subadr);
				rtgetblk(0, buffer, 32);
				for (k = 0; k <= 31; ++k) {
					buffer[k] = ~buffer[k];
				}
				rtputblk(0, buffer, 32);
				if (page <= 3) {
					putch('.');
				}
			}
#ifdef _TMK1553B_LINUX
			fflush(stdout);
#endif
			if (dir == RT_TRANSMIT) {
				break;
			}
		}
	}
	srand(1);
	for (page = 0; page < pages; page++) {
		rtdefpage(page);
		gotoxy(11, page + BDUMP_Y);
		for (dir = RT_RECEIVE; ; dir = RT_TRANSMIT) {
			for (subadr = 0; subadr <= 31; subadr++) {
				rtdefsubaddr(dir, subadr);
				rtgetblk(0, buffer, 32);
				error = 0;
				for (k = 0; k <= 31; ++k) {
					if (buffer[k] != (_UINT16)(~(rand() + rand()))) {
						error = 1;
						++data_err;
					}
				}
				if (page <= 3) {
					if (error) {
						textcolor(RED);
						putch('B');
						textcolor(LIGHTGRAY);
					} else {
						putch('\xB1');
					}
				}
			}
#ifdef _TMK1553B_LINUX
			fflush(stdout);
#endif
			if (dir == RT_TRANSMIT) {
				break;
			}
		}
	}
	if (!avtomat)
#ifdef ENG
		msg_out("Inverse RAM test completed");
#else
		msg_out("Инверсный тест БЗУ завершен");
#endif
// else
//  comment("Инверсный тест БЗУ завершен");
	gotoxy(1, BDUMP_Y - 3);
	clreol();
	gotoxy(1, BDUMP_Y - 2);
	clreol();
	clear_dump();
// rt_clear_ram();
	dir = RT_RECEIVE;
	subadr = 0x10;
	out_subadr();
	rt_out_ram(dir, subadr);
	out_status_word();
	rt_out_sw();
	out_inn();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	return (data_err > 0L);
}

void comment(const char *str)                          /* вывод коментария */
{
	if (!avtomat) {
		window(1, 1, 80, 25);
		gotoxy(1, 24);
		clreol();
		cputs(str);
		window(Window[nwin].nX + 1, Window[nwin].nY,
		       Window[nwin].nX + Window[nwin].nDX - 3,
		       Window[nwin].nY + Window[nwin].nDY - 1);
	} else {
		gotoxy(1, 10);
		clreol();
		cputs(str);
	}
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

void kbd_pause(const char *str)                        /* программируемая пауза */
{
	unsigned long p;
#ifdef _TMK1553B_LINUX
	unsigned short key;
#endif

	while (*str == '\t' || *str == ' ') {
		str++;
	}
	if (*str != '\0') {
		/* пауза программируемой длительности */
#ifdef _TMK1553B_DOS
		if (sscanf(str, "%U", &p) != 1)
#endif
#ifdef _TMK1553B_LINUX
			if (sscanf(str, "%lu", &p) != 1)
#endif
			{
				msg_out(inp_err);
				return;
			}
		for (; p-- ;);
		return;
	}
	gotoxy(1, 10);                          /* ожидание нажатия клавиши */
	textcolor(CYAN);
#ifdef ENG
	cputs("Pause. ESC - return to command mode, other - continue execution.");
#else
	cputs("Пауза. ESC - возврат в командный режим, другая - продолжить выполнение.");
#endif
	textcolor(LIGHTGRAY);
#ifdef _TMK1553B_LINUX
	fflush(stdout);
	key = bioskey(0);
#endif
#ifdef _TMK1553B_DOS
	while (kbhit() == 0);
#endif
	gotoxy(1, 10);
	clreol();
#ifdef _TMK1553B_DOS
	if ((bioskey(0) & 0x00FF) == ESCAPE)
#endif
#ifdef _TMK1553B_LINUX
		if ((key & 0x00FF) == ESCAPE)
#endif
			if (enter_mode) {
				fclose(cmd_file);
				enter_mode = 0;
			}
}

void statistic_err_out()   /* вывод статистики ошибок */
{
	_INT16 savenDY;
	_UINT16 i;
	_UINT16 nErrs;

	window(1, 1, 80, 23);
	clrscr();
#ifdef ENG
	cputs(" Mode │ Base│SAdr.│ Nw.│Err.│Data ║ Mode │ Base│SAdr.│ Nw.│Err.│Data\015\012");
#else
	cputs(" Режим│ База│ПАдр.│Nсл.│Err.│Data ║ Режим│ База│ПАдр.│Nсл.│Err.│Data\015\012");
#endif
	nErrs = (channel_err < (unsigned long)ERRLEN) ? (_UINT16)channel_err : ERRLEN;
	for (i = 0; i < nErrs; i++) {
#ifdef ENG
		if (at_bad_mode[i] == DATA_BC_RT) {
			cputs("BC->RT│");
		} else {
			cputs("RT->BC│");
		}
#else
		if (at_bad_mode[i] == DATA_BC_RT) {
			cputs("КК->ОУ│");
		} else {
			cputs("ОУ->КК│");
		}
#endif
		cprintf(" %02Xh │ %02Xh │ %2d │%04X│%04X ",
		        at_bad_base[i],
		        at_bad_sa[i],
		        at_bad_num[i],
		        at_bad_bad[i],
		        at_bad_good[i]);
		if (i & 1) {
			cputs("\015\012");
		} else {
			cputs("║");
		}
	}
	savenDY = Window[nwin].nDY;
	Window[nwin].nDY = 23;
#ifdef ENG
	msg_out("End of errors statistics");
#else
	msg_out("Конец статистики ошибок");
#endif
	Window[nwin].nDY = savenDY;
	all_screen();
}

void statistic_bad_out(const char *str)                        /* вывод статистики канала */
{
	_INT16 savenDY;
	_UINT16 i;

	if (end_ctrl(str)) {
		return;
	}
	window(1, 1, 80, 23);
	clrscr();
#ifdef ENG
	cputs(" N start  │ State W.│   Status Word    ║ N start  │ State W.│   Status Word\015\012");
#else
	cputs(" N старта │ Сл.сост.│    Отв.слово     ║ N старта │ Сл.сост.│    Отв.слово\015\012");
#endif
	for (i = 0; (unsigned long)i < bad_starts && i < 42; i++) {
		cprintf("%9lu │ ", bad_sts[i]);
		out_bin(bad_sws[i], 7);
		cputs(" │ ");
		out_bin(bad_aws[i], 16);
		if (i & 1) {
			cputs(" \015\012");
		} else {
			cputs(" ║");
		}
	}
	savenDY = Window[nwin].nDY;
	Window[nwin].nDY = 23;
#ifdef ENG
	msg_out("End of exceptions statistics");
#else
	msg_out("Конец статистики сбоев");
#endif
	Window[nwin].nDY = savenDY;
	all_screen();
}

void statistic_out(const char *str)                    /* вывод статистики канала */
{
	if (end_ctrl(str)) {
		return;
	}
	gotoxy(1, 10);
	clreol();
#ifdef ENG
	cprintf("    Starts - %lu, interrupts_2 - %lu, ", good_starts + bad_starts + to_errors, bad_starts);
	cprintf("errors - %lu, TO_errors - %lu.", channel_err, to_errors);
	if (avtomat) {
#ifdef _TMK1553B_LINUX
		fflush(stdout);
#endif
		return;
	}
	msg_out("Bus channel statistics");
#else
	cprintf("    Cтартов - %lu, прерываний_2 - %lu, ", good_starts + bad_starts + to_errors, bad_starts);
	cprintf("ошибок - %lu, TO_ошибок - %lu.", channel_err, to_errors);
	if (avtomat) {
#ifdef _TMK1553B_LINUX
		fflush(stdout);
#endif
		return;
	}
	msg_out("Статистика канала");
#endif
	gotoxy(1, 10);
	clreol();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

void statistic_clear(const char *str)          /* очистка переменных статистики */
{
	if (end_ctrl(str)) {
		return;
	}
	avt_err = 0;
	to_errors = channel_err = data_err = good_starts = bad_starts = 0L;
	gotoxy(1, 10);
	clreol();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

void fields_chg(const char *str)                    /* управление выделением полей */
{
	if (end_ctrl(str)) {
		return;
	}
	fields = !fields;
	all_screen();
}

void bc_help(const char *str)                                  /* вывод подсказки */
{
	_INT16 savenDY;

	if (end_ctrl(str)) {
		return;
	}
	window(1, 1, 80, 23);
	clrscr();
#ifdef ENG
	cputs("               Main commands in BC mode :\015\012");
	cputs("TMK number_D - select device in current window\015\012");
	cputs("ERR num_D, SL num_D, ATBC, ATRT, ATS - automatic test commands\015\012");
	cputs("SC code_B   - set control code\015\012");
	cputs("SB base_H   - set base\015\012");
	cputs("SN number_D - set bus\015\012");
	cputs("BE [adr_D, data_H] - edit buffer\015\012");
	cputs("BC - clear buffer\015\012");
	cputs("BF start_D, end_D, data_H - fill buffer\015\012");
	cputs("I - reset Bus Controller\015\012");
	cputs("G - single start\015\012");
	cputs("GL - loop start\015\012");
	cputs("SP pause_D - set pause between starts in a loop\015\012");
	cputs("STO/STBO/STC - output/clear statistics\015\012");
	cputs("T - test all RAM bases\015\012");
	cputs("R filename - read buffer from file        W filename - write buffer to file\015\012");
	cputs("R< filename - execute command file\015\012");
	cputs("P [pause_D] - programmable pause\015\012");
	cputs("C count_D - set command file counter\015\012");
	cputs(";... - comment\015\012");
	cputs("Q  - exit from program                    H  - help\015\012");
	savenDY = Window[nwin].nDY;
	Window[nwin].nDY = 23;
	msg_out("Help end");
#else
	cputs("               Основные команды в режиме КК :\015\012");
	cputs("TMK number_D - выбор устройства в текущем окне\015\012");
	cputs("ERR num_D, SL num_D, ATBC, ATRT, ATS - команды автоматического тестирования\015\012");
	cputs("SC code_B   - задание кода управления\015\012");
	cputs("SB base_H   - задание базы\015\012");
	cputs("SN number_D - задание номера канала\015\012");
	cputs("BE [adr_D, data_H] - редактирование буфера\015\012");
	cputs("BC - очистка буфера\015\012");
	cputs("BF start_D, end_D, data_H - заполнение буфера константой\015\012");
	cputs("I - сброс (инициализация) контроллера канала\015\012");
	cputs("G - старт обмена\015\012");
	cputs("GL - старт обмена в цикле\015\012");
	cputs("SP pause_D - задание паузы между стартами в цикле\015\012");
	cputs("STO/STBO/STC - вывод/очистка статистики обмена\015\012");
	cputs("T - тест всех баз БЗУ\015\012");
	cputs("R filename - чтение в буфер из файла        W filename - запись буфера в файл\015\012");
	cputs("R< filename - ввод команд из файла\015\012");
	cputs("P [pause_D] - пауза программируемой длительности\015\012");
	cputs("C count_D - установка числа повторов командного файла\015\012");
	cputs(";... - коментарий\015\012");
	cputs("Q  - выход из программы                     H  - помощь\015\012");
	savenDY = Window[nwin].nDY;
	Window[nwin].nDY = 23;
	msg_out("Конец подсказки");
#endif
	Window[nwin].nDY = savenDY;
	all_screen();
}

void rt_help(const char *str)                                  /* вывод подсказки */
{
	_INT16 savenDY;

	if (end_ctrl(str)) {
		return;
	}
	window(1, 1, 80, 23);
	clrscr();
#ifdef ENG
	cputs("                  Main commands in RT mode :\015\012");
	cputs("TMK number_D - select device in current window\015\012");
	cputs("SAT adr_D - set RT address\015\012");
	cputs("FLAG 0/1, HBIT 0/1, BCST 0/1, DTBL 0/1 - set modes\015\012");
	cputs("SW code_B - set status word\015\012");
	cputs("Sbit_name/Cbit_name - set/clear status word bit, \015\012");
	cputs("                      bit_name - BS, SR, TF, SF, DN\015\012");
	cputs("PG/SPG/CPG page_D - set page/PC page/bus page\015\012");
	cputs("SMR/CMR - lock/unlock current subaddress\015\012");
	cputs("SR/ST subadr_H - set subaddress Rx/Tx\015\012");
	cputs("BE [adr_D, data_H] - edit buffer\015\012");
	cputs("BF start_D, end_D, data_H - fill buffer\015\012");
	cputs("BC - clear buffer\015\012");
	cputs("I - reset Remote Terminal\015\012");
	cputs("RSW - read state word\015\012");
	cputs("SFL - flag writing loop\015\012");
	cputs("R filename - read buffer from file        W filename - write buffer to file\015\012");
	cputs("R< filename - execute command file\015\012");
	cputs("P [pause_D] - programmable pause\015\012");
	cputs(";... - comment\015\012");
	cputs("Q  - exit from program                    H  - help\015\012");
	savenDY = Window[nwin].nDY;
	Window[nwin].nDY = 23;
	msg_out("Help end");
#else
	cputs("                  Основные команды в режиме ОУ :\015\012");
	cputs("TMK number_D - выбор устройства в текущем окне\015\012");
	cputs("SAT adr_D - задание адреса ОУ в МК\015\012");
	cputs("FLAG 0/1, HBIT 0/1, BCST 0/1, DTBL 0/1 - установка режимов\015\012");
	cputs("SW code_B - задание ответного слова\015\012");
	cputs("Sbit_name/Cbit_name - установка/сброс бита ответного слова, \015\012");
	cputs("                      где bit_name - BS, SR, TF, SF, DN\015\012");
	cputs("PG/SPG/CPG page_D - задание страницы БЗУ со стороны ПЭВМ/МК\015\012");
	cputs("SMR/CMR - блокировка/разблокировка текущего подадреса\015\012");
	cputs("SR/ST subadr_H - задание подадреса в зоне приема/передачи\015\012");
	cputs("BE [adr_D, data_H] - редактирование буфера\015\012");
	cputs("BF start_D, end_D, data_H - заполнение буфера константой\015\012");
	cputs("BC - очистка буфера\015\012");
	cputs("I - сброс (инициализация) оконечного устройства\015\012");
	cputs("RSW - чтение слова состояния\015\012");
	cputs("SFL - циклическая запись флагов\015\012");
	cputs("R filename - чтение в буфер из файла        W filename - запись буфера в файл\015\012");
	cputs("R< filename - ввод команд из файла\015\012");
	cputs("P [pause_D] - пауза программируемой длительности\015\012");
	cputs(";... - коментарий\015\012");
	cputs("Q  - выход из программы                     H  - помощь\015\012");
	savenDY = Window[nwin].nDY;
	Window[nwin].nDY = 23;
	msg_out("Конец подсказки");
#endif
	Window[nwin].nDY = savenDY;
	all_screen();
}

void cmds_open2(char *str)           /* выполнение командного файла  в цикле */
{
	if (cmd_count == 0) {
		return;
	}
	cur_count = cmd_count;
	cmds_open(str);
}

_INT16 start_loop_n(unsigned long counter)
{
	_INT16 ps_f;

	if (counter == 1) {
		goto one_start;
	}
	gotoxy(1, 11);
	textcolor(CYAN);
#ifdef ENG
	cputs("Loop start. Press PrtScr to abort...");
#else
	cputs("Циклический запуск. Нажмите PrtScr для выхода...");
#endif
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	textcolor(LIGHTGRAY);
	prtscr_flag = 0;
	while (counter == 0 || counter > 1) { /* повторять пока не нажата клавиша */
		start();
#ifdef _TMK1553B_LINUX
		bioskey_f12();
#endif
		if (_pause_) {
			_wait_(_pause_);
		}
		if (prtscr_flag) {
			break;
		}
		if (err_level && ((to_errors + channel_err + bad_starts) >= err_level)) {
			break;
		}
		if (counter) {
			counter--;
		}
	}
	gotoxy(1, 11);
	if (nwin == nmainwin) {
		textcolor(WHITE);
		cputs("════════════════════════════════════════════════════════════════════════════");
	} else {
		cputs("────────────────────────────────────────────────────────────────────────────");
	}
	textcolor(LIGHTGRAY);
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	ps_f = prtscr_flag;
	prtscr_flag = 0;
one_start:
	if (!(err_level && ((to_errors + channel_err + bad_starts) >= err_level))) {
		start_1("");
	}
	statistic_out("");
// if ((to_errors+channel_err+bad_starts) <= err_level && !(ps_f && counter)) return 0;
	if ((to_errors + channel_err + bad_starts) == 0L && !(ps_f && counter)) {
		return 0;
	} else {
		return 1;
	}
}

_INT16 start_1(const char *str)                                  /* единичный запуск */
{
	if (end_ctrl(str)) {
		return 0;
	}
	if (!avtomat) {
		gotoxy(1, 10);
		clreol();
	}
	out_base();
	out_ctrl_code();
	start();
	if (avtomat) {
		select_window(rtwin);
		out_at();
		out_inn();
		out_status_word();
		out_page();
		out_subadr();
		rt_out_sw();
		if (avt_out) {
			rt_out_ram(dir, subadr);
		}
		select_window(bcwin);
	}
	out_inn();
	bc_out_sw();
	bc_out_ram(base);
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	if (avtomat) {
		statistic_out("");
	}
	if ((to_errors + channel_err + bad_starts) < 1) {
		return 0;
	} else {
		return 1;
	}
}

_INT16 start_loop(const char *str)            /* запуск в цикле */
/* Поддержка старта: 0 - выход по завершению,
                     1 - выход по PrtScr или по ERR_LEVEL */
{
	unsigned long counter;

	while (*str == '\t' || *str == ' ') {
		str++;
	}
	counter = 0;
	if (*str != '\0') {
		/* циклический запуск по счетчику */
#ifdef _TMK1553B_DOS
		if (sscanf(str, "%U", &counter) != 1)
#endif
#ifdef _TMK1553B_LINUX
			if (sscanf(str, "%lu", &counter) != 1)
#endif
			{
				msg_out(inp_err);
				return 1;
			}
	} else {
		counter = 0;
	}
	return start_loop_n(counter);
}

_INT16 start_loop_file(const char *str)            /* запуск в цикле */
/* Поддержка старта: 0 - выход по завершению,
                     1 - выход по PrtScr или по ERR_LEVEL */
{
	unsigned long counter;
	_INT16 ps_f;
	char szt[8];
	_UINT16 cw, dw[33];
	_INT16 i, n;
	FILE *file;

	while (*str == '\t' || *str == ' ') {
		str++;
	}
	counter = 0;
	if (*str != '\0') {
		/* циклический запуск по счетчику */
#ifdef _TMK1553B_DOS
		if (sscanf(str, "%U", &counter) != 1)
#endif
#ifdef _TMK1553B_LINUX
			if (sscanf(str, "%lu", &counter) != 1)
#endif
			{
				msg_out(inp_err);
				return 1;
			}
	} else {
		counter = 0;
	}
	cw = bcgetw(0);
	n = cw & 0x1F;
	if (n == 0) {
		n = 32;
	}
	if (cw & 0x0400) {
		file = fopen(szIDataFName, "wt");
	} else {
		file = fopen(szODataFName, "rt");
	}
	if (file == NULL) {
		msg_out(open_err);
		return 1;
	}
	if (counter == 1) {
		goto one_start;
	}
	gotoxy(1, 11);
	textcolor(CYAN);
#ifdef ENG
	cputs("Loop start. Press PrtScr to abort...");
#else
	cputs("Циклический запуск. Нажмите PrtScr для выхода...");
#endif
	textcolor(LIGHTGRAY);
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	prtscr_flag = 0;
	while (counter == 0 || counter > 1) { /* повторять пока не нажата клавиша */

		if ((cw & 0x0400) == RT_RECEIVE) {
			for (i = 0; i < n; ++i) {
				if (fgets(szt, 6, file) == 0) {
					fseek(file, 0L, SEEK_SET);
					if (fgets(szt, 6, file) == 0) {
						msg_out(format_err);
						fclose(file);
						return 1;
					}
				}
				if (szt[0] == '\n') {
					break;
				}
				if (szt[0] == ';') {
					--i;
					continue;
				}
#ifdef _TMK1553B_DOS
				if (sscanf(szt, "%X", &dw[i]) != 1)
#endif
#ifdef _TMK1553B_LINUX
					if (sscanf(szt, "%hX", &dw[i]) != 1)
#endif
					{
						msg_out(format_err);
						fclose(file);
						return 1;
					}
			}
			bcputblk(1, dw, n);
		}

		start();

		if ((cw & 0x0400) == RT_TRANSMIT) {
			bcgetblk(1, dw, n + 1);
			for (i = 0; i <= n; ++i) {
				if (fprintf(file, "%04X\n", dw[i]) == EOF) {
					msg_out(open_err);
					fclose(file);
					return 1;
				}
			}
			fprintf(file, "\n");
		}

		if (_pause_) {
			_wait_(_pause_);
		}
		if (prtscr_flag) {
			break;
		}
		if (err_level && ((to_errors + channel_err + bad_starts) >= err_level)) {
			break;
		}
		if (counter) {
			counter--;
		}
	}
	gotoxy(1, 11);
	if (nwin == nmainwin) {
		textcolor(WHITE);
		cputs("════════════════════════════════════════════════════════════════════════════");
	} else {
		cputs("────────────────────────────────────────────────────────────────────────────");
	}
	textcolor(LIGHTGRAY);
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	ps_f = prtscr_flag;
	prtscr_flag = 0;
	fclose(file);

one_start:
	if (!(err_level && ((to_errors + channel_err + bad_starts) >= err_level))) {
		start_1("");
	}
	statistic_out("");
// if ((to_errors+channel_err+bad_starts) <= err_level && !(ps_f && counter)) return 0;
	if ((to_errors + channel_err + bad_starts) == 0L && !(ps_f && counter)) {
		return 0;
	} else {
		return 1;
	}
}

_INT16 start()
/* Поддержка старта: 0 - выход по тайм-ауту, 1 - выход по прерыванию */
{
	unsigned long p;

#ifdef _TMK1553B_LINUX
	CheckTmkEvent(0);
#endif
	int_num[Window[nwin].nTMK] = 0;
//#ifdef LPT_SYN
// set_lpt_data(0xFF);
// set_lpt_data(0);
//#endif
	bcstart(base, ctrl_code[tmkselected()][base]);
	p = biostime(0, 0L);    /* ожидание прерывания 2 тика */
	do {
#ifdef _TMK1553B_LINUX
		CheckTmkEvent(0);
#endif
		if (int_num[Window[nwin].nTMK] != 0) {
			return 1;
		}
	} while (biostime(0, 0L) - p < WAITDLY);
	bc_sw = 0;
	to_errors++;
	return 0;
}

#ifdef _TMK1553B_DOS
void interrupt prtscr_brk(...)           /* обработчик прерывания от PrtScr */
{
	prtscr_flag = 1;
}
#endif

void cmd_reset(const char *str)                               /* сброс контроллера */
{
	if (end_ctrl(str)) {
		return;
	}
	switch (Window[nwin].nMode) {
	case BC_MODE:
		bcreset();
		bcdefbase(base);
		if (!avtomat) {
			msg_out(bc_reset_ok);
		}
		break;
	case RT_MODE:
		rtreset();
		wr_at(terminal_adr);
		rtdefpage(bram_page);
		rtdefsubaddr(dir, subadr);
		if (!avtomat) {
			msg_out(rt_reset_ok);
		}
		break;
	}
}

void msg_out(const char *msg)                  /* вывод строки сообщения */
{
	gotoxy(1, Window[nwin].nDY);
	textcolor(CYAN);
#ifdef ENG
	cprintf("%s. Press any key...", msg);
#else
	cprintf("%s. Нажмите любую клавишу...", msg);
#endif
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
#ifdef _TMK1553B_DOS
	while (!kbhit());
#endif
	bioskey(0);
	if (nwin == nmainwin) {
		textcolor(WHITE);
	} else {
		textcolor(LIGHTGRAY);
	}
	gotoxy(1, Window[nwin].nDY);
	hor_line(Window[nwin].nDX - 2);
	textcolor(LIGHTGRAY);
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

#ifdef ELCUS
void rd_sw_loop(char *str)               /* чтение слова состояния в цикле */
{
	if (end_ctrl(str)) {
		return;
	}
	gotoxy(1, 10);
	textcolor(CYAN);
#ifdef ENG
	cputs("Reading State Word Register. Press any key to abort...");
#else
	cputs("Чтение регистра состояния. Нажмите любую клавишу для выхода...");
#endif
	textcolor(LIGHTGRAY);
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	while (kbhit() == 0) {                 /* повторять пока не нажата клавиша */
		rt_sw = rtgetstate();
	}
#ifdef _TMK1553B_DOS
	bioskey(0);
#endif
	gotoxy(1, 10);
	clreol();
	rt_out_sw();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

void wr_mr_loop(char *str)                     /* циклическая запись в РРЖ */
{
	if (end_ctrl(str)) {
		return;
	}
	gotoxy(1, 10);
	textcolor(CYAN);
#ifdef ENG
	cputs("Writing Mode Register. Press any key to abort...");
#else
	cputs("Запись в регистр режима. Нажмите любую клавишу для выхода...");
#endif
	textcolor(LIGHTGRAY);
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	while (kbhit() == 0) {                 /* повторять пока не нажата клавиша */
		rtunlock();
	}
	fLock = 0;
#ifdef _TMK1553B_DOS
	bioskey(0);
#endif
	gotoxy(1, 10);
	clreol();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}
#endif //def ELCUS

void set_flag(char *str)         /* установка флагов */
{
	if (end_ctrl(str)) {
		return;
	}
	gotoxy(1, 10);
	textcolor(CYAN);
#ifdef ENG
	cputs("Writing Flags. Press any key to abort...");
#else
	cprintf("Запись флагов. Нажмите любую клавишу для выхода...");
#endif
	textcolor(LIGHTGRAY);
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
	while (!kbhit()) {                 /* повторять пока не нажата клавиша */
		if (dir == RT_TRANSMIT) {
			rtsetflag();
		} else {
			rtclrflag();
		}
	}
#ifdef _TMK1553B_DOS
	bioskey(0);
#endif
	gotoxy(1, 10);
	clreol();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

void read_sw(const char *str)                 /* чтение регистра состояния */
{
	if (end_ctrl(str)) {
		return;
	}
	rt_sw = rtgetstate();
	rt_out_sw();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

void read_sp(char *str)                 /* чтение регистра состояния */
{
	if (end_ctrl(str)) {
		return;
	}
	rt_sp = inport(aTmkConfig[Window[nmainwin].nTMK].wPorts1 + 0x8);
	rt_out_sp();
#ifdef _TMK1553B_LINUX
	fflush(stdout);
#endif
}

void check_int()          /* обнаружение прохождения прерывания и оповещение */
{
	_INT16 cx, cy, fRestore, savenwin;

	fRestore = 0;
	savenwin = nwin;
#ifdef _TMK1553B_LINUX
	CheckTmkEvent(0);
#endif
	for (nwin = 0; nwin <= nMaxWin; nwin++) {
		if (Window[nwin].nMode == RT_MODE && int_num[Window[nwin].nTMK]) {
			if (Window[nwin].fVisible) {
				if (!fRestore) {
					fRestore = 1;
					cx = wherex();
					cy = wherey();
				}
				fInt = 1;
				select_window(nwin);
				out_inn();
				rt_out_sw();
#ifdef _TMK1553B_LINUX
				fflush(stdout);
#endif
#ifdef ENG
				if (!avtomat) {
					msg_out("Interrupt");
				}
#else
				if (!avtomat) {
					msg_out("Прерывание");
				}
#endif
				fInt = 0;
				int_num[Window[nwin].nTMK] = 0;
			}
		}
	}
	nwin = savenwin;
	if (fRestore) {
		select_window(nwin);
		window(1, 1, 80, 25);
		gotoxy(cx, cy);
	}
}

_INT16 in_key()                    /* ввод символа с проверкой прерывания */
{
	_INT16 key, cx, cy;

	if (enter_mode) {
		check_int();
		key = fgetc(cmd_file);        /* чтение символа из командного файла */
		if (key == EOF || fSkipCmds) {
			fclose(cmd_file);
			if (fCmds_f) {
				cmds_close_f(ex_fname);
				if (!prtscr_flag) {
					cmds_open_f(ex_fname);
				}
			}
			if (fCmds_f) {
				check_int();
				key = fgetc(cmd_file);
			} else {
				if (Window[nwin].nMode == BC_MODE && cmd_count != 1 && --cur_count != 0 && !fSkipCmds) {
					cx = wherex();
					cy = wherey(); /* циклическое выполнение */
					select_window(nwin);
					gotoxy(59, 2);
					cprintf("%010lu", cur_count);
					window(1, 1, 80, 25);
					gotoxy(cx, cy);
					cmds_open(ex_fname);
					check_int();
					key = fgetc(cmd_file);
				} else {
					if (Window[nwin].nMode == BC_MODE) {
						cx = wherex();
						cy = wherey(); /* завершение цикла */
						select_window(nwin);
						gotoxy(59, 2);
						cprintf("%010lu", cmd_count);
						window(1, 1, 80, 25);
						gotoxy(cx, cy);
					}
					enter_mode = 0;
					fGoto = 0;
					fSkipCmds = 0;
#ifdef _TMK1553B_DOS
					while (kbhit() == 0)
#endif
						check_int();
					key = bioskey(0);
				}
			}
		}
	} else {
#ifdef _TMK1553B_DOS
		while (kbhit() == 0)
#endif
			check_int();
		key = bioskey(0);
	}
	return key;
}

void in_cmd(char *str)
/* ввод командной строки в str, селектирование управляющих и функциональных */
{
	_INT16 key;                       /* код принятый с клавиатуры, ... */
	_INT16 fInCmd;

	str_ptr = 0;
	fInCmd = 0;
	do {
		if (!fInCmd) {
			window(1, 1, 80, 25);
			gotoxy(1, 23);
			clreol();
#ifdef ENG
			cputs("Command: ");
#else
			cputs("Команда: ");
#endif
			cmd_str[str_ptr] = '\0';
			cprintf(cmd_str);
#ifdef _TMK1553B_LINUX
			fflush(stdout);
#endif
		}
		fInCmd = 0;
		key = in_key();
		if ((key & 0x00FF) && key != GPLUSW) {
			if ((key &= 0x00FF) < 0x20 && key != CR && key != LF) {
				make_ctrl(key);
				continue;
			}
			key = str[str_ptr++] = toupper(key &= 0x00FF);
			putch(key);
			fInCmd = 1;
#ifdef _TMK1553B_LINUX
			fflush(stdout);
#endif
		} else {
			window(Window[nwin].nX + 1, Window[nwin].nY,
			       Window[nwin].nX + Window[nwin].nDX - 2,
			       Window[nwin].nY + Window[nwin].nDY - 1);
			make_func(key >> 8);
			continue;
		}
	} while (key != CR && key != LF);
	str[--str_ptr] = '\0';
}

void make_ctrl(_INT16 key)
/* обработка управляющих кодов, key - управляющий код. */
{
	switch (key) {
	case BACKSPACE:
		if (str_ptr != 0) {   /* забой */
			--str_ptr;
			cputs("\b \b");
		}
		break;
	case ESCAPE:       /* отмена всей строки */
		str_ptr = 0;
		break;
	case TAB:
		str_ptr = 0;
		nmainwin = !nmainwin;
#ifdef _TMK1553B_LINUX
		CheckTmkEvent(0);
#endif
		int_num[Window[0].nTMK] = 0;
		int_num[Window[1].nTMK] = 0;
		select_window(nmainwin);
		all_screen();
		break;
	}
}

void make_func(_UINT16 key)
/*
обработка функциональных клавиш, F1..F10
key - расширенный код ASCII
*/
{
	switch (key) {
	case UP:   /* возврат к предыдущей команде */
//            str_ptr=strcpy(cmd_str, old_cmd)-cmd_str;
		strcpy(cmd_str, old_cmd);
		str_ptr = strlen(cmd_str);
		break;
	case F01:
		switch (Window[nwin].nMode) {
		case BC_MODE:
			bc_help("");
			break;
		case RT_MODE:
			rt_help("");
			break;
		}
		break;
	case F02:
		buf_edit("");
		break;
	case F03:
#ifdef _TMK1553B_LINUX
		CheckTmkEvent(0);
#endif
		int_num[Window[nwin].nTMK] = 0;
		switch (Window[nwin].nMode) {
		case RT_MODE:
			set_mode(BC_MODE);
			base = 0;
			bcdefbase(0);
			all_screen();
			break;
		case BC_MODE:
			set_mode(RT_MODE);
			cram_page = bram_page = 0;
			rtdefpage(0);
			dir = RT_RECEIVE;
			subadr = 1;
			rtdefsubaddr(dir, subadr);
			all_screen();
			break;
		}
		break;
	case F04:
		str_ptr = 0;
		nmainwin = !nmainwin;
#ifdef _TMK1553B_LINUX
		CheckTmkEvent(0);
#endif
		int_num[Window[0].nTMK] = 0;
		int_num[Window[1].nTMK] = 0;
		select_window(nmainwin);
		all_screen();
		break;
	case F05:
		switch (Window[nwin].nMode) {
		case BC_MODE:
			start_loop_file("");
			break;
		case RT_MODE:
			break;
		}
		break;
	case F06:
		switch (Window[nwin].nMode) {
		case BC_MODE:
			start_1("");
			break;
		case RT_MODE:
			read_sw("");
			break;
		}
		break;
	case F07:
		switch (Window[nwin].nMode) {
		case BC_MODE:
			start_loop("");
			break;
		case RT_MODE:
			rtlock(dir, subadr);
			fLock = 1;
			break;
		}
		break;
	case F08:
		cmd_reset("");
		break;
	case F09:
		fields_chg("");
		break;
	case F10:
		make_quit("");
		break;
	case GPLUS:
		if (Window[nwin].nMode == RT_MODE) {
			dir = (dir == RT_TRANSMIT) ? RT_RECEIVE : RT_TRANSMIT;
			out_subadr();
			rt_out_ram(dir, subadr);
#ifdef _TMK1553B_LINUX
			fflush(stdout);
#endif
		}
		break;
	case PGUP:
		switch (Window[nwin].nMode) {
		case BC_MODE:
			if (base > 0) {
				base--;
				out_base();
				out_ctrl_code();
				bc_out_ram(base);
#ifdef _TMK1553B_LINUX
				fflush(stdout);
#endif
			}
			break;
		case RT_MODE:
			if (subadr > 0) {
				subadr--;
				out_subadr();
				rt_out_ram(dir, subadr);
#ifdef _TMK1553B_LINUX
				fflush(stdout);
#endif
			}
		}
		break;
	case PGDN:
		switch (Window[nwin].nMode) {
		case BC_MODE:
			if (base < tmkMaxBase[tmkselected()]) {
				base++;
				out_base();
				out_ctrl_code();
				bc_out_ram(base);
#ifdef _TMK1553B_LINUX
				fflush(stdout);
#endif
			}
			break;
		case RT_MODE:
			if (subadr < 31) {
				subadr++;
				out_subadr();
				rt_out_ram(dir, subadr);
#ifdef _TMK1553B_LINUX
				fflush(stdout);
#endif
			}
		}
		break;
	}
}
