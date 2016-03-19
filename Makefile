
localhost: index.fcgi

upload:
	htdocscompress htdocs htdocs_upload

index.fcgi:
	#-rm -f htdocs\bin\index.exe
	#-rm -f htdocs\bin\index.fcgi
	##b2 >makecgi_output.txt 2>&1
	#b2
	#mv htdocs/bin/index.exe index.fcgi
	scons

clean:
	rm -rf bin

distclean: clean
	rm -rf dist
	rm -rf htdocs_upload

.PHONY: localhost upload index.fcgi clean distclean
