/*
 * SPDX-FileCopyrightText: 2026 Álex Jiménez
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef GC_COLORS_H
#define GC_COLORS_H

#include <glibmm/ustring.h>

namespace GC
{

/* color is only an alias for 'Glib::ustring' and is a possible value of
   css 'background' property */
using Color = Glib::ustring;

static const Color BLACK = "linear-gradient(135deg, #8a8a8a 0%, #4a4a4a 30%, "
                           "#2a2a2a 70%, #5a5a5a 100%)";

static const Color GREEN = "linear-gradient(135deg, #5a8a2a 0%, #3a6a1a 30%, "
                           "#2a4a0a 70%, #4a7a1a 100%)";

static const Color LIGHT_BLUE = "linear-gradient(135deg, #a8d0e6 0%, #70b0d0 "
                                "30%, #4090b0 70%, #80c0e0 100%)";

static const Color NAVY_BLUE = "linear-gradient(135deg, #2a4a6a 0%, #0a2a4a "
                               "30%, #001a30 70%, #1a3a5a 100%)";

static const Color WHITE = "linear-gradient(135deg, #f0f0f5 0%, #d0d0d8 30%, "
                           "#b0b0b8 70%, #e0e0e8 100%)";

}

#endif /* GC_COLORS_H */
