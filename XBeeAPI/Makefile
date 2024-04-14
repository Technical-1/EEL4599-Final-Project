all: diagram
	pdflatex report
	biber report
	pdflatex report # Recompile twice for references

diagram:
	make -C diagrams

clean:
	@-rm -rf *.aux *.log *.out *.pdf *.temp *.zip *.bbl *.bcf *.blg *.run.xml
	make -C diagrams clean
