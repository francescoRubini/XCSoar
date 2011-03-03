/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2011 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "Screen/TopWindow.hpp"
#include "Screen/OpenGL/Cache.hpp"
#include "Screen/OpenGL/Surface.hpp"
#include "Screen/Android/Event.hpp"
#include "Android/Main.hpp"
#include "Android/NativeView.hpp"

void
TopWindow::on_pause()
{
  if (paused)
    return;

  TextCache::flush();

  SurfaceDestroyed();

  native_view->deinitSurface();

  paused_mutex.Lock();
  paused = true;
  resumed = false;
  paused_cond.signal();
  paused_mutex.Unlock();
}

void
TopWindow::on_resume()
{
  if (!paused)
    return;

  /* tell TopWindow::expose() to reinitialize OpenGL */
  resumed = true;

  /* schedule a redraw */
  invalidate();
}

static bool
match_pause_and_resume(const Event &event, void *ctx)
{
  return event.type == Event::PAUSE || event.type == Event::RESUME;
}

void
TopWindow::pause()
{
  surface_valid = false;

  event_queue->purge(match_pause_and_resume, NULL);
  event_queue->push(Event::PAUSE);

  paused_mutex.Lock();
  while (!paused)
    paused_cond.wait(paused_mutex);
  paused_mutex.Unlock();
}

void
TopWindow::resume()
{
  event_queue->purge(match_pause_and_resume, NULL);
  event_queue->push(Event::RESUME);
}
