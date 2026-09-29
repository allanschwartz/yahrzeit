YAML := $(wildcard *.yml)
BASE := $(YAML:.yml=)

HTML := $(addsuffix .html,$(BASE))
PNG  := $(addsuffix .png,$(BASE))
SVG  := $(addsuffix .svg,$(BASE))
BOM  := $(addsuffix .bom.tsv,$(BASE))

all: $(HTML)

%.html: %.yml
	wireviz $<

clean:
	rm -f $(HTML) $(PNG) $(SVG) $(BOM)

.PHONY: all clean
