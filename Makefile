all: bin/pathTracing
build/camera.o: src/camera.c src/camera.h src/base.h
	clang -o $@ -c $< -O2 -pthread -g -flto -DDOUBLE_AS_REAL
build/frame.o: src/frame.c src/frame.h src/shapes.h src/base.h src/transform.h \
  src/dynamicArray.h
	clang -o $@ -c $< -O2 -pthread -g -flto -DDOUBLE_AS_REAL
build/lightSource.o: src/lightSource.c src/lightSource.h src/base.h \
  src/object.h src/shapes.h src/textures.h
	clang -o $@ -c $< -O2 -pthread -g -flto -DDOUBLE_AS_REAL
build/mainOnServer.o: src/mainOnServer.c src/frame.h src/shapes.h src/base.h \
  src/transform.h src/multithreads.h src/render.h src/camera.h \
  src/dynamicArray.h src/lightSource.h src/object.h src/textures.h \
  src/random.h
	clang -o $@ -c $< -O2 -pthread -g -flto -DDOUBLE_AS_REAL
build/multithreads.o: src/multithreads.c src/multithreads.h src/render.h \
  src/base.h src/camera.h src/dynamicArray.h src/lightSource.h \
  src/object.h src/shapes.h src/textures.h src/random.h
	clang -o $@ -c $< -O2 -pthread -g -flto -DDOUBLE_AS_REAL
build/object.o: src/object.c src/object.h src/base.h src/shapes.h \
  src/textures.h
	clang -o $@ -c $< -O2 -pthread -g -flto -DDOUBLE_AS_REAL
build/random.o: src/random.c src/random.h src/base.h
	clang -o $@ -c $< -O2 -pthread -g -flto -DDOUBLE_AS_REAL
build/render.o: src/render.c src/render.h src/base.h src/camera.h \
  src/dynamicArray.h src/lightSource.h src/object.h src/shapes.h \
  src/textures.h src/random.h
	clang -o $@ -c $< -O2 -pthread -g -flto -DDOUBLE_AS_REAL
build/shapes.o: src/shapes.c src/shapes.h src/base.h src/dynamicArray.h
	clang -o $@ -c $< -O2 -pthread -g -flto -DDOUBLE_AS_REAL
build/textures.o: src/textures.c src/textures.h src/base.h
	clang -o $@ -c $< -O2 -pthread -g -flto -DDOUBLE_AS_REAL
build/transform.o: src/transform.c src/transform.h src/base.h
	clang -o $@ -c $< -O2 -pthread -g -flto -DDOUBLE_AS_REAL
bin/pathTracing: build/camera.o build/frame.o build/lightSource.o build/mainOnServer.o build/multithreads.o build/object.o build/random.o build/render.o build/shapes.o build/textures.o build/transform.o
	clang $^ -o $@ -lm -lpthread -flto

.PHONY: clean
clean: 
	- rm bin/*
	- rm build/*.o
