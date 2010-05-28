all : 

PYT = $(wildcard *.pyt)

.PHONY: test tests
test : tests
tests : $(PYT)

.PHONY: $(PYT)
$(PYT):
	python $@

.PHONY: clean
clean:
	rm *.pyc
