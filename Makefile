# Makefile for PubSub Project
# FTN - Industrial Communication Protocols

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -I./src
TARGET = pubsub_demo

# Source files
SOURCES = src/main.cpp \
          src/core/Publisher.cpp \
          src/core/Subscriber.cpp \
          src/core/PubSubEngine.cpp \
          src/Network.cpp \
          src/utils/MessageValidator.cpp \
          src/utils/CommandLineParser.cpp \
          src/utils/NetworkUtils.cpp \
          src/utils/MessageFormatter.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	@echo "Linking..."
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)
	@echo "Build complete! Executable: ./$(TARGET)"

# Compile source files to object files
%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	@echo "Cleaning..."
	rm -f $(OBJECTS) $(TARGET)
	@echo "Clean complete!"

# Run the program
run: $(TARGET)
	./$(TARGET)

# Help
help:
	@echo "PubSub Project Makefile"
	@echo "======================="
	@echo "Usage:"
	@echo "  make          - Build the project"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make run      - Build and run the program"
	@echo "  make help     - Show this help message"

.PHONY: all clean run help
