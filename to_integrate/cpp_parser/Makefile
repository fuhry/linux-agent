include settings.mk
PROJECTD = src
DISTRIBD = dist

all:
	cd $(PROJECTD); make
	mkdir -p $(DISTRIBD)
	cp $(PROJECTD)/$(EXECNAME) $(DISTRIBD)

clean:
	cd $(PROJECTD); make clean

cleaner:
	cd $(PROJECTD); make cleaner
	rm -f $(DISTRIBD)/$(EXECNAME)
	rmdir $(DISTRIBD) 
