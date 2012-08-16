m4_divert(`-1')

m4_define(`m4_defs',
``#'define ANIM_FRAMES $#
`#'define ANIM_HEIGHT m4_height
const char * animation[ANIM_FRAMES][ANIM_HEIGHT];')

m4_divert(`0')`'m4_dnl
`#'pragma once
m4_defs(m4_frames)
