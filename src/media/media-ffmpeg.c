/* media-ffmpeg.c - analyse audio files or streams via ffmpeg

   Copyright (C) 2006 Sebastian Freundt

This file is part of SXEmacs

SXEmacs is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SXEmacs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. */


/* Synched up with: Not in FSF. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "lisp.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#if defined HAVE_STDINT_H
# include <stdint.h>
#endif	/* HAVE_STDINT_H */

#include "buffer.h"
#include "sysdep.h"
#include "sysfile.h"

#include "media-ffmpeg.h"

static int media_ffmpeg_bitrate(AVCodecContext*);
static AVFormatContext *media_ffmpeg_open_file(const char*);
AVFormatContext *media_ffmpeg_open_data(char*, size_t);

static void media_ffmpeg_analyse_audio(media_substream*, AVFormatContext*, int);

#define MYSELF MDRIVER_FFMPEG

Lisp_Object Qffmpeg;

#define __FFMPEG_DEBUG__(args...)	fprintf(stderr, "FFMPEG " args)
#ifndef FFMPEG_DEBUG_FLAG
#define FFMPEG_DEBUG(args...)
#else
#define FFMPEG_DEBUG(args...)		__FFMPEG_DEBUG__(args)
#endif
#define FFMPEG_DEBUG_AVF(args...)	FFMPEG_DEBUG("[avformat]: " args)
#define FFMPEG_DEBUG_AVC(args...)	FFMPEG_DEBUG("[avcodec]: " args)
#define FFMPEG_DEBUG_AVS(args...)	FFMPEG_DEBUG("[stream]: " args)
#define FFMPEG_CRITICAL(args...)	__FFMPEG_DEBUG__("CRITICAL: " args)


DECLARE_MEDIA_DRIVER_OPEN_METH(media_ffmpeg);
DECLARE_MEDIA_DRIVER_CLOSE_METH(media_ffmpeg);
DECLARE_MEDIA_DRIVER_PRINT_METH(media_ffmpeg);
DECLARE_MEDIA_DRIVER_READ_METH(media_ffmpeg);
DECLARE_MEDIA_DRIVER_REWIND_METH(media_ffmpeg);

DEFINE_MEDIA_DRIVER_CUSTOM(media_ffmpeg,
			   media_ffmpeg_open, media_ffmpeg_close,
			   media_ffmpeg_print, NULL,
			   media_ffmpeg_read, NULL,
			   media_ffmpeg_rewind, NULL);


static int
media_ffmpeg_bitrate(AVCodecContext *enc)
{
	int bitrate;

	/* for PCM codecs, compute bitrate directly */
	switch ((unsigned int)enc->codec_id) {
	case CODEC_ID_PCM_S32LE:
	case CODEC_ID_PCM_S32BE:
	case CODEC_ID_PCM_U32LE:
	case CODEC_ID_PCM_U32BE:
		bitrate = enc->sample_rate * enc->channels * 32;
		break;
	case CODEC_ID_PCM_S24LE:
	case CODEC_ID_PCM_S24BE:
	case CODEC_ID_PCM_U24LE:
	case CODEC_ID_PCM_U24BE:
	case CODEC_ID_PCM_S24DAUD:
		bitrate = enc->sample_rate * enc->channels * 24;
		break;
	case CODEC_ID_PCM_S16LE:
	case CODEC_ID_PCM_S16BE:
	case CODEC_ID_PCM_U16LE:
	case CODEC_ID_PCM_U16BE:
		bitrate = enc->sample_rate * enc->channels * 16;
		break;
	case CODEC_ID_PCM_S8:
	case CODEC_ID_PCM_U8:
	case CODEC_ID_PCM_ALAW:
	case CODEC_ID_PCM_MULAW:
		bitrate = enc->sample_rate * enc->channels * 8;
		break;
	default:
		bitrate = enc->bit_rate;
		break;
	}
	return bitrate;
}

