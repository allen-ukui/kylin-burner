/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * Libburner-media
 * Copyright (C) Philippe Rouquier 2005-2009 <bonfire-app@wanadoo.fr>
 *
 * Libburner-media is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The Libburner-media authors hereby grant permission for non-GPL compatible
 * GStreamer plugins to be used and distributed together with GStreamer
 * and Libburner-media. This permission is above and beyond the permissions granted
 * by the GPL license by which Libburner-media is covered. If you modify this code
 * you may extend this exception to your version of the code, but you are not
 * obligated to do so. If you do not wish to do so, delete this exception
 * statement from your version.
 * 
 * Libburner-media is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <glib.h>

#include "scsi-mmc1.h"

#include "scsi-error.h"
#include "scsi-utils.h"
#include "scsi-base.h"
#include "scsi-command.h"
#include "scsi-opcodes.h"
#include "scsi-read-cd.h"

#if G_BYTE_ORDER == G_LITTLE_ENDIAN

struct _BurnerReadCDCDB {
	uchar opcode;

	uchar rel_add		:1;
	uchar reserved1		:1;
	uchar sec_type		:3;
	uchar reserved0		:3;

	uchar start_lba		[4];
	uchar len		[3];

	uchar reserved2		:1;
	uchar error		:2;
	uchar edc		:1;
	uchar user_data		:1;
	uchar header		:2;
	uchar sync		:1;

	uchar subchannel	:3;
	uchar reserved3		:5;

	uchar ctl;
};

#else

struct _BurnerReadCDCDB {
	uchar opcode;

	uchar reserved0		:3;
	uchar sec_type		:3;
	uchar reserved1		:1;
	uchar rel_add		:1;

	uchar start_lba		[4];
	uchar len		[3];

	uchar sync		:1;
	uchar header		:2;
	uchar user_data		:1;
	uchar edc		:1;
	uchar error		:2;
	uchar reserved2		:1;

	uchar reserved3		:5;
	uchar subchannel	:3;

	uchar ctl;
};

#endif

typedef struct _BurnerReadCDCDB BurnerReadCDCDB;

BURNER_SCSI_COMMAND_DEFINE (BurnerReadCDCDB,
			     READ_CD,
			     BURNER_SCSI_READ);

BurnerScsiResult
burner_mmc1_read_block (BurnerDeviceHandle *handle,
			 gboolean user_data,
			 BurnerScsiBlockType type,
			 BurnerScsiBlockHeader header,
			 BurnerScsiBlockSubChannel channel,
			 int start,
			 int size,
			 unsigned char *buffer,
			 int buffer_len,
			 BurnerScsiErrCode *error)
{
	BurnerReadCDCDB *cdb;
	BurnerScsiResult res;

	g_return_val_if_fail (handle != NULL, BURNER_SCSI_FAILURE);

	cdb = burner_scsi_command_new (&info, handle);
	BURNER_SET_32 (cdb->start_lba, start);

	/* NOTE: if we just want to test if block is readable len can be 0 */
	BURNER_SET_24 (cdb->len, size);

	/* reladr should be O */
	/* no sync field included */
	cdb->sync = 0;

	/* sector type */
	cdb->sec_type = type;

	/* header ?*/
	cdb->header = header;

	/* returns user data ?*/
	cdb->user_data = user_data;

	/* no EDC */
	/* no error/C2 error */

	/* subchannel */
	cdb->subchannel = channel;

	if (buffer)
		memset (buffer, 0, buffer_len);

	res = burner_scsi_command_issue_sync (cdb,
					       buffer,
					       buffer_len,
					       error);
	burner_scsi_command_free (cdb);
	return res;
}
