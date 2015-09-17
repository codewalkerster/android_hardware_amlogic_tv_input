
#include <stdlib.h>
#include <fcntl.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <linux/videodev2.h>
#include <dirent.h>

#include <binder/MemoryHeapBase.h>
#include <binder/MemoryBase.h>

using namespace android;


#include "CTvScreenCapture.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

int CTvScreenCapture::xioctl(int fd, int request, void *arg)
{
	int r = 0;

	do {
		r = ioctl(fd, request, arg);
	} while (-1 == r && EINTR == errno);

	return r;
}

int CTvScreenCapture::OpenCamera(struct camera *pCameraDev)
{
	int iOutRet = 0, iRet;
	struct stat st;

	do {
		if(-1 == stat(pCameraDev->device_name, &st)) {
			LOGD( "Cannot identify '%s'\n", pCameraDev->device_name);
			iOutRet = FAILED;
			break;
		}

		if (!S_ISCHR(st.st_mode)) {
			LOGD("%s is no device\n", pCameraDev->device_name);
			iOutRet = FAILED;
			break;
		}

		pCameraDev->fd = open(pCameraDev->device_name, O_RDWR	| O_NONBLOCK, 0); //  O_NONBLOCK
		if (SUCCEED > pCameraDev->fd) {
			LOGD("Cannot open '%s'\n", pCameraDev->device_name);
			iOutRet = FAILED;
			break;
		}
	} while(FALSE);

	return iOutRet;
}


int CTvScreenCapture::InitVCap(sp<IMemory> Mem)
{
	int iOutRet = FAILED;

	do {
		m_pMem = Mem;
		m_pData = (char *)m_pMem->pointer();
		LOGD("VVVVVVVVVVVVVVVVVVVVVVVVVVVVV %p\n", m_pData);
		//default
		m_capV4l2Cam.device_name = "/dev/video11";
		m_capV4l2Cam.buffers = NULL;
		m_capV4l2Cam.width = 640;
		m_capV4l2Cam.height = 480;
		m_capV4l2Cam.display_depth = 16; //5; /* RGB24 */
		m_capV4l2Cam.frame_number = 25;  //fps
		iOutRet = OpenCamera(&m_capV4l2Cam) ;
		if(SUCCEED != iOutRet) {
			LOGD("ERROR:::Open Camera device failed\n");
			break;
		}
	} while(FALSE);

	return iOutRet;
}

int CTvScreenCapture::InitMmap(struct camera *cam)
{
	int iOutRet = SUCCEED, iRet;
	struct v4l2_requestbuffers req;

	do {
		CLEAR(req);

		req.count = 4;
		req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		req.memory = V4L2_MEMORY_MMAP;

		iRet = xioctl(cam->fd, VIDIOC_REQBUFS, &req);
		if (FAILED == iRet) {
			if (EINVAL == errno) {
				LOGD("VIDIOC_REQBUFS %s does not support memory mapping\n", cam->device_name);
			}
			iOutRet = iRet;
			break;
		}

		if (req.count < 2) {
			LOGD("Insufficient buffer memory on %s\n", cam->device_name);
			iOutRet = FAILED;
			break;
		}

		cam->buffers = (struct buffer *)calloc(req.count, sizeof(*(cam->buffers)));
		if (!cam->buffers) {
			LOGD("Out of memory\n");
			iOutRet = FAILED;
			break;
		}

		for (m_capNumBuffers = 0; m_capNumBuffers < req.count; ++m_capNumBuffers) {
			struct v4l2_buffer buf;

			CLEAR(buf);

			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = m_capNumBuffers;

			if (FAILED == xioctl(cam->fd, VIDIOC_QUERYBUF, &buf)) {
				LOGD("VIDIOC_QUERYBUF ERROR\n");
				iOutRet = FAILED;
				goto IS_ERROR;
			}

			cam->buffers[m_capNumBuffers].length = buf.length;
			cam->buffers[m_capNumBuffers].start = mmap(NULL /* start anywhere */,
												  buf.length, PROT_READ | PROT_WRITE /* required */,
												  MAP_SHARED /* recommended */, cam->fd, buf.m.offset);

			if (MAP_FAILED == cam->buffers[m_capNumBuffers].start) {
				iOutRet = FAILED;
				break;
			}


		}

		LOGD("END m_capNumBuffers : %d\n", m_capNumBuffers);
	} while(FALSE);
IS_ERROR:
	return iOutRet;
}

