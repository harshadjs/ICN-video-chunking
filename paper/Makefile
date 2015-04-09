SRC=icn-video-chunking.tex
FLAGS=-halt-on-error
PDF=$(SRC:.tex=.pdf)

all: $(PDF)
%.pdf: %.tex $(wildcard figures/*) $(wildcard figures/*/*) $(wildcard sections/*)
	latexmk --pdf $<
	latexmk -c
clean: ; latexmk -C