char *media_ffmpeg_streaminfo(Lisp_Media_Stream *ms)
{
	AVFormatContext *avfc = NULL;
	char *out;
	int chars_left = 4095;

	avfc = media_stream_data(ms);
	out = xmalloc_atomic(chars_left+1);
	out[0] = '\0';
	out[chars_left] = '\0';

	/* cannot use ffmpeg on corrupt streams */
	if (media_stream_driver(ms) != MYSELF || avfc == NULL)
		return out;

#if 0				/* dunno what to do with this stuff now */
	if (avfc->author && *avfc->author) {
		strncat(out, " :author \"", chars_left);
		chars_left -= 10;
		strncat(out, avfc->author, chars_left);
		chars_left -= strlen(avfc->author);
		strncat(out, "\"", chars_left--);
	}
	if (avfc->title && *avfc->title) {
		strncat(out, " :title: \"", chars_left);
		chars_left -= 10;
		strncat(out, avfc->title, chars_left);
		chars_left -= strlen(avfc->title);
		strncat(out, "\"", chars_left--);
	}
	if (avfc->year) {
		char year[12];
		int sz = snprintf(year, sizeof(year), "%d", avfc->year);
		assert(sz>=0 && sz<sizeof(year));
		strncat(out, " :year ", chars_left);
		chars_left -= 7;
		strncat(out, year, chars_left);
	}
#endif

	return out;
}

static void
media_ffmpeg_print(Lisp_Object ms, Lisp_Object pcfun, int ef)
{
	return;
}

static AVFormatContext*
media_ffmpeg_open_file(const char *file)
{
	AVFormatContext *avfc = avformat_alloc_context();

	/* open the file */
	if (avformat_open_input(&avfc, file, NULL, NULL) < 0) {
		FFMPEG_DEBUG_AVF("opening file failed.\n");
		if (avfc)
			xfree(avfc);
		return NULL;
	}

	/* Retrieve stream information */
	if (avformat_find_stream_info(avfc, NULL) < 0) {
		FFMPEG_DEBUG_AVS("opening stream inside file failed.\n");
		avformat_close_input(&avfc);
		if (avfc)
			xfree(avfc);
		return NULL;
	}

	return avfc;
}


#if 0				/* no idea wtf to do with this shit */
static int
media_ffmpeg_vio_open(URLContext *h, const char *filename, int flags)
{
	return 0;
}

static int
media_ffmpeg_vio_read(URLContext *h, unsigned char *buf, int size)
{
	media_data *sd = (media_data*)h->priv_data;

	FFMPEG_DEBUG_AVS("reading %d bytes to 0x%x, respecting seek %ld\n",
			 size, (unsigned int)buf, sd->seek);

	if ((long int)sd->length <= sd->seek) {
		FFMPEG_DEBUG_AVS("eof\n");
		return -1;
	}

	memcpy(buf, sd->data+sd->seek, size);
	sd->seek += size;

	return size;
}

static int
media_ffmpeg_vio_write(URLContext *h, unsigned char *buf, int size)
{
	return -1;
}

static int64_t
media_ffmpeg_vio_seek(URLContext *h, int64_t pos, int whence)
{
	media_data *sd = (media_data*)h->priv_data;

	FFMPEG_DEBUG_AVS("seeking to %ld via %d\n", (long int)pos, whence);

	switch (whence) {
	case SEEK_SET:
		sd->seek = pos;
		break;
	case SEEK_CUR:
		sd->seek = sd->seek+pos;
		break;
	case SEEK_END:
		sd->seek = sd->length+pos;
		break;
	default:
		/* be prolific */
		abort();
	}
	return sd->seek;
}

static int
media_ffmpeg_vio_close(URLContext *h)
{
	if (h->priv_data)
		xfree(h->priv_data);
	h->priv_data = NULL;
	return 0;
}

/* this is a memory-i/o protocol in case we have to deal with string data */
static URLProtocol media_ffmpeg_protocol = {
	"SXEmff",
	media_ffmpeg_vio_open,
	media_ffmpeg_vio_read,
	media_ffmpeg_vio_write,
	media_ffmpeg_vio_seek,
	media_ffmpeg_vio_close,
};
#endif

/** Size of probe buffer, for guessing file type from file contents. */
#define PROBE_BUF_MIN 2048
#define PROBE_BUF_MAX 131072

