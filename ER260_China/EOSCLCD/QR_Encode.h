#ifndef __QRENCODE_H
#define __QRENCODE_H

//----------------------------------------------------------
#if (0)
/* These types must be 8-uchar integer */
typedef char      		CHAR;
typedef unsigned char  	UCHAR;
typedef unsigned char  	uchar;
typedef unsigned char  	BYTE;
typedef char      		INT8;
typedef unsigned char 	UINT8;

/* These types must be 16-uchar integer */
typedef short      		SHORT;
typedef unsigned short 	USHORT;
typedef unsigned short 	WORD;
typedef unsigned short 	WCHAR;

/* These types must be 16-uchar, 32-uchar or larger integer */
typedef int       		INT;
typedef unsigned int  	UINT;
typedef int      		INT16;
typedef unsigned int  	UINT16;

/* These types must be 32-uchar integer */
typedef long      		LONG;
typedef unsigned long  	ULONG;
typedef unsigned long  	DWORD;
typedef unsigned long 	UINT32;
typedef long      		INT32;

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif
//typedef enum {FALSE = 0, TRUE = !FALSE} BYTE;

typedef unsigned int  	u16;

#endif

#define min(a,b)	(((a) < (b)) ? (a) : (b))


//4种纠错等级，可恢复的码字比例为：
#define QR_LEVEL_L	0	//7%的字码可被修正
#define QR_LEVEL_M	1	//15%的字码可被修正
#define QR_LEVEL_Q	2	//25%的字码可被修正
#define QR_LEVEL_H	3	//30%的字码可被修正

//编码形式
#define QR_MODE_NUMERAL		0
#define QR_MODE_ALPHABET	1
#define QR_MODE_8BIT		2
#define QR_MODE_KANJI		3
#define QR_MODE_CHINESE		4

//Number of bits per length field
//Encoding		Ver.1–9		10–26		27–40
//Numeric		10			12			14
//Alphanumeric	9			11			13
//Byte			8			16			16
//Kanji			8			10			12
//Chinese
//P17 字符计数指示的位数
#define QR_VRESION_S	0
#define QR_VRESION_M	1
#define QR_VRESION_L	2

#define QR_MARGIN	4
#define	QR_VER1_SIZE	21// 版本的行列数

#if (1)//ccr2016-06-28>>>>>
#define MAX_ALLCODEWORD		(1536)//180 //P14,P35 数据容量[码字]* (E) (VER:40), 所有码字为8位
#define MAX_DATACODEWORD	150 //P27     最大信息码子(Ver：40-L)，所有码字为8位
#define MAX_BLOCKMODE       25  //ccr2016-06-28
#define MAX_CODEBLOCK		140	//最大纠错码字 Ver：36.37.38_L_第二块
#define MAX_MODULESIZE		41//57	// 21:Version=1,最大字符=17(8.5个汉字)
								// 25:Version=2,最大字符=32(16个汉字)
								// 29:Version=3,最大字符=49(24.5个汉字)
								// 33:Version=4,最大字符=78(39个汉字)
								// 37:Version=5,最大字符=106(53个汉字)
								// 41:Version=6,最大字符=134(67个汉字)
							 	// 45:Version=7,最大字符=154(77个汉字)
							 	// 49:Version=8,最大字符=192(96个汉字)
							 	// 53:Version=9,最大字符=230(115个汉字)
							 	// 57:Version=10,最大字符=254(127个汉字)
#else//ccr2016-06-28<<<<<<
#define MAX_ALLCODEWORD		2048//3706//400// //P14,P35 数据容量[码字]* (E) (VER:40), 所有码字为8位
#define MAX_DATACODEWORD	1536//2956//400// //P27     最大信息码子(Ver：40-L)，所有码字为8位
#define MAX_BLOCKMODE       MAX_DATACODEWORD    //ccr2016-06-28
#define MAX_CODEBLOCK		153	//最大纠错码字 Ver：36.37.38_L_第二块
#define MAX_MODULESIZE		45//57	// 21:Version=1,最大字符=17(8.5个汉字)
								// 25:Version=2,最大字符=32(16个汉字)
								// 29:Version=3,最大字符=49(24.5个汉字)
								// 33:Version=4,最大字符=78(39个汉字)
								// 37:Version=5,最大字符=106(53个汉字)
								// 41:Version=6,最大字符=134(67个汉字)
							 	// 45:Version=7,最大字符=154(77个汉字)
							 	// 49:Version=8,最大字符=192(96个汉字)
							 	// 53:Version=9,最大字符=230(115个汉字)
							 	// 57:Version=10,最大字符=254(127个汉字)
#endif
//#define	MAX_MODULESIZE	177//P14 每边的模块数（A） (VER:40   ) Ver：40 = 21+（Ver-1）*4

extern int m_nSymbleSize;
extern BYTE m_byModuleData[MAX_MODULESIZE][MAX_MODULESIZE];
/////////////////////////////////////////////////////////////////////////////
typedef struct
{
	u16 ncRSBlock;		//纠错的块数
	u16 ncAllCodeWord;	//码字总数
	u16 ncDataCodeWord;	//指定纠错等级下的数据码字数
}RS_BLOCKINFO;
typedef struct
{
	u16 nVersionNo;			//ver 1~40
	u16 ncAllCodeWord; 		//码字总数=数据码字+纠错码字

	u16 ncDataCodeWord[4];	//指定纠错等级下的数据码字(0=L,1=M,2=Q,3=H)
	u16 ncAlignPoint;		//P61 表E1 校正图形 个数
	u16 nAlignPoint[6];		//P61 表E1 校正图形	行列坐标
	//(0=L,1=M,2=Q,3=H)
	RS_BLOCKINFO RS_BlockInfo1[4];	//纠错块1
	RS_BLOCKINFO RS_BlockInfo2[4];	//纠错块2
}QR_VERSIONINFO;

BYTE EncodeData(char *lpsSource);

int GetEncodeVersion(int nVersion, char *lpsSource, int ncLength);
//BYTE EncodeSourceData(char *lpsSource, int ncLength, int nVerGroup);
int EncodeSourceData(char *lpsSource, int ncLength, int nVerGroup);

int GetBitLength(BYTE nMode, int ncData, int nVerGroup);

int SetBitStream(int nIndex, WORD wData, int ncData);

BYTE IsNumeralData(unsigned char c);
BYTE IsAlphabetData(unsigned char c);
BYTE IsKanjiData(unsigned char c1, unsigned char c2);
BYTE IsChineseData(unsigned char c1, unsigned char c2);

BYTE AlphabetToBinaly(unsigned char c);
WORD KanjiToBinaly(WORD wc);
WORD ChineseToBinaly(WORD wc);
void GetRSCodeWord(BYTE *lpbyRSWork, int ncDataCodeWord, int ncRSCodeWord);

void FormatModule(void);

void SetFunctionModule(void);
void SetFinderPattern(int x, int y);
void SetAlignmentPattern(int x, int y);
void SetVersionPattern(void);
void SetCodeWordPattern(void);
void SetMaskingPattern(int nPatternNo);
void SetFormatInfoPattern(int nPatternNo);
int CountPenalty(void);
void Print_2DCode(void);
//ccr2016-06-22>>>>>>>>>>>>>>>>>>>>>>>>>
extern int  QRWidth;//二维码的宽度(字节数)
extern int  QRSize;//二维码的总字节数
extern BYTE m_byAllCodeWord[];
#endif
