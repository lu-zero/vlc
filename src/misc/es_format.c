/*****************************************************************************
 * es_format.c : es_format_t helpers.
 *****************************************************************************
 * Copyright (C) 2008 VLC authors and VideoLAN
 * $Id$
 *
 * Author: Laurent Aimar <fenrir@videolan.org>
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

/*****************************************************************************
 * Preamble
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_es.h>


/*****************************************************************************
 * BinaryLog: computes the base 2 log of a binary value
 *****************************************************************************
 * This functions is used by MaskToShift, to get a bit index from a binary
 * value.
 *****************************************************************************/
static int BinaryLog( uint32_t i )
{
    int i_log = 0;

    if( i == 0 ) return -31337;

    if( i & 0xffff0000 ) i_log += 16;
    if( i & 0xff00ff00 ) i_log += 8;
    if( i & 0xf0f0f0f0 ) i_log += 4;
    if( i & 0xcccccccc ) i_log += 2;
    if( i & 0xaaaaaaaa ) i_log += 1;

    return i_log;
}

/**
 * It transforms a color mask into right and left shifts
 * FIXME copied from video_output.c
 */
static void MaskToShift( int *pi_left, int *pi_right, uint32_t i_mask )
{
    uint32_t i_low, i_high;            /* lower and higher bits of the mask */

    if( !i_mask )
    {
        *pi_left = *pi_right = 0;
        return;
    }

    /* Get bits */
    i_low = i_high = i_mask;

    i_low &= - (int32_t)i_low;          /* lower bit of the mask */
    i_high += i_low;                    /* higher bit of the mask */

    /* Transform bits into an index. Also deal with i_high overflow, which
     * is faster than changing the BinaryLog code to handle 64 bit integers. */
    i_low =  BinaryLog (i_low);
    i_high = i_high ? BinaryLog (i_high) : 32;

    /* Update pointers and return */
    *pi_left =   i_low;
    *pi_right = (8 - i_high + i_low);
}

/* */
void video_format_FixRgb( video_format_t *p_fmt )
{
    /* FIXME find right default mask */
    if( !p_fmt->i_rmask || !p_fmt->i_gmask || !p_fmt->i_bmask )
    {
        switch( p_fmt->i_chroma )
        {
        case VLC_CODEC_RGB15:
            p_fmt->i_rmask = 0x7c00;
            p_fmt->i_gmask = 0x03e0;
            p_fmt->i_bmask = 0x001f;
            break;

        case VLC_CODEC_RGB16:
            p_fmt->i_rmask = 0xf800;
            p_fmt->i_gmask = 0x07e0;
            p_fmt->i_bmask = 0x001f;
            break;

        case VLC_CODEC_RGB24:
            p_fmt->i_rmask = 0xff0000;
            p_fmt->i_gmask = 0x00ff00;
            p_fmt->i_bmask = 0x0000ff;
            break;
        case VLC_CODEC_RGB32:
            p_fmt->i_rmask = 0x00ff0000;
            p_fmt->i_gmask = 0x0000ff00;
            p_fmt->i_bmask = 0x000000ff;
            break;

        default:
            return;
        }
    }

    MaskToShift( &p_fmt->i_lrshift, &p_fmt->i_rrshift,
                 p_fmt->i_rmask );
    MaskToShift( &p_fmt->i_lgshift, &p_fmt->i_rgshift,
                 p_fmt->i_gmask );
    MaskToShift( &p_fmt->i_lbshift, &p_fmt->i_rbshift,
                 p_fmt->i_bmask );
}