int CTvScreenCapture::InitCamera(struct camera *cam)
{
	int iOutRet = SUCCEED, iRet;
	struct v4l2_capability *cap = &(cam->v4l2_cap);
	struct v4l2_cropcap *cropcap = &(cam->v4l2_cropcap);
	struct v4l2_crop *crop = &(cam->crop);
	struct v4l2_format *fmt = &(cam->v4l2_fmt);
	unsigned int min;

	do {
		iRet = xioctl(cam->fd, VIDIOC_QUERYCAP, cap);
		if(FAILED == iRet) {
			if(EINVAL == errno) {
				LOGD("%s is no V4L2 device\n", cam->device_name);
			}
			iOutRet = iRet;
			break;
		}

		if(!(cap->capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
			LOGD("%s is no video capture device\n", cam->device_name);
			iOutRet = FAILED;
			break;
		}

		if(!(cap->capabilities & V4L2_CAP_STREAMING)) {
			LOGD("%s does not support streaming i/o\n", cam->device_name);
			iOutRet = FAILED;
			break;
		}

		LOGD("VIDOOC_QUERYCAP camera driver is [%s]  card is [%s] businfo is [%s] version is [%d]\n", cap->driver,
			 cap->card, cap->bus_info, cap->version);

		/* Select video input, video standard and tune here. */

		CLEAR(*cropcap);
		cropcap->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		crop->c.width = cam->width;
		crop->c.height = cam->height;
		crop->c.left = 0;
		crop->c.top = 0;
		crop->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		CLEAR(*fmt);
		fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmt->fmt.pix.width = cam->width;
		fmt->fmt.pix.height = cam->height;
		fmt->fmt.pix.pixelformat = V4L2_PIX_FMT_NV21;
		fmt->fmt.pix.field = V4L2_FIELD_INTERLACED;
		iRet = xioctl(cam->fd, VIDIOC_S_FMT, fmt);
		if (FAILED == iRet) {
			iOutRet = iRet;
			LOGD("VIDIOC_S_FMT is ERROR\n");
			break;
		}

		/* Note VIDIOC_S_FMT may change width and height. */
		/* Buggy driver paranoia. */
		min = fmt->fmt.pix.width * 2;
		if (fmt->fmt.pix.bytesperline < min) {
			fmt->fmt.pix.bytesperline = min;
		}

		min = fmt->fmt.pix.bytesperline * fmt->fmt.pix.height;
		if (fmt->fmt.pix.sizeimage < min) {
			fmt->fmt.pix.sizeimage = min;
		}

		iRet = InitMmap(cam);
		if(FAILED == iRet) {
			LOGD("INIT MMAP FAILED\n");
			iOutRet = iRet;
			break;
		}

	} while(FALSE);

	return iOutRet;

}


int CTvScreenCapture::SetVideoParameter(int width, int height, int frame)
{
	int iOutRet = SUCCEED, iRet;

	do {
		m_capV4l2Cam.width = width;
		m_capV4l2Cam.height = height;
		m_capV4l2Cam.frame_number = frame;

		iRet = InitCamera(&m_capV4l2Cam);
		if(SUCCEED != iRet) {
			iOutRet = iRet;
			break;
		}

	} while(FALSE);

	return iOutRet ;
}

int CTvScreenCapture::StartCapturing(struct camera *cam)
{
	unsigned int i;
	int iOutRet = SUCCEED, iRet;
	enum v4l2_buf_type type;

	do {
		for (i = 0; i < m_capNumBuffers; ++i) {
			struct v4l2_buffer buf;

			//CLEAR(buf);

			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = i;

			iRet = xioctl(cam->fd, VIDIOC_QBUF, &buf);
			if(FAILED == iRet) {
				iOutRet = iRet;
				goto IS_ERROR;
			}
		}


		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		iRet = xioctl(cam->fd, VIDIOC_STREAMON, &type);
		if(FAILED == iRet) {
			iOutRet = iRet;
			break;
		}
	} while(FALSE);
IS_ERROR:

	return iOutRet;
}

int CTvScreenCapture::VideoStart()
{
	int iOutRet = SUCCEED, iRet;

	do {
		iRet = StartCapturing(&m_capV4l2Cam);
		if(FAILED == iRet) {
			iOutRet = iRet;
			break;
		}

	} while(FALSE);

	return iOutRet;
}

void CTvScreenCapture::yuv_to_rgb32(unsigned char y, unsigned char u, unsigned char v, unsigned char *rgb)
{
	register int r, g, b;
	int rgb24;

	r = (1192 * (y - 16) + 1634 * (v - 128) ) >> 10;
	g = (1192 * (y - 16) - 833 * (v - 128) - 400 * (u - 128) ) >> 10;
	b = (1192 * (y - 16) + 2066 * (u - 128) ) >> 10;

	r = r > 255 ? 255 : r < 0 ? 0 : r;
	g = g > 255 ? 255 : g < 0 ? 0 : g;
	b = b > 255 ? 255 : b < 0 ? 0 : b;

	rgb24 = (int)((r << 16) | (g  << 8) | b);

	/*ARGB*/
	*rgb = (unsigned char)r;
	rgb ++;
	*rgb = (unsigned char)g;
	rgb++;
	*rgb = (unsigned char)b;
	rgb++;
	*rgb = 0xff;
}


void CTvScreenCapture::nv21_to_rgb32(unsigned char *buf, unsigned char *rgb, int width, int height, int *len)
{
	int x, y, z = 0;
	int h, w;
	int blocks;
	unsigned char Y1, Y2, U, V;

	*len = 0;

	blocks = (width * height) * 2;

	for(h = 0, z = 0; h < height; h += 2) {
		for (y = 0; y < width * 2; y += 2) {

			Y1 = buf[ h * width + y + 0];
			V = buf[ blocks / 2 + h * width / 2 + y % width + 0 ];
			Y2 = buf[ h * width + y + 1];
			U = buf[ blocks / 2 + h * width / 2 + y % width + 1 ];

			yuv_to_rgb32(Y1, U, V, &rgb[z]);
			yuv_to_rgb32(Y2, U, V, &rgb[z + 4]);
			z += 8;
		}
	}
	*len = z;
	LOGD("z  +++++++++++++++++++++++++++++++++   z  %d\n", z);
}


int CTvScreenCapture::GetVideoData(int *length)
{
	int iOutRet = SUCCEED, iRet;

	*length = 0;
	while(true) {
		fd_set fds;
		struct timeval tv;
		FD_ZERO(&fds);
		FD_SET(m_capV4l2Cam.fd, &fds);
		/* Timeout. */
		tv.tv_sec = 0;
		tv.tv_usec = 30000;
		iRet = select(m_capV4l2Cam.fd + 1, &fds, NULL, NULL, &tv);
		if (FAILED == iRet) {
			LOGD("select FAILED\n");
			if (EINTR == errno) {
				LOGD("select FAILED Continue\n");
				continue;
			}

		}

		if (0 == iRet) {
			LOGD("select timeout\n");
			continue ;
		}

		struct v4l2_buffer buf;
		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		iRet = xioctl(m_capV4l2Cam.fd, VIDIOC_DQBUF, &buf);
		if(FAILED == iRet) {
			if(errno == EAGAIN) {
				LOGD("GetVideoData EAGAIN \n");
			}

			continue;
		}

		LOGD("DDDDDDDDDDAAAAAAAAAAAAAAAAAAAATTTTTTTTTTTTTTAAAAAAAAAAAAAAAAAAAAAAAAAAAAA  %d %d\n", buf.length, iRet);
		int tmpLen = 0;
		nv21_to_rgb32((unsigned char *)m_capV4l2Cam.buffers[buf.index].start, (unsigned char *)m_pData, m_capV4l2Cam.width, m_capV4l2Cam.height, &tmpLen);
		//memcpy(m_pData,m_capV4l2Cam.buffers[buf.index].start, buf.length +1);
		*length = buf.length;
		break;

	}

	if(*length > 0) {
		mCapEvt.mFrameWide = m_capV4l2Cam.width;
		mCapEvt.mFrameHeight = m_capV4l2Cam.height;
		mCapEvt.mFrameNum = 1;
		mCapEvt.mFrameSize = *length;
	} else {
		mCapEvt.mFrameWide = 0;
		mCapEvt.mFrameHeight = 0;
		mCapEvt.mFrameNum = 0;
		mCapEvt.mFrameSize = 0;
	}

	if(NULL != mpObserver) {
		mpObserver->onTvEvent(mCapEvt);
	}

	return iOutRet;
}

int CTvScreenCapture::StopCapturing(struct camera *cam)
{
	int iOutRet = SUCCEED, iRet;
	enum v4l2_buf_type type;

	do {
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		iRet = xioctl(cam->fd, VIDIOC_STREAMOFF, &type);
		if(FAILED == iRet) {
			iOutRet = iRet;
			break;
		}
	} while(FALSE);

	return iOutRet;
}

int  CTvScreenCapture::VideoStop()
{
	StopCapturing(&m_capV4l2Cam);
	return SUCCEED;
}

int CTvScreenCapture::UninitCamera(struct camera *cam)
{
	unsigned int i;

	for (i = 0; i < m_capNumBuffers; ++i) {
		if(cam->buffers[i].start == NULL) {
			break;
		}

		if (FAILED == munmap(cam->buffers[i].start, cam->buffers[i].length)) {
			LOGD("ERROR::munmap cam buffer failed\n");
			break;
		}
	}

	if(NULL != cam->buffers)
		free(cam->buffers);

	cam->buffers = NULL;

	return SUCCEED;
}

int CTvScreenCapture::CloseCamera(struct camera *cam)
{
	int iOutRet = SUCCEED, iRet;

	do {
		iRet = close(cam->fd);
		if(FAILED == iRet) {
			iOutRet = iRet;
			break;
		}

		cam->fd = -1;
	} while(FALSE);

	return iOutRet;
}

int CTvScreenCapture::DeinitVideoCap()
{
	UninitCamera(&m_capV4l2Cam);
	CloseCamera(&m_capV4l2Cam);
	return SUCCEED ;
}


CTvScreenCapture::CTvScreenCapture()
{
	m_capNumBuffers = 0;
	memset(&m_capV4l2Cam, 0x00, sizeof(camera));
	mpObserver = NULL;
}
CTvScreenCapture::~CTvScreenCapture()
{
	memset(&m_capV4l2Cam, 0x00, sizeof(camera));
	m_pData = NULL;
}

