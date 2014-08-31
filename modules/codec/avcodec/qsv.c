/*****************************************************************************
 * qsv.c: Video Acceleration helpers
 *****************************************************************************
 * Copyright (C) 2009 Geoffroy Couprie
 * Copyright (C) 2009 Laurent Aimar
 * $Id$
 *
 * Authors: Geoffroy Couprie <geal@videolan.org>
 *          Laurent Aimar <fenrir _AT_ videolan _DOT_ org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_picture.h>
#include <vlc_fourcc.h>
#include <vlc_cpu.h>
#include <vlc_plugin.h>

#include <libavcodec/avcodec.h>
#include <libavcodec/qsv.h>

#include "avcodec.h"
#include "va.h"

static int Open(vlc_va_t *, AVCodecContext *, const es_format_t *);
static void Close(vlc_va_t *);

vlc_module_begin()
    set_description(N_("QuickSync Video Acceleration (QSV)"))
    set_capability("hw decoder", 0)
    set_category(CAT_INPUT)
    set_subcategory(SUBCAT_INPUT_VCODEC)
    set_callbacks(Open, Close)
vlc_module_end()

struct vlc_va_sys_t
{
    // to free the hwaccel
    AVCodecContext      *avctx;
};

typedef struct vlc_va_sys_t vlc_va_qsv_t;

static void Close(vlc_va_t *external)
{
    vlc_va_qsv_t *p_va = vlc_va_qsv_Get(external);

    av_qsv_default_free(p_va->avctx);
}

static int Open(vlc_va_t *external, AVCodecContext *avctx,
                const es_format_t *fmt)
{
    msg_Dbg( external, "QSV decoder Open");

    vlc_va_qsv_t *p_va = calloc( 1, sizeof(*p_va) );

    if (!p_va) {
        return VLC_EGENERIC;
    }

    p_va->avctx = avctx;

    if (av_qsv_default_init(avctx) < 0) {
        return VLC_EGENERIC;
    }

    return VLC_SUCCESS;
}