void video_format_Setup( video_format_t *p_fmt, vlc_fourcc_t i_chroma,
                         int i_width, int i_height,
                         int i_visible_width, int i_visible_height,
                         int i_sar_num, int i_sar_den )
{
    p_fmt->i_chroma         = vlc_fourcc_GetCodec( VIDEO_ES, i_chroma );
    p_fmt->i_width          = i_width;
    p_fmt->i_visible_width  = i_visible_width;
    p_fmt->i_height         = i_height;
    p_fmt->i_visible_height = i_visible_height;
    p_fmt->i_x_offset       =
    p_fmt->i_y_offset       = 0;
    vlc_ureduce( &p_fmt->i_sar_num, &p_fmt->i_sar_den,
                 i_sar_num, i_sar_den, 0 );

    switch( p_fmt->i_chroma )
    {
    case VLC_CODEC_YUVA:
        p_fmt->i_bits_per_pixel = 32;
        break;
    case VLC_CODEC_YUV420A:
        p_fmt->i_bits_per_pixel = 20;
        break;
    case VLC_CODEC_YUV422A:
        p_fmt->i_bits_per_pixel = 24;
        break;
    case VLC_CODEC_I444:
    case VLC_CODEC_J444:
        p_fmt->i_bits_per_pixel = 24;
        break;
    case VLC_CODEC_I422:
    case VLC_CODEC_YUYV:
    case VLC_CODEC_YVYU:
    case VLC_CODEC_UYVY:
    case VLC_CODEC_VYUY:
    case VLC_CODEC_J422:
        p_fmt->i_bits_per_pixel = 16;
        break;
    case VLC_CODEC_I440:
    case VLC_CODEC_J440:
        p_fmt->i_bits_per_pixel = 16;
        break;
    case VLC_CODEC_I411:
    case VLC_CODEC_YV12:
    case VLC_CODEC_I420:
    case VLC_CODEC_J420:
        p_fmt->i_bits_per_pixel = 12;
        break;
    case VLC_CODEC_YV9:
    case VLC_CODEC_I410:
        p_fmt->i_bits_per_pixel = 9;
        break;
    case VLC_CODEC_Y211:
        p_fmt->i_bits_per_pixel = 8;
        break;
    case VLC_CODEC_YUVP:
        p_fmt->i_bits_per_pixel = 8;
        break;

    case VLC_CODEC_RGB32:
    case VLC_CODEC_RGBA:
    case VLC_CODEC_ARGB:
        p_fmt->i_bits_per_pixel = 32;
        break;
    case VLC_CODEC_RGB24:
        p_fmt->i_bits_per_pixel = 24;
        break;
    case VLC_CODEC_RGB15:
    case VLC_CODEC_RGB16:
        p_fmt->i_bits_per_pixel = 16;
        break;
    case VLC_CODEC_RGB8:
        p_fmt->i_bits_per_pixel = 8;
        break;

    case VLC_CODEC_GREY:
    case VLC_CODEC_RGBP:
        p_fmt->i_bits_per_pixel = 8;
        break;

    case VLC_CODEC_XYZ12:
        p_fmt->i_bits_per_pixel = 48;
        break;

    default:
        p_fmt->i_bits_per_pixel = 0;
        break;
    }
}

void video_format_CopyCrop( video_format_t *p_dst, const video_format_t *p_src )
{
    p_dst->i_x_offset       = p_src->i_x_offset;
    p_dst->i_y_offset       = p_src->i_y_offset;
    p_dst->i_visible_width  = p_src->i_visible_width;
    p_dst->i_visible_height = p_src->i_visible_height;
}

void video_format_ScaleCropAr( video_format_t *p_dst, const video_format_t *p_src )
{
    p_dst->i_x_offset       = (uint64_t)p_src->i_x_offset       * p_dst->i_width  / p_src->i_width;
    p_dst->i_y_offset       = (uint64_t)p_src->i_y_offset       * p_dst->i_height / p_src->i_height;
    p_dst->i_visible_width  = (uint64_t)p_src->i_visible_width  * p_dst->i_width  / p_src->i_width;
    p_dst->i_visible_height = (uint64_t)p_src->i_visible_height * p_dst->i_height / p_src->i_height;

    p_dst->i_sar_num *= p_src->i_width;
    p_dst->i_sar_den *= p_dst->i_width;
    vlc_ureduce(&p_dst->i_sar_num, &p_dst->i_sar_den,
                p_dst->i_sar_num, p_dst->i_sar_den, 65536);

    p_dst->i_sar_num *= p_dst->i_height;
    p_dst->i_sar_den *= p_src->i_height;
    vlc_ureduce(&p_dst->i_sar_num, &p_dst->i_sar_den,
                p_dst->i_sar_num, p_dst->i_sar_den, 65536);
}

