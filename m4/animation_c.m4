m4_divert(`-1')

m4_define(`m4_frames_array',
`const char * animation[ANIM_FRAMES][ANIM_HEIGHT] = {
$0_($@)
};')

m4_define(`m4_frames_array_', `m4_ifelse(`$#', `1',
`  {
m4_undivert(`$1')`'m4_dnl
  }',
`  {
m4_undivert(`$1')  },
/*** frame break **************************************************************/
$0(m4_shift($@))')')
m4_divert(`0')`'m4_dnl
`#'include "animation.h"
m4_frames_array(m4_frames)
