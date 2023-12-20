CFLAGS += -Ilib
ifeq ($(OS),Windows_NT)
	LDFLAGS += -s -lopengl32 -lgdi32
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		LDFLAGS += -framework OpenGL -framework Cocoa
	else ifeq ($(UNAME_S),Linux)
		LDFLAGS += -s -lGLU -lGL -lX11 -lm
	endif
endif

joodle : src/game.c src/tigr.c
	gcc $^ -Os -o $@ $(CFLAGS) $(LDFLAGS)
