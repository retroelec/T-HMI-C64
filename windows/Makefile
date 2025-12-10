TARGET = windows/c64win.exe

SRC_DIR = src
BUILD_DIR = buildwin
SRC = $(shell find $(SRC_DIR) -name '*.cpp')
OBJ = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC))

INCLUDES = -I$(SDL2DIR)/include -I$(SRC_DIR)
LIBS     = -L$(SDL2DIR)/lib -lmingw32 -lSDL2main -lSDL2
CXXFLAGS += -std=c++17 -pthread
LDFLAGS  += -pthread

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) $(INCLUDES) $(LIBS) -mwindows -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -c $(INCLUDES) $< -o $@

clean:
	rm -rf $(BUILD_DIR) *.exe

