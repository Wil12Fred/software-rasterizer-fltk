FLTK_CONFIG ?= fltk-config
FLTK_INCLUDE = `$(FLTK_CONFIG) --cxxflags`
FLTK_LIBS = `$(FLTK_CONFIG) --ldflags`

INCLUDE  =  $(FLTK_INCLUDE) -I./math/numerical -I./math/euclidean -I./renderer -I./structures -I.
CXXFLAGS = -g -Wall -Wno-register $(INCLUDE) 
LIBS = $(FLTK_LIBS) 

OBJS =  raylite.o \
        ./math/euclidean/euclidean.o \
        ./math/euclidean/transformation.o \
        ./renderer/camera.o \
        ./renderer/engine.o \
        ./renderer/object.o \
        ./renderer/parser.o \
        ./renderer/renderer.o \
        ./renderer/triangle.o \
        ./renderer/polygon.o \
        ./renderer/viewport.o

TARGET =	raylite

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

# unit tests of geometry.h and the 4x4 math: no FLTK needed
MATH_INCLUDE = -I./math/numerical -I./math/euclidean -I./renderer -I./structures -I.
tests/test_geometry: tests/test_geometry.cpp renderer/geometry.h math/euclidean/euclidean.cpp math/euclidean/transformation.cpp
	$(CXX) -g -Wall -Wno-register $(MATH_INCLUDE) -o $@ tests/test_geometry.cpp math/euclidean/euclidean.cpp math/euclidean/transformation.cpp

unit: tests/test_geometry
	./tests/test_geometry

test: unit $(TARGET)
	./tests/smoke.sh

clean:
	rm -f $(OBJS) $(TARGET) tests/test_geometry

.PHONY: all unit test clean
