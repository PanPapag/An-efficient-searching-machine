all: myfind SplitterMerger Searcher

myfind: src/Initializer.o	src/Handler.o
		gcc src/Initializer.o src/Handler.o -o myfind

SplitterMerger: src/SplitterMerger.o
		gcc src/SplitterMerger.o -o SplitterMerger

Searcher: src/Searcher.o
		gcc src/Searcher.o -o Searcher

Searcher.o: src/Searcher.c headers/Utilities.h headers/Searcher.h

SplitterMerger.o:	src/SplitterMerger.c headers/Utilities.h headers/SplitterMerger.h

Handler.o: src/Handler.c headers/Handler.h

Initializer.o:	src/Initializer.c headers/Utilities.h headers/Initializer.h headers/Handler.h


clean:
	rm -r myfind src/Handler.o src/Initializer.o
	rm -r SplitterMerger src/SplitterMerger.o
	rm -r Searcher src/Searcher.o
