#include <stdio.h>
#include <SDL.h>
#include "common.h"
#include "event.h"
#include "debug/debug.h"
#include "debug/display.h"
#include "scancode.h"
#include "ikbd.h"
#include "cpu.h"
#include "mfp.h"
#include "state.h"
#include "screen.h"

static struct state *laststate;

static int event_key(SDL_KeyboardEvent key, int state)
{
  SDL_Keysym k;

  k = key.keysym;

  if((k.sym == ' ') || (k.sym == 27) ||
     ((k.sym >= '0') && (k.sym <= '9')) ||
     ((k.sym >= 'a') && (k.sym <= 'z')) ||
     (k.sym == '\'') || (k.sym == ',') || (k.sym == '-') || (k.sym == '.') ||
     (k.sym == '/') || (k.sym == ';') || (k.sym == '=') || (k.sym == '[') ||
     (k.sym == '\\') || (k.sym == ']') || (k.sym == '`') ||
     (k.sym == '\r') || (k.sym == '\b') || (k.sym == '\t')
     ) {
    ikbd_queue_key(scancode[k.sym], state);
  } else if((k.sym >= SDLK_F1) && (k.sym <= SDLK_F10)) {
    ikbd_queue_key(SCAN_F1+k.sym-SDLK_F1, state);
  } else if(k.sym == SDLK_F11) {
    if(k.mod & KMOD_CTRL && state == EVENT_RELEASE) {
      printf("DEBUG: Toggle screen disable\n");
      screen_disable(!screen_check_disable());
    } else {
      if(state == EVENT_RELEASE) {
        if(debugger)
          return EVENT_DEBUG;
        else {
          state_save("ostis.state", state_collect());
          SDL_Quit();
          exit(0);
        }
      }
    }
  } else if(k.sym == SDLK_PAUSE) {
    if(k.mod & KMOD_CTRL) {
      if(state == EVENT_RELEASE) {
        screen_toggle_fullscreen();
      }
    } else {
      if(state == EVENT_RELEASE) {
        screen_toggle_grab();
      }
    }
  } else if(k.sym == SDLK_PRINTSCREEN) {
    if(k.mod & KMOD_ALT) {
      if(state == EVENT_RELEASE) {
        printf("DEBUG: Start debugger\n");
      }
    } else {
      if(state == EVENT_RELEASE) {
        cprint_all = !cprint_all;
      }
    }
  } else if(k.sym == SDLK_F12) {
    if(state == EVENT_RELEASE) {
      printf("-------------------------------------------\n");
      cpu_print_status(CPU_USE_CURRENT_PC);
      printf("- - - - - - - - - - - - - - - - - - - - - -\n");
      mfp_print_status();
      printf("- - - - - - - - - - - - - - - - - - - - - -\n");
      ikbd_print_status();
      printf("DEBUG: Speed: %g FPS\n", screen_fps());
      printf("-------------------------------------------\n");
    }
  } else if(k.sym == SDLK_F13) {
    if(state == EVENT_RELEASE) {
      printf("DEBUG: Collecting state\n");
      laststate = state_collect();
    }
  } else if(k.sym == SDLK_F14) {
    if(state == EVENT_RELEASE) {
      printf("DEBUG: Restoring state\n");
      state_restore(laststate);
    }
  } else if(k.sym == SDLK_LSHIFT) {
    ikbd_queue_key(SCAN_LSHIFT, state);
  } else if(k.sym == SDLK_RSHIFT) {
    ikbd_queue_key(SCAN_RSHIFT, state);
  } else if(k.sym == SDLK_LCTRL) {
    ikbd_queue_key(SCAN_CONTROL, state);
  } else if(k.sym == SDLK_LALT) {
    ikbd_queue_key(SCAN_ALT, state);
  } else if(k.sym == SDLK_UP) {
    ikbd_queue_key(SCAN_UP, state);
  } else if(k.sym == SDLK_DOWN) {
    ikbd_queue_key(SCAN_DOWN, state);
  } else if(k.sym == SDLK_LEFT) {
    ikbd_queue_key(SCAN_LEFT, state);
  } else if(k.sym == SDLK_RIGHT) {
    ikbd_queue_key(SCAN_RIGHT, state);
  } else if(k.sym == SDLK_END) {
    ikbd_queue_key(SCAN_INSERT, state);
  } else if(k.sym == SDLK_PAGEDOWN) {
    ikbd_queue_key(SCAN_DELETE, state);
  } else {
    printf("Unimplemented key: %d\n", k.sym);
  }
  return EVENT_NONE;
}