AVFormatContext*
media_ffmpeg_open_data(char *data, size_t size)
{
	AVFormatContext *avfc = avformat_alloc_context();
	AVProbeData *pd = NULL;
	AVIOContext *bioctx = NULL;
	AVInputFormat *fmt = NULL;
	char file[] = "SXEmff:SXEmacs.mp3\000";
	media_data *sd = NULL;

	/* initialise our media_data */
	sd = xnew_and_zero(media_data);
	sd->length = size;
	sd->seek = 0;
	sd->data = data;

	/* register ffmpeg byteio */
	bioctx = xnew_and_zero(AVIOContext);
	avio_open(&bioctx, file, AVIO_FLAG_READ);

	/* erm, register us at the byteio context */
#if 0
	((URLContext*)(bioctx->opaque))->priv_data = sd;
#endif

	/* take a probe */
	pd = xnew_and_zero(AVProbeData);
	pd->filename = file;
	pd->buf = NULL;
	pd->buf_size = 0;

	pd->buf = (void*)sd->data;
	pd->buf_size = PROBE_BUF_MIN;
	fmt = av_probe_input_format(pd, 1);

	/* if still no format found, error */
	if (!fmt) {
		xfree(pd);
		xfree(bioctx);
		xfree(sd);
		xfree(avfc);
		return NULL;
	}

	/* open the file */
	if (avformat_open_input(&avfc, file, fmt, NULL) < 0) {
		xfree(pd);
		xfree(bioctx);
		xfree(sd);
		xfree(avfc);
		return NULL;
	}

	/* Retrieve stream information */
	if (avformat_find_stream_info(avfc, NULL) < 0) {
		xfree(pd);
		xfree(bioctx);
		xfree(sd);
		xfree(avfc);
		return NULL;
	}

	return avfc;
}

static void
media_ffmpeg_close(ms_driver_data_t data)
{
	AVFormatContext *avfc = (AVFormatContext*)data;
	FFMPEG_DEBUG_AVF("closing AVFormatContext: 0x%lx\n",
			 (long unsigned int)avfc);
	if (avfc && avfc->iformat)
		avformat_close_input(&avfc);
}

static void
media_ffmpeg_analyse_audio(media_substream *mss, AVFormatContext *avfc, int st)
{
	mtype_audio_properties *mtap;
	const char *name = NULL;
	const char *codec_name = NULL;
	/* libavformat cruft */
	AVStream *avst = NULL;
	AVCodecContext *avcc = NULL;

	/* unpack the stream and codec context from the container, again */
	if (avfc)
	  avst = avfc->streams[st];
	if (avst)
	  avcc = avst->codec;

	/* initialise */
	mtap = xnew_and_zero(mtype_audio_properties);

	/* copy the name */
	if (avfc && avfc->iformat)
		name = avfc->iformat->name;
	if (avcc && avcc->codec)
		codec_name = avcc->codec->name;

	mtap->name = name;
	mtap->codec_name = codec_name;
	if (avcc ) {
		mtap->channels = avcc->channels;
		mtap->samplerate = avcc->sample_rate;
		mtap->bitrate = media_ffmpeg_bitrate(avcc);

		/* samplewidth and framesize */
		switch (avcc->sample_fmt) {
		case AV_SAMPLE_FMT_U8:
			mtap->samplewidth = 8;
			mtap->framesize = mtap->channels * 1;
			mtap->msf = sxe_msf_U8;
			break;
		case AV_SAMPLE_FMT_S16:
			mtap->samplewidth = 16;
			mtap->framesize = mtap->channels * 2;
			mtap->msf = sxe_msf_S16;
			break;
#if defined AV_SAMPLE_FMT_S24
		case AV_SAMPLE_FMT_S24:
			mtap->samplewidth = 32;
			mtap->framesize = mtap->channels * 4;
			mtap->msf = sxe_msf_S24;
			break;
#endif	/* SAMPLE_FMT_S24 */
		case AV_SAMPLE_FMT_S32:
			mtap->samplewidth = 32;
			mtap->framesize = mtap->channels * 4;
			mtap->msf = sxe_msf_S32;
			break;
		case AV_SAMPLE_FMT_FLT:
			mtap->samplewidth = 8*sizeof(float);
			mtap->framesize = mtap->channels * sizeof(float);
			mtap->msf = sxe_msf_FLT;
			break;
		case AV_SAMPLE_FMT_NONE:
		default:
			mtap->samplewidth = 0;
			break;
		}
	}
	mtap->endianness = 0;

	/* now assign */
	media_substream_type_properties(mss).aprops = mtap;
	media_substream_data(mss) = &st;
}

