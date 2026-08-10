all: bin/main
build/camera.o: src/camera.c src/camera.h src/base.h
	clang -o $@ -c $< -O2 -pthread -g -DDEBUG -flto -DDOUBLE_AS_REAL
build/frame.o: src/frame.c src/frame.h src/shapes.h src/base.h src/transform.h \
  src/dynamicArray.h
	clang -o $@ -c $< -O2 -pthread -g -DDEBUG -flto -DDOUBLE_AS_REAL
build/lightSource.o: src/lightSource.c src/lightSource.h src/base.h \
  src/object.h src/shapes.h src/textures.h
	clang -o $@ -c $< -O2 -pthread -g -DDEBUG -flto -DDOUBLE_AS_REAL
build/main.o: src/main.c src/dynamicArray.h src/camera.h src/base.h src/frame.h \
  src/shapes.h src/transform.h src/multithreads.h src/render.h \
  src/lightSource.h src/object.h src/textures.h src/random.h \
  /opt/homebrew/include/SDL2/SDL.h /opt/homebrew/include/SDL2/SDL_main.h \
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
	clang -o $@ -c $< `sdl2-config --cflags` -O2 -pthread -g -DDEBUG -flto -DDOUBLE_AS_REAL
build/multithreads.o: src/multithreads.c src/multithreads.h src/render.h \
  src/base.h src/camera.h src/dynamicArray.h src/lightSource.h \
  src/object.h src/shapes.h src/textures.h src/random.h
	clang -o $@ -c $< -O2 -pthread -g -DDEBUG -flto -DDOUBLE_AS_REAL
build/object.o: src/object.c src/object.h src/base.h src/shapes.h \
  src/textures.h
	clang -o $@ -c $< -O2 -pthread -g -DDEBUG -flto -DDOUBLE_AS_REAL
build/random.o: src/random.c src/random.h src/base.h
	clang -o $@ -c $< -O2 -pthread -g -DDEBUG -flto -DDOUBLE_AS_REAL
build/render.o: src/render.c src/render.h src/base.h src/camera.h \
  src/dynamicArray.h src/lightSource.h src/object.h src/shapes.h \
  src/textures.h src/random.h
	clang -o $@ -c $< -O2 -pthread -g -DDEBUG -flto -DDOUBLE_AS_REAL
build/shapes.o: src/shapes.c src/shapes.h src/base.h src/dynamicArray.h
	clang -o $@ -c $< -O2 -pthread -g -DDEBUG -flto -DDOUBLE_AS_REAL
build/textures.o: src/textures.c src/textures.h src/base.h
	clang -o $@ -c $< -O2 -pthread -g -DDEBUG -flto -DDOUBLE_AS_REAL
build/transform.o: src/transform.c src/transform.h src/base.h
	clang -o $@ -c $< -O2 -pthread -g -DDEBUG -flto -DDOUBLE_AS_REAL
bin/main: build/camera.o build/frame.o build/lightSource.o build/main.o build/multithreads.o build/object.o build/random.o build/render.o build/shapes.o build/textures.o build/transform.o 
	clang $^ -o $@ `sdl2-config --libs` -lm -lpthread -flto

.PHONY: clean
clean: 
	- rm bin/*
	- rm build/*.o
