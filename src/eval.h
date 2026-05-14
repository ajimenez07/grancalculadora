/*
 * SPDX-FileCopyrightText: 2026 Álex Jiménez
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef GC_EVAL_H
#define GC_EVAL_H

#include "display.h"

namespace GC
{

  double eval (GC::DisplayAst::Expr *input, Glib::ustring &msg);
}

#endif