//Simplify transforms to have something more managable. Order: angle, hflip.
void transform_GetBasicOps( video_transform_t transform, int *angle, int *hflip ) {

    *angle = 0;
    *hflip = 0;

    switch ( transform ) {

        case TRANSFORM_R90:
            *angle = 90;
            break;
        case TRANSFORM_R180:
            *angle = 180;
            break;
        case TRANSFORM_R270:
            *angle = 270;
            break;
        case TRANSFORM_HFLIP:
            *hflip = 1;
            break;
        case TRANSFORM_VFLIP:
            *angle = 180;
            *hflip = 1;
            break;
        case TRANSFORM_TRANSPOSE:
            *angle = 90;
            *hflip = 1;
            break;
        case TRANSFORM_ANTI_TRANSPOSE:
            *angle = 270;
            *hflip = 1;
            break;
    }
}

video_transform_t transform_FromBasicOps( int angle, int hflip )
{
    switch ( angle ) {
        case 0:
            return hflip ? TRANSFORM_HFLIP : TRANSFORM_IDENTIY;
        case 90:
            return hflip ? TRANSFORM_TRANSPOSE : TRANSFORM_R90;
        case 180:
            return hflip ? TRANSFORM_VFLIP : TRANSFORM_R180;
        case 270:
            return hflip ? TRANSFORM_ANTI_TRANSPOSE : TRANSFORM_R270;
        default:
            return TRANSFORM_IDENTIY;
    }
}

video_transform_t video_format_GetTransform( video_orientation_t src, video_orientation_t dst )
{
    int angle1 = 0;
    int hflip1 = 0;

    transform_GetBasicOps(  (video_transform_t)src, &angle1, &hflip1 );

    int angle2 = 0;
    int hflip2 = 0;

    transform_GetBasicOps( transform_Inverse( (video_transform_t)dst ), &angle2, &hflip2 );

    int angle = (angle1 + angle2) % 360;
    int hflip = (hflip1 + hflip2) % 2;

    return transform_FromBasicOps(angle, hflip);
}

void video_format_TransformTo( video_format_t *fmt, video_orientation_t dst_orientation )
{
    video_transform_t transform = video_format_GetTransform(fmt->orientation, dst_orientation);

    video_format_TransformBy(fmt, transform);
}

void video_format_TransformBy( video_format_t *fmt, video_transform_t transform )
{
    /* Get destination orientation */

    int angle1 = 0;
    int hflip1 = 0;

    transform_GetBasicOps( transform, &angle1, &hflip1 );

    int angle2 = 0;
    int hflip2 = 0;

    transform_GetBasicOps( (video_transform_t)( fmt->orientation ), &angle2, &hflip2 );

    int angle = (angle2 - angle1 + 360) % 360;
    int hflip = (hflip2 - hflip1 + 2) % 2;

    video_orientation_t dst_orient = ORIENT_NORMAL;

    if( hflip ) {

        if( angle == 0 )
            dst_orient = ORIENT_HFLIPPED;
        else if( angle == 90 )
            dst_orient = ORIENT_ANTI_TRANSPOSED;
        else if( angle == 180 )
            dst_orient = ORIENT_VFLIPPED;
        else if( angle == 270 )
            dst_orient = ORIENT_TRANSPOSED;
    }
    else {

        if( angle == 90 )
            dst_orient = ORIENT_ROTATED_90;
        else if( angle == 180 )
            dst_orient = ORIENT_ROTATED_180;
        else if( angle == 270 )
            dst_orient = ORIENT_ROTATED_270;
    }

    /* Apply transform */

    video_format_t scratch = *fmt;

    if( ORIENT_IS_SWAP( fmt->orientation ) != ORIENT_IS_SWAP( dst_orient )) {

        fmt->i_width = scratch.i_height;
        fmt->i_visible_width = scratch.i_visible_height;
        fmt->i_height = scratch.i_width;
        fmt->i_visible_height = scratch.i_visible_width;
        fmt->i_sar_num = scratch.i_sar_den;
        fmt->i_sar_den = scratch.i_sar_num;
    }

    unsigned int delta_y = scratch.i_height - scratch.i_visible_height - scratch.i_y_offset;
    unsigned int delta_x = scratch.i_width - scratch.i_visible_width - scratch.i_x_offset;

    switch ( transform )
    {
        case TRANSFORM_R90:
            fmt->i_x_offset = delta_y;
            fmt->i_y_offset = scratch.i_x_offset;
            break;
        case TRANSFORM_R180:
            fmt->i_x_offset = delta_x;
            fmt->i_y_offset = delta_y;
            break;
        case TRANSFORM_R270:
            fmt->i_x_offset = scratch.i_y_offset;
            fmt->i_y_offset = delta_x;
            break;
        case TRANSFORM_HFLIP:
            fmt->i_x_offset = delta_x;
            break;
        case TRANSFORM_VFLIP:
            fmt->i_y_offset = delta_y;
            break;
        case TRANSFORM_TRANSPOSE:
            fmt->i_x_offset = scratch.i_y_offset;
            fmt->i_y_offset = scratch.i_x_offset;
            break;
        case TRANSFORM_ANTI_TRANSPOSE:
            fmt->i_x_offset = delta_y;
            fmt->i_y_offset = delta_x;
            break;
    }

    fmt->orientation = dst_orient;
}

