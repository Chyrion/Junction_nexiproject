gui: py/out.txt
	/usr/bin/env python py/main.py

py/out.txt: data.txt
	/usr/bin/env python py/parser.py

.PHONY: data.txt
data.txt:
	cd rdr && $(MAKE) run
	cp rdr/data.txt ./

.PHONY: clean
clean:
	rm rdr/data.txt
	rm ./data.txt
	rm py/out.txt