int sy_do_nothing(int);
int
sy_do_nothing(int a)
{
	return 0;
}
/* main analysis function */
static ms_driver_data_t
media_ffmpeg_open(Lisp_Media_Stream *ms)
{
	/* stream stuff */
	media_substream *mss;
	/* libavformat stuff */
	AVFormatContext *avfc = NULL;
	AVStream *avst = NULL;
	AVCodecContext *avcc = NULL;
	AVCodec *avc = NULL;

	/* initialise */
	av_register_all();

	switch (media_stream_kind(ms)) {
	case MKIND_FILE: {
		mkind_file_properties *mkfp = NULL;
		const char *file;
		int file_len = 0;

		/* this is complete fucking bullshit */
		sy_do_nothing(file_len);

		/* open the file */
		mkfp = media_stream_kind_properties(ms).fprops;
		TO_EXTERNAL_FORMAT(LISP_STRING, mkfp->filename,
				   ALLOCA, (file, file_len), Qnil);
		avfc = media_ffmpeg_open_file(file);
		if (!avfc) {
			media_stream_set_meths(ms, NULL);
			media_stream_driver(ms) = MDRIVER_UNKNOWN;
			return NULL;
		}

		/* store the filesize */
		/* mkfp->filesize = avfc->file_size; */
		break;
	}
	case MKIND_STRING: {
		mkind_string_properties *mksp = NULL;
		char *data;
		uint32_t size;

		/* open the file */
		mksp = media_stream_kind_properties(ms).sprops;
		data = mksp->stream_data;
		size = mksp->size;
		avfc = media_ffmpeg_open_data(data, size);

		if (!avfc) {
			media_stream_set_meths(ms, NULL);
			media_stream_driver(ms) = MDRIVER_UNKNOWN;
			return NULL;
		}
		break;
	}
	case MKIND_UNKNOWN:
	case MKIND_FIFO:
	case MKIND_STREAM:
	case NUMBER_OF_MEDIA_KINDS:
	default:
		break;
	}

	if (avfc)
		/* check if there is at least one usable stream */
		for (size_t st = 0; st < avfc->nb_streams; st++) {
			avst = avfc->streams[st];
			avcc = avst->codec;
			if (avcc &&
			    avcc->codec_id != CODEC_ID_NONE &&
			    avcc->codec_type != AVMEDIA_TYPE_DATA &&
			    (avc = avcodec_find_decoder(avcc->codec_id)) &&
			    (avc && (avcodec_open2(avcc, avc, NULL) >= 0))) {

				/* create a substream */
				mss = make_media_substream_append(ms);

				switch ((unsigned int)avcc->codec_type) {
				case AVMEDIA_TYPE_AUDIO:
					/* assign substream props */
					media_substream_type(mss) = MTYPE_AUDIO;
					media_ffmpeg_analyse_audio(mss, avfc, st);
					/* set some stream handlers */
					media_stream_set_meths(ms, media_ffmpeg);
					break;
				case AVMEDIA_TYPE_DATA:
					media_substream_type(mss) = MTYPE_IMAGE;
					break;
				default:
					media_substream_type(mss) = MTYPE_UNKNOWN;
					break;
				}
			}
		}

	/* keep the format context */
	media_stream_data(ms) = avfc;

	/* set me as driver indicator */
	media_stream_driver(ms) = MYSELF;

	return avfc;
}

static inline void
handle_packet(AVFormatContext *avfc, AVPacket *pkt)
{
#if 0
       AVFrame picture;
       AVStream *st;
       int ret, got_picture;

       st = avfc->streams[pkt->stream_index];

       /* XXX: allocate picture correctly */
       avcodec_get_frame_defaults(&picture);

       ret = avcodec_decode_video(
               st->codec, &picture, &got_picture, pkt->data, pkt->size);

       if (!got_picture) {
               /* no picture yet */
               goto discard_packet;
       }
#endif

       FFMPEG_DEBUG_AVF("got video frame\n");

#if 0                          /* not yet */
discard_packet:
#endif
       av_free_packet(pkt);
       return;
}




