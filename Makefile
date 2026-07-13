all: bin/main
build/base.o: src/base.c src/base.h
	clang -o $@ -c $< -O2 -g -pthread
build/main.o: src/main.c src/dynamicArray.h src/multithreads.h src/render.h \
  src/base.h src/object.h src/shapes.h /opt/homebrew/include/SDL2/SDL.h \
  /opt/homebrew/include/SDL2/SDL_main.h \
  /opt/homebrew/include/SDL2/SDL_stdinc.h \
  /opt/homebrew/include/SDL2/SDL_config.h \
  /opt/homebrew/include/SDL2/SDL_platform.h \
  /opt/homebrew/include/SDL2/begin_code.h \
  /opt/homebrew/include/SDL2/close_code.h \
  /opt/homebrew/include/SDL2/SDL_config_macosx.h \
  /opt/homebrew/include/SDL2/SDL_assert.h \
  /opt/homebrew/include/SDL2/SDL_atomic.h \
  /opt/homebrew/include/SDL2/SDL_audio.h \
  /opt/homebrew/include/SDL2/SDL_error.h \
  /opt/homebrew/include/SDL2/SDL_endian.h \
  /opt/homebrew/include/SDL2/SDL_mutex.h \
  /opt/homebrew/include/SDL2/SDL_thread.h \
  /opt/homebrew/include/SDL2/SDL_rwops.h \
  /opt/homebrew/include/SDL2/SDL_clipboard.h \
  /opt/homebrew/include/SDL2/SDL_cpuinfo.h \
  /opt/homebrew/include/SDL2/SDL_events.h \
  /opt/homebrew/include/SDL2/SDL_video.h \
  /opt/homebrew/include/SDL2/SDL_pixels.h \
  /opt/homebrew/include/SDL2/SDL_rect.h \
  /opt/homebrew/include/SDL2/SDL_surface.h \
  /opt/homebrew/include/SDL2/SDL_blendmode.h \
  /opt/homebrew/include/SDL2/SDL_keyboard.h \
  /opt/homebrew/include/SDL2/SDL_keycode.h \
  /opt/homebrew/include/SDL2/SDL_scancode.h \
  /opt/homebrew/include/SDL2/SDL_mouse.h \
  /opt/homebrew/include/SDL2/SDL_joystick.h \
  /opt/homebrew/include/SDL2/SDL_guid.h \
  /opt/homebrew/include/SDL2/SDL_gamecontroller.h \
  /opt/homebrew/include/SDL2/SDL_sensor.h \
  /opt/homebrew/include/SDL2/SDL_quit.h \
  /opt/homebrew/include/SDL2/SDL_gesture.h \
  /opt/homebrew/include/SDL2/SDL_touch.h \
  /opt/homebrew/include/SDL2/SDL_filesystem.h \
  /opt/homebrew/include/SDL2/SDL_haptic.h \
  /opt/homebrew/include/SDL2/SDL_hidapi.h \
  /opt/homebrew/include/SDL2/SDL_hints.h \
  /opt/homebrew/include/SDL2/SDL_loadso.h \
  /opt/homebrew/include/SDL2/SDL_log.h \
  /opt/homebrew/include/SDL2/SDL_messagebox.h \
  /opt/homebrew/include/SDL2/SDL_metal.h \
  /opt/homebrew/include/SDL2/SDL_power.h \
  /opt/homebrew/include/SDL2/SDL_render.h \
  /opt/homebrew/include/SDL2/SDL_shape.h \
  /opt/homebrew/include/SDL2/SDL_system.h \
  /opt/homebrew/include/SDL2/SDL_timer.h \
  /opt/homebrew/include/SDL2/SDL_version.h \
  /opt/homebrew/include/SDL2/SDL_locale.h \
  /opt/homebrew/include/SDL2/SDL_misc.h
	clang -o $@ -c $< `sdl2-config --cflags` -O2 -g -pthread
build/multithreads.o: src/multithreads.c src/multithreads.h src/render.h \
  src/base.h src/dynamicArray.h src/object.h src/shapes.h
	clang -o $@ -c $< -O2 -g -pthread
build/object.o: src/object.c src/object.h src/base.h src/shapes.h
	clang -o $@ -c $< -O2 -g -pthread
build/render.o: src/render.c src/render.h src/base.h src/dynamicArray.h \
  src/object.h src/shapes.h
	clang -o $@ -c $< -O2 -g -pthread
build/shapes.o: src/shapes.c src/shapes.h src/base.h
	clang -o $@ -c $< -O2 -g -pthread
bin/main: build/base.o build/main.o build/multithreads.o build/object.o build/render.o build/shapes.o 
	clang $^ -o $@ `sdl2-config --libs` -lm -lpthread

.PHONY: clean
clean: 
	- rm bin/main
	- rm build/*.o