void video_format_ApplyRotation( const video_format_t * restrict in, video_format_t * restrict out )
{
    *out = *in;

    video_format_TransformTo(out, ORIENT_NORMAL);
}

bool video_format_IsSimilar( const video_format_t *p_fmt1, const video_format_t *p_fmt2 )
{
    video_format_t v1 = *p_fmt1;
    video_format_t v2 = *p_fmt2;

    if( v1.i_chroma != v2.i_chroma )
        return false;

    if( v1.i_width != v2.i_width || v1.i_height != v2.i_height ||
        v1.i_visible_width != v2.i_visible_width ||
        v1.i_visible_height != v2.i_visible_height ||
        v1.i_x_offset != v2.i_x_offset || v1.i_y_offset != v2.i_y_offset )
        return false;
    if( v1.i_sar_num * v2.i_sar_den != v2.i_sar_num * v1.i_sar_den )
        return false;

    if( v1.orientation != v2.orientation)
        return false;

    if( v1.i_chroma == VLC_CODEC_RGB15 ||
        v1.i_chroma == VLC_CODEC_RGB16 ||
        v1.i_chroma == VLC_CODEC_RGB24 ||
        v1.i_chroma == VLC_CODEC_RGB32 )
    {
        video_format_FixRgb( &v1 );
        video_format_FixRgb( &v2 );

        if( v1.i_rmask != v2.i_rmask ||
            v1.i_gmask != v2.i_gmask ||
            v1.i_bmask != v2.i_bmask )
            return false;
    }
    return true;
}
void video_format_Print( vlc_object_t *p_this,
                         const char *psz_text, const video_format_t *fmt )
{
    msg_Dbg( p_this,
             "%s sz %ix%i, of (%i,%i), vsz %ix%i, 4cc %4.4s, sar %i:%i, msk r0x%x g0x%x b0x%x",
             psz_text,
             fmt->i_width, fmt->i_height, fmt->i_x_offset, fmt->i_y_offset,
             fmt->i_visible_width, fmt->i_visible_height,
             (char*)&fmt->i_chroma,
             fmt->i_sar_num, fmt->i_sar_den,
             fmt->i_rmask, fmt->i_gmask, fmt->i_bmask );
}

void es_format_Init( es_format_t *fmt,
                     int i_cat, vlc_fourcc_t i_codec )
{
    fmt->i_cat                  = i_cat;
    fmt->i_codec                = i_codec;
    fmt->i_original_fourcc      = 0;
    fmt->i_profile              = -1;
    fmt->i_level                = -1;
    fmt->i_id                   = -1;
    fmt->i_group                = 0;
    fmt->i_priority             = ES_PRIORITY_SELECTABLE_MIN;
    fmt->psz_language           = NULL;
    fmt->psz_description        = NULL;

    fmt->i_extra_languages      = 0;
    fmt->p_extra_languages      = NULL;

    memset( &fmt->audio, 0, sizeof(audio_format_t) );
    memset( &fmt->audio_replay_gain, 0, sizeof(audio_replay_gain_t) );
    memset( &fmt->video, 0, sizeof(video_format_t) );
    memset( &fmt->subs, 0, sizeof(subs_format_t) );

    fmt->b_packetized           = true;
    fmt->i_bitrate              = 0;
    fmt->i_extra                = 0;
    fmt->p_extra                = NULL;
}

void es_format_InitFromVideo( es_format_t *p_es, const video_format_t *p_fmt )
{
    es_format_Init( p_es, VIDEO_ES, p_fmt->i_chroma );
    video_format_Copy( &p_es->video, p_fmt );
}

