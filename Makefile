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

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
