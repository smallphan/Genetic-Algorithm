CXX = g++
CXXFLAGS = -std=c++11

TARGETS = map IGA NGA
EXES = $(TARGETS:%=%.exe)
RESULT = result

all: $(EXES) map.txt $(RESULT) $(RESULT)/IGA.log $(RESULT)/IGA.seq $(RESULT)/NGA.log $(RESULT)/NGA.seq figure

%.exe: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

map map.txt: map.exe
	./$<

$(RESULT): 
	@mkdir $(RESULT) 2> NUL || echo "Directory already exists"

$(RESULT)/IGA.log $(RESULT)/IGA.seq: IGA.exe map.txt $(RESULT)
	./$< 

$(RESULT)/NGA.log $(RESULT)/NGA.seq: NGA.exe map.txt $(RESULT)
	./$< 

figure: $(RESULT)/IGA.seq $(RESULT)/NGA.seq
	python figure.py 

clean:
	@if exist $(RESULT) ( \
		rmdir /s /q $(RESULT) \
	)
	@for %%i in ($(EXES) map.txt) do ( \
		if exist %%i del %%i \
	)

.PHONY: all clean figure map
