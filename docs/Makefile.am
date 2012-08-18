docfiles = canal.tex canal.pdf

CLEANFILES = *.aux canal.log canal.out canal.toc canal.pdf

canal.pdf: canal.tex doxygen-canal/refman.tex
	echo Calling pdflatex twice to handle all references.
	$(PDFLATEX) canal.tex
	makeindex canal.idx
	$(PDFLATEX) canal.tex
	latex_count=5 ; \
	while egrep -s 'Rerun (LaTeX|to get cross-references right)' canal.log && [ $$latex_count -gt 0 ] ;\
	    do \
	      echo "Rerunning latex...." ;\
	      $(PDFLATEX) canal.tex ;\
	      latex_count=`expr $$latex_count - 1` ;\
	    done

doxygen-canal/refman.tex: doxygen.conf ../lib/*.h
	$(DOXYGEN) doxygen.conf
	sed -i'~' '4d' doxygen-canal/refman.tex
	sed -i'~' '/\\printindex$$/d' doxygen-canal/refman.tex
	sed -i'~' '/\\end{document}$$/d' doxygen-canal/refman.tex
	sed -i'~' 's/\\chapter{Class Index}$$/\\chapter{Library Class Index}/' doxygen-canal/refman.tex
	sed -i'~' 's/\\chapter{Class Documentation}$$/\\chapter{Library Class Documentation}/' doxygen-canal/refman.tex

clean-local:
	-rm -rf doxygen-canal
