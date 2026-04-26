#ifndef LS_CAMERA_H
#define LS_CAMERA_H

#include <gst/gst.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Expose the tee element so other modules (recording, streaming) can tap into the video feed */
GstElement* ls_camera_get_tee(void);

/* Expose the main pipeline so other modules can sync their state */
GstElement* ls_camera_get_pipeline(void);

#ifdef __cplusplus
}
#endif

#endif
