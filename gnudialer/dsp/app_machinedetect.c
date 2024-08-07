/*
 * Asterisk -- A telephony toolkit for Linux.
 *
 * Copyright (C) 1999-2004, Digium, Inc.
 *
 * Mark Spencer <markster@digium.com>
 *
 * MachineDetect Applicaiton by Ben Hencke <brainstar at gmail dot com>
 * Version 1.01 2005-02-11 - attempts to work correctly when blocking on ast_waitfor()
 *							for VOIP channels that remove silence.
 *
 * Largely based on WaitForSilence by:
 *
 * WaitForSilence Application by David C. Troy <dave@popvox.com>
 * Version 1.00 2004-01-29
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License
 */

#include <asterisk/file.h>
#include <asterisk/logger.h>
#include <asterisk/channel.h>
#include <asterisk/pbx.h>
#include <asterisk/dsp.h>
#include <asterisk/module.h>
#include <asterisk/options.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

static char *tdesc = "Machine Detect";
static char *app = "MachineDetect";
static char *synopsis = "Detects answering machines and waits for silence";
static char *descrip =
"MachineDetect(x|y|z) Waits for answering machine greetings to finish and sets a flag.\n"
"  Waits for 'x' milliseconds of silence, 'y' times.\n"
"  ${MACHINE} will be set to 1 if 'z' miliseconds of total (non-continuous)\n"
"  audio noise is detected.\n"
"  It will not clear the flag, so it is possible to run through several\n"
"  calls to the application with different detection settings in sequence\n"
"Examples:\n"
"  - MachineDetect(700,2,2200) will detect most machines. (default)\n"
"  - MachineDetect(500,3,2200) will get through a \"hello, hello, hello\"\n";

STANDARD_LOCAL_USER;

LOCAL_USER_DECL;

static int do_waiting(struct ast_channel *chan, int duration, int *totalnonsilence) {

	struct ast_frame *f;
	int totalsilence = 0;
	int dspsilence = 0;
	int gotsilence = 0;
	static int silencethreshold = 40;
	int rfmt = 0;
	int res = 0;
	struct ast_dsp *sildet;	 /* silence detector dsp */

	struct timeval starttime,endtime;
	int toduration, newduration, diff;

	rfmt = chan->readformat; /* Set to linear mode */
	res = ast_set_read_format(chan, AST_FORMAT_SLINEAR);
	if (res < 0) {
		ast_log(LOG_WARNING, "Unable to set to linear mode, giving up\n");
		return -1;
	}

	sildet = ast_dsp_new(); /* Create the silence detector */
	if (!sildet) {
		ast_log(LOG_WARNING, "Unable to create silence detector :(\n");
		return -1;
	}
	ast_dsp_set_threshold(sildet, silencethreshold);


	//wait for the channel up to duration miliseconds
	// Await silence...
	f = NULL;
	toduration = duration;
	for(;;) {

		//if we dont get the channel, then we can assume it is silence strpped and just blocks on the read
		gettimeofday(&starttime,0);
		res = ast_waitfor(chan, toduration);
		//see if it timed out (ie we got all the silence we needed)
		if (res <= 0) {
			ast_verbose(VERBOSE_PREFIX_3 "Bailing out, timeout on ast_waitfor(chan, %d). Must have got the silence we needed\n", duration);
			gotsilence = 1;
			break;
		}

		//if we didnt time out, and chan is avail
		//then calc diff silence to check for and run the dsp
		gettimeofday(&endtime,0);

		//calc diff in ms
		endtime.tv_usec -= starttime.tv_usec;
		endtime.tv_sec -= starttime.tv_sec;
		if (endtime.tv_usec < 0) {
			endtime.tv_usec += 1000000;
			endtime.tv_sec--;
		}
		diff = (endtime.tv_sec*1000 + endtime.tv_usec/1000);
		newduration = duration - diff;

		//read this packet
		f = ast_read(chan);
		if (!f)
			break;
		if (f->frametype == AST_FRAME_VOICE ) { //&& f->subclass == AST_FORMAT_SLINEAR) { //FIXME is this needed?
			dspsilence = 0;
			ast_dsp_silence(sildet, f, &dspsilence);
			if (dspsilence) {
				//dsp silence is accumulated silence
				totalsilence = dspsilence;
				//reduce the next timeout to account for the transmitted silence
				toduration -= dspsilence;
			} else {
				//reset, and wait for more silence
				totalsilence = 0;
				toduration = duration;
				*totalnonsilence += f->samples/8; //samples is in 8000hz. a 160 sample = 20ms
			}

			//ast_verbose(VERBOSE_PREFIX_3 "dspsilence: %d, newduration: %d\n cur frame: %d", dspsilence, newduration, f->samples);

			if (totalsilence >= newduration) {
				ast_verbose(VERBOSE_PREFIX_3 "exiting with %d silence > %d required\n", totalsilence, newduration);
				// Ended happily with silence
				ast_frfree(f);
				gotsilence = 1;
				break;
			}

		}
		ast_frfree(f);
	}
	ast_dsp_free(sildet);
	return gotsilence;
}

static int machinedetect_exec(struct ast_channel *chan, void *data)
{
	int res=1;
	struct localuser *u;
	int duration = 700;
	int iterations = 2, i;
	int totalnonsilence = 0;
	int machinetime = 2200;

	res = ast_answer(chan); // Answer the channel

	if (data) {

		if (sscanf(data, "%d|%d|%d", &duration, &iterations, &machinetime) != 3) {
			ast_log(LOG_WARNING, "Using default value of 2200ms for machine threshold\n");
		} else if (sscanf(data, "%d|%d", &duration, &iterations) != 2) {
			ast_log(LOG_WARNING, "Using default value of 2 iterations\n");
		} else if (sscanf(data, "%d", &duration) != 1) {
			ast_log(LOG_WARNING, "Using default silence value of 700ms\n");
		}
	}

	ast_verbose(VERBOSE_PREFIX_3 "Waiting %d time(s) for %d ms silence. Machine threshhold is %d\n", iterations, duration, machinetime);
	LOCAL_USER_ADD(u);
	res = 1;
	for ( i=0; (i<iterations) && (res == 1); i++) {
		res = do_waiting(chan, duration, &totalnonsilence);
	}

	if (totalnonsilence >= machinetime) {
		ast_verbose(VERBOSE_PREFIX_3 "Detected answering machine. Threshhold is %d and got %d\n",machinetime, totalnonsilence);
		pbx_builtin_setvar_helper (chan, "MACHINE", "1");
	} else {
		ast_verbose(VERBOSE_PREFIX_3 "Detected person. Threshhold is %d and got %d\n",machinetime, totalnonsilence);
	}

	LOCAL_USER_REMOVE(u);
	if (res > 0)
		res = 0;
	return res;
}

int unload_module(void)
{
	STANDARD_HANGUP_LOCALUSERS;
	return ast_unregister_application(app);
}

int load_module(void)
{
	return ast_register_application(app, machinedetect_exec, synopsis, descrip);
}

char *description(void)
{
	return tdesc;
}

int usecount(void)
{
	int res;
	STANDARD_USECOUNT(res);
	return res;
}

char *key()
{
	return ASTERISK_GPL_KEY;
}



