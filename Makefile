# Plasma Engine — OpenGL port of Cube World plasma:: (msys64 MinGW64)
# Build: bash build.sh   or   make

export PATH := /mingw64/bin:$(PATH)
export TMP  := /home/camil/tmp
export TEMP := /home/camil/tmp

CXX      := g++
CC       := gcc
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra \
            -I. -Iengine/include -Icube/include \
            -Ithird_party/sqlite3
CFLAGS   := -O2 -w -Ithird_party/sqlite3
LDFLAGS  :=
LDLIBS   := -lglfw3 -lglew32 -lopengl32 -lgdi32 -lm -lpthread

SQLITE   := third_party/sqlite3/sqlite3.c

ENGINE_SRCS := \
	engine/src/Object.cpp \
	engine/src/NamedObject.cpp \
	engine/src/Node.cpp \
	engine/src/Display.cpp \
	engine/src/OpenGLEngine.cpp \
	engine/src/Widget.cpp \
	engine/src/Shape.cpp \
	engine/src/Font.cpp \
	engine/src/Filters.cpp \
	engine/src/PlxLoader.cpp \
	engine/src/UiDraw.cpp \
	engine/src/UiInput.cpp \
	engine/src/Tessellate.cpp \
	engine/src/Keyable.cpp \
	engine/src/Attribute.cpp

CUBE_SRCS := \
	cube/src/BlobDescramble.cpp \
	cube/src/CubModel.cpp \
	cube/src/AssetDatabase.cpp \
	cube/src/TextureCatalog.cpp \
	cube/src/GuiHud.cpp \
	cube/src/stb_image_impl.cpp \
	cube/src/stb_truetype_impl.cpp

APP_SRCS := app/main.cpp

SRCS := $(ENGINE_SRCS) $(CUBE_SRCS) $(APP_SRCS) $(SQLITE)
OBJS := $(SRCS:.cpp=.o)
OBJS := $(OBJS:.c=.o)

TARGET := plasma_test.exe

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET) .
