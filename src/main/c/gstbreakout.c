/* GStreamer
 * Copyright (C) 2018 FIXME <fixme@example.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
/**
 * SECTION:element-gstbreakout
 *
 * The breakout element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! breakout ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/video-frame.h>
#include <gst/video/gstvideofilter.h>
#include "gstbreakout.h"

#include <stdio.h>
#include "gstbreakout-marshal.h"

// This is index into the callback array
enum
{
  /* signals */
  SIGNAL_TRANSFORM_IP,

  LAST_SIGNAL
};


GST_DEBUG_CATEGORY_STATIC (gst_breakout_debug_category);
#define GST_CAT_DEFAULT gst_breakout_debug_category

/* prototypes */

static void gst_breakout_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_breakout_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);
static void gst_breakout_dispose (GObject * object);
static void gst_breakout_finalize (GObject * object);

static gboolean gst_breakout_start (GstBaseTransform * trans);
static gboolean gst_breakout_stop (GstBaseTransform * trans);
static gboolean gst_breakout_set_info (GstVideoFilter * filter, GstCaps * incaps,
    GstVideoInfo * in_info, GstCaps * outcaps, GstVideoInfo * out_info);
static GstFlowReturn gst_breakout_transform_frame (GstVideoFilter * filter,
    GstVideoFrame * inframe, GstVideoFrame * outframe);
static GstFlowReturn gst_breakout_transform_frame_ip (GstVideoFilter * filter,
    GstVideoFrame * frame);

static GstFlowReturn gst_breakout_prepare_output_buffer (GstBaseTransform * trans,
                                        GstBuffer *input, GstBuffer **outbuf);

enum
{
  PROP_0
};

/* pad templates */

#define VIDEO_SRC_CAPS \
    GST_VIDEO_CAPS_MAKE(GST_VIDEO_FORMATS_ALL)

#define VIDEO_SINK_CAPS \
    GST_VIDEO_CAPS_MAKE(GST_VIDEO_FORMATS_ALL)

static guint gst_breakout_signals[LAST_SIGNAL] = { 0 };

/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstBreakout, gst_breakout, GST_TYPE_VIDEO_FILTER,
  GST_DEBUG_CATEGORY_INIT (gst_breakout_debug_category, "breakout", 0,
  "debug category for breakout element"));

GstSegment defaultSegment;

static void
gst_breakout_class_init (GstBreakoutClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);
  GstVideoFilterClass *video_filter_class = GST_VIDEO_FILTER_CLASS (klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
      gst_pad_template_new ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
        gst_caps_from_string (VIDEO_SRC_CAPS)));
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
      gst_pad_template_new ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
        gst_caps_from_string (VIDEO_SINK_CAPS)));

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass),
      "FIXME Long name", "Generic", "FIXME Description",
      "FIXME <fixme@example.com>");

  gobject_class->set_property = gst_breakout_set_property;
  gobject_class->get_property = gst_breakout_get_property;
  gobject_class->dispose = gst_breakout_dispose;
  gobject_class->finalize = gst_breakout_finalize;

  base_transform_class->prepare_output_buffer = gst_breakout_prepare_output_buffer;

  base_transform_class->start = GST_DEBUG_FUNCPTR (gst_breakout_start);
//  klass->parent_set_caps = GST_DEBUG_FUNCPTR(base_transform_class->set_caps);
  base_transform_class->stop = GST_DEBUG_FUNCPTR (gst_breakout_stop);
//  base_transform_class->set_caps = GST_DEBUG_FUNCPTR(gst_breakout_set_caps);

  video_filter_class->set_info = GST_DEBUG_FUNCPTR (gst_breakout_set_info);
