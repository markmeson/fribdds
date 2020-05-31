#include "dds.h"
#include "mainframe.h"
#include <cstring>

extern mainframe *frame;
extern char buffer[2048];

const int g_iDXT1 = 0x31545844;
const int g_iDXT2 = 0x32545844;
const int g_iDXT3 = 0x33545844;
const int g_iDXT4 = 0x34545844;
const int g_iDXT5 = 0x35545844;

dds::dds() {
	m_fh = NULL;
	m_fsize = 0;
	m_bIsextended = false;
	m_pxlsize = 0;
	m_imgsize = 0;
	m_bisalpha = false;
	memset(&m_hdr, 0, sizeof(DDS_HEADER));
	memset(&m_dx10hdr, 0, sizeof(DDS_HEADER_DXT10));
}

bool dds::setfile(const char *path, const char *fname) {
	int ipath = strlen(path);
	int iname = strlen(fname);
	if(ipath > 499 || iname > 99)
		return false;

	sprintf(m_fpath, "%s", path);
	sprintf(m_fname, "%s", fname);
	return true;
}

char *dds::getfile() {
	return m_fpath;
}

int dds::open() {
	if(!strlen(m_fpath))
		return FILENOTFOUND;

	m_fh = fopen(m_fpath, "rb");
	if(!m_fh)
		return FILEOPENERR;

	int loadresult;
	if((loadresult = loadheader())) {
		close();
		return loadresult;
	}

	int w = m_hdr.dwWidth;
	int h = m_hdr.dwHeight;
	m_bisalpha = (m_hdr.ddspf.dwFlags & DDPF_ALPHAPIXELS);
	m_pxlsize = m_hdr.ddspf.dwRGBBitCount / 8;
	m_imgsize = w * h;
	m_imgdata = malloc(m_imgsize * 3);

	if(m_hdr.ddspf.dwFlags & DDPF_FOURCC) {
		if(m_hdr.ddspf.dwFourCC == g_iDXT1) { //DXT1 compression
			m_pxlsize = 3;
			DXT1_COLOR cmpclr[4];
			DXT1_COMPRESSED_PIXELS cmppxls;
			int units = m_imgsize / 16;
			int mapsperrow = m_hdr.dwWidth / 4;
			int clridx;
			for(int i = 0; i < units; i++) {
				fread(&cmppxls, 8, 1, m_fh);
				cmpclr[0] = cmppxls.co_0;
				cmpclr[1] = cmppxls.co_1;
				if(cmpclr[0].color565 > cmpclr[1].color565) {
					cmpclr[2].r = (cmpclr[0].r  * 2) / 3 + cmpclr[1].r / 3;
					cmpclr[2].g = (cmpclr[0].g  * 2) / 3 + cmpclr[1].g / 3;
					cmpclr[2].b = (cmpclr[0].b  * 2) / 3 + cmpclr[1].b / 3;
					cmpclr[3].r = cmpclr[0].r / 3 + (cmpclr[1].r * 2) / 3;
					cmpclr[3].g = cmpclr[0].g / 3 + (cmpclr[1].g * 2) / 3;
					cmpclr[3].b = cmpclr[0].b / 3 + (cmpclr[1].b * 2) / 3;
				} else {
					cmpclr[2].r = cmpclr[0].r / 2 + cmpclr[1].r / 2;
					cmpclr[2].g = cmpclr[0].g / 2 + cmpclr[1].g / 2;
					cmpclr[2].b = cmpclr[0].b / 2 + cmpclr[1].b / 2;
					cmpclr[3].r = 0x00;
					cmpclr[3].g = 0x00;
					cmpclr[3].b = 0x00;
				} 
				for(int j = 0; j < 16; j++) {
					int clridx = (cmppxls.map >> j*2) & 3;
					int row = (i / mapsperrow) * 4 + j / 4;
					int col = (i % mapsperrow) * 4 + j % 4;
					int loc = (row * m_hdr.dwWidth + col) * 3;
					((char *)m_imgdata)[loc] = (cmpclr[clridx].r * 255) / 31;
					((char *)m_imgdata)[loc + 1] = (cmpclr[clridx].g * 255) / 63;
					((char *)m_imgdata)[loc + 2] = (cmpclr[clridx].b * 255) / 31;
				}
			}
		} else if(m_hdr.ddspf.dwFourCC == g_iDXT3) {
			m_pxlsize = 4;
			m_alphadata = malloc(m_imgsize);
			DXT1_COLOR cmpclr[4];
			DXT3_MAP d3map;
			int clridx, units = m_imgsize / 16;
			int mapsperrow = m_hdr.dwWidth / 4;
			for(int i = 0; i < units; i++) {
				fread(&d3map, sizeof(d3map), 1, m_fh);
				cmpclr[0] = d3map.pxls.co_0;
				cmpclr[1] = d3map.pxls.co_1;
				if(cmpclr[0].color565 > cmpclr[1].color565) {
					cmpclr[2].r = (cmpclr[0].r  * 2) / 3 + cmpclr[1].r / 3;
					cmpclr[2].g = (cmpclr[0].g  * 2) / 3 + cmpclr[1].g / 3;
					cmpclr[2].b = (cmpclr[0].b  * 2) / 3 + cmpclr[1].b / 3;
					cmpclr[3].r = cmpclr[0].r / 3 + (cmpclr[1].r * 2) / 3;
					cmpclr[3].g = cmpclr[0].g / 3 + (cmpclr[1].g * 2) / 3;
					cmpclr[3].b = cmpclr[0].b / 3 + (cmpclr[1].b * 2) / 3;
				} else {
					cmpclr[2].r = cmpclr[0].r / 2 + cmpclr[1].r / 2;
					cmpclr[2].g = cmpclr[0].g / 2 + cmpclr[1].g / 2;
					cmpclr[2].b = cmpclr[0].b / 2 + cmpclr[1].b / 2;
					cmpclr[3].r = 0x00;
					cmpclr[3].g = 0x00;
					cmpclr[3].b = 0x00;
				}
				for(int j = 0; j < 16; j++) {
					int clridx = (d3map.pxls.map >> j*2) & 3;
					int row = (i / mapsperrow) * 4 + j / 4;
					int col = (i % mapsperrow) * 4 + j % 4;
					int loc = (row * m_hdr.dwWidth + col) * 3;
					((char *)m_imgdata)[loc] = (cmpclr[clridx].r * 255) / 31;
					((char *)m_imgdata)[loc + 1] = (cmpclr[clridx].g * 255) / 63;
					((char *)m_imgdata)[loc + 2] = (cmpclr[clridx].b * 255) / 31;
					((char *)m_alphadata)[row * m_hdr.dwWidth + col] = (d3map.alphamap >> j*4) & 0xF;
				}
			}
		} else if(m_hdr.ddspf.dwFourCC == g_iDXT4 || m_hdr.ddspf.dwFourCC == g_iDXT5) {
			m_pxlsize = 4;
			m_alphadata = malloc(m_imgsize);
			DXT1_COLOR cmpclr[4];
			BYTE bc3alphas[8];
			BC3_MAP bc3map;
			int clridx, units = m_imgsize / 16;
			int mapsperrow = m_hdr.dwWidth / 4; 
			for(int i = 0; i < units; i++) {
				fread(&bc3map, sizeof(bc3map), 1, m_fh);
				bc3alphas[0] = bc3map.ref_alpha[0];
				bc3alphas[1] = bc3map.ref_alpha[1];

				if( bc3alphas[0] > bc3alphas[1] ) {
					// 6 interpolated alpha values.
					bc3alphas[2] = 6/7*bc3alphas[0] + 1/7*bc3alphas[1]; // bit code 010
					bc3alphas[3] = 5/7*bc3alphas[0] + 2/7*bc3alphas[1]; // bit code 011
					bc3alphas[4] = 4/7*bc3alphas[0] + 3/7*bc3alphas[1]; // bit code 100
					bc3alphas[5] = 3/7*bc3alphas[0] + 4/7*bc3alphas[1]; // bit code 101
					bc3alphas[6] = 2/7*bc3alphas[0] + 5/7*bc3alphas[1]; // bit code 110
					bc3alphas[7] = 1/7*bc3alphas[0] + 6/7*bc3alphas[1]; // bit code 111
				} else {
					// 4 interpolated alpha values.
					bc3alphas[2] = 4/5*bc3alphas[0] + 1/5*bc3alphas[1]; // bit code 010
					bc3alphas[3] = 3/5*bc3alphas[0] + 2/5*bc3alphas[1]; // bit code 011
					bc3alphas[4] = 2/5*bc3alphas[0] + 3/5*bc3alphas[1]; // bit code 100
					bc3alphas[5] = 1/5*bc3alphas[0] + 4/5*bc3alphas[1]; // bit code 101
					bc3alphas[6] = 0;                         // bit code 110
					bc3alphas[7] = 255;                       // bit code 111
				}

				cmpclr[0] = bc3map.pxls.co_0;
				cmpclr[1] = bc3map.pxls.co_1;
				if(cmpclr[0].color565 > cmpclr[1].color565) {
					cmpclr[2].r = (cmpclr[0].r  * 2) / 3 + cmpclr[1].r / 3;
					cmpclr[2].g = (cmpclr[0].g  * 2) / 3 + cmpclr[1].g / 3;
					cmpclr[2].b = (cmpclr[0].b  * 2) / 3 + cmpclr[1].b / 3;
					cmpclr[3].r = cmpclr[0].r / 3 + (cmpclr[1].r * 2) / 3;
					cmpclr[3].g = cmpclr[0].g / 3 + (cmpclr[1].g * 2) / 3;
					cmpclr[3].b = cmpclr[0].b / 3 + (cmpclr[1].b * 2) / 3;
				} else {
					cmpclr[2].r = cmpclr[0].r / 2 + cmpclr[1].r / 2;
					cmpclr[2].g = cmpclr[0].g / 2 + cmpclr[1].g / 2;
					cmpclr[2].b = cmpclr[0].b / 2 + cmpclr[1].b / 2;
					cmpclr[3].r = 0x00;
					cmpclr[3].g = 0x00;
					cmpclr[3].b = 0x00;
				}

				for(int j = 0; j < 16; j++) {
					int clridx = (bc3map.pxls.map >> j*2) & 3;
					long long alphatbl = *(long long *)bc3map.idx_alpha & 0x00FFFFFFFFFFFF;
					int row = (i / mapsperrow) * 4 + j / 4;
					int col = (i % mapsperrow) * 4 + j % 4;
					int loc = (row * m_hdr.dwWidth + col) * 3;
					((char *)m_imgdata)[loc] = (cmpclr[clridx].r * 255) / 31;
					((char *)m_imgdata)[loc + 1] = (cmpclr[clridx].g * 255) / 63;
					((char *)m_imgdata)[loc + 2] = (cmpclr[clridx].b * 255) / 31;
					((char *)m_alphadata)[row * m_hdr.dwWidth + col] = bc3alphas[(alphatbl >> j*3) & 7];
				}
			}
		} else return UNSUPPFMT;
	} else {
		if(m_pxlsize == 4) {
			m_alphadata = malloc(m_imgsize);
			for(int i = 0; i < m_imgsize * 4; i+=4) {
				fread((char *)m_imgdata+i*3/4, 1, 3, m_fh);
				fread((char *)m_alphadata+(i/4), 1, 1, m_fh);
			}
		} else //not a DXT1 compressed image
			if(m_pxlsize == 2) {
				if(m_bisalpha)
					m_alphadata = malloc(m_imgsize);
				int px;
				int r, g, b, a;
				int ashift, rshift, gshift, bshift;
				ashift = rshift = gshift = bshift = 0;
				int i = 0;
				for(int i = 0; i < 16; i++) {
					if(!ashift && ((m_hdr.ddspf.dwABitMask >> i) & 1))
						ashift = i;
					if(!rshift && ((m_hdr.ddspf.dwRBitMask >> i) & 1))
						rshift = i;
					if(!gshift && ((m_hdr.ddspf.dwGBitMask >> i) & 1))
						gshift = i;
					//bshift should always be 0...
//					if(!bshift && ((m_hdr.ddspf.dwBBitMask >> i) & 1))
//						bshift = i;
				}
				for(int i = 0; i < m_imgsize; i++) {
					fread(&px, 2, 1, m_fh);
					r = px & m_hdr.ddspf.dwRBitMask;
					g = px & m_hdr.ddspf.dwGBitMask;
					b = px & m_hdr.ddspf.dwBBitMask;
					if(rshift == 11) { //5/6/5 color (no alpha)
						((char *)m_imgdata)[i*3] = ((r >> rshift) * 255) / 31;
						((char *)m_imgdata)[i*3+1] = ((g >> gshift) * 255) / 63;
						((char *)m_imgdata)[i*3+2] = ((b /*>> bshift*/) * 255) / 31;
						if(m_bisalpha) {
							a = px & m_hdr.ddspf.dwABitMask;
							((char *)m_alphadata)[i] = ((a >> ashift) * 255) / 15;
						}
					}
					if(rshift == 10) { //1/5/5/5 color (1 bit alpha)
						((char *)m_imgdata)[i*3] = ((r >> rshift) * 255) / 31;
						((char *)m_imgdata)[i*3+1] = ((g >> gshift) * 255) / 31;
						((char *)m_imgdata)[i*3+2] = ((b >> bshift) * 255) / 31;
						if(m_bisalpha) {
							if(px & m_hdr.ddspf.dwABitMask)
								((char *)m_alphadata)[i] = 255;
							else
								((char *)m_alphadata)[i] = 0;
						}
					}
					if(rshift == 8) { //4/4/4/4 color (4 bit alpha)
						((char *)m_imgdata)[i*3] = ((r >> rshift) * 255) / 15;
						((char *)m_imgdata)[i*3+1] = ((g >> gshift) * 255) / 15;
						((char *)m_imgdata)[i*3+2] = ((b /*>> bshift*/) * 255) / 15;
						if(m_bisalpha) {
							a = px & m_hdr.ddspf.dwABitMask;
							((char *)m_alphadata)[i] = ((a >> ashift) * 255) / 15;
						}
					}
				}
			} else
				fread(m_imgdata, 1, m_imgsize * m_pxlsize, m_fh);
	}

	if(m_pxlsize > 2) {
		//swap BGR -> RGB
		char tmp;
		for(int i = 0; i < m_imgsize * 3; i+=3) {
			tmp = ((char *)m_imgdata)[i];
			((char *)m_imgdata)[i] = ((char *)m_imgdata)[i+2];
			((char *)m_imgdata)[i+2] = tmp;
		}
	}

	fclose(m_fh);
	sprintf(buffer, "Done loading %s", m_fpath);
	frame->setstatus(buffer);
	return 0;
}

