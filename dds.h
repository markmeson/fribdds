#ifndef DDS_H
#define DDS_H

#define INVALIDFILE 1
#define FILEOPENERR 2
#define FILENOTFOUND 3
#define UNSUPPFMT 4

#define DDPF_ALPHAPIXELS	0x00001
#define DDPF_ALPHA				0x00002
#define DDPF_FOURCC				0x00004
#define DDPF_RGB					0x00040
#define DDPF_YUV					0x00200
#define DDPF_LUMINANCE		0x20000

#include <cstdio>

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned DWORD;
typedef unsigned UINT;

#include "ddstypes.h"

class dds {
	public:
		dds();
		bool setfile(const char *fname);
		char *getfile();
		int open();
		void close();
		int loadheader();
		void showinfo();
		void printheader();
		bool m_bIsextended;
		DDS_HEADER *getheader();
		void *getimgdata();
		void *getalphadata();
		int getpxlsize();
		int getimgsize();
		bool isalpha();

	private:
		char m_fname[500];
		int m_fsize;
		FILE *m_fh;
		DDS_HEADER m_hdr;
		DDS_HEADER_DXT10 m_dx10hdr;
		DWORD m_pData;
		DWORD m_pData2;
		void *m_imgdata;
		void *m_alphadata;
		int m_pxlsize;
		int m_imgsize;
		bool m_bisalpha;
};

#endif
