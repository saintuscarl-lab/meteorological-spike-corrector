CC	= gcc
CFLAGS 	= -Wall -Wextra -Werror -std=c99 -D_GNU_SOURCE -Iinclude
LDFLAGS	= -lyaml -lm
TARGET 	= weather_qc
README	= README
TEST = tests/bats/*.bats

TEST_UNITS = tests/unit/test_config \
	tests/unit/test_validation \
	tests/unit/test_neighbor_finder \
	tests/unit/test_spike_detection


OBJS 	= src/weather_qc.o \
	src/qc_config.o \
	src/qc_loader.o \
	src/qc_timeseries.o \
	src/qc_validation.o \
	src/qc_spike_detector.o \
	src/qc_neighbor_finder.o \
	src/qc_predictor.o \
	src/qc_observation.o \
	src/qc_output.o


.PHONY: all clean test valgrind couverture html

all: $(TARGET)

# Build
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

## Les dépendances
src/weather_qc.o: src/weather_qc.c \
	include/qc_config.h \
	include/qc_timeseries.h \
	include/qc_loader.h \
	include/qc_predictor.h \
	include/qc_observation.h \
	include/qc_validation.h \
	include/qc_spike_detector.h \
	include/qc_output.h

src/qc_config.o: src/qc_config.c \
	include/qc_config.h

src/qc_loader.o: src/qc_loader.c \
	include/qc_loader.h \
	include/qc_timeseries.h \
	include/qc_config.h

src/qc_timeseries.o: src/qc_timeseries.c \
	include/qc_timeseries.h

src/qc_validation.o: src/qc_validation.c \
	include/qc_validation.h \
	include/qc_timeseries.h \
	include/qc_config.h

src/qc_spike_detector.o: src/qc_spike_detector.c \
	include/qc_spike_detector.h \
	include/qc_neighbor_finder.h \
	include/qc_config.h \
	include/qc_timeseries.h

src/qc_neighbor_finder.o: src/qc_neighbor_finder.c \
	include/qc_neighbor_finder.h \
	include/qc_config.h \
	include/qc_timeseries.h

src/qc_predictor.o: 	src/qc_predictor.c \
	include/qc_predictor.h \
	include/qc_timeseries.h

src/qc_observation.o: 	src/qc_observation.c \
	include/qc_observation.h \
	include/qc_timeseries.h

src/qc_output.o: 	src/qc_output.c \
	include/qc_output.h \
	include/qc_timeseries.h

### Les tests unitaires
tests/unit/test_config: tests/unit/test_config.c src/qc_config.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) -lcunit

tests/unit/test_validation: tests/unit/test_validation.c src/qc_validation.o src/qc_config.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) -lcunit -lm

tests/unit/test_neighbor_finder: tests/unit/test_neighbor_finder.c src/qc_neighbor_finder.o src/qc_config.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) -lcunit -lm

tests/unit/test_spike_detection: tests/unit/test_spike_detection.c src/qc_spike_detector.o src/qc_neighbor_finder.o src/qc_config.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) -lcunit -lm


# Documentation
html: 
	pandoc -s -c misc/github-pandoc.css -M pagetitle="Contrôle de qualité et correction des spikes météorologiques" $(README).md -o $(README).html


# Test
test: $(TARGET) $(TEST_UNITS)
	@echo "\n  Execution des tests unitaires (CUnit)"
	@for t in $(TEST_UNITS); do ./$$t; done
	@echo "\n  Execution des tests bats "
	bats $(TEST)

# Couverture	
couverture: CFLAGS += -fprofile-arcs -ftest-coverage
couverture: LDFLAGS += -lgcov
couverture: clean $(TARGET) $(TEST_UNITS)
	@echo "\n--- Execution des tests unitaires (CUnit) ---"
	@for t in $(TEST_UNITS); do ./$$t || exit 1; done
	@echo "\n--- Execution des tests Bats ---"
	bats $(TEST)
	@echo "\n*** RAPPORT DE COUVERTURE GCOV ***"
	cd src && gcov -b *.c

# Fuite
valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET) -c config.yaml


# Cleanup
clean:	
	rm -f src/*.o $(TARGET) $(TEST_UNITS) $(README).html
	rm -f src/*.gcda src/*.gcno src/*.gcov  *.gcov tests/unit/*.gcno tests/unit/*.gcda