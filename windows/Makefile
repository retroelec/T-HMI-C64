TARGET = windows/c64win.exe

SRC_DIR = src
BUILD_DIR = buildwin
SRC = $(shell find $(SRC_DIR) -name '*.cpp')
OBJ = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC))

INCLUDES = -I$(SDL2DIR)/include -I$(SRC_DIR)
LIBS = -L$(SDL2DIR)/lib -static -lmingw32 -lSDL2main -lSDL2 \
       -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 \
       -lshell32 -lsetupapi -lversion -luuid
CXXFLAGS += -std=c++17 -pthread -ffunction-sections -fdata-sections
LDFLAGS  += -pthread -static-libgcc -static-libstdc++ -Wl,--gc-sections

STRIP = x86_64-w64-mingw32-strip

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) $(LIBS) -mwindows -o $@
	$(STRIP) -s $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

