uuLall: $(patsubst %.yml,%.html,$(wildcard *.yml))

%.html: %.yml
	wireviz $<

clean:
	rm -f *.html *.png *.svg *.bom.tsv

.PHONY: all clean