void dds::close() {
	m_fh = NULL;
	m_fsize = 0;
	m_pxlsize = 0;
	m_imgsize = 0;
	m_bIsextended = false;
	memset(&m_hdr, 0, sizeof(DDS_HEADER));
	memset(&m_dx10hdr, 0, sizeof(DDS_HEADER_DXT10));
	sprintf(buffer, "Closed %s", strlen(m_fname) > 0 ? m_fname : m_fpath);
	frame->setstatus(buffer);
}

int dds::loadheader() {
	unsigned tag;
	fseek(m_fh, 0, SEEK_END);
	m_fsize = ftell(m_fh);
	fseek(m_fh, 0, SEEK_SET);

	if(m_fsize < 4 + sizeof(DDS_HEADER))
		return INVALIDFILE;

	fread(&tag, 4, 1, m_fh);
	if(tag != 0x20534444)
		return INVALIDFILE;

	fread(&m_hdr, 1, sizeof(DDS_HEADER), m_fh);
	if(m_hdr.ddspf.dwFlags & DDPF_FOURCC) {
		memcpy(buffer, &m_hdr.ddspf.dwFourCC, 4);
		buffer[4] = 0;
		if(!strcmp(buffer, "DX10")) {
			printf("Extended DX10 Header Present\n");
			m_bIsextended = true;
			fread(&m_dx10hdr, 1, sizeof(DDS_HEADER_DXT10), m_fh);
		}
	}

	return 0;
}

