all: myfind SplitterMerger Searcher

myfind: Initializer.o	Handler.o
		gcc Initializer.o Handler.o -o myfind

SplitterMerger: SplitterMerger.o
		gcc SplitterMerger.o -o SplitterMerger

Searcher: Searcher.o
		gcc Searcher.o -o Searcher

Searcher.o: Searcher.c Utilities.h Searcher.h

SplitterMerger.o:	SplitterMerger.c Utilities.h SplitterMerger.h

Handler.o: Handler.c Handler.h

Initializer.o:	Initializer.c Utilities.h Initializer.h Handler.h


clean:
	rm -r myfind Initializer.o Handler.o
	rm -r SplitterMerger SplitterMerger.o
	rm -r Searcher Searcher.o