//  video_filter_class->transform_frame = GST_DEBUG_FUNCPTR (gst_breakout_transform_frame);
  video_filter_class->transform_frame_ip = GST_DEBUG_FUNCPTR (gst_breakout_transform_frame_ip);

  gst_breakout_signals[SIGNAL_TRANSFORM_IP] =
      g_signal_new ("new_sample", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (GstBreakoutClass, new_sample),
      NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, G_TYPE_NONE);

  gst_segment_init (&defaultSegment, GST_FORMAT_TIME);
}

static void
gst_breakout_init (GstBreakout *breakout)
{
  breakout->cur = NULL;
  breakout->caps = NULL;

  GstIterator* it = gst_element_iterate_sink_pads(&(breakout->base_breakout.element.element));
  GstIteratorResult result = GST_ITERATOR_OK;
  GstPad* pad = NULL;
  GValue p = G_VALUE_INIT;
  while (result == GST_ITERATOR_OK && pad == NULL) {
    result = gst_iterator_next(it, &p);
    pad = GST_PAD(p.data[0].v_pointer);
  }
  gst_iterator_free(it);
  if (pad != NULL) {
    breakout->sink = pad;
    gst_object_ref(breakout->sink);
  } else {
    breakout->sink = NULL;
  }

  gst_base_transform_set_in_place(&(breakout->base_breakout.element), TRUE);

  GST_DEBUG_OBJECT (breakout->sink, " PADDY!");
}