int es_format_Copy( es_format_t *dst, const es_format_t *src )
{
    int i;
    memcpy( dst, src, sizeof( es_format_t ) );
    dst->psz_language = src->psz_language ? strdup( src->psz_language ) : NULL;
    dst->psz_description = src->psz_description ? strdup( src->psz_description ) : NULL;
    if( src->i_extra > 0 && dst->p_extra )
    {
        dst->i_extra = src->i_extra;
        dst->p_extra = malloc( src->i_extra );
        if(dst->p_extra)
            memcpy( dst->p_extra, src->p_extra, src->i_extra );
        else
            dst->i_extra = 0;
    }
    else
    {
        dst->i_extra = 0;
        dst->p_extra = NULL;
    }

    dst->subs.psz_encoding = dst->subs.psz_encoding ? strdup( src->subs.psz_encoding ) : NULL;

    if( src->video.p_palette )
    {
        dst->video.p_palette =
            (video_palette_t*)malloc( sizeof( video_palette_t ) );
        if(dst->video.p_palette)
        {
            memcpy( dst->video.p_palette, src->video.p_palette,
                sizeof( video_palette_t ) );
        }
    }

    if( dst->i_extra_languages && src->p_extra_languages)
    {
        dst->i_extra_languages = src->i_extra_languages;
        dst->p_extra_languages = (extra_languages_t*)
            malloc(dst->i_extra_languages * sizeof(*dst->p_extra_languages ));
        if( dst->p_extra_languages )
        {
            for( i = 0; i < dst->i_extra_languages; i++ ) {
                if( src->p_extra_languages[i].psz_language )
                    dst->p_extra_languages[i].psz_language = strdup( src->p_extra_languages[i].psz_language );
                else
                    dst->p_extra_languages[i].psz_language = NULL;
                if( src->p_extra_languages[i].psz_description )
                    dst->p_extra_languages[i].psz_description = strdup( src->p_extra_languages[i].psz_description );
                else
                    dst->p_extra_languages[i].psz_description = NULL;
            }
        }
        else
            dst->i_extra_languages = 0;
    }
    else
        dst->i_extra_languages = 0;
    return VLC_SUCCESS;
}

void es_format_Clean( es_format_t *fmt )
{
    free( fmt->psz_language );
    free( fmt->psz_description );

    if( fmt->i_extra > 0 ) free( fmt->p_extra );

    free( fmt->video.p_palette );
    free( fmt->subs.psz_encoding );

    if( fmt->i_extra_languages > 0 && fmt->p_extra_languages )
    {
        int i;
        for( i = 0; i < fmt->i_extra_languages; i++ )
        {
            free( fmt->p_extra_languages[i].psz_language );
            free( fmt->p_extra_languages[i].psz_description );
        }
        free( fmt->p_extra_languages );
    }

    /* es_format_Clean can be called multiple times */
    memset( fmt, 0, sizeof(*fmt) );
}

bool es_format_IsSimilar( const es_format_t *p_fmt1, const es_format_t *p_fmt2 )
{
    if( p_fmt1->i_cat != p_fmt2->i_cat ||
        vlc_fourcc_GetCodec( p_fmt1->i_cat, p_fmt1->i_codec ) !=
        vlc_fourcc_GetCodec( p_fmt2->i_cat, p_fmt2->i_codec ) )
        return false;

    switch( p_fmt1->i_cat )
    {
    case AUDIO_ES:
    {
        audio_format_t a1 = p_fmt1->audio;
        audio_format_t a2 = p_fmt2->audio;

        if( a1.i_format && a2.i_format && a1.i_format != a2.i_format )
            return false;
        if( a1.i_rate != a2.i_rate ||
            a1.i_physical_channels != a2.i_physical_channels ||
            a1.i_original_channels != a2.i_original_channels )
            return false;
        return true;
    }

    case VIDEO_ES:
    {
        video_format_t v1 = p_fmt1->video;
        video_format_t v2 = p_fmt2->video;
        if( !v1.i_chroma )
            v1.i_chroma = vlc_fourcc_GetCodec( p_fmt1->i_cat, p_fmt1->i_codec );
        if( !v2.i_chroma )
            v2.i_chroma = vlc_fourcc_GetCodec( p_fmt2->i_cat, p_fmt2->i_codec );
        return video_format_IsSimilar( &p_fmt1->video, &p_fmt2->video );
    }

    case SPU_ES:
    default:
        return true;
    }
}

