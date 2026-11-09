#ifndef FFCONF_DEF
#define FFCONF_DEF 80386

/* Function Configurations */
#define FF_FS_READONLY   0   /* 0: Read/Write */
#define FF_FS_MINIMIZE   0   /* 0: Full basic API (f_stat, f_open, f_close, f_write, etc.) */
#define FF_USE_FIND      0
#define FF_USE_MKFS      0   /* 0: Disable format function to save Flash */
#define FF_USE_FASTSEEK  0
#define FF_USE_EXPAND    0
#define FF_USE_CHMOD     0
#define FF_USE_LABEL     0
#define FF_USE_FORWARD   0
#define FF_USE_STRFUNC   0   /* 0: Use f_write directly for speed & minimal flash */
#define FF_PRINT_LLI     0
#define FF_PRINT_FLOAT   0
#define FF_STRF_ENCODE   0

/* Locale and Namespace Configurations */
#define FF_CODE_PAGE     437 /* Standard US ASCII */
#define FF_USE_LFN       0   /* 0: 8.3 filenames (e.g. LOG0001.CSV) -> saves ~15KB Flash */
#define FF_MAX_LFN       255
#define FF_LFN_UNICODE   0
#define FF_LFN_BUF       255
#define FF_SFN_BUF       12
#define FF_FS_RPATH      0

/* Drive/Volume Configurations */
#define FF_VOLUMES       1   /* 1 SD card drive ("0:") */
#define FF_STR_VOLUME_ID 0
#define FF_MULTI_PARTITION 0

#define FF_MIN_SS        512 /* Standard 512-byte sector */
#define FF_MAX_SS        512
#define FF_LBA64         0
#define FF_MIN_GPT       0x10000000
#define FF_USE_TRIM      0

/* System Configurations */
#define FF_FS_TINY       0
#define FF_FS_EXFAT      0   /* Standard FAT32 (SD/SDHC up to 32GB) */
#define FF_FS_NORTC      1   /* 1: Use fixed timestamp, no external RTC needed */
#define FF_NORTC_MON     11
#define FF_NORTC_MDAY    9
#define FF_NORTC_YEAR    2026
#define FF_FS_NOFSINFO   0
#define FF_FS_LOCK       0
#define FF_FS_REENTRANT  0

#endif /* FFCONF_DEF */

