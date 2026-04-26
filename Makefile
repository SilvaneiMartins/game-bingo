# --- Ferramentas ---
CXX      := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -O2
TARGET   := BingoMartins.exe
MAKE_CMD := $(MAKE)

# --- Caminhos do projeto ---
SRC := $(wildcard Source/*.cpp)
OBJ := $(SRC:.cpp=.o)

# --- Raylib ---
RAYLIB_DIR         := ./raylib/raylib
RAYLIB_INCLUDE_DIR := $(RAYLIB_DIR)/src
RAYLIB_LIB_DIR     := $(RAYLIB_DIR)/src
RAYLIB_STATIC_LIB  := $(RAYLIB_LIB_DIR)/libraylib.a

INC_FLAGS := -I$(RAYLIB_INCLUDE_DIR)
LIB_FLAGS := -L$(RAYLIB_LIB_DIR)
LDFLAGS   := -lraylib -lopengl32 -lgdi32 -lwinmm -lshell32

# --- Regras ---
all: $(TARGET)

$(TARGET): $(RAYLIB_STATIC_LIB) $(SRC)
	$(CXX) $(CXXFLAGS) $(INC_FLAGS) $(SRC) -o $(TARGET) $(LIB_FLAGS) $(LDFLAGS)

$(RAYLIB_STATIC_LIB):
	$(MAKE_CMD) -C $(RAYLIB_DIR)/src PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=STATIC RAYLIB_BUILD_MODE=RELEASE

run: all
	.\$(TARGET)

clean:
	@if exist $(TARGET) del $(TARGET)
	@$(MAKE_CMD) -C $(RAYLIB_DIR)/src clean
	@echo Limpeza concluida.

print-paths:
	@echo Source........: $(SRC)
	@echo Include raylib: $(RAYLIB_INCLUDE_DIR)
	@echo Lib raylib....: $(RAYLIB_LIB_DIR)

.PHONY: all run clean print-paths