static size_t
media_ffmpeg_read(media_substream *mss, void *outbuf, size_t length)
{
/* read at most `length' frames into `outbuf' */
/* convert them to internal format */
	/* stream stuff */
	Lisp_Media_Stream *ms = mss->up;
	mtype_audio_properties *mtap;
	media_sample_format_t *fmt;
	/* libavformat */
	AVFormatContext *avfc;
	AVStream *avst;
	AVCodecContext *avcc;
	/* AVCodec *avc; */
	AVPacket pkt;
	/* buffering */
	/* the size we present here, is _not_ the size we want, however
	 * ffmpeg is quite pedantic about the buffer size,
	 * we just pass the least possible value here to please him.
         * Roughly enough for 1 second of 48khz 32bit audio */
	int size = 192000;
	uint16_t framesize;
	/* result */
	long int bufseek = 0, si = -1;
	int declen, dec, rf_status = 0;

	/* check the integrity of the media stream */
	if (media_stream_driver(ms) != MYSELF)
		return 0;

	/* fetch the format context */
	avfc = media_stream_data(ms);

	if (!avfc)
		return 0;

	si = (long int)mss->substream_data;
	avst = avfc->streams[si];
	avcc = avst->codec;
	/* avc = avcc->codec; */

	/* unpack the substream */
	if ((mtap = media_substream_type_properties(mss).aprops) == NULL) {
		FFMPEG_DEBUG_AVS("stream information missing. Uh Oh.\n");
		return 0;
	}

	/* fetch audio info */
	framesize = mtap->framesize;
	fmt = mtap->msf;

	/* initialise the packet */
	pkt.pts = pkt.dts = pkt.size = 0;
	FFMPEG_DEBUG_AVF("initialised packet: "
			 "pts:%lld dts:%lld psz:%d\n",
			 (long long int)pkt.pts,
			 (long long int)pkt.dts,
			 pkt.size);

	/* read a frame and decode it */
	while ((size_t)bufseek <= length*framesize &&
	       (rf_status = av_read_frame(avfc, &pkt)) >= 0) {
		if (pkt.stream_index != si) {
			FFMPEG_DEBUG_AVF("SKIP reason: "
					 "sought after stream %ld, got %ld\n",
					 (long int)si,
					 (long int)pkt.stream_index);
			handle_packet(avfc, &pkt);
			continue;
		}

		FFMPEG_DEBUG_AVF("read frame: "
				 "pts:%lld dts:%lld psz:%d\n",
				 (long long int)pkt.pts,
				 (long long int)pkt.dts,
				 pkt.size);

		dec = pkt.size;
		/* decode the demuxed packet */
		int got_frame = 0;

		sy_do_nothing(declen);

		declen = avcodec_decode_audio4(
			avcc, (void*)((char*)outbuf+bufseek),
			&got_frame, &pkt);

		if (dec > 0 && size > 0) {
			FFMPEG_DEBUG_AVF("pts:%lld dts:%lld psz:%d s:%d d:%d\n",
					 (long long int)pkt.pts,
					 (long long int)pkt.dts,
					 pkt.size, size, declen);

			/* memcpy(outbuf+bufseek, (char*)buffer, size); */
			bufseek += size;
		}

		FFMPEG_DEBUG_AVF("packet state: "
				 "pts:%lld dts:%lld psz:%d\n",
				 (long long int)pkt.pts,
				 (long long int)pkt.dts,
				 (int)pkt.size);
		av_free_packet(&pkt);
	}
	av_free_packet(&pkt);

	FFMPEG_DEBUG_AVF("finished reading, bufseek=%ld, rf_status=%ld\n",
			 (long int)bufseek, (long int)rf_status);

	/* convert the pig */
	size = bufseek/framesize;
	MEDIA_SAMPLE_FORMAT_UPSAMPLE(fmt)(outbuf, outbuf, size*mtap->channels);

	/* shutdown */
	return size;
}

static void
media_ffmpeg_rewind(media_substream *mss)
{
/* rewind the stream to the first frame */
	/* libavformat */
	AVFormatContext *avfc;
	AVStream *avst;
	Lisp_Media_Stream *ms = mss->up;
	int64_t start_time;
	int res = 0;
	long int si;

	/* check the integrity of the media stream */
	if (media_stream_driver(ms) != MYSELF)
		return;

	FFMPEG_DEBUG_AVF("rewind substream 0x%lx\n",
			 (long unsigned int)mss);

	/* fetch the format context */
	if (!(avfc = media_stream_data(ms)))
		return;

	si = (long int)mss->substream_data;
	avst = avfc->streams[si];
	if ((start_time = avst->start_time) < 0) {
		start_time = 0;
	}

	FFMPEG_DEBUG_AVF("rewind (idx:%ld) to %lld\n",
			 si, (long long int)start_time);

	/* ... and reset the stream */
	res = av_seek_frame(avfc, si, AV_NOPTS_VALUE, AVSEEK_FLAG_BACKWARD);

	if (res >= 0) {
		FFMPEG_DEBUG_AVF("rewind succeeded\n");
		return;
	} else {
		FFMPEG_DEBUG_AVF("rewind exitted with %d\n", res);
		return;
	}
}


