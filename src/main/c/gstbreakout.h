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
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _GST_BREAKOUT_H_
#define _GST_BREAKOUT_H_

#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>

G_BEGIN_DECLS

#define GST_TYPE_BREAKOUT   (gst_breakout_get_type())
#define GST_BREAKOUT(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_BREAKOUT,GstBreakout))
#define GST_BREAKOUT_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_BREAKOUT,GstBreakoutClass))
#define GST_IS_BREAKOUT(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_BREAKOUT))
#define GST_IS_BREAKOUT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_BREAKOUT))
#define GST_BREAKOUT_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS((obj), GST_TYPE_BREAKOUT, GstBreakoutClass))

typedef struct _GstBreakout GstBreakout;
typedef struct _GstBreakoutClass GstBreakoutClass;


struct _GstBreakout
{
  GstVideoFilter base_breakout;

  GstVideoFrame* cur;
  GstCaps*       caps;
  GstPad*        sink;

};

struct _GstBreakoutClass
{
  GstVideoFilterClass base_breakout_class;

  /* signals */
  void          (*new_sample)                   (GstBreakout *breakout);

//  gboolean      (*parent_set_caps)  (GstBaseTransform *trans, GstCaps *incaps, GstCaps *outcaps);
  GstSample *   (*pull_sample)       (GstBreakout *breakout);
};

GType gst_breakout_get_type (void);

GstSample* gst_breakout_pull_sample   (GstBreakout* breakout);

//gboolean   gst_breakout_set_caps      (GstBaseTransform *trans,
//    GstCaps *incaps,
//    GstCaps *outcaps);

G_END_DECLS

#endif
