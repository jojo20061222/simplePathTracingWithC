all: bin/pathTracing
build/base.o: src/base.c src/base.h
	clang -o $@ -c $< -O2 -pthread -g -flto
build/frame.o: src/frame.c src/frame.h src/shapes.h src/base.h \
  src/dynamicArray.h
	clang -o $@ -c $< -O2 -pthread -g -flto
build/lightSource.o: src/lightSource.c src/lightSource.h src/base.h \
  src/object.h src/shapes.h src/textures.h
	clang -o $@ -c $< -O2 -pthread -g -flto
build/multithreads.o: src/multithreads.c src/multithreads.h src/render.h \
  src/base.h src/dynamicArray.h src/lightSource.h src/object.h \
  src/shapes.h src/textures.h src/random.h
	clang -o $@ -c $< -O2 -pthread -g -flto
build/object.o: src/object.c src/object.h src/base.h src/shapes.h \
  src/textures.h
	clang -o $@ -c $< -O2 -pthread -g -flto
build/pathTracing.o: src/pathTracing.c src/multithreads.h src/render.h \
  src/base.h src/dynamicArray.h src/lightSource.h src/object.h \
  src/shapes.h src/textures.h src/random.h src/frame.h
	clang -o $@ -c $< -O2 -pthread -g -flto
build/random.o: src/random.c src/random.h
	clang -o $@ -c $< -O2 -pthread -g -flto
build/render.o: src/render.c src/render.h src/base.h src/dynamicArray.h \
  src/lightSource.h src/object.h src/shapes.h src/textures.h \
  src/random.h
	clang -o $@ -c $< -O2 -pthread -g -flto
build/shapes.o: src/shapes.c src/shapes.h src/base.h src/dynamicArray.h
	clang -o $@ -c $< -O2 -pthread -g -flto
build/textures.o: src/textures.c src/textures.h src/base.h
	clang -o $@ -c $< -O2 -pthread -g -flto
bin/pathTracing: build/base.o build/frame.o build/lightSource.o build/multithreads.o build/object.o build/pathTracing.o build/random.o build/render.o build/shapes.o build/textures.o
	clang $^ -o $@ -lm -lpthread -flto

.PHONY: clean
clean: 
	- rm bin/pathTracing
	- rm build/*.o