void
gst_breakout_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstBreakout *breakout = GST_BREAKOUT (object);

  GST_DEBUG_OBJECT (breakout, "set_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_breakout_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstBreakout *breakout = GST_BREAKOUT (object);

  GST_DEBUG_OBJECT (breakout, "get_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_breakout_dispose (GObject * object)
{
  GstBreakout *breakout = GST_BREAKOUT (object);

  GST_DEBUG_OBJECT (breakout, "dispose");

  /* clean up as possible.  may be called multiple times */
  if (breakout->sink != NULL) {
    gst_object_unref(breakout->sink);
    breakout->sink = NULL;
  }

  G_OBJECT_CLASS (gst_breakout_parent_class)->dispose (object);
}

void
gst_breakout_finalize (GObject * object)
{
  GstBreakout *breakout = GST_BREAKOUT (object);

  GST_DEBUG_OBJECT (breakout, "finalize");

  /* clean up object here */

  G_OBJECT_CLASS (gst_breakout_parent_class)->finalize (object);
}

static gboolean
gst_breakout_start (GstBaseTransform * trans)
{
  GstBreakout *breakout = GST_BREAKOUT (trans);

  GST_DEBUG_OBJECT (breakout, "start");

  return TRUE;
}

static gboolean
gst_breakout_stop (GstBaseTransform * trans)
{
  GstBreakout *breakout = GST_BREAKOUT (trans);

  GST_DEBUG_OBJECT (breakout, "stop");

  return TRUE;
}

static gboolean
gst_breakout_set_info (GstVideoFilter * filter, GstCaps * incaps,
    GstVideoInfo * in_info, GstCaps * outcaps, GstVideoInfo * out_info)
{
  GstBreakout *breakout = GST_BREAKOUT (filter);

  GST_DEBUG_OBJECT (breakout, "set_info");

  return TRUE;
}

/* transform */
static GstFlowReturn
gst_breakout_transform_frame (GstVideoFilter * filter, GstVideoFrame * inframe,
    GstVideoFrame * outframe)
{
  GstBreakout *breakout = GST_BREAKOUT (filter);

  GST_DEBUG_OBJECT (breakout, "transform_frame");

  return GST_FLOW_OK;
}

static GstFlowReturn
gst_breakout_transform_frame_ip (GstVideoFilter * filter, GstVideoFrame * frame)
{
  GstBreakout *breakout = GST_BREAKOUT (filter);
  GST_DEBUG_OBJECT (breakout, "transform_frame_ip");

//  guint8* src = GST_VIDEO_FRAME_PLANE_DATA (frame, 0);
//  printf("width: %d, height: %d\n", GST_VIDEO_FRAME_WIDTH(frame), GST_VIDEO_FRAME_HEIGHT(frame));

  breakout->cur = frame;
  g_signal_emit (breakout, gst_breakout_signals[SIGNAL_TRANSFORM_IP], 0);
  breakout->cur = NULL;

  return GST_FLOW_OK;
}

/**
 * gst_app_sink_pull_sample:
 * @appsink: a #GstAppSink
 *
 * This function blocks until a sample or EOS becomes available or the appsink
 * element is set to the READY/NULL state.
 *
 * This function will only return samples when the appsink is in the PLAYING
 * state. All rendered buffers will be put in a queue so that the application
 * can pull samples at its own rate. Note that when the application does not
 * pull samples fast enough, the queued buffers could consume a lot of memory,
 * especially when dealing with raw video frames.
 *
 * If an EOS event was received before any buffers, this function returns
 * %NULL. Use gst_app_sink_is_eos () to check for the EOS condition.
 *
 * Returns: (transfer full): a #GstSample or NULL when the appsink is stopped or EOS.
 *          Call gst_sample_unref() after usage.
 */

GstSample *
gst_breakout_pull_sample (GstBreakout * breakout)
{
  GstSample *sample = NULL;
  GstBuffer *buffer;
  GstVideoFrame* current;

  current = breakout->cur;
  if (current == NULL)
    goto nothing;
  buffer = current->buffer;
  GST_DEBUG_OBJECT (breakout, "we have a buffer %p with a ref count %d", buffer, (int)buffer->mini_object.refcount);
  GstCaps* caps = breakout->caps;
  gboolean needsUnref = FALSE;
  if (breakout->sink != NULL) {
    caps = gst_pad_get_current_caps(breakout->sink);
    needsUnref = TRUE;
  }
  sample = gst_sample_new (buffer, caps , &defaultSegment, NULL);
  if (needsUnref)
    gst_caps_unref(caps);

  return sample;

  /* special conditions */
nothing:
  {
    GST_DEBUG_OBJECT (breakout, "there is no buffer at this point. The method should be called from within a callback. Return NULL");
    return NULL;
  }
}

static GstFlowReturn gst_breakout_prepare_output_buffer (GstBaseTransform * trans,
                                        GstBuffer *input, GstBuffer **output) {
  if (gst_buffer_is_writable(input)) {
    *output = input;
    gst_buffer_ref(*output);
  } else {
    *output = gst_buffer_make_writable(gst_buffer_copy_deep(input));
  }
  printf("ref count %d, in=%p out=%p\n", (int)((*output)->mini_object.refcount), input, *output);
  return GST_FLOW_OK;
}


//gboolean gst_breakout_set_caps  (GstBaseTransform *trans, GstCaps *incaps,
//    GstCaps *outcaps) {
//  GST_TRACE_OBJECT(trans, "setting caps to in=\"%" GST_PTR_FORMAT "\", out=\"%" GST_PTR_FORMAT "\"", incaps, outcaps);
//  GstBreakout *breakout = GST_BREAKOUT (trans);
//  GstBreakoutClass* klass = GST_BREAKOUT_GET_CLASS(trans);
//  if (!breakout->caps)
//    gst_caps_replace (&breakout->caps, incaps);
//  return klass->parent_set_caps (trans, incaps, outcaps);
//  return TRUE;
//}

static gboolean
plugin_init (GstPlugin * plugin)
{
  /* FIXME Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register (plugin, "breakout", GST_RANK_NONE,
      GST_TYPE_BREAKOUT);
}

/* FIXME: these are normally defined by the GStreamer build system.
   If you are creating an element to be included in gst-plugins-*,
   remove these, as they're always defined.  Otherwise, edit as
   appropriate for your external plugin package. */
#ifndef VERSION
#define VERSION "0.0.FIXME"
#endif
#ifndef PACKAGE
#define PACKAGE "FIXME_package"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "FIXME_package_name"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://FIXME.org/"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    breakout,
    "FIXME plugin description",
    plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)