static SDL_Joystick *joystick;

void event_init()
{
  SDL_JoystickEventState(SDL_ENABLE);
  joystick = SDL_JoystickOpen(0);
}

void event_exit()
{
}

static int event_mouse(SDL_MouseMotionEvent m)
{
  ikbd_queue_motion(m.xrel, m.yrel);
  return EVENT_NONE;
}

static int event_button(SDL_MouseButtonEvent b, int state)
{
  switch (b.button) {
  case SDL_BUTTON_LEFT:
    ikbd_button(SCAN_LEFT_BUTTON, state);
    break;
  case SDL_BUTTON_RIGHT:
    ikbd_button(SCAN_RIGHT_BUTTON, state);
    break;
  }

  return EVENT_NONE;
}

static int event_joystick(SDL_JoyAxisEvent a)
{
  static int last = 0;
  int direction = last;
  if (a.axis == 0) {
    direction &= ~(IKBD_JOY_RIGHT | IKBD_JOY_LEFT);
    if (a.value > 5000)
      direction |= IKBD_JOY_RIGHT;
    else if (a.value < -5000)
      direction |= IKBD_JOY_LEFT;
  } else if(a.axis == 1) {
    direction &= ~(IKBD_JOY_UP | IKBD_JOY_DOWN);
    if (a.value > 5000)
      direction |= IKBD_JOY_DOWN;
    else if (a.value < -5000)
      direction |= IKBD_JOY_UP;
  } 
  if (direction != last)
    ikbd_joystick(direction);
  last = direction;
  return EVENT_NONE;
}

static int event_fire(SDL_JoyButtonEvent b)
{
  if(b.button == 14)
    ikbd_fire(b.state);
  return EVENT_NONE;
}

int event_parse(SDL_Event ev)
{
  switch(ev.type) {
  case SDL_KEYDOWN:
    return event_key(ev.key, EVENT_PRESS);
  case SDL_KEYUP:
    return event_key(ev.key, EVENT_RELEASE);
  case SDL_MOUSEMOTION:
    return event_mouse(ev.motion);
  case SDL_MOUSEBUTTONDOWN:
    return event_button(ev.button, EVENT_PRESS);
  case SDL_MOUSEBUTTONUP:
    return event_button(ev.button, EVENT_RELEASE);
  case SDL_JOYAXISMOTION:
    return event_joystick(ev.jaxis);
  case SDL_JOYBUTTONDOWN:
  case SDL_JOYBUTTONUP:
    return event_fire(ev.jbutton);
  case SDL_WINDOWEVENT:
    if(ev.window.windowID == screen_window_id) {
      if(ev.window.event == SDL_WINDOWEVENT_RESIZED) {
        if((ev.window.data1 % 512) == 0 && (ev.window.data2 % 314) == 0)
          screen_make_texture(SDL_SCALING_NEAREST);
        else
          screen_make_texture(SDL_SCALING_LINEAR);
      }
    }
    break;
  case SDL_QUIT:
    if(debugger)
      return EVENT_DEBUG;
    else {
      SDL_Quit();
      exit(0);
    }
  }

  return EVENT_NONE;
}

int event_main()
{
  SDL_Event ev;
  
  if(!SDL_PollEvent(&ev)) {
    return EVENT_NONE;
  }

  if(ev.window.windowID == screen_window_id) {
    if(!debugger || (debugger && !debug_update_win)) {
      return event_parse(ev);
    } else {
      return EVENT_NONE;
    }
  } else if(debugger && ev.window.windowID == debug_window_id) {
    return debug_event_parse(ev);
  } else {
    switch(ev.type) {
    case SDL_JOYAXISMOTION:
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
      return event_parse(ev);
    case SDL_QUIT:
      if(debugger && !debug_update_win) {
        return EVENT_DEBUG;
      } else {
        SDL_Quit();
        exit(0);
      }
    }
  }

  return EVENT_NONE;
}

