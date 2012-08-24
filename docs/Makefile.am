dist_doc_DATA = canal.tex canal.pdf

CLEANFILES = *.aux canal.log canal.out canal.toc canal.pdf

canal.pdf: canal.tex doxygen-lib/refman.tex doxygen-tool/refman.tex
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

doxygen-lib/refman.tex: doxygen-lib.conf ../lib/*.h
	$(DOXYGEN) doxygen-lib.conf
	sed -i'~' '4d' doxygen-lib/refman.tex
	sed -i'~' '/\\printindex$$/d' doxygen-lib/refman.tex
	sed -i'~' '/\\end{document}$$/d' doxygen-lib/refman.tex
	sed -i'~' 's/\\chapter{Class Index}$$/\\chapter{Library Class Index}/' doxygen-lib/refman.tex
	sed -i'~' 's/\\chapter{Class Documentation}$$/\\chapter{Library Class Documentation}/' doxygen-lib/refman.tex

doxygen-tool/refman.tex: doxygen-tool.conf ../lib/*.h
	$(DOXYGEN) doxygen-tool.conf
	sed -i'~' '4d' doxygen-tool/refman.tex
	sed -i'~' '/\\printindex$$/d' doxygen-tool/refman.tex
	sed -i'~' '/\\end{document}$$/d' doxygen-tool/refman.tex
	sed -i'~' 's/\\chapter{Class Index}$$/\\chapter{Tool Class Index}/' doxygen-tool/refman.tex
	sed -i'~' 's/\\chapter{Class Documentation}$$/\\chapter{Tool Class Documentation}/' doxygen-tool/refman.tex

clean-local:
	-rm -rf doxygen-lib
	-rm -rf doxygen-tool
