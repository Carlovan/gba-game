SOURCES := $(wildcard sources/*.cpp)
BINS    := $(addprefix bin/, $(notdir $(SOURCES:.cpp=.o)))

build: $(BINS)

bin/%.o: sources/%.cpp %.h
	$(CC) -c -o $@ $(CFLAGS) $<