void dds::showinfo() {
	printheader();
	printf("Main Surface Data Offset: 0x%.8X\n", m_pData);
	printf("Extra Data Offset: 0x%.8X\n\n", m_pData2);
}

void dds::printheader() {
	sprintf(buffer, "Details for %s", m_fname);
	wxDialog dlgDetails(frame, -1, wxString(buffer), wxDefaultPosition, wxSize(400, 500), wxCAPTION);

	int printed = 0;
	printed = sprintf(buffer, "\nHeader info for file: %s\n\n", m_fpath);
	printed += sprintf(buffer+printed, "Size: %d\n", m_hdr.dwSize);
	printed += sprintf(buffer+printed, "Flags: 0x%.8X\n", m_hdr.dwFlags);
	printed += sprintf(buffer+printed, "Height: %d\n", m_hdr.dwHeight);
	printed += sprintf(buffer+printed, "Width: %d\n", m_hdr.dwWidth);
	printed += sprintf(buffer+printed, "PitchOrLinearSize: %d\n", m_hdr.dwPitchOrLinearSize);
	printed += sprintf(buffer+printed, "Depth: %d\n", m_hdr.dwDepth);
	printed += sprintf(buffer+printed, "MipMapCount: %d\n", m_hdr.dwMipMapCount);
	printed += sprintf(buffer+printed, "Reserved[11] (omitted)\n");
	printed += sprintf(buffer+printed, "Pixel Format:\n");
		printed += sprintf(buffer+printed, "  Size: %d\n", m_hdr.ddspf.dwSize);
		//buffer[0] = 0;
		printed += sprintf(buffer+printed, "  Flags: ");
		if(m_hdr.ddspf.dwFlags & DDPF_ALPHAPIXELS)
			printed += sprintf(buffer+printed, "DDPF_ALPHAPIXELS ");
		if(m_hdr.ddspf.dwFlags & DDPF_ALPHA)
			printed += sprintf(buffer+printed, "DDPF_ALPHA ");
		if(m_hdr.ddspf.dwFlags & DDPF_FOURCC)
			printed += sprintf(buffer+printed, "DDPF_FOURCC ");
		if(m_hdr.ddspf.dwFlags & DDPF_RGB)
			printed += sprintf(buffer+printed, "DDPF_RGB ");
		if(m_hdr.ddspf.dwFlags & DDPF_YUV)
			printed += sprintf(buffer+printed, "DDPF_YUV ");
		if(m_hdr.ddspf.dwFlags & DDPF_LUMINANCE)
			printed += sprintf(buffer+printed, "DDPF_LUMINANCE ");
//		buffer[4] = 0;
		printed += sprintf(buffer+printed, "\n  FourCC: ");
		snprintf(buffer+printed, 5, "%s", m_hdr.ddspf.dwFourCC ? (char *)&m_hdr.ddspf.dwFourCC : "None");
		printed += 4;
		printed += sprintf(buffer+printed, "\n  RGBBitCount: %d\n", m_hdr.ddspf.dwRGBBitCount);
		printed += sprintf(buffer+printed, "  RBitMask: 0x%.8X\n", m_hdr.ddspf.dwRBitMask);
		printed += sprintf(buffer+printed, "  GBitMask: 0x%.8X\n", m_hdr.ddspf.dwGBitMask);
		printed += sprintf(buffer+printed, "  BBitMask: 0x%.8X\n", m_hdr.ddspf.dwBBitMask);
		printed += sprintf(buffer+printed, "  ABitMask: 0x%.8X\n", m_hdr.ddspf.dwABitMask);
	printed += sprintf(buffer+printed, "Caps: %d\n", m_hdr.dwCaps);
	printed += sprintf(buffer+printed, "Caps2: %d\n", m_hdr.dwCaps2);
	printed += sprintf(buffer+printed, "Caps3: %d\n", m_hdr.dwCaps3);
	printed += sprintf(buffer+printed, "Caps4: %d\n", m_hdr.dwCaps4);
	printed += sprintf(buffer+printed, "Reserved2: 0x%.8X\n", m_hdr.dwReserved2);
	
	wxStaticText lblDetails(&dlgDetails, -1, buffer, wxPoint(10, 0));
	dlgDetails.ShowModal();

	if(m_bIsextended) {
		printf("Extended DX10 Header:\n");
			printf("\tDXGI Format: %d\n", m_dx10hdr.dxgiFormat);
			printf("\tResource Dimension: %d\n", m_dx10hdr.resourceDimension);
			printf("\tMisc Flags: 0x%.8X\n", m_dx10hdr.miscFlag);
			printf("\tArray Size: %d\n", m_dx10hdr.arraySize);
			printf("\tMisc Flags 2: 0x%.8X\n", m_dx10hdr.miscFlags2);
	}
}

DDS_HEADER *dds::getheader() {
	return &m_hdr;
}

void *dds::getimgdata() {
	return m_imgdata;
}

void *dds::getalphadata() {
	return m_alphadata;
}

int dds::getimgsize() {
	return m_imgsize;
}

int dds::getpxlsize() {
	return m_pxlsize;
}

bool dds::isalpha() {
	return m_bisalpha;
}
