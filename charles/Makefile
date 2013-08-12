include settings.mk
PROJECTD = src
DISTRIBD = dist

all:
	cd $(PROJECTD); make
	cd $(PROJECTD); make lib
	mkdir -p $(DISTRIBD)
	cp $(PROJECTD)/$(EXECNAME) $(PROJECTD)/$(FSARNAME) $(PROJECTD)/$(FSHEADER) $(DISTRIBD)

clean:
	cd $(PROJECTD); make clean

cleaner:
	cd $(PROJECTD); make cleaner
	rm -f $(DISTRIBD)/$(EXECNAME) $(DISTRIBD)/$(FSARNAME) $(DISTRIBD)/$(FSHEADER)
	rmdir $(DISTRIBD) 
