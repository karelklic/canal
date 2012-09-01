dist_doc_DATA = canal.tex canal.pdf

CLEANFILES = *.aux canal.log canal.out canal.toc canal.pdf

canal.pdf: canal.tex doxygen-lib/doxygen-lib.tex doxygen-tool/doxygen-tool.tex
	echo Calling pdflatex twice to handle all references.
	TEXINPUTS="doxygen-lib:" $(PDFLATEX) canal.tex
	makeindex canal.idx
	TEXINPUTS="doxygen-lib:" $(PDFLATEX) canal.tex
	latex_count=5 ; \
	while egrep -s 'Rerun (LaTeX|to get cross-references right)' canal.log && [ $$latex_count -gt 0 ] ;\
	    do \
	      echo "Rerunning latex...." ;\
	      TEXINPUTS="doxygen-lib:" $(PDFLATEX) canal.tex ;\
	      latex_count=`expr $$latex_count - 1` ;\
	    done

# refman.tex, annontated.tex, and hierarchy.tex must be renamed to
# avoid conflicts between library documentation and tool
# documentation.
doxygen-lib/doxygen-lib.tex: doxygen-lib.conf ../lib/*.h
	$(DOXYGEN) doxygen-lib.conf
	sed -i'~' '4d' doxygen-lib/refman.tex
	sed -i'~' '/\\printindex$$/d' doxygen-lib/refman.tex
	sed -i'~' '/\\end{document}$$/d' doxygen-lib/refman.tex
	sed -i'~' 's/\\chapter{Class Index}$$/\\chapter{Library Class Index}/' doxygen-lib/refman.tex
	sed -i'~' 's/\\chapter{Class Documentation}$$/\\chapter{Library Class Documentation}/' doxygen-lib/refman.tex


	sed -i'~' 's/\\input{hierarchy}$$/\\input{doxygen-lib-hierarchy}/' doxygen-lib/refman.tex
	sed -i'~' 's/\\input{annotated}$$/\\input{doxygen-lib-annotated}/' doxygen-lib/refman.tex
	mv doxygen-lib/{,doxygen-lib-}hierarchy.tex
	mv doxygen-lib/{,doxygen-lib-}annotated.tex
	mv doxygen-lib/{refman,doxygen-lib}.tex

doxygen-tool/doxygen-tool.tex: doxygen-tool.conf ../lib/*.h
	$(DOXYGEN) doxygen-tool.conf
	sed -i'~' '4d' doxygen-tool/refman.tex
	sed -i'~' '/\\printindex$$/d' doxygen-tool/refman.tex
	sed -i'~' '/\\end{document}$$/d' doxygen-tool/refman.tex
	sed -i'~' 's/\\chapter{Class Index}$$/\\chapter{Tool Class Index}/' doxygen-tool/refman.tex
	sed -i'~' 's/\\chapter{Class Documentation}$$/\\chapter{Tool Class Documentation}/' doxygen-tool/refman.tex
	sed -i'~' 's/\\input{hierarchy}$$/\\input{doxygen-tool-hierarchy}/' doxygen-tool/refman.tex
	sed -i'~' 's/\\input{annotated}$$/\\input{doxygen-tool-annotated}/' doxygen-tool/refman.tex
	mv doxygen-tool/{,doxygen-tool-}hierarchy.tex
	mv doxygen-tool/{,doxygen-tool-}annotated.tex
	mv doxygen-tool/{refman,doxygen-tool}.tex

clean-local:
	-rm -rf doxygen-lib
	-rm -rf doxygen-tool