typedef struct PacketQueue {
	AVPacketList *first_pkt, *last_pkt;
	int nb_packets;
	int size;
	int abort_request;
	struct sxe_semaphore_s sem;
} PacketQueue;

AVPacket flush_pkt;

/* packet queue handling */
#if 0
static void
packet_queue_init(PacketQueue *q)
{
	memset(q, 0, sizeof(PacketQueue));
	SXE_SEMAPH_INIT(&q->sem);
}

static void
packet_queue_flush(PacketQueue *q)
{
	AVPacketList *pkt, *pkt1;

	SXE_SEMAPH_LOCK(&q->sem);
	for (pkt = q->first_pkt; pkt != NULL; pkt = pkt1) {
		pkt1 = pkt->next;
		av_free_packet(&pkt->pkt);
		av_freep(&pkt);
	}
	q->last_pkt = NULL;
	q->first_pkt = NULL;
	q->nb_packets = 0;
	q->size = 0;
	SXE_SEMAPH_UNLOCK(&q->sem);
}

static void
packet_queue_end(PacketQueue *q)
{
	packet_queue_flush(q);
	SXE_SEMAPH_FINI(&q->sem);
}

static int
packet_queue_put(PacketQueue *q, AVPacket *pkt)
{
	AVPacketList *pkt1;

	/* duplicate the packet */
	if (pkt!=&flush_pkt && av_dup_packet(pkt) < 0)
		return -1;

	pkt1 = av_malloc(sizeof(AVPacketList));
	if (!pkt1)
		return -1;
	pkt1->pkt = *pkt;
	pkt1->next = NULL;


	SXE_SEMAPH_LOCK(&q->sem);

	if (!q->last_pkt)

		q->first_pkt = pkt1;
	else
		q->last_pkt->next = pkt1;
	q->last_pkt = pkt1;
	q->nb_packets++;
	q->size += pkt1->pkt.size;
	/* XXX: should duplicate packet data in DV case */
	SXE_SEMAPH_SIGNAL(&q->sem);
	SXE_SEMAPH_UNLOCK(&q->sem);
	return 0;
}

static void
packet_queue_abort(PacketQueue *q)
{
	SXE_SEMAPH_LOCK(&q->sem);

	q->abort_request = 1;

	SXE_SEMAPH_SIGNAL(&q->sem);
	SXE_SEMAPH_UNLOCK(&q->sem);
}
#endif
/* return < 0 if aborted, 0 if no packet and > 0 if packet.  */
static int
packet_queue_get(PacketQueue *q, AVPacket *pkt, int block)
	__attribute__((unused));

static int
packet_queue_get(PacketQueue *q, AVPacket *pkt, int block)
{
	AVPacketList *pkt1;
	int ret;

	SXE_SEMAPH_LOCK(&q->sem);

	for(;;) {
		if (q->abort_request) {
			ret = -1;
			break;
		}

		pkt1 = q->first_pkt;
		if (pkt1) {
			q->first_pkt = pkt1->next;
			if (!q->first_pkt)
				q->last_pkt = NULL;
			q->nb_packets--;
			q->size -= pkt1->pkt.size;
			*pkt = pkt1->pkt;
			av_free(pkt1);
			ret = 1;
			break;
		} else if (!block) {
			ret = 0;
			break;
		} else {
			SXE_SEMAPH_WAIT(&q->sem);
		}
	}
	SXE_SEMAPH_UNLOCK(&q->sem);
	return ret;
}

#if 0
static void
dump_stream_info(const AVFormatContext *s)
{
	AVDictionaryEntry *tag = NULL;

	while ((tag = av_dict_get(s->metadata, "", tag,
				  AV_DICT_IGNORE_SUFFIX))) {
		fprintf(stderr, "%s: %s\n", tag->key, tag->value);
	}
}
#endif
enum {
	AV_SYNC_AUDIO_MASTER, /* default choice */
	AV_SYNC_VIDEO_MASTER,
	AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
};



Lisp_Object
media_ffmpeg_available_formats(void)
{
	Lisp_Object formats;
	AVInputFormat *avif = NULL;

	formats = Qnil;

	av_register_all();
	avif = av_iformat_next(avif);

	while (avif) {
		if (avif->name) {
			const Lisp_Object fmtname =
				Fintern(build_string(avif->name), Qnil);
			formats = Fcons(fmtname, formats);
		}
		avif = avif->next;
	}

	return formats;
}

#undef MYSELF